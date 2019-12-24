#ifndef GAME_SERVER_INFCLASS_CONTROLLER_H
#define GAME_SERVER_INFCLASS_CONTROLLER_H

#include <game/server/gamecontroller.h>
#include <game/server/gamemodes/DDRace.h>

class CCharacter;
class CGameContext;
class CInfClassPlayer;

class CGameControllerInfClass : public CGameControllerDDRace
{
public:
	CGameControllerInfClass(CGameContext *pGameServer);

	bool PreSpawn(const CInfClassPlayer *pPlayer, vec2 *pPos);

	void OnCharacterSpawn(CCharacter *pChr) override;
};

extern IGameController *CreateInfCGameServer();

#endif // GAME_SERVER_INFCLASS_CONTROLLER_H
