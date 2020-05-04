#include "lmdfuncs.h"
#include "lmdaddr.h"

int clientCheck = 0; // global variable to determine if cliCheck is being used or not
p_playerInfo_t p_playerInfo[32]; // a playerInfo struct array holding plugin-related information for all clients. 
static unsigned long	holdrand = 0x89abcdef; // for irand
std::vector<devilsTrap_t> devilTraps;

Auths_GetPlayer* Auths_GetPlayerRank = (Auths_GetPlayer*) AUTHSGETPLAYERRANK;
Auths_Get* Auths_GetRank = (Auths_Get*) AUTHSGETRANK;
G_UseTargets2define* G_UseTargets3 = (G_UseTargets2define*) USETARGETS2;
LmdEntFromNum* Lmd_EntFromNum = (LmdEntFromNum*) LMDENTFROMNUM;
gentity_t* (*IterateEnts)(gentity_t* ent) = (gentity_t*(*)(gentity_t*)) 0x20192fc0;
void (*Lmd_SetAnim)(gentity_t* ent, int setAnimParts, int anim, int setAnimFlags, int blendTime) = (void(*)(gentity_t*, int, int, int, int))LMDSETANIM;
level_locals_t* g_level = (level_locals_t*) LEVELSTRUCT;
accList_t* accList = (accList_t*) ACCS;
weaponEntry_t** weaponEntries = (weaponEntry_t**) 0x20393a30;
int* ProfessionDataDataIndex = (int*) 0x20391a50;
int* AuthDataDataIndex = (int*) 0x2038ca78;
profSkill_t* Merc_Weapons = (profSkill_t*) 0x20392b80;
int (*Lmd_Prof_Merc_GetSkill_Weapons)(Account_t* acc, profSkill_t* skill) = (int(*)(Account_t*, profSkill_t*)) 0x201b5ce0;
// spawn_t* Lmd_Spawns = (spawn_t*) SPAWNS;
Account_t* (*Lmd_AccFromName)(const char* name) = (Account_t*(*)(const char*))LMDACCFROMNAME;
void (*Lmd_TestLine)(vec3_t start, vec3_t end, int color, int time) = (void(*)(vec3_t, vec3_t, int, int)) LMDTESTLINE;
void (*Lmd_TeleportPlayer)( gentity_t *player, vec3_t origin, vec3_t angles ) = (void(*)(gentity_t*, vec3_t, vec3_t)) TELEPORT;
gentity_t* (*Lmd_PlayEffect)(int fxID, vec3_t org, vec3_t ang) = (gentity_t*(*)(int, vec3_t, vec3_t)) PLAYEFFECT;
int (*Lmd_NumFromName)(gentity_t* ent, const char* name) = (int(*)(gentity_t*, const char*))NUMFROMNAME;
void (*Lmd_Edit)(SpawnData_t* ent, const char* key, const char* value) = (void(*)(SpawnData_t*, const char*, const char*))EDIT;
void (*Lmd_PlayerDie)( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) = (void(*)(gentity_t*, gentity_t*, gentity_t*, int, int))PLAYERDIE;
int (*Lmd_SetSD)(gentity_t* ent, SpawnData_t* spawnData) = (int(*)(gentity_t*, SpawnData_t*)) RESETENT;
void (*Lmd_Damage)( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) = (void(*)(gentity_t*, gentity_t*, gentity_t*, vec3_t, vec3_t, int, int, int)) DAMAGE;
qboolean (*Lmd_RadiusDamage)( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, gentity_t *missile, int mod) = (qboolean(*)(vec3_t, gentity_t*, float, float, gentity_t*, gentity_t*, int)) RADIUSDAMAGE;
int (*Lmd_GetProf)( Account_t* acc ) = (int(*)(Account_t*)) GETPROF; // 1 for god, 2 for bot, 3 for force user (call jedisith), 4 for merc
int (*Lmd_JediSith)( Account_t* acc ) = (int(*)(Account_t*)) JEDISITH; // 1 for jedi, 2 for sith
void (*Lmd_SecCode)( Account_t* acc, int value ) = (void(*)(Account_t*, int)) SECCODE;
void (*Lmd_SetLevel)( Account_t* acc, int value ) = (void(*)(Account_t*, int)) SETLEVEL;
void (*Lmd_SetValueForKey)( char *s, const char *key, const char *value ) = (void(*)(char*, const char*, const char*)) SETVALUEFORKEY;
void (*Lmd_RemoveKey)( char *s, const char *key ) = (void(*)(char*, const char*)) REMOVEKEY;
qboolean (*Lmd_SpawnString)( const char *key, const char *defaultString, char **out ) = (qboolean(*)(const char*, const char*, char**)) SPAWNSTRING;
char* (*Lmd_NewString)( const char *string ) = (char*(*)(const char*)) NEWSTRING;
SpawnData_t* (*Lmd_SDFromString)( const char *str ) = (SpawnData_t*(*)(const char*)) SDFROMSTRING;
gentity_t* (*Lmd_EntFromSD)( SpawnData_t* sd ) = (gentity_t*(*)(SpawnData_t*)) ENTFROMSD;
#if (LMD >= 2472)
gentity_t* (*TrySpawn)( const char *str ) = (gentity_t*(*)(const char*)) TRYSPAWN;
#endif
void (*Lmd_FreeEntity)(gentity_t* ed) = (void(*)(gentity_t*)) FREEENTITY;
gentity_t* (*Lmd_Spawn)() = (gentity_t*(*)()) GSPAWN;
void (*Lmd_ChangePwd)( Account_t* acc, const char *pwd ) = (void(*)(Account_t*, const char*)) 0x20170830;
char * (*Lmd_ValueForKey)( const char *s, const char *key ) = (char*(*)(const char*, const char*)) VALUEFORKEY;
char* (*Lmd_SkillValue)( Account_t* acc, const char *skill ) = (char*(*)(Account_t*, const char*)) SKILLVALUE;
void (*Lmd_SetSkill)( Account_t* acc, const char *skill, const char *val ) = (void(*)(Account_t*, const char*, const char*)) SETSKILL;
void (*Lmd_sprintf)( char *dest, int size, const char *fmt, ...) = (void(*)(char*, int, const char*, ...)) 0x2023e190;
int (*Lmd_GetSkill)( gentity_t* ent, int prof, int skillID ) = (int(*)(gentity_t*, int, int)) GETSKILL;
void (*Lmd_EditSkill)( Account_t* acc, int prof, int skillID, int level ) = (void(*)(Account_t*, int, int, int)) EDITSKILL;
void* (*Lmd_Accounts_GetAccountCategoryData)(Account_t* acc, int index) = (void*(*)(Account_t*, int)) 0x201709c0;
int (*Lmd_EffectIndex)(const char* fx) = (int(*)(const char*)) 0x2014dd90;
void (*Weapon_FireProjectile)(gentity_t* ent, int weaponNum, int altFire, weaponFire_t* data) = (void(*)(gentity_t*, int, int, weaponFire_t*)) 0x201c3f20;
void (*registerSpawnableEntity)(char *name, spawnt* spawn, const entLogicalData_t logical, const entityInfo_t *info) = (void(*)(char*, spawnt*h, entLogicalData_t , const entityInfo_t*)) REGSPAWN;
qboolean (*Lmd_OnSameTeam)(gentity_t* ent1, gentity_t* ent2) = (qboolean(*)(gentity_t*, gentity_t*)) ONSAMETEAM;
/* skill ids:
heal -> 0
jump -> 1
speed -> 2
push -> 3
pull -> 4
mindtrick -> 5
grip -> 6
lightning -> 7
rage -> 8
protect -> 9
absorb -> 10
teamheal -> 11
energize -> 12
drain -> 13
seeing -> 14
saber attack -> 15
saber defend -> 16
saber throw -> 17 */

