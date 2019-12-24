#include "infcplayer.h"

#include "infcgamecontext.h"

MACRO_ALLOC_POOL_ID_IMPL(CInfClassPlayer, MAX_CLIENTS)

CInfClassPlayer::CInfClassPlayer(CInfClassGameContext *pGameServer, int ClientID, int Team)
	: CPlayer(pGameServer, ClientID, Team)
	, m_pInfcGameContext(pGameServer)
{
}
