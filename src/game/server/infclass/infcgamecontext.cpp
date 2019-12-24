#include "infcgamecontext.h"
#include "infcgamecontroller.h"
#include "infcplayer.h"

#include <base/system.h>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>

#include <game/server/score.h>

#include "../gamemodes/DDRace.h"

CInfClassGameContext::CInfClassGameContext()
	: CGameContext()
{
}

void CInfClassGameContext::OnInit()
{
	// init everything
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConfig = Kernel()->RequestInterface<IConfigManager>()->Values();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_pAntibot = Kernel()->RequestInterface<IAntibot>();
	m_pAntibot->RoundStart(this);
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);
	// 0.7: m_CommandManager.Init(m_pConsole, this, NewCommandHook, RemoveCommandHook);

	m_GameUuid = RandomUuid();
	Console()->SetTeeHistorianCommandCallback(CommandCallback, this);

	uint64 aSeed[2];
	secure_random_fill(aSeed, sizeof(aSeed));
	m_Prng.Seed(aSeed);
	m_World.m_Core.m_pPrng = &m_Prng;

	DeleteTempfile();

	// 0.7: // HACK: only set static size for items, which were available in the first 0.7 release
	//      // so new items don't break the snapshot delta
	//      static const int OLD_NUM_NETOBJTYPES = 23;
	static const int OLD_NUM_NETOBJTYPES = NUM_NETOBJTYPES;
	for(int i = 0; i < OLD_NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	m_Layers.Init(Kernel());
	m_Collision.Init(&m_Layers);

	Console()->ExecuteFile(g_Config.m_SvResetFile, -1);

	LoadMapSettings();

	// select gametype
	m_pInfcGameController = new CGameControllerInfClass(this);
	m_pController = m_pInfcGameController;

	const char *pCensorFilename = "censorlist.txt";
	IOHANDLE File = Storage()->OpenFile(pCensorFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		dbg_msg("censorlist", "failed to open '%s'", pCensorFilename);
	}
	else
	{
		CLineReader LineReader;
		LineReader.Init(File);
		char *pLine;
		while((pLine = LineReader.Get()))
		{
			m_aCensorlist.add(pLine);
		}
		io_close(File);
	}

	if(!m_pScore)
	{
		m_pScore = new CScore(this, ((CServer *)Server())->DbPool());
	}

	// create all entities from the game layer
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(pTileMap->m_Data);
	for(int y = 0; y < pTileMap->m_Height; y++)
	{
		for(int x = 0; x < pTileMap->m_Width; x++)
		{
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;

			if(Index >= ENTITY_OFFSET)
			{
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				m_pController->OnEntity(Index-ENTITY_OFFSET, Pos, LAYER_GAME, pTiles[y * pTileMap->m_Width + x].m_Flags);
			}
		}
	}

	// 0.7: InitConsoleCommands()

	// clamp sv_player_slots to 0..MaxClients
	// if(Config()->m_SvMaxClients < Config()->m_SvPlayerSlots)
	// 	Config()->m_SvPlayerSlots = Config()->m_SvMaxClients;
}

void CInfClassGameContext::OnConsoleInit()
{
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();

	m_ChatPrintCBIndex = Console()->RegisterPrintCallback(0, SendChatResponse, this);

	// 0.6:
	InitConsoleCommands();
}

void CInfClassGameContext::InitConsoleCommands()
{
	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);

#ifdef TEEWOLRDS_0_7
	Console()->Chain("sv_vote_kick", ConchainSettingUpdate, this);
	Console()->Chain("sv_vote_kick_min", ConchainSettingUpdate, this);
	Console()->Chain("sv_vote_spectate", ConchainSettingUpdate, this);
	Console()->Chain("sv_teambalance_time", ConchainSettingUpdate, this);
	Console()->Chain("sv_player_slots", ConchainSettingUpdate, this);

	Console()->Chain("sv_scorelimit", ConchainGameinfoUpdate, this);
	Console()->Chain("sv_timelimit", ConchainGameinfoUpdate, this);
	Console()->Chain("sv_matches_per_map", ConchainGameinfoUpdate, this);
#endif
}

