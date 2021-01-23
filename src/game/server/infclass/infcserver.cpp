#include "infcserver.h"

#include "classes/infcplayerclass.h"
#include "infcgamecontext.h"
#include "infcplayer.h"

CInfClassServer::CInfClassServer()
	: CServer()
{
}

const char *CInfClassServer::ClientClan(int ClientID)
{
	CInfClassGameContext *pServer = static_cast<CInfClassGameContext*>(GameServer());
	CInfClassPlayer *pPlayer = pServer->GetPlayer(ClientID);
	CInfClassPlayerClass *pClass = pPlayer ? pPlayer->GetCharacterClass() : nullptr;
	if (!pClass)
	{
		return "unknown";
	}
	return pClass->GetClassDisplayName();
}

#define CreateServer CreateInfcServer
extern CServer *CreateServer() { return new CInfClassServer(); }
