#include "infcgamecontext.h"
#include "infcgamecontroller.h"

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

IGameServer *CreateModGameServer() { return new CInfClassGameContext; }