gentity_t* Lmd_Find(gentity_t *from, int fieldofs, const char *match)
{
	char	*s;

	while ((from = IterateEnts(from)) != NULL)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!stricmp (s, match))
			return from;
	}

	return NULL;
}

char* Lmd_KeyValue(SpawnData_t* spawnData, const char* key)
{
	int i = 0;
	while (i < spawnData->keys.count)
	{
		if (!stricmp(spawnData->keys.pairs[i].key, key))
		{
			return spawnData->keys.pairs[i].value;
		}
		i++;
	}
	return "";
}

void Lmd_SetOrigin( gentity_t *ent, vec3_t origin ) {
	VectorCopy( origin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->r.currentOrigin );
}

vec_t VectorNormalize( vec3_t v ) {
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);
	if ( length ) {
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}

void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up) {
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * (M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (right)
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}
	if (up)
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}

qboolean BG_KnockDownable(playerState_t *ps)
{
	if (!ps)
	{ //just for safety
		return qfalse;
	}

	if (ps->m_iVehicleNum)
	{ //riding a vehicle, don't knock me down
		return qfalse;
	}

	if (ps->emplacedIndex)
	{ //using emplaced gun or eweb, can't be knocked down
		return qfalse;
	}

	//ok, I guess?
	return qtrue;
}

