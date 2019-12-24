#include "infcplayer.h"

#include "infcgamecontext.h"
#include "infcgamecontroller.h"

#include <game/server/entities/character.h>

MACRO_ALLOC_POOL_ID_IMPL(CInfClassPlayer, MAX_CLIENTS)

CInfClassPlayer::CInfClassPlayer(CInfClassGameContext *pGameServer, int ClientID, int Team)
	: CPlayer(pGameServer, ClientID, Team)
	, m_pInfcGameContext(pGameServer)
{
}

void CInfClassPlayer::TryRespawn()
{
	vec2 SpawnPos;

	if(!m_pInfcGameContext->GetController()->PreSpawn(this, &SpawnPos))
		return;

	m_Spawning = false;
	m_pCharacter = new(m_ClientID) CCharacter(&GameServer()->m_World);
	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos);
}
