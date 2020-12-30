#include "infccharacter.h"

#include "../classes/infcplayerclass.h"

#include <engine/shared/config.h>

#include <generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/infclass/infcgamecontext.h>
#include <game/server/player.h>

CInfClassCharacter::CInfClassCharacter(CInfClassGameContext *pContext)
	: CCharacter(pContext->GameWorld())
	, m_pContext(pContext)
	, m_Grounded(false)
{
}

void CInfClassCharacter::Tick()
{
	CCharacter::Tick();
#ifdef TEEWORLDS_0_7
	if (m_Core.m_TriggeredEvents & COREEVENTFLAG_AIR_JUMP) {
		m_pClass->OnJumped(JumpType::Air);
	} else if (m_Core.m_TriggeredEvents & COREEVENTFLAG_GROUND_JUMP) {
		m_pClass->OnJumped(JumpType::Ground);
	}
#else
	if (m_Core.m_TriggeredEvents & COREEVENT_AIR_JUMP) {
		m_pClass->OnJumped(JumpType::Air);
	} else if (m_Core.m_TriggeredEvents & COREEVENT_GROUND_JUMP) {
		m_pClass->OnJumped(JumpType::Ground);
	}
#endif
	bool grounded = IsGrounded();
	if (m_Grounded != grounded) {
		if (grounded) {
			m_pClass->OnGrounded();
		}
		m_Grounded = grounded;
	}
}

void CInfClassCharacter::FireWeapon()
{
	if(m_ReloadTimer != 0)
		return;

	DoWeaponSwitch();

	bool FullAuto = false;
	if(GetActiveWeapon() == WEAPON_GRENADE || GetActiveWeapon() == WEAPON_SHOTGUN || GetActiveWeapon() == WEAPON_LASER)
		FullAuto = true;


	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[GetActiveWeapon()].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	// check for ammo
	if(!m_aWeapons[GetActiveWeapon()].m_Ammo)
	{
		// 125ms is a magical limit of how fast a human can click
		m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
		if(m_LastNoAmmoSound+Server()->TickSpeed() <= Server()->Tick())
		{
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
			m_LastNoAmmoSound = Server()->Tick();
		}
		return;
	}

	if(Config()->m_Debug)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "shot player='%d:%s' team=%d weapon=%d", m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->GetTeam(), GetActiveWeapon());
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}

	m_pClass->OnWeaponFired(GetActiveWeapon());

	m_AttackTick = Server()->Tick();

	if(m_aWeapons[GetActiveWeapon()].m_Ammo > 0) // -1 == unlimited
		m_aWeapons[GetActiveWeapon()].m_Ammo--;

	if(!m_ReloadTimer)
		m_ReloadTimer = g_pData->m_Weapons.m_aId[GetActiveWeapon()].m_Firedelay * Server()->TickSpeed() / 1000;
}

vec2 CInfClassCharacter::GetDirection() const
{
	return normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
}

void CInfClassCharacter::EnableJump()
{
	m_Core.m_Jumped &= ~2;
}

void CInfClassCharacter::TakeAllWeapons()
{
	for (WeaponStat &weapon : m_aWeapons)
	{
		weapon.m_Got = false;
	}
}

void CInfClassCharacter::SetReloadTimer(int ReloadTimer)
{
	m_ReloadTimer = ReloadTimer;
}

void CInfClassCharacter::SetClass(CInfClassPlayerClass *pClass)
{
	m_pClass = pClass;
}