void CInfClassGameContext::OnPreProcessedMessage(int MsgID, CUnpacker *pUnpacker, int ClientID, void *pRawMsg)
{
	CPlayer *pPlayer = m_apPlayers[ClientID];

	switch(MsgID)
	{
#ifdef TEEWORLDS_0_7
		case NETMSGTYPE_CL_SKINCHANGE:
			return;
#else
		case NETMSGTYPE_CL_CHANGEINFO:
		{
			// TODO!
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastChangeInfo && pPlayer->m_LastChangeInfo + Server()->TickSpeed() * g_Config.m_SvInfoChangeDelay > Server()->Tick())
				return;

			bool SixupNeedsUpdate = false;

			CNetMsg_Cl_ChangeInfo *pMsg = (CNetMsg_Cl_ChangeInfo *)pRawMsg;
			if(!str_utf8_check(pMsg->m_pName) || !str_utf8_check(pMsg->m_pClan) || !str_utf8_check(pMsg->m_pSkin))
			{
				return;
			}
			pPlayer->m_LastChangeInfo = Server()->Tick();
			pPlayer->UpdatePlaytime();

			// set infos
			if(Server()->WouldClientNameChange(ClientID, pMsg->m_pName) && !ProcessSpamProtection(ClientID))
			{
				char aOldName[MAX_NAME_LENGTH];
				str_copy(aOldName, Server()->ClientName(ClientID), sizeof(aOldName));

				Server()->SetClientName(ClientID, pMsg->m_pName);

				char aChatText[256];
				str_format(aChatText, sizeof(aChatText), "'%s' changed name to '%s'", aOldName, Server()->ClientName(ClientID));
				SendChat(-1, CGameContext::CHAT_ALL, aChatText);

				// reload scores
				Score()->PlayerData(ClientID)->Reset();
				m_apPlayers[ClientID]->m_Score = -9999;
				Score()->LoadPlayerData(ClientID);
			
				SixupNeedsUpdate = true;
			}

			if(str_comp(Server()->ClientClan(ClientID), pMsg->m_pClan))
				SixupNeedsUpdate = true;
			Server()->SetClientClan(ClientID, pMsg->m_pClan);

			if(Server()->ClientCountry(ClientID) != pMsg->m_Country)
				SixupNeedsUpdate = true;
			Server()->SetClientCountry(ClientID, pMsg->m_Country);

			str_copy(pPlayer->m_TeeInfos.m_SkinName, pMsg->m_pSkin, sizeof(pPlayer->m_TeeInfos.m_SkinName));
			pPlayer->m_TeeInfos.m_UseCustomColor = pMsg->m_UseCustomColor;
			pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
			pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
			if(!Server()->IsSixup(ClientID))
				pPlayer->m_TeeInfos.ToSixup();

			if(SixupNeedsUpdate)
			{
				protocol7::CNetMsg_Sv_ClientDrop Drop;
				Drop.m_ClientID = ClientID;
				Drop.m_pReason = "";
				Drop.m_Silent = true;

				protocol7::CNetMsg_Sv_ClientInfo Info;
				Info.m_ClientID = ClientID;
				Info.m_pName = Server()->ClientName(ClientID);
				Info.m_Country = pMsg->m_Country;
				Info.m_pClan = pMsg->m_pClan;
				Info.m_Local = 0;
				Info.m_Silent = true;
				Info.m_Team = pPlayer->GetTeam();

				for(int p = 0; p < 6; p++)
				{
					Info.m_apSkinPartNames[p] = pPlayer->m_TeeInfos.m_apSkinPartNames[p];
					Info.m_aSkinPartColors[p] = pPlayer->m_TeeInfos.m_aSkinPartColors[p];
					Info.m_aUseCustomColors[p] = pPlayer->m_TeeInfos.m_aUseCustomColors[p];
				}

				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(i != ClientID)
					{
						Server()->SendPackMsg(&Drop, MSGFLAG_VITAL | MSGFLAG_NORECORD, i);
						Server()->SendPackMsg(&Info, MSGFLAG_VITAL | MSGFLAG_NORECORD, i);
					}
				}
			}
			else
			{
				protocol7::CNetMsg_Sv_SkinChange Msg;
				Msg.m_ClientID = ClientID;
				for(int p = 0; p < 6; p++)
				{
					Msg.m_apSkinPartNames[p] = pPlayer->m_TeeInfos.m_apSkinPartNames[p];
					Msg.m_aSkinPartColors[p] = pPlayer->m_TeeInfos.m_aSkinPartColors[p];
					Msg.m_aUseCustomColors[p] = pPlayer->m_TeeInfos.m_aUseCustomColors[p];
				}

				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
			}

			Server()->ExpireServerInfo();
			return;
		}
#endif
		default:
			break;
	}

	CGameContext::OnPreProcessedMessage(MsgID, pUnpacker, ClientID, pRawMsg);
}

void CInfClassGameContext::OnClientConnected(int ClientID)
{
	{
		bool Empty = true;
		for(auto &pPlayer : m_apPlayers)
		{
			if(pPlayer)
			{
				Empty = false;
				break;
			}
		}
		if(Empty)
		{
			m_NonEmptySince = Server()->Tick();
		}
	}

	// Check which team the player should be on
	const int StartTeam = g_Config.m_SvTournamentMode ? TEAM_SPECTATORS : m_pController->GetAutoTeam(ClientID);

	if(!m_apPlayers[ClientID])
		m_apPlayers[ClientID] = new(ClientID) CInfClassPlayer(this, ClientID, StartTeam);
	else
	{
		delete m_apPlayers[ClientID];
		m_apPlayers[ClientID] = new(ClientID) CInfClassPlayer(this, ClientID, StartTeam);
	}

	SendMotd(ClientID);
	SendSettings(ClientID);

	Server()->ExpireServerInfo();
}

void CInfClassGameContext::AnnounceSkinChange(int ClientID)
{
	protocol7::CNetMsg_Sv_SkinChange Msg;
	Msg.m_ClientID = ClientID;
	for(int p = 0; p < protocol7::NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = m_apPlayers[ClientID]->m_TeeInfos.m_apSkinPartNames[p];
		Msg.m_aUseCustomColors[p] = m_apPlayers[ClientID]->m_TeeInfos.m_aUseCustomColors[p];
		Msg.m_aSkinPartColors[p] = m_apPlayers[ClientID]->m_TeeInfos.m_aSkinPartColors[p];
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
}

void CInfClassGameContext::SendSkinChange(int ClientID, int TargetID)
{
	//CInfClassPlayer *pPlayer = static_cast<CInfClassPlayer*>(m_apPlayers[ClientID]);
}

IGameServer *CreateModGameServer() { return new CInfClassGameContext; }
