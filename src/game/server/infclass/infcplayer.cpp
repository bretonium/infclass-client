#include "infcplayer.h"

#include <game/server/infclass/entities/infccharacter.h>

#include "classes/infcplayerclass.h"

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
	m_pInfcCharacter = new CInfClassCharacter(&GameServer()->m_World);
	m_pCharacter = m_pInfcCharacter;

	m_pCharacter->Spawn(this, SpawnPos);
	GameServer()->CreatePlayerSpawn(SpawnPos);
}

void CInfClassPlayer::SyncSkin()
{
	m_pInfcPlayerClass->SetupSkin();
	const CTeeInfo &skin = m_pInfcPlayerClass->GetSkinInfo();

	for(int p = 0; p < NUM_SKINPARTS; p++)
	{
		str_copy(m_TeeInfos.m_apSkinPartNames[p], skin.m_apSkinPartNames[p], 24);
		m_TeeInfos.m_aUseCustomColors[p] = skin.m_aUseCustomColors[p];
		m_TeeInfos.m_aSkinPartColors[p] = skin.m_aSkinPartColors[p];
	}
	m_TeeInfos.FromSixup();
	m_pInfcGameContext->AnnounceSkinChange(GetCID());
}

void CInfClassPlayer::SetCharacterClass(int ClassId)
{
	CInfClassPlayerClass *pClass = m_pInfcGameContext->CreateInfClass(ClassId);
	SetCharacterClass(pClass);
}

void CInfClassPlayer::SetCharacterClass(CInfClassPlayerClass *pClass)
{
	if(m_pInfcPlayerClass)
		delete m_pInfcPlayerClass;

	m_pInfcPlayerClass = pClass;
	SyncSkin();
}
