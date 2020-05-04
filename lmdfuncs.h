#pragma once
#include "jka/game/jassapi.h"
#include "jka/game/q_shared.h"
#include "jka/game/g_local.h"
#include "API/Lmd_Professions_Public.h"
#include "starstructs.h"
#include "windows.h"
#include <vector>

#define _CRT_SECURE_NO_WARNINGS

#define NUM_BUILDABLES 2
#define PROFDATA(acc) ((profData_t*) Lmd_Accounts_GetAccountCategoryData(acc, *ProfessionDataDataIndex))
#define AUTHDATA(acc) ((authFileDataList_t*) Lmd_Accounts_GetAccountCategoryData(acc, *AuthDataDataIndex))
#define IsClient(ent) (ent && ent->s.number < 32)

typedef void G_UseTargets2define(gentity_t *entity, gentity_t *activator, const char *string );
typedef gentity_t * LmdEntFromNum(int entNum);
typedef gentity_t * G_Findz(gentity_t *from, int fieldofs, const char *match);
typedef int Auths_GetPlayer(gentity_t *player);
typedef int Auths_Get(Account_t *acc);
typedef void spawnt(gentity_t *ent);

extern p_playerInfo_t p_playerInfo[32];
extern starSkill_t starSkills[NUM_SKILLS];
extern buildObj_t buildObj[NUM_BUILDABLES];
extern int clientCheck;
extern pCmd_t pCmds[];
extern std::vector<devilsTrap_t> devilTraps;

extern eng_syscall_t g_syscall;
extern plugininfo_t g_plugininfo;
extern Auths_GetPlayer* Auths_GetPlayerRank;
extern Auths_Get* Auths_GetRank;
extern G_UseTargets2define* G_UseTargets3;
extern LmdEntFromNum* Lmd_EntFromNum;
// void (*Lmd_SetAnim)(gentity_t* ent, int setAnimParts, int anim, int setAnimFlags, int blendTime) = (void(*)(gentity_t*, int, int, int, int))LMDSETANIM;
extern level_locals_t* g_level;
extern profSkill_t* Merc_Weapons;
extern int* ProfessionDataDataIndex;
extern int* AuthDataDataIndex;
extern weaponEntry_t** weaponEntries;
extern int (*Lmd_Prof_Merc_GetSkill_Weapons)(Account_t* acc, profSkill_t* skill);
// spawn_t* Lmd_Spawns = (spawn_t*) SPAWNS;
extern accList_t* accList;
extern Account_t* (*Lmd_AccFromName)(const char* name);
extern void (*Lmd_TestLine)(vec3_t start, vec3_t end, int color, int time);
extern void (*Lmd_TeleportPlayer)( gentity_t *player, vec3_t origin, vec3_t angles );
extern gentity_t* (*Lmd_PlayEffect)(int fxID, vec3_t org, vec3_t ang);
extern int (*Lmd_NumFromName)(gentity_t* ent, const char* name);
extern void (*Lmd_Edit)(SpawnData_t* ent, const char* key, const char* value);
extern void (*Lmd_PlayerDie)( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath );
extern int (*Lmd_SetSD)(gentity_t* ent, SpawnData_t* spawnData);
extern void (*Lmd_Damage)( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod );
extern qboolean (*Lmd_RadiusDamage)( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, gentity_t *missile, int mod);
extern int (*Lmd_GetProf)( Account_t* acc ); // 1 for god, 2 for bot, 3 for force user (call jedisith), 4 for merc
extern int (*Lmd_JediSith)( Account_t* acc ); // 1 for jedi, 2 for sith
extern void (*Lmd_SecCode)( Account_t* acc, int value );
extern void (*Lmd_SetLevel)( Account_t* acc, int value );
extern void (*Lmd_SetValueForKey)( char *s, const char *key, const char *value );
extern void (*Lmd_RemoveKey)( char *s, const char *key );
extern qboolean (*Lmd_SpawnString)( const char *key, const char *defaultString, char **out );
extern char* (*Lmd_NewString)( const char *string );
extern SpawnData_t* (*Lmd_SDFromString)( const char *str );
extern gentity_t* (*Lmd_EntFromSD)( SpawnData_t* sd );
extern void (*Lmd_FreeEntity)(gentity_t* ed);
extern void (*Lmd_ChangePwd)( Account_t* acc, const char *pwd );
extern char * (*Lmd_ValueForKey)( const char *s, const char *key );
extern char* (*Lmd_SkillValue)( Account_t* acc, const char *skill );
extern void (*Lmd_SetSkill)( Account_t* acc, const char *skill, const char *val );
extern void (*Lmd_sprintf)( char *dest, int size, const char *fmt, ...);
extern int (*Lmd_GetSkill)( gentity_t* ent, int prof, int skillID );
extern void (*Lmd_EditSkill)( Account_t* acc, int prof, int skillID, int level );
extern gentity_t* (*Lmd_Spawn)();
extern gentity_t* (*TrySpawn)(const char* str);
extern void* (*Lmd_Accounts_GetAccountCategoryData)(Account_t* acc, int index);
extern int (*Lmd_EffectIndex)(const char* fx);
extern void (*Weapon_FireProjectile)(gentity_t* ent, int weaponNum, int altFire, weaponFire_t* data);
extern void (*registerSpawnableEntity)(char *name, spawnt* spawn, const entLogicalData_t logical, const entityInfo_t *info);
extern qboolean (*Lmd_OnSameTeam)(gentity_t* ent1, gentity_t* ent2);

gentity_t* Lmd_Find(gentity_t *from, int fieldofs, const char *match);
char* Lmd_KeyValue(SpawnData_t* spawnData, const char* key);
void Lmd_SetOrigin( gentity_t *ent, vec3_t origin );
vec_t VectorNormalize( vec3_t v );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
vec_t S_VectorLength( const vec3_t v );
vec_t S_Distance( const vec3_t p1, const vec3_t p2 );
void trap_SetConfigstring( int num, const char *string );
void trap_GetConfigstring( int num, char *buffer, int bufferSize );
void ForceCvar( int clientNum, const char *cvar, const char *value );
void G_Knockdown( gentity_t *victim );
static int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create );
int G_ModelIndex( const char *name );
int numLines(FILE * pFile);
void bbox_f(int clientNum, int time = 5000);

int Auths_HasFlag(authFileDataList_t* authData, int flag);
int Auths_HasCmd(authFileDataList_t* authData, const char* cmd, qboolean scanFile = qfalse);
int Player_HasFlag(gentity_t* ent, int flag);
int Player_HasCmd(gentity_t* ent, const char* cmd, qboolean scanFile = qtrue);
gentity_t* AddEvent( void (*func)(gentity_t*), int time );
void S_AnimReset(gentity_t* ent);
int S_SetAnim(gentity_t* ent, int anim);
void CopyWPFire(weaponFire_t* w1, weaponFire_t* w2);
const char *Lmd_GetStr(char *refName);
char* delSubStr(char* src, char* sub);

int PIReset(int clientNum);
void CopyWPEntry(weaponEntry_t** w1, weaponEntry_t* w2, int size);
int irand(int min, int max);
char * str_replace( char *string, char *substr, char *replacement );
int cliCheck(gentity_t* ent, const char* checkString);
void cliAimTrace(trace_t* tr, gentity_t* ent, int distance = 26635, vec3_t mins = 0, vec3_t maxs = 0);
int GetCredits(gentity_t *to);
void SetCredits(gentity_t *to, int newCr);
void TrapRemove(gentity_t* to);
void InitCvars();
void InitStructs();