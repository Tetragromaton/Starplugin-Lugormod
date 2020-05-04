#pragma once
#include "jka/game/q_shared.h"
#include "jka/game/g_local.h"

typedef struct weaponDamage_s {
unsigned int damage;
unsigned int splashdamage;
unsigned int splashradius;
unsigned int dflags;
unsigned int mod;
unsigned int splashmod;
}weaponDamage_t;

typedef struct weaponLauncher_s {
unsigned int rate;
unsigned int energy;
float spread;
float spreadrate;
}weaponLauncher_t;

typedef struct weaponProjectile_s {
unsigned int size;
unsigned int velocity;
qboolean gravity;
unsigned int life;
int bounce;
}weaponProjectile_t;


typedef struct weaponFire_s weaponFire_t;
struct weaponFire_s {
int projectiles;
void (*fire)(gentity_t *ent, gentity_t *missile, weaponFire_t *data);
weaponLauncher_t launcher;
weaponDamage_t damage;
weaponProjectile_t projectile;
int flags;
void *extra;
};

typedef struct weaponEntry_s
{
	int dummy1;
	weaponFire_t primary;
	weaponFire_t secondary;
	int dummy2;
} weaponEntry_t;

typedef struct p_playerInfo_s {
	// client data
	int bounty;
	int bountyOn;
	int bountyCr;
	int killCount;
	int lastKill;
	int conKillCount;
	int isPro;
	int powerupCd;
	int gotoClientCd;
	int lastCmd;
	int msgCount;
	int lastMsg;
	int msgStatus;
	int chatCmds;
	int muteTime;
	int oddball;
	int check;
	int regenSlowed;
	int pwShield;
	// skill data
	int fastRegen;
	int cloaked;
	int poisoned;
	int trapped;
	int hyperPushClient;
	int mDuelInProgress;
	int mDuelIndex;
	int * cds;
	weaponEntry_t * weaponEnt;
	int fireTime;
} p_playerInfo_t;

typedef struct starSkill_s
{
	char * name;
	char * desc;
	char * cmdName;
	int level;
	int pts;
	int (*function)(gentity_t*);
	int cd;
	int prof;
	int fp;
	qboolean isPassive;
} starSkill_t;

typedef struct buildObj_s
{
	char* name;
	char* desc;
	int reqCrystal;
	int reqOrg;
	int reqIron;
	gentity_t* (*spawn)(gentity_t*);
} buildObj_t;

typedef struct devilsTrap_s
{
	vec3_t center;
	float radius;
	int activeTime;
	gentity_t* owner;
	int index;
} devilsTrap_t;

typedef struct pCmd_s
{
	// arguments passed to the function to maintain chat cmd support
	// return value indicates whether to supercede or to ignore
	qboolean (*function)(gentity_t*, int, char**);
	char* cmd; // command name
	qboolean loggedInUse; // do you need to be logged in to use this command?
	int adminLevel; // admin level required
	int lmdCheck; // check auth files? 0 = dont check, 1 = check auths, 2 = check auths and files
	qboolean listedCmd; // should this command be documented?
	qboolean lastEntry; // for convenience, if the item is the last entry of the cmd array
} pCmd_t;

enum skillIds_s
{
	SKILL_TREATINJURY = 0,
	SKILL_FASTREGEN,
	SKILL_SLOWREGEN,
	SKILL_FORCEBEAM,
	SKILL_TIMEBOMB,
	SKILL_DISABLEJP,
	SKILL_WPPROF,
	SKILL_BREACH,
	SKILL_CLOAK,
	SKILL_CHARGE,
	SKILL_RESISTANCE,
	SKILL_LANDING,
	SKILL_SLOW,
	SKILL_ADRENALINE,
	SKILL_POISON,
	NUM_SKILLS
};

enum animReturns_s
{
	ANIM_BUSY = 0,
	ANIM_INVALIDNUM,
	ANIM_SUCCESS
};

typedef struct entLogicalData_s{
	// Whether this entity can be logical.
	// If qtrue, the entity will be logical if 'check()' is NULL or returns qtrue.
	qboolean allow;

	// Called before creating the entity to determine if it should be logical.
	// All keys and values are available through G_SpawnString / G_SpawnInt and other calls.
	// If this is NULL, then the entity will always be logical, provided 'allow' is qtrue.
	qboolean (*check)(); 
}entLogicalData_t;

typedef struct entityInfoData_s {
	char *key;
	char *value;
} entityInfoData_t;

typedef struct {
	char *description;
	entityInfoData_t *spawnflags; // Terminate array with {NULL}
	entityInfoData_t *keys; // Terminate array with {NULL}
} entityInfo_t;

typedef struct {
	char *name;
	void (*spawn)(gentity_t *ent);
	entLogicalData_t logical;
	entityInfo_t *info;
}spawn_t;

// Macros to simplify creation of always-logical or never-logical ents.
#define Logical_True {qtrue, NULL}
#define Logical_False {qfalse, NULL}