void G_Knockdown( gentity_t *victim )
{
	if ( victim && victim->client && BG_KnockDownable(&victim->client->ps) )
	{
		victim->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
		victim->client->ps.forceDodgeAnim = 0;
		victim->client->ps.forceHandExtendTime = g_level->time + 3000; // special!
		victim->client->ps.quickerGetup = qfalse;
	}
}

vec_t S_VectorLength( const vec3_t v ) {
	return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

vec_t S_Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return S_VectorLength( v );
}

void trap_SetConfigstring( int num, const char *string ) {
	g_syscall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	g_syscall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void ForceCvar( int clientNum, const char *cvar, const char *value )
{
	bool wipe = true;
	char buf[1024];
	trap_GetConfigstring(1, buf, 1024);
	if (strstr(buf, JASS_VARARGS("\\%s\\", cvar))) { wipe = false; }
	Lmd_SetValueForKey(buf, cvar, value);
	g_syscall(G_SEND_SERVER_COMMAND, clientNum, JASS_VARARGS("cs 1 \"%s\"", buf));
	if (wipe)
	{
		Lmd_RemoveKey(buf, cvar);
		g_syscall(G_SEND_SERVER_COMMAND, clientNum, JASS_VARARGS("cs 1 \"%s\"", buf));
	}
	return;
}

static int G_FindConfigstringIndex( const char *name, int start, int max, qboolean create ) {
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i=1 ; i<max ; i++ ) {
		trap_GetConfigstring( start + i, s, sizeof( s ) );
		if ( !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	trap_SetConfigstring( start + i, name );

	return i;
}

char* delSubStr(char* src, char* sub)
{
	char* p = strstr(src, sub);
	if (p != NULL)
		memmove(p, p + strlen(sub), strlen(p + strlen(sub)) + 1);
	return p;
}

int G_ModelIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_MODELS, MAX_MODELS, qtrue);
}

