#include "infccharacter.h"

CInfClassCharacter::CInfClassCharacter(CGameWorld *pWorld)
	: CCharacter(pWorld)
{
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
