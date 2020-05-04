#include "hookfuncs.h"
#include "lmdfuncs.h"
#include "lmdaddr.h"
#include "skills.h"

//needed variables 
BYTE oldTargetLCUse[6]; //here the original function bytes will be stored 
BYTE oldLmdDropUse[6];
BYTE oldDelayUse[6];
BYTE oldFPRegen[6];
BYTE oldWP[6];
BYTE oldTP[6];
BYTE oldPowerupSP[6];
BYTE oldDamage[6];
BYTE oldFPUsable[6];
BYTE oldKnockDown[6];
BYTE oldWeapon[6];
BYTE oldPlayerDie[6];
BYTE oldCanBeEnemy[6];
jmp_far jump;
DWORD flOldProtect;

void PCheckMode(gentity_t* self, gentity_t* activator)
{
	void* ptr;
			int dptr = 0;
			if (!stricmp(self->target2, "account"))
			{
				if (!activator->client->pers.Lmd.account)
				{ 
					G_UseTargets3(activator, activator, self->target5);
					return;
				}
				ptr = (void*)((int) activator->client->pers.Lmd.account + atoi(self->target3));
				dptr = 1;
			}
			else
			{
				// no need to check for activator->client, because it is checked in the use function for multipurpose
				ptr = (void*)((int) activator->client + atoi(self->target3));
			}
			if (!stricmp(self->cmd, "string"))
			{
				if (dptr)
				{
					if (atoi(self->cmd2) > 0)
					{
						if (!stricmp(*(char**) ptr, self->target4))
							G_UseTargets3(activator, activator, self->target);
						else
							G_UseTargets3(activator, activator, self->target5);
					}
					else
					{
						if (!strcmp(*(char**) ptr, self->target4))
							G_UseTargets3(activator, activator, self->target);
						else
							G_UseTargets3(activator, activator, self->target5);
					}
				}
				else
				{
					if (atoi(self->cmd2) > 0)
					{
						if (!stricmp((char*) ptr, self->target4))
							G_UseTargets3(activator, activator, self->target);
						else
							G_UseTargets3(activator, activator, self->target5);
					}
					else
					{
						if (!strcmp((char*) ptr, self->target4))
							G_UseTargets3(activator, activator, self->target);
						else
							G_UseTargets3(activator, activator, self->target5);
					}
				}
			}
			if (!stricmp(self->cmd, "integer"))
			{
				if (atoi(self->cmd2) == 0)
				{
					if (*(int*) ptr == atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
				else if (atoi(self->cmd2) == 1)
				{
					if (*(int*) ptr >= atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
				else
				{
					if (*(int*) ptr <= atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
			}
			if (!stricmp(self->cmd, "float"))
			{
				if (atoi(self->cmd2) == 0)
				{
					if (floor(*(float*) ptr) == atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
				else if (atoi(self->cmd2) == 1)
				{
					if (*(float*) ptr >= atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
				else
				{
					if (*(float*) ptr <= atoi(self->target4))
						G_UseTargets3(activator, activator, self->target);
					else
						G_UseTargets3(activator, activator, self->target5);
				}
			}
			return;
}

void TargetLCUse(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	char ss[1024] = "";
	gentity_t* en;
	if (!self->message) return;
	if (activator->client)
	{
		if (!stricmp(self->message, "god"))
		{
			if (atoi(self->target2) == 1 && !(activator->flags & FL_GODMODE) || atoi(self->target2) == 0 && activator->flags & FL_GODMODE)
			activator->flags ^= FL_GODMODE;
			return;
		}

		if (!stricmp(self->message, "give"))
		{
			if (strstr(self->target2, "\n") || strstr(self->target2, ";") || strstr(self->target2, "\r")) return;
			if (strnicmp(self->target2, "credits", 7))
			g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("giveother %d %s", activator->s.clientNum, self->target2));
			return;
		}

		if (!stricmp(self->message, "jail"))
		{
			if (strstr(self->target2, "\n") || strstr(self->target2, ";") || strstr(self->target2, "\r")) return;
			g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("jail %d %s", activator->s.clientNum, self->target2));
			return;
		}

		if (!stricmp(self->message, "strip"))
		{
			if (strstr(self->target2, "\n") || strstr(self->target2, ";") || strstr(self->target2, "\r")) return;
			g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("strip %d %s", activator->s.clientNum, self->target2));
			return;
		}

		if (!stricmp(self->message, "exec"))
		{
			if (strstr(self->target2, "\n") || strstr(self->target2, ";") || strstr(self->target2, "\r")) return;
			g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("exec pubcfg/%s", self->target2));
			return;
		}
		if (!stricmp( self->message, "modify"))
		{
			en = NULL;
			while ((en = Lmd_Find(en, FOFS(targetname), self->target)) != NULL)
			{
				Lmd_Edit(en->spawnData, self->target2, self->target3);
				Lmd_SetSD(en, en->spawnData);
			}
	        return;
		}
		if (!stricmp(self->message, "spawn"))
		{
			if (strlen(self->target2) > 0)
				sprintf(ss, "%sclassname,%s,", ss, self->target2);
			if (strlen(self->target3) > 0)
				sprintf(ss, "%sorigin,%s,", ss, self->target3);
			if (strlen(self->target4) > 0)
				sprintf(ss, "%s%s", ss, self->target4);
			TrySpawn(ss);
			return;
		}
		if (!stricmp(self->message, "delete"))
		{
			en = NULL;
			while ((en = Lmd_Find(en, FOFS(targetname), self->target)) != NULL)
			{
				Lmd_FreeEntity(en);
			}
			return;
		}
		if (!stricmp(self->message, "entcheck"))
		{
			int i = 0;
			int j = 0;
			int count = 0;
			while (i < g_level->num_entities + 1024)
			{
				en = Lmd_EntFromNum(i);
				if (!en) { i++; continue; }
				if (!en->spawnData) { i++; continue; }
				while (j < en->spawnData->keys.count)
				{
					if (!stricmp(en->spawnData->keys.pairs[j].key, self->target2))
					{
						if (!stricmp(en->spawnData->keys.pairs[j].value, self->target3))
						{
							count++;
						}
					}
					j++;
				}
				j = 0;
				i++;
			}
			if (count < atoi(self->target4)) G_UseTargets3(activator, activator, self->target5);
			else G_UseTargets3(activator, activator, self->target);
			return;
		}
		// fires a target for all players matching a playercheck string and in a certain bbox
		if (!stricmp(self->message, "fire"))
		{
			bool allowed = true;
			vec3_t point1;
			vec3_t point8;
			int i = 0;
			if (strlen(self->target3) > 0 && strlen(self->target4) > 0)
			{
				sscanf(self->target3, "%f %f %f", &point8[0], &point8[1], &point8[2]);
				sscanf(self->target4, "%f %f %f", &point1[0], &point1[1], &point1[2]);
			}
			while (i < 32)
			{
				en = Lmd_EntFromNum(i);
				if (!en) { i++; continue; }
				if (!en->client) { i++; continue; }
				if (!en->client->pers.connected) { i++; continue; }
				if (strlen(self->target3) > 0 && strlen(self->target4) > 0)
				{
					if ( ((point8[0] <= en->client->ps.origin[0]) && (en->client->ps.origin[0] <= point1[0])) && ((point8[1] <= en->client->ps.origin[1]) && (en->client->ps.origin[1] <= point1[1])) && ((point8[2] <= en->client->ps.origin[2]) && (en->client->ps.origin[2] <= point1[2])) )
					{
						allowed = true;
					}
					else
					{
						allowed = false;
					}
				}
				if (allowed)
				{
					if (cliCheck(en, self->target2))
					{
						G_UseTargets3(en, en, self->target);
					}
				}
				i++;
			}
		}
		// a playercheck mode. fairly complicated, so here is an explanation:
		/* param = target2 = determines checking in account or client struct. default: client
		** param2 = target3 = determines the offset to check for.
		** param3 = target4 = determines the value to check for.
		** cmode = cmd2 = determines ctype-specific comparison type:
			0 = equal, 1 = greater, 2 = less for integers, default: equal
			0 = case-sensitive, > 0 = case-insensitive, default : sensitive
		** ctype = cmd = determines checking for integer value or string value. default: integer
		** target = target = fired on success
		** target2 = target5 = fired on failure
		*/
		if (!stricmp(self->message, "pcheck"))
		{
			PCheckMode(self, activator);
		}
		if (!stricmp(self->message, "heal"))
		{
			activator->health += atoi(self->target2);
			activator->client->ps.stats[STAT_HEALTH] += atoi(self->target2);
			if (activator->client->ps.stats[STAT_MAX_HEALTH] < activator->client->ps.stats[STAT_HEALTH])
			{
				activator->health = activator->client->ps.stats[STAT_MAX_HEALTH];
				activator->client->ps.stats[STAT_HEALTH] = activator->client->ps.stats[STAT_MAX_HEALTH];
			}
			if (activator->client->ps.stats[STAT_HEALTH] <= 0)
			{
				activator->flags &= ~FL_GODMODE;
				Lmd_PlayerDie(activator, activator, activator, 100000, MOD_SUICIDE);
			}
			activator->client->ps.stats[STAT_ARMOR] += atoi(self->target3);
			if (activator->client->ps.stats[STAT_MAX_HEALTH] < activator->client->ps.stats[STAT_ARMOR])
			{
				activator->client->ps.stats[STAT_ARMOR] = activator->client->ps.stats[STAT_MAX_HEALTH];
			}
			if (activator->client->ps.stats[STAT_ARMOR] < 0)
			{
				activator->client->ps.stats[STAT_ARMOR] = 0;
			}
			return;
		}
		if (!stricmp(self->message, "originplayer"))
		{
			vec3_t angles, forward, location, nudge;
			char newOrigin[256], newAngles[256];
			trace_t tr;
			gentity_t* ent = NULL;
			angles[0] = activator->playerState->viewangles[0];
			angles[1] = activator->playerState->viewangles[1]+180;
			angles[2] = activator->playerState->viewangles[2];
			VectorCopy(activator->client->ps.origin, location);
			AngleVectors(activator->client->ps.viewangles, forward, NULL, NULL);
			// set location out in front of your view
			forward[2] = 0; //no elevation change
			VectorNormalize(forward);
			VectorMA(activator->client->ps.origin, atoi(Lmd_KeyValue(self->spawnData, "offset")), forward, location);
			// trap_trace check
			g_syscall(G_TRACE, &tr, activator->client->ps.origin, 0, 0, location, activator->s.number, activator->clipmask);
			sscanf(Lmd_KeyValue(self->spawnData, "nudge"), "%f %f %f", &nudge[0], &nudge[1], &nudge[2]);
			VectorCopy(location, tr.endpos);
			VectorAdd(location, nudge, location);
			sprintf(newOrigin, "%g %g %g", location[0], location[1], location[2]);
			sprintf(newAngles, "0 %g 0", angles[1] - 180);
			while ((ent = Lmd_Find(ent, FOFS(targetname), self->target)) != NULL)
			{
				Lmd_Edit(ent->spawnData, "origin", newOrigin);
				Lmd_Edit(ent->spawnData, "angles", newAngles);
				Lmd_SetSD(ent, ent->spawnData);
			}
			return;
		}
		if (!stricmp(self->message, "oddball"))
		{
			p_playerInfo[activator->s.number].oddball = atoi(self->target2);
			return;
		}
		if (!stricmp(self->message, "check") && clientCheck) // only for usage by cliCheck
		{
			p_playerInfo[activator->s.number].check = atoi(self->target2);
			return;
		}
	}
	return;
}
// NOTE: This isn't really the use function, it is a printing function called by the use function
// spawnflags 4 is considered while calling this so no need to check it inside the function
void Use_Target_Print(gentity_t *self, int clientNum)
{
	char msg[1024];
	char temp[128];
	char list[1024];
	char* tok;
	void *ptr;
	int i = 1;
	strncpy(msg, self->message, 1024);
	strncpy(list, Lmd_KeyValue(self->spawnData, "list"), 1024);
	if (self->activator && self->activator->client && self->activator->client->pers.connected)
	{
		gentity_t* activator = self->activator;
		strcpy(msg, str_replace(msg, "\\id", activator->client->pers.netname));
		strcpy(msg, str_replace(msg, "\\h", JASS_VARARGS("%d", activator->client->ps.stats[STAT_HEALTH])));
		strcpy(msg, str_replace(msg, "\\s", JASS_VARARGS("%d", activator->client->ps.stats[STAT_ARMOR])));
		if (strcmp(list, ""))
		{
			tok = strtok(list, " ");
			while (tok != NULL)
			{
				ptr = (void*)((int) activator->client + atoi(tok));
				if (strstr(msg, JASS_VARARGS("$s(%d)", i)))
				{
					strcpy(msg, str_replace(msg, JASS_VARARGS("$s(%d)", i), (char*) ptr));
				}
				if (strstr(msg, JASS_VARARGS("$d(%d)", i)))
				{
					strcpy(msg, str_replace(msg, JASS_VARARGS("$d(%d)", i), JASS_VARARGS("%d", *(int*) ptr)));
				}
				if (strstr(msg, JASS_VARARGS("$f(%d)", i)))
				{
					sprintf(temp, "%f", *(float*) ptr);
					strcpy(msg, str_replace(msg, JASS_VARARGS("$f(%d)", i), temp));
				}
				if (activator->client->pers.Lmd.account)
				{
					ptr = (void*)((int) activator->client->pers.Lmd.account + atoi(tok));
					if (strstr(msg, JASS_VARARGS("$as(%d)", i)))
					{
						strcpy(msg, str_replace(msg, JASS_VARARGS("$as(%d)", i), *(char**) ptr));
					}
					if (strstr(msg, JASS_VARARGS("$ad(%d)", i)))
					{
						strcpy(msg, str_replace(msg, JASS_VARARGS("$ad(%d)", i), JASS_VARARGS("%d", *(int*) ptr)));
					}
					if (strstr(msg, JASS_VARARGS("$af(%d)", i)))
					{
						sprintf(temp, "%f", *(float*) ptr);
						strcpy(msg, str_replace(msg, JASS_VARARGS("$af(%d)", i), temp));
					}
				}
				tok = strtok(NULL, " ");
				i++;
			}
		}
	}
		if (self->spawnflags & 8)
		{
			g_syscall(G_SEND_SERVER_COMMAND, clientNum, JASS_VARARGS("print \"%s\n\"", msg));
			return;
		}
		if (self->spawnflags & 16)
		{
			tok = strtok(msg, "\n");
			while (tok != NULL)
			{
				g_syscall(G_SEND_SERVER_COMMAND, clientNum, JASS_VARARGS("chat \"%s\"", tok));
				tok = strtok(NULL, "\n");
			}
			return;
		}
		g_syscall(G_SEND_SERVER_COMMAND, clientNum, JASS_VARARGS("cp \"%s\"", msg));
}

void SP_target_level_change( gentity_t *self )
{
	char *s = new char[1024];
	// a bit of a key spam, but not like it does any harm
	Lmd_SpawnString( "mode", "", &s );
	self->message = Lmd_NewString(s);
	Lmd_SpawnString( "target", "", &s );
	self->target = Lmd_NewString(s);
	Lmd_SpawnString( "target2", "", &s );
	self->target5 = Lmd_NewString(s);
	Lmd_SpawnString( "param", "", &s );
	self->target2 = Lmd_NewString(s);
	Lmd_SpawnString( "param2", "", &s );
	self->target3 = Lmd_NewString(s);
	Lmd_SpawnString( "param3", "", &s );
	self->target4 = Lmd_NewString(s);
	Lmd_SpawnString( "ctype", "integer", &s );
	self->cmd = Lmd_NewString(s);
	Lmd_SpawnString( "cmode", "0", &s );
	self->cmd2 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = TargetLCUse;
}

void TargetDelayThink(gentity_t* self)
{
	self->think = NULL;
	int* buf = &self->health;
	for (int i = 0; i < 32; i++)
	{
		if (!Lmd_EntFromNum(i)->client || !buf[i])
			continue;
		if (buf[i] <= g_level->time)
		{
			buf[i] = 0;
			G_UseTargets3(self, Lmd_EntFromNum(i), self->target);
			continue;
		}
		self->think = TargetDelayThink;
	}
	self->nextthink = g_level->time + 1000;
}

void __cdecl TargetDelayUse(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (activator->client && self->spawnflags & 2)
	{
		int* buf = &self->health;
		if (buf[activator->s.clientNum] && self->spawnflags & 1)
			return;
		buf[activator->s.clientNum] = g_level->time + ((int)self->wait + (int)self->random * crandom()) * 1000;
		if (!self->think)
		{
			self->think = TargetDelayThink;
			self->nextthink = g_level->time + 1000;
		}
		return;
	}

	WriteProcessMemory(GetCurrentProcess(), (void*)TARGET_DELAY_ADDR, (void*)&oldDelayUse, 6, NULL);
	((void(__cdecl*)(gentity_t*, gentity_t*, gentity_t*))TARGET_DELAY_ADDR)(self, other, activator);
	jump.arg = (DWORD)(&TargetDelayUse);
	WriteProcessMemory(GetCurrentProcess(), (void*)TARGET_DELAY_ADDR, (void*)&jump, 6, NULL);
}
#if LMD < 2463 // done pretty much the same way in versions > 2.4.6.2, with the exception of an "offset" key
void __cdecl LmdDropUse(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (self->spawnflags & 1)
	{
		vec3_t forward;
		activator->client->ps.viewangles[0] = 0;
		AngleVectors(activator->client->ps.viewangles, forward, NULL, NULL);
		VectorMA(activator->r.currentOrigin, 64, forward, self->s.origin);
		self->s.angles[1] = activator->client->ps.viewangles[1];
	}

	WriteProcessMemory(GetCurrentProcess(), (void*)LMD_DROP_ADDR, (void*)&oldLmdDropUse, 6, NULL);
	((void*(__cdecl*)(gentity_t*, gentity_t*, gentity_t*))LMD_DROP_ADDR)(self, other, activator);
	jump.arg = (DWORD)(&LmdDropUse);
	WriteProcessMemory(GetCurrentProcess(), (void*)LMD_DROP_ADDR, (void*)&jump, 6, NULL);
}
#endif

void target_powerup_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	// make sure there is a valid activator and a valid self
	// NOTE: unlike target_print, this use function is meaningless when called by a non-client entity
	if (!self || !self->inuse) return;
	if (!activator || !activator->client) return;
	// custom spawnflags for activating non-powerup effects, idea inspired by requests on robophred's site
	if (self->spawnflags & 1)
	{
		// invulnerability bubble
		if (self->count == 1)
		{
		    activator->client->ps.eFlags |= EF_INVULNERABLE;
			if (self->spawnflags & 2)
			{
				if (activator->client->invulnerableTimer <= g_level->time)
					activator->client->invulnerableTimer = g_level->time + self->wait * 1000;
				else
					activator->client->invulnerableTimer += self->wait * 1000;
			}
			else
			{
				activator->client->invulnerableTimer = g_level->time + self->wait * 1000;
			}
		}
		// electrocution effect
		if (self->count == 2)
		{
			if (self->spawnflags & 2)
			{
				if (activator->client->ps.electrifyTime <= g_level->time)
					activator->client->ps.electrifyTime = g_level->time + self->wait * 1000;
				else
					activator->client->ps.electrifyTime += self->wait * 1000;
			}
			else
			{
				activator->client->ps.electrifyTime = g_level->time + self->wait * 1000;
			}
		}
		// /shield command effect
		// NOTE: this does not actually give you powerup 7, use another target_powerup entity for that, this gives only the bullet deflection
		if (self->count == 3)
		{
			// keep track of the expiration time for the effect so it can be removed on ClientThink
			if (self->spawnflags & 2)
			{
				if (p_playerInfo[activator->s.number].pwShield <= g_level->time)
					p_playerInfo[activator->s.number].pwShield = g_level->time + self->wait * 1000;
				else
					p_playerInfo[activator->s.number].pwShield += self->wait * 1000;
			}
			else
			{
				p_playerInfo[activator->s.number].pwShield = g_level->time + self->wait * 1000;
			}
			activator->flags |= 0x80000; // this is for bullet deflection
		}
		return;
	}
	// invalid powerup check
	if (self->count < 0 || self->count > 15) return;
	if (self->spawnflags & 2)
	{
		if (activator->client->ps.powerups[self->count] <= g_level->time)
			activator->client->ps.powerups[self->count] = g_level->time + self->wait * 1000;
		else
			activator->client->ps.powerups[self->count] += self->wait * 1000;
	}
	else
	{
		activator->client->ps.powerups[self->count] = g_level->time + self->wait * 1000;
	}
	return;
}
// I seem to have a talent for doing unnecessary stuff, it's not so hard to find the use address for this, but I'm lazy
void SP_target_powerup(gentity_t* self)
{
	char* s;
	// FIXME: are absolute values necessary?
	Lmd_SpawnString("powerup", "1", &s);
	self->count = abs(atoi(s));
	Lmd_SpawnString("wait", "30", &s);
	self->wait = atoi(s);
	Lmd_SpawnString("spawnflags", "0", &s);
	self->spawnflags = abs(atoi(s));

	Lmd_SetOrigin(self, self->s.origin);
	self->use = target_powerup_use;
}

int Lmd_WPoints(gentity_t* ent)
{
	int n = 0;
	if (!ent) return 0;
	if (!ent->client) return 0;
	if (!ent->client->pers.Lmd.account) return 0;
	if (Lmd_GetProf(ent->client->pers.Lmd.account) < 4) return 0;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 0) n = 0;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 1) n = 4;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 2) n = 9;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 3) n = 14;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 4) n = 17;
	if (Lmd_Prof_Merc_GetSkill_Weapons(ent->client->pers.Lmd.account, Merc_Weapons) == 5) n = 20;
	if (HasSkill(ent->client->pers.Lmd.account, SKILL_WPPROF, qtrue)) n += 10;
	return n;
}

void S_ForcePowerRegenerate( gentity_t *self, int overrideAmt )
{ //called on a regular interval to regenerate force power.
	if ( !self->client )
	{
		return;
	}

	if (0 <= self->s.number && self->s.number < 32)
	{
	if (p_playerInfo[self->s.number].isPro > 1337)
	{
		overrideAmt = self->client->ps.fd.forcePowerMax;
		self->client->ps.fd.forcePowerRegenDebounceTime = g_level->time + 100;
	}
	// the force user fast regen skill
	if (self->client->pers.Lmd.account != NULL) {
	if (p_playerInfo[self->s.number].cds[SKILL_FASTREGEN] != 0) {
		if (Lmd_GetProf(self->client->pers.Lmd.account) == 3) {
		if (p_playerInfo[self->s.number].cds[SKILL_FASTREGEN] + 1000*30 >= g_level->time) overrideAmt *= 2;
		else { 
			if (p_playerInfo[self->s.number].fastRegen) { Lmd_PlayEffect(Lmd_EffectIndex("concussion/explosion"), self->client->ps.origin, self->client->ps.viewangles);
			p_playerInfo[self->s.number].fastRegen = 0; }
		}
		}
	}
	}
	// the merc slow regen skill
	if (p_playerInfo[self->s.number].regenSlowed > 0)
	{
		if (p_playerInfo[self->s.number].regenSlowed + 10*1000 <= g_level->time)
		{
			p_playerInfo[self->s.number].regenSlowed = 0;
			Lmd_PlayEffect(Lmd_EffectIndex("repeater/concussion"), self->client->ps.origin, self->client->ps.viewangles);
		}
		else
		{
			if (g_level->time % 3) return;
		}
	}
	}



	if ( overrideAmt )
	{ //custom regen amount
		self->client->ps.fd.forcePower += overrideAmt;
	}
	else
	{ //otherwise, just 1
		self->client->ps.fd.forcePower++;
	}

	if ( self->client->ps.fd.forcePower > self->client->ps.fd.forcePowerMax )
	{ //cap it off at the max (default 100)
		self->client->ps.fd.forcePower = self->client->ps.fd.forcePowerMax;
	}
}

