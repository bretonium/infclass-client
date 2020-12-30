#ifndef GAME_SERVER_INFCLASS_ENTITIES_CHARACTER_H
#define GAME_SERVER_INFCLASS_ENTITIES_CHARACTER_H

#include <game/server/entities/character.h>

class CInfClassGameContext;
class CInfClassPlayerClass;

class CInfClassCharacter : public CCharacter
{
	MACRO_ALLOC_HEAP()
public:
	CInfClassCharacter(CInfClassGameContext *pContext);

	void Tick() override;
	void FireWeapon() override;

	vec2 GetDirection() const;

	void EnableJump();
	void TakeAllWeapons();
	void SetReloadTimer(int ReloadTimer);

	CInfClassPlayerClass *GetClass() { return m_pClass; }
	void SetClass(CInfClassPlayerClass *pClass);

	CInfClassGameContext *GameContext() const { return m_pContext; }

protected:
	CInfClassGameContext *m_pContext = nullptr;
	CInfClassPlayerClass *m_pClass = nullptr;
	bool m_Grounded = false;
};

#endif // GAME_SERVER_INFCLASS_ENTITIES_CHARACTER_H
