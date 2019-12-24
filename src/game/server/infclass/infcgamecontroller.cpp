#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>

#include "entities/infccharacter.h"

#include "infcgamecontroller.h"
#include "infcplayer.h"

CGameControllerInfClass::CGameControllerInfClass(CGameContext *pGameServer) : CGameControllerDDRace(pGameServer)
{
	m_pGameType = "InfClassR7";
}

bool CGameControllerInfClass::PreSpawn(const CInfClassPlayer *pPlayer, vec2 *pPos)
{
	return CanSpawn(pPlayer->GetTeam(), pPos);
}

void CGameControllerInfClass::OnCharacterSpawn(CCharacter *pChr)
{
	// We work only with InfClass characters
	CInfClassCharacter *pCharacter = static_cast<CInfClassCharacter *>(pChr);

	// default health
	pCharacter->IncreaseHealth(10);

	// DDNet stuff needed for CGameControllerDDRace
	pChr->SetTeams(&m_Teams);
	pChr->SetTeleports(&m_TeleOuts, &m_TeleCheckOuts);
	m_Teams.OnCharacterSpawn(pChr->GetPlayer()->GetCID());
}