qboolean Weapon_Fire(gentity_t *ent, weapon_t weaponNum, qboolean altFire) {
	if(weaponEntries[weaponNum] == NULL)
		return qfalse;

	weaponEntry_t* weapon;

	if (ent->s.number < 32)
	{
		weapon = &p_playerInfo[ent->s.number].weaponEnt[weaponNum];
	}
	else
		weapon = weaponEntries[weaponNum];

	/*if (gameMode(GM_INSTGIB) && weaponNum == WP_CONCUSSION) {
		altFire = qtrue;
	}*/

	weaponFire_t *data = (altFire) ? &weapon->secondary : &weapon->primary;

		if(data->projectiles == 0)
			data->fire(ent, NULL, data);
		else {
			int i;
			for(i = 0; i < data->projectiles; i++)
				Weapon_FireProjectile(ent, weaponNum, altFire, data);
		}

		if (ent->s.number < 32 && altFire == qfalse && weapon->primary.launcher.rate != 0)
			p_playerInfo[ent->s.number].fireTime = g_level->time + weapon->primary.launcher.rate;
		if (ent->s.number < 32 && altFire == qtrue && weapon->secondary.launcher.rate != 0) 
			p_playerInfo[ent->s.number].fireTime = g_level->time + weapon->secondary.launcher.rate;

return qtrue;
}

