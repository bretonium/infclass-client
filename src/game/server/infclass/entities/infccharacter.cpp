#include "infccharacter.h"

#include "../classes/infcplayerclass.h"

CInfClassCharacter::CInfClassCharacter(CGameWorld *pWorld)
	: CCharacter(pWorld)
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

void CInfClassCharacter::SetClass(CInfClassPlayerClass *pClass)
{
	m_pClass = pClass;
}