int PIReset(int clientNum)
{
	int i = 0;
	if (clientNum >= 32) return 0;
	// reset the playerInfo struct for the client slot
	p_playerInfo[clientNum].bounty = 0;
	p_playerInfo[clientNum].bountyOn = 0;
	p_playerInfo[clientNum].bountyCr = 0;
	p_playerInfo[clientNum].fastRegen = 0;
	p_playerInfo[clientNum].gotoClientCd = 0;
	p_playerInfo[clientNum].isPro = 0;
	p_playerInfo[clientNum].killCount = 0;
	p_playerInfo[clientNum].lastKill = 0;
	p_playerInfo[clientNum].conKillCount = 0;
	p_playerInfo[clientNum].powerupCd = 0;
	p_playerInfo[clientNum].lastCmd = 0;
	p_playerInfo[clientNum].lastMsg = 0;
	p_playerInfo[clientNum].msgCount = 0;
	p_playerInfo[clientNum].msgStatus = 0;
	p_playerInfo[clientNum].oddball = 0;
	p_playerInfo[clientNum].chatCmds = 0;
	p_playerInfo[clientNum].regenSlowed = 0;
	p_playerInfo[clientNum].pwShield = 0;
	p_playerInfo[clientNum].check = 0;
	p_playerInfo[clientNum].cloaked = 0;
	p_playerInfo[clientNum].poisoned = 0;
	p_playerInfo[clientNum].muteTime = 0;
	p_playerInfo[clientNum].trapped = 0;
	p_playerInfo[clientNum].fireTime = 0;
	p_playerInfo[clientNum].hyperPushClient = -1;
	p_playerInfo[clientNum].mDuelInProgress = 0;
	p_playerInfo[clientNum].mDuelIndex = 0;
	// see if anyone was dueling this client slot
	while (i < 32)
	{
		if (p_playerInfo[i].mDuelIndex == clientNum)
		{
			if (p_playerInfo[i].mDuelInProgress == -1)
			{
				p_playerInfo[i].mDuelInProgress = 0;
				p_playerInfo[i].mDuelIndex = 0;
			}
			if (p_playerInfo[i].mDuelInProgress == 1)
			{
				p_playerInfo[i].mDuelInProgress = 0;
				p_playerInfo[i].mDuelIndex = 0;
				Lmd_EntFromNum(i)->client->ps.powerups[7] = g_level->time;
				Lmd_EntFromNum(i)->client->ps.stats[STAT_HEALTH] = Lmd_EntFromNum(i)->client->ps.stats[STAT_MAX_HEALTH];
				Lmd_EntFromNum(i)->client->ps.stats[STAT_ARMOR] = Lmd_EntFromNum(i)->client->ps.stats[STAT_MAX_HEALTH];
				Lmd_EntFromNum(i)->health = Lmd_EntFromNum(i)->client->ps.stats[STAT_MAX_HEALTH];
				g_syscall(G_SEND_SERVER_COMMAND, i, "print \"The merc duel has been terminated because your opponent disconnected.\n\"");
			}
		}
		i++;
	}
	i = 0;
	if (p_playerInfo[clientNum].cds)
		delete[] p_playerInfo[clientNum].cds;
	p_playerInfo[clientNum].cds = new int[NUM_SKILLS];
	while (i < NUM_SKILLS)
	{
		p_playerInfo[clientNum].cds[i] = 0;
		i++;
	}
	if (p_playerInfo[clientNum].weaponEnt)
		delete[] p_playerInfo[clientNum].weaponEnt;
	p_playerInfo[clientNum].weaponEnt = new weaponEntry_t[20];
	CopyWPEntry(weaponEntries, p_playerInfo[clientNum].weaponEnt, 20);
	return 1;
}

char *
str_replace( char *string, char *substr, char *replacement ){
  char *tok = NULL;
  char *newstr = NULL;
  if (!strstr(string,substr)) return string;
  while (strstr( string, substr ))
  {
  tok = NULL;
  newstr = NULL;
  tok = strstr( string, substr );
  if( tok == NULL ) return strdup( string );
  newstr = (char *) malloc( strlen( string ) - strlen( substr ) + strlen( replacement ) + 1 );
  if( newstr == NULL ) return NULL;
  memcpy( newstr, string, tok - string );
  memcpy( newstr + (tok - string), replacement, strlen( replacement ) );
  memcpy( newstr + (tok - string) + strlen( replacement ), tok + strlen( substr ), strlen( string ) - strlen( substr ) - ( tok - string ) );
  memset( newstr + strlen( string ) - strlen( substr ) + strlen( replacement ), 0, 1 );
  strcpy(string,newstr);
  }
  return newstr;
}