void Hook_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod )
{
	if (targ)
	{
		if (targ->s.number < 32)
		{ 
			if (p_playerInfo[targ->s.number].mDuelInProgress)
			{
				if (attacker && attacker->inuse && attacker->s.number < 32)
				{
					if (attacker != targ && p_playerInfo[targ->s.number].mDuelIndex != attacker->s.number) return;
				}
			}
			if (p_playerInfo[targ->s.number].cloaked)
			{
				p_playerInfo[targ->s.number].cloaked = 0;
				targ->s.eFlags &= ~EF_NODRAW;
				targ->client->ps.eFlags &= ~EF_NODRAW;
				targ->r.svFlags &= ~SVF_NOCLIENT;
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), targ->client->ps.origin, targ->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), targ->client->ps.origin, targ->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), targ->client->ps.origin, targ->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), targ->client->ps.origin, targ->client->ps.viewangles);
				Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), targ->client->ps.origin, targ->client->ps.viewangles);
			}
			if (targ->client->pers.Lmd.account)
			{
				if (HasSkill(targ->client->pers.Lmd.account, SKILL_LANDING, qtrue) && mod == MOD_FALLING && damage < 999)
					return;
			}
		}
	}
	if (attacker)
	{
		if (attacker->s.number < 32)
		{
				if (p_playerInfo[attacker->s.number].mDuelInProgress)
				{
					if (targ && targ->inuse && targ->s.number < 32)
					{
						if (p_playerInfo[attacker->s.number].mDuelIndex != targ->s.number && attacker != targ)
							return;
					}
				}
				if (p_playerInfo[attacker->s.number].cloaked)
				{
					p_playerInfo[attacker->s.number].cloaked = 0;
					attacker->s.eFlags &= ~EF_NODRAW;
					attacker->client->ps.eFlags &= ~EF_NODRAW;
					attacker->r.svFlags &= ~SVF_NOCLIENT;
					Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), attacker->client->ps.origin, attacker->client->ps.viewangles);
					Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), attacker->client->ps.origin, attacker->client->ps.viewangles);
					Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), attacker->client->ps.origin, attacker->client->ps.viewangles);
					Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), attacker->client->ps.origin, attacker->client->ps.viewangles);
					Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), attacker->client->ps.origin, attacker->client->ps.viewangles);
				}
		}
	}

	WriteProcessMemory(GetCurrentProcess(), (void*)DAMAGE, (void*)&oldDamage, 6, NULL);
	((void*(__cdecl*)(gentity_t*, gentity_t*, gentity_t*, vec3_t, vec3_t, int, int, int))DAMAGE)(targ, inflictor, attacker, dir, point, damage, dflags, mod);
	jump.arg = (DWORD)(&Hook_Damage);
	WriteProcessMemory(GetCurrentProcess(), (void*)DAMAGE, (void*)&jump, 6, NULL);
}

