#ifndef GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H
#define GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H

// 0.7: #include <game/server/alloc.h>
// 0.6:

#include <game/server/entity.h>
#include <base/vmath.h>

#ifdef TEEWORLDS_0_7
#include <generated/protocol.h>

struct CTeeInfo
{
	char m_apSkinPartNames[NUM_SKINPARTS][24];
	int m_aUseCustomColors[NUM_SKINPARTS];
	int m_aSkinPartColors[NUM_SKINPARTS];
};
#else
#include <game/server/teeinfo.h>

static const int SKINPART_BODY = protocol7::SKINPART_BODY;
static const int SKINPART_MARKING = protocol7::SKINPART_MARKING;
static const int SKINPART_DECORATION = protocol7::SKINPART_DECORATION;
static const int SKINPART_HANDS = protocol7::SKINPART_HANDS;
static const int SKINPART_FEET = protocol7::SKINPART_FEET;
static const int SKINPART_EYES = protocol7::SKINPART_EYES;
static const int NUM_SKINPARTS = protocol7::NUM_SKINPARTS;
#endif

class CConfig;
class CGameContext;
class CGameWorld;
class CInfClassCharacter;
class CInfClassGameContext;
class CInfClassPlayer;
class IServer;

class CInfClassCharacter;

enum class JumpType {
	Air,
	Ground,
};

class CInfClassPlayerClass
{
public:
	virtual ~CInfClassPlayerClass() = default;

	void SetCharacter(CInfClassCharacter *character);

	const CTeeInfo &GetSkinInfo() const { return m_SkinInfo; }
	virtual void SetupSkin();

	// Events
	virtual void OnCharacterSpawned();
	virtual void OnGrounded() { }
	virtual void OnJumped(JumpType jumpType);
	virtual void OnWeaponFired(int Weapon);

	virtual void OnHammerFired();
	virtual void OnGunFired();
	virtual void OnShotgunFired();
	virtual void OnGrenadeFired();
	virtual void OnLaserFired();
	virtual void OnNinjaFired();

	CInfClassGameContext *GameContext() const;
	CGameContext *GameServer() const;
	CGameWorld *GameWorld() const;
	CConfig *Config();
	IServer *Server();
	CInfClassPlayer *GetPlayer();
	int GetCID();
	vec2 GetPos() const;
	vec2 GetDirection() const;
	float GetProximityRadius() const;

protected:
	explicit CInfClassPlayerClass();

	CTeeInfo m_SkinInfo;
	CInfClassCharacter *m_pCharacter = nullptr;
};

#endif // GAME_SERVER_INFCLASS_CLASSES_PLAYER_CLASS_H