// checks if a given player can fire a playercheck or not
int cliCheck(gentity_t* ent, const char* checkString)
{
	if (!ent) return 0;
	if (!ent->client) return 0;
	if (!ent->client->pers.connected) return 0;
	if (ent->s.number >= 32) return 0;
	if (strlen(checkString) < 1) return 1;
	gentity_t* chk = TrySpawn(JASS_VARARGS("classname,lmd_playercheck,origin,0 0 0,%s", checkString));
	gentity_t* ass = TrySpawn("classname,lmd_multipurpose,origin,0 0 0,mode,check,param,1,targetname,aee56f8e1a95a20a35444d4ce6f9598a");
	Lmd_Edit(chk->spawnData, "target", "aee56f8e1a95a20a35444d4ce6f9598a");
	Lmd_SetSD(chk, chk->spawnData);
	clientCheck = 1;
	chk->use(chk, ent, ent);
	clientCheck = 0;
	Lmd_FreeEntity(chk);
	Lmd_FreeEntity(ass);
	if (p_playerInfo[ent->s.number].check)
	{
		p_playerInfo[ent->s.number].check = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

// just shorthands for credit modification
int GetCredits(gentity_t *to)
{
	return to->client->pers.Lmd.account->credits;
}
void SetCredits(gentity_t *to, int newCr)
{
	to->client->pers.Lmd.account->credits = newCr;
	if (to->client->pers.Lmd.account->modifiedTime == 0)
	to->client->pers.Lmd.account->modifiedTime = g_level->time;
}

int numLines(FILE * pFile)
{
	int number_of_lines = 0;
	int ch = 0;
	do 
	{
		ch = fgetc(pFile);
		if(ch == '\n')
    		number_of_lines++;
	} while (ch != EOF);

// last line doesn't end with a new line!
// but there has to be a line at least before the last line
// if(ch != '\n' && number_of_lines != 0) 
    // number_of_lines++;
	rewind(pFile);
	return number_of_lines;
}

int irand(int min, int max)
{
	int		result;

	assert((max - min) < 32768);

	max++;
	holdrand = (holdrand * 214013L) + 2531011L;
	result = holdrand >> 17;
	result = ((result * (max - min)) >> 15) + min;
	return(result);
}

void bbox_f(int clientNum, int time)
{
	char arg1[16];
	int entNum;
	trace_t tr;
	gentity_t* ent;
	vec3_t point1, point2, point3, point4, point5, point6, point7, point8;
	if (g_syscall(G_ARGC) < 2)
	{
		cliAimTrace(&tr, Lmd_EntFromNum(clientNum));
		if (tr.fraction < 1.0 && tr.entityNum != 1022) entNum = tr.entityNum;
		else return;
	}
	else
	{
		g_syscall(G_ARGV, 1, arg1, sizeof(arg1));
		entNum = atoi(arg1);
	}
	ent = Lmd_EntFromNum(entNum);

	if (ent == NULL) return;
  
	if (!ent->inuse)
		return;  
  
	VectorCopy(ent->r.absmax, point1);
	VectorCopy(ent->r.absmin, point8);
	VectorSet(point2, point1[0], point8[1], point1[2]);
	VectorSet(point3, point8[0], point8[1], point1[2]);
	VectorSet(point4, point8[0], point1[1], point1[2]);
	VectorSet(point5, point8[0], point1[1], point8[2]);
	VectorSet(point6, point1[0], point1[1], point8[2]);
	VectorSet(point7, point1[0], point8[1], point8[2]);
  
	//top
	Lmd_TestLine(point1, point2, 0x00000ff, time);
	Lmd_TestLine(point2, point3, 0x00000ff, time);
	Lmd_TestLine(point3, point4, 0x00000ff, time);
	Lmd_TestLine(point4, point1, 0x00000ff, time);
	//botton
	Lmd_TestLine(point8, point5, 0x00000ff, time);
	Lmd_TestLine(point5, point6, 0x00000ff, time);
	Lmd_TestLine(point6, point7, 0x00000ff, time);
	Lmd_TestLine(point7, point8, 0x00000ff, time);
	//sides
	Lmd_TestLine(point3, point8, 0x00000ff, time);
	Lmd_TestLine(point6, point1, 0x00000ff, time);
	Lmd_TestLine(point2, point7, 0x00000ff, time);
	Lmd_TestLine(point4, point5, 0x00000ff, time);
}

void TrapRemove(gentity_t* to)
{
	int i = 0;
	while (i < 32)
	{
		if (p_playerInfo[i].trapped == to->genericValue1 + 1)
			p_playerInfo[i].trapped = 0;
		i++;
	}
	i = 0;
	while (i < devilTraps.size())
	{
		if (devilTraps[i].index == to->genericValue1)
		devilTraps.erase(devilTraps.begin() + i);
		i++;
	}
	Lmd_FreeEntity(to);
}

void cliAimTrace(trace_t* tr, gentity_t* ent, int distance, vec3_t mins, vec3_t maxs)
{
	vec3_t location, forward, storage;
	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorCopy(ent->client->ps.origin, location);
	VectorCopy(ent->client->ps.origin, storage);
	storage[2] += 25;
	VectorMA(ent->client->ps.origin, distance, forward, location);
	g_syscall(G_TRACE, tr, storage, mins, maxs, location, ent->s.number, ent->clipmask);
}

int Auths_HasFlag(authFileDataList_t* authData, int flag)
{
	int i = 0;
	if (!authData) return 0;
	while (i < authData->count)
	{
		if (authData->list[i].file->flags & flag)
			return 1;
		i++;
	}
	return 0;
}

int Auths_HasCmd(authFileDataList_t* authData, const char* cmd, qboolean scanFile)
{
	int i = 0;
	int j = 0;
	char str[1024];
	FILE * pFile;
	if (!authData) return 0;
	while (i < authData->count)
	{
		while (j < authData->list[i].file->count)
		{
			if (authData->list[i].file->autoGenerated && authData->list[i].file->rank <= 1)
				return 1;
			if (!stricmp(authData->list[i].file->entries[j]->cmd->name, cmd))
				return 1;
			j++;
		}
		j = 0;
		if (scanFile == qtrue)
		{
			pFile = fopen(JASS_VARARGS("%s/data/default/authfiles/%s.auth", JASS_GETSTRCVAR("fs_game"), authData->list[i].file->name), "r+");
			if (pFile)
			{
				while (!feof(pFile))
				{
					fgets(str, 1024, pFile);
					if (!stricmp(str, cmd) || !strnicmp(JASS_VARARGS("%s:", cmd), str, strlen(cmd + 1)))
					{
						fclose(pFile);
						return 1;
					}
				}
				fclose(pFile);
			}
		}
		i++;
	}
	return 0;
}

int Player_HasFlag(gentity_t* ent, int flag)
{
	if (Auths_HasFlag(ent->client->sess.LMDX.auth, flag)) return 1;
	if (!ent->client->pers.Lmd.account) return 0;
	if (Auths_HasFlag(AUTHDATA(ent->client->pers.Lmd.account), flag)) return 1;
	return 0;
}


int Player_HasCmd(gentity_t* ent, const char* cmd, qboolean scanFile)
{
	if (Auths_HasCmd(ent->client->sess.LMDX.auth, cmd, scanFile)) return 1;
	if (ent->client->pers.Lmd.account == NULL) return 0;
	if (Auths_HasCmd(AUTHDATA(ent->client->pers.Lmd.account), cmd, scanFile)) return 1;
	return 0;
}

gentity_t* AddEvent( void (*func)(gentity_t*), int time )
{
	gentity_t* to = TrySpawn("classname,lmd_multipurpose,origin,0 0 0,mode,event");
	to->think = func;
	to->nextthink = time;
	return to;
}

void S_AnimReset(gentity_t* ent)
{
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoTimer = 0;
}

int S_SetAnim(gentity_t* ent, int anim)
{
	if (ent->client->ps.weaponTime > 0
		|| ent->client->ps.forceHandExtend
		|| ent->client->ps.groundEntityNum == 1023
		|| ent->health < 1)
	{
		return ANIM_BUSY;
	}

	if (anim >= 0 && anim <= 1543)
	{
		ent->client->ps.saberMove = 0;
		ent->client->ps.saberBlocking = 0;
		ent->client->ps.saberBlocked = 0;
		Lmd_SetAnim(ent, 3, anim, 3, 0);
		ent->client->ps.legsTimer = Q3_INFINITE;
		ent->client->ps.torsoTimer = Q3_INFINITE;
		return ANIM_SUCCESS;
	}
	else
	{
		return ANIM_INVALIDNUM;
	}
}

void CopyWPFire(weaponFire_t* w1, weaponFire_t* w2)
{
	if (w1 == NULL) w2 = NULL;
	if (w2 == NULL) return;
	w2->extra = w1->extra;
	w2->projectiles = w1->projectiles;
	w2->flags = w1->flags;
	w2->fire = w1->fire;
	w2->damage.damage = w1->damage.damage;
	w2->damage.dflags = w1->damage.dflags;
	w2->damage.mod = w1->damage.mod;
	w2->damage.splashmod = w1->damage.splashmod;
	w2->damage.splashdamage = w1->damage.splashdamage;
	w2->damage.splashradius = w1->damage.splashradius;
	w2->launcher.energy = w1->launcher.energy;
	w2->launcher.rate = w1->launcher.rate;
	w2->launcher.spread = w1->launcher.spread;
	w2->launcher.spreadrate = w1->launcher.spreadrate;
	w2->projectile.bounce = w1->projectile.bounce;
	w2->projectile.gravity =  w1->projectile.gravity;
	w2->projectile.life = w1->projectile.life;
	w2->projectile.size = w1->projectile.size;
	w2->projectile.velocity = w1->projectile.velocity;
	return;
}

void CopyWPEntry(weaponEntry_t** w1, weaponEntry_t* w2, int size)
{
	if (w1 == NULL) w2 = NULL;
	if (w2 == NULL) return;
	int i = 0;
	while (i < size)
	{
		if (w1[i] == NULL) { i++; continue; }
		w2[i].dummy1 = w1[i]->dummy1;
		w2[i].dummy2 = w1[i]->dummy2;
		CopyWPFire(&w1[i]->primary, &w2[i].primary);
		CopyWPFire(&w1[i]->secondary, &w2[i].secondary);
		i++;
	}
	return;
}

const char *Lmd_GetStr(char *refName)
{
	/*
	static char text[1024]={0};
	trap_SP_GetStringTextString(va("%s_%s", refSection, refName), text, sizeof(text));
	return text;
	*/

	//Well, it would've been lovely doing it the above way, but it would mean mixing
	//languages for the client depending on what the server is. So we'll mark this as
	//a stringed reference with @@@ and send the refname to the client, and when it goes
	//to print it will get scanned for the stringed reference indication and dealt with
	//properly.
	static char text[1024]={0};
	sprintf(text, "@@@%s", refName);
	return text;
}

void InitCvars()
{
	g_syscall(G_CVAR_REGISTER,0,"^6S^7tar^6P^7lugin ^6V^7ersion      ",g_plugininfo.version,CVAR_SERVERINFO);
	g_syscall(G_CVAR_REGISTER,0,"star_teleportEnabled","1",CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"star_teleportCost","100",CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"bboxlevel","16777215",CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"automute","0",CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"star_teleportCd", "180", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"killstreak", "1", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"powerupcost", "10", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"powerupcd", "300", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"powerupmaxtime", "20", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"disallowedVoteFlags", "0", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"chatCommands", "1", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"floodLevel", "1", CVAR_ARCHIVE);
	g_syscall(G_CVAR_REGISTER,0,"disableCrEditing", "0", CVAR_ARCHIVE);

	g_syscall(G_PRINT, "Cvars initialized.\n");
}

void InitStructs()
{
	int i = 0;
	// initialize p_playerInfo array, required for unloading support
	while (i < 32)
	{
		PIReset(i);
		i++;
	}

	g_syscall(G_PRINT, "Structs initialized.\n");
}