int Hook_ForcePowerUsableOn(gentity_t *attacker, gentity_t *other, forcePowers_t forcePower)
{
	int r;

	if (attacker && attacker->s.number < 32 && attacker->client && other && other->s.number < 32 && other->client)
	{
		if (p_playerInfo[other->s.number].mDuelInProgress > 0)
		{
			if (p_playerInfo[other->s.number].mDuelIndex != attacker->s.number)
				return 0;
		}
	}

	WriteProcessMemory(GetCurrentProcess(), (void*)FPUSABLE, (void*)&oldFPUsable, 6, NULL);
	r = ((int(__cdecl*)(gentity_t*, gentity_t*, forcePowers_t))FPUSABLE)(attacker, other, forcePower);
	jump.arg = (DWORD)(&Hook_ForcePowerUsableOn);
	WriteProcessMemory(GetCurrentProcess(), (void*)FPUSABLE, (void*)&jump, 6, NULL);

	return r;
}

void Hook_PlayerDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	gentity_t* dueler;
	if (self && self->inuse && self->s.number < 32)
	{
		if (p_playerInfo[self->s.number].mDuelInProgress == 1)
		{
			dueler = Lmd_EntFromNum(p_playerInfo[self->s.number].mDuelIndex);
			p_playerInfo[dueler->s.number].mDuelInProgress = 0;
			p_playerInfo[dueler->s.number].mDuelIndex = 0;
			p_playerInfo[self->s.number].mDuelInProgress = 0;
			p_playerInfo[self->s.number].mDuelIndex = 0;

			g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("print \"%s ^7(^1%d^7/^2%d^7) defeated %s ^7in a merc duel!\n\"", dueler->client->pers.netname,
				dueler->client->ps.stats[STAT_HEALTH], dueler->client->ps.stats[STAT_ARMOR], self->client->pers.netname));

			dueler->client->ps.stats[STAT_HEALTH] = dueler->client->ps.stats[STAT_MAX_HEALTH];
			dueler->health = dueler->client->ps.stats[STAT_MAX_HEALTH];
			dueler->client->ps.stats[STAT_ARMOR] = dueler->client->ps.stats[STAT_MAX_HEALTH];
			dueler->client->ps.powerups[7] = g_level->time;
		}
	}

	WriteProcessMemory(GetCurrentProcess(), (void*)PLAYERDIE, (void*)&oldPlayerDie, 6, NULL);
	((void*(__cdecl*)(gentity_t*, gentity_t*, gentity_t*, int, int))PLAYERDIE)(self, inflictor, attacker, damage, meansOfDeath);
	jump.arg = (DWORD)(&Hook_PlayerDie);
	WriteProcessMemory(GetCurrentProcess(), (void*)PLAYERDIE, (void*)&jump, 6, NULL);
}

