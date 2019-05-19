#ifndef GAME_SERVER_INFCLASS_CONTROLLER_H
#define GAME_SERVER_INFCLASS_CONTROLLER_H

#include <game/server/gamecontroller.h>
#include <game/server/gamemodes/DDRace.h>

class CGameContext;

class CGameControllerInfClass : public CGameControllerDDRace
{
public:
	CGameControllerInfClass(CGameContext *pGameServer);
};

extern IGameController *CreateInfCGameServer();

#endif // GAME_SERVER_INFCLASS_CONTROLLER_H