void InitHooks()
{
	gentity_t* re;
	jump.instr_push = 0x68;
	jump.instr_ret = 0xC3;

	jump.arg = (DWORD)&SP_target_level_change;
	ReadProcessMemory(GetCurrentProcess(), (void*)SP_LEVEL_CHANGE, (void*)&oldTargetLCUse, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)SP_LEVEL_CHANGE, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&TargetDelayUse;
	ReadProcessMemory(GetCurrentProcess(), (void*)TARGET_DELAY_ADDR, (void*)&oldDelayUse, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)TARGET_DELAY_ADDR, (void*)&jump, 6, NULL);
	
#if LMD < 2463
	jump.arg = (DWORD)&LmdDropUse;
	ReadProcessMemory(GetCurrentProcess(), (void*)LMD_DROP_ADDR, (void*)&oldLmdDropUse, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)LMD_DROP_ADDR, (void*)&jump, 6, NULL);
#endif

#if PRINTHOOK > 0
	jump.arg = (DWORD)&Use_Target_Print;
	ReadProcessMemory(GetCurrentProcess(), (void*)TPUSE, (void*)&oldTP, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)TPUSE, (void*)&jump, 6, NULL);
#endif

	jump.arg = (DWORD)&S_ForcePowerRegenerate;
	ReadProcessMemory(GetCurrentProcess(), (void*)FORCEREGEN, (void*)&oldFPRegen, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)FORCEREGEN, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&Lmd_WPoints;
	ReadProcessMemory(GetCurrentProcess(), (void*)WPOINTS, (void*)&oldWP, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)WPOINTS, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&SP_target_powerup;
	ReadProcessMemory(GetCurrentProcess(), (void*)POWERUPSP, (void*)&oldPowerupSP, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)POWERUPSP, (void*)&jump, 6, NULL); 

	jump.arg = (DWORD)&Hook_Damage;
	ReadProcessMemory(GetCurrentProcess(), (void*)DAMAGE, (void*)&oldDamage, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)DAMAGE, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&Weapon_Fire;
	ReadProcessMemory(GetCurrentProcess(), (void*)0x201c3dc0, (void*)&oldWeapon, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)0x201c3dc0, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&Hook_PlayerDie;
	ReadProcessMemory(GetCurrentProcess(), (void*)PLAYERDIE, (void*)&oldPlayerDie, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)PLAYERDIE, (void*)&jump, 6, NULL);

	jump.arg = (DWORD)&Hook_ForcePowerUsableOn;
	ReadProcessMemory(GetCurrentProcess(), (void*)FPUSABLE, (void*)&oldFPUsable, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)FPUSABLE, (void*)&jump, 6, NULL);

	VirtualProtect((void*) TLCADDR, 4, PAGE_READWRITE, &flOldProtect);
	WriteProcessMemory(GetCurrentProcess(), (void*) TLCADDR, "lmd_multipurpose", 17, NULL);
	VirtualProtect((void*) TLCADDR, 4, flOldProtect, &flOldProtect);
	// memcpy((void*) 0x2037EFA0, &levelc, 3);

	// respawn all entities with spawn function hooked
	if (g_level)
	{
		if (g_level->time)
		{
			re = NULL;
			while ((re = Lmd_Find(re, FOFS(classname), "target_level_change")) != NULL)
			{
				Lmd_Edit(re->spawnData, "classname", "lmd_multipurpose");
				Lmd_SetSD(re, re->spawnData);
			}
			re = NULL;
			while ((re = Lmd_Find(re, FOFS(classname), "target_powerup")) != NULL)
			{
				Lmd_SetSD(re, re->spawnData);
			}
		}
	}

	g_syscall(G_PRINT, "Hooks initialized.\n");
}

void RemoveHooks()
{
	gentity_t* re;
	WriteProcessMemory(GetCurrentProcess(), (void*)SP_LEVEL_CHANGE, (void*)&oldTargetLCUse, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)TARGET_DELAY_ADDR, (void*)&oldDelayUse, 6, NULL);
#if LMD < 2463
	WriteProcessMemory(GetCurrentProcess(), (void*)LMD_DROP_ADDR, (void*)&oldLmdDropUse, 6, NULL);
#endif
#if PRINTHOOK > 0
	WriteProcessMemory(GetCurrentProcess(), (void*)TPUSE, (void*)&oldTP, 6, NULL);
#endif
	WriteProcessMemory(GetCurrentProcess(), (void*)FORCEREGEN, (void*)&oldFPRegen, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)WPOINTS, (void*)&oldWP, 6, NULL); 
	WriteProcessMemory(GetCurrentProcess(), (void*)POWERUPSP, (void*)&oldPowerupSP, 6, NULL); 
	WriteProcessMemory(GetCurrentProcess(), (void*)DAMAGE, (void*)&oldDamage, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)0x201c3dc0, (void*)&oldWeapon, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)PLAYERDIE, (void*)&oldPlayerDie, 6, NULL);
	WriteProcessMemory(GetCurrentProcess(), (void*)FPUSABLE, (void*)&oldFPUsable, 6, NULL);
	VirtualProtect((void*) TLCADDR, 4, PAGE_READWRITE, &flOldProtect);
	WriteProcessMemory(GetCurrentProcess(), (void*) TLCADDR, "target_level_change", 20, NULL);
	VirtualProtect((void*) TLCADDR, 4, flOldProtect, &flOldProtect);
	// for unloading support, respawn all entities with spawn function hooked, clean-up
	if (!shuttingDown)
	{
		re = NULL;
		while ((re = Lmd_Find(re, FOFS(classname), "lmd_multipurpose")) != NULL)
		{
			Lmd_Edit(re->spawnData, "mapname", "nothing");
			Lmd_Edit(re->spawnData, "classname", "target_level_change");
			Lmd_SetSD(re, re->spawnData);
		}
		re = NULL;
		while ((re = Lmd_Find(re, FOFS(classname), "target_powerup")) != NULL)
		{
			Lmd_SetSD(re, re->spawnData);
		}
	}
}