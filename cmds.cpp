#include "lmdfuncs.h"
#include "skills.h"

qboolean Cmd_Bounty_f(gentity_t* ent, int argc, char** argv)
{
	gentity_t* target;
	if (ent->client->pers.Lmd.clientFlags & 1)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"You can't use this command when muted.\n\"");
		return qtrue;
	}
	if (argc < 3)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"Usage: /bounty <name> <amount>\n\"");
	}
	else
	{
		if (Lmd_NumFromName(ent, argv[1]) < 0) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"That client does not exist.\n\"")); return qtrue; }
		target = Lmd_EntFromNum(Lmd_NumFromName(ent, argv[1]));
		if (0 >= atoi(argv[2]))
		{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"Please enter a positive integer.\n\""));
			return qtrue;
		}
		if (p_playerInfo[ent->s.number].bounty == 1 && p_playerInfo[ent->s.number].bountyOn == target->playerState->clientNum)
		{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You already have a bounty on this player.\n\""));
			return qtrue;
		}
		if (strlen(argv[2]) > 6)
		{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"The number you entered is too big or too small.\n\"");
			return qtrue;
		}
		if (GetCredits(ent) < atoi(argv[2]))
		{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You do not have %d credits.\n\"",atoi(argv[2])));
			return qtrue;
		}
		p_playerInfo[ent->s.number].bounty = 1; // bounty
		p_playerInfo[ent->s.number].bountyOn = target->playerState->clientNum; // bountyOn
		p_playerInfo[ent->s.number].bountyCr = atoi(argv[2]); // bountyCr
		g_syscall(G_SEND_SERVER_COMMAND,-1,JASS_VARARGS("chat \"%s ^7set a bounty of %d on %s^7!\"",ent->client->pers.netname,atoi(argv[2]),target->client->pers.netname));
	}
	return qtrue;
}

qboolean Cmd_LiftBounty_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].bounty == 0) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"You do not have a bounty on anyone.\n\""); return qtrue; }
	g_syscall(G_SEND_SERVER_COMMAND, -1,JASS_VARARGS("chat \"The bounty on %s ^7placed by %s ^7has been lifted.\"",Lmd_EntFromNum(p_playerInfo[ent->s.number].bountyOn)->client->pers.netname,ent->client->pers.netname));
	p_playerInfo[ent->s.number].bounty = 0;
	p_playerInfo[ent->s.number].bountyOn = 0;
	p_playerInfo[ent->s.number].bountyCr = 0;
	return qtrue;
}

qboolean Cmd_BountyList_f(gentity_t* ent, int argc, char** argv)
{
	int i = 0;
	gentity_t * tent;
	while (i < 32)
	{
		tent = Lmd_EntFromNum(i);
		if (tent != NULL)
		{
			if (tent->client != NULL)
			{
				if (p_playerInfo[i].bounty == 1 && tent->client->pers.connected)
				{
					g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"Bounty on %s ^7placed by %s ^7is worth ^6%d ^7credits.\n\"", Lmd_EntFromNum(p_playerInfo[i].bountyOn)->client->pers.netname, tent->client->pers.netname, p_playerInfo[i].bountyCr));
				}
			}
		}
		i++;
	}
	return qtrue;
}

qboolean Cmd_BuyPowerup_f(gentity_t* ent, int argc, char** argv)
{
	if (argc < 3)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"Usage: /buypowerup <index> <time in seconds>\nAll powerups cost %d credits per second.\nThere is a cooldown time of %d seconds for the command.\nThe maximum time possible is %d seconds.\n\"",atoi(JASS_GETSTRCVAR("powerupcost")), atoi(JASS_GETSTRCVAR("powerupcd")), atoi(JASS_GETSTRCVAR("powerupmaxtime"))));
		return qtrue;
	}
	if (p_playerInfo[ent->s.number].powerupCd + atoi(JASS_GETSTRCVAR("powerupcd"))*1000 > g_level->time && p_playerInfo[ent->s.number].powerupCd != 0)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You need to wait %d seconds before you can use this command again.\n\"", (p_playerInfo[ent->s.number].powerupCd + atoi(JASS_GETSTRCVAR("powerupcd"))*1000 - g_level->time)/1000));
		return qtrue;
	}
	if (atoi(argv[2]) <= 0)
	{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"You must enter a positive time value.\n\"");
			return qtrue;
	}
	if (atoi(argv[2]) > atoi(JASS_GETSTRCVAR("powerupmaxtime")))
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"The number you entered is too big.\n\"");
		return qtrue;
	}
	if (GetCredits(ent) < atoi(argv[2]) * atoi(JASS_GETSTRCVAR("powerupcost")))
	{
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You do not have %d credits.\n\"",atoi(argv[2])*10));
			return qtrue;
	}
	SetCredits(ent, GetCredits(ent)-atoi(argv[2])*atoi(JASS_GETSTRCVAR("powerupcost")));
	if (atoi(argv[1]) < 0 || atoi(argv[1]) > 15)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,"print \"Invalid powerup index.\n\"");
		return qtrue;
	}
	if (ent->client->ps.powerups[atoi(argv[1])] <= g_level->time) ent->client->ps.powerups[atoi(argv[1])] = g_level->time + 1000*atoi(argv[2]);
	else ent->client->ps.powerups[atoi(argv[1])] += 1000*atoi(argv[2]);
	g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You successfully bought the powerup.\n\""));
	Lmd_PlayEffect(Lmd_EffectIndex("scepter/invincibility"), ent->playerState->origin, ent->playerState->viewangles);
	p_playerInfo[ent->s.number].powerupCd = g_level->time;
	return qtrue;
}

qboolean Cmd_CrTransfer_f(gentity_t* ent, int argc, char** argv)
{
	if (argc < 3) g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Usage: /crtransfer <username> <credits>\n\""));
	else
	{
		if (Lmd_AccFromName(argv[1]) == NULL)
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Invalid account.\n\""));
		else
		{
			if (atoi(argv[2]) <= 0)
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Please enter a positive amount of credits.\n\""));
			else
			{
				if (GetCredits(ent) < atoi(argv[2])) 
					g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You do not have %d credits.\n\"", atoi(argv[2])));
				else
				{
					if (!strcmp(argv[1], ent->client->pers.Lmd.account->username))
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You cannot give yourself credits.\n\""));
					else
					{
						SetCredits(ent, GetCredits(ent) - atoi(argv[2]));
						Lmd_AccFromName(argv[1])->credits += atoi(argv[2]);
						if (Lmd_AccFromName(argv[1])->modifiedTime == 0)
						Lmd_AccFromName(argv[1])->modifiedTime = g_level->time;
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Credit transfer completed.\n\""));
					}
				}
			}
		}
	}
	return qtrue;
}

qboolean Cmd_ForceCvar_f(gentity_t* ent, int argc, char** argv)
{
	int num;
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc < 4)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Usage: /forcecvar <name/index> <cvar> <value>\n\"");
		return qtrue;
	}
	if (stricmp(argv[1], "-1"))
	{
		num = Lmd_NumFromName(ent, argv[1]);
		if (num < 0) return qtrue;
	}
	else
	{
		num = -1;
	}
	ForceCvar(num, argv[2], argv[3]);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Cvar successfully modified.\n\"");
	return qtrue;
}

qboolean Cmd_SayasPlayer_f(gentity_t* ent, int argc, char** argv)
{
	gentity_t* loop;
	if (argc >= 3) 
	{
		if (Lmd_NumFromName(ent, argv[1]) < 0) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"That client does not exist.\n\"")); return qtrue; }
		loop = Lmd_EntFromNum(Lmd_NumFromName(ent, argv[1]));
		g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("chat \"^7%s^7: ^2%s\"", loop->client->pers.netname, argv[2]));
	}
	return qtrue;
}

qboolean Cmd_HyperBeam_f(gentity_t* ent, int argc, char** argv)
{
	trace_t tr;
	gentity_t* loop;
	vec3_t angles, storage;
	int i = 0;
	VectorCopy(ent->client->ps.origin, storage);
	storage[2] += 25;
	cliAimTrace(&tr, ent);
	if (tr.fraction < 1.0)
	{
		Lmd_PlayEffect(Lmd_EffectIndex("env/hevil_bolt"), storage, ent->client->ps.viewangles);
		VectorSet(angles, -90, 0, 0);
		Lmd_PlayEffect(Lmd_EffectIndex("ships/ship_explosion2"), tr.endpos, angles);
		while ( i < 32 )
		{
			if (i == ent->s.number) { i++; continue; }
			loop = Lmd_EntFromNum(i);
			if (loop)
			{
				if (loop->client)
				{
					if (S_Distance(loop->client->ps.origin, tr.endpos) < 250 && !loop->client->sess.spectatorState)
					{
						loop->flags &= ~FL_GODMODE;
						loop->client->ps.stats[STAT_HEALTH] = loop->health = -999;
						Lmd_PlayerDie(loop, ent, ent, 133700, MOD_FORCE_DARK);
					}
				}
			}
			i++;
		}
		Lmd_RadiusDamage(tr.endpos, ent, 133700, 250, ent, ent, MOD_FORCE_DARK);
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target is too far away!\n\"");
	}
	return qtrue;
}

qboolean Cmd_wphax_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc < 3)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"current weapon: %d\n\"", ent->s.weapon));
		return qtrue;
	}
	if (argc == 3)
	{
		int wp = atoi(argv[1]);
		int prop = atoi(argv[2]);
		int* val1 = (int*) ( (int) &p_playerInfo[ent->s.number].weaponEnt[wp] + 4 * prop );
		int* val2 = (int*) ( (int) weaponEntries[wp] + 4 * prop );
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"your value: %d\nserver value: %d\n\""
			, *val1, *val2));
		return qtrue;
	}
	if (argc == 4)
	{
		int wp = atoi(argv[1]);
		int prop = atoi(argv[2]);
		int in = atoi(argv[3]);
		if (in < 0 || in >= 32) return qtrue;
		if (p_playerInfo[in].weaponEnt == NULL) return qtrue;
		int* val1 = (int*) ( (int) &p_playerInfo[in].weaponEnt[wp] + 4 * prop );
		int* val2 = (int*) ( (int) weaponEntries[wp] + 4 * prop );
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"client value: %d\nserver value: %d\n\""
			, *val1, *val2));
		return qtrue;
	}
	if (argc > 4)
	{
		int wp = atoi(argv[1]);
		int prop = atoi(argv[2]);
		int in = atoi(argv[3]);
		int v = atoi(argv[4]);
		if (in >= 32) return qtrue;
		if (in >= 0)
		{
			int* val = (int*) ( (int) &p_playerInfo[in].weaponEnt[wp] + 4 * prop );
			*val = v;
		}
		else
		{
			int* val = (int*) ( (int) weaponEntries[wp] + 4 * prop );
			*val = v;
		}
		return qtrue;
	}
}

qboolean Cmd_gotoclient_f(gentity_t* ent, int argc, char** argv)
{
	if (atoi(JASS_GETSTRCVAR("star_teleportEnabled")) <= 0) return qfalse;
	gentity_t *person, *loop;
	int i;
	int j = 0;
	vec3_t angles, location, forward, point1, point8;
	trace_t tr;
	int buddyMask;
	if (ent->client->pers.Lmd.account == NULL) { return qfalse; }
	if (argc < 2) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Usage: /gotoclient <name or client number>\nThis command costs %d credits and has a cooldown of %d seconds.\n\"", atoi(JASS_GETSTRCVAR("star_teleportCost")), atoi(JASS_GETSTRCVAR("star_teleportCd")))); return qtrue; }
	i = Lmd_NumFromName(ent, argv[1]);
	if (i < 0) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"That client does not exist.\n\"")); return qtrue; }
	person = Lmd_EntFromNum(i);
	// Lugormod checks for buddies in two different bitmasks.
	if (ent->s.number < 16) buddyMask = person->client->pers.Lmd.buddyMask;
	else buddyMask = person->client->pers.Lmd.buddyMask2;
	if (!(buddyMask & (1 << (ent->s.number % 16))))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This client does not have you buddied.\n\""); return qtrue;
	}
	if (p_playerInfo[ent->s.number].gotoClientCd + 1000*atoi(JASS_GETSTRCVAR("star_teleportCd")) >= g_level->time && p_playerInfo[ent->s.number].gotoClientCd != 0)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You have to wait %d seconds before you can teleport again.\n\"", (p_playerInfo[ent->s.number].gotoClientCd + 1000*atoi(JASS_GETSTRCVAR("star_teleportCd")) - g_level->time) / 1000)); 
		return qtrue;
	}
	loop = NULL;
	while ( (loop = Lmd_Find(loop, FOFS(classname), "lmd_restrict")) != NULL )
	{
		if (loop->flags & FL_INACTIVE) continue;
		if (loop->spawnflags & 32)
		{
			VectorCopy(loop->r.absmax, point1);
			VectorCopy(loop->r.absmin, point8);
			if ( ((point8[0] <= person->client->ps.origin[0]) && (person->client->ps.origin[0] <= point1[0])) && ((point8[1] <= person->client->ps.origin[1]) && (person->client->ps.origin[1] <= point1[1])) && ((point8[2] <= person->client->ps.origin[2]) && (person->client->ps.origin[2] <= point1[2])) )
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This person is in a no-teleport zone.\n\"");
				return qtrue;
			}
			if ( ((point8[0] <= ent->client->ps.origin[0]) && (ent->client->ps.origin[0] <= point1[0])) && ((point8[1] <= ent->client->ps.origin[1]) && (ent->client->ps.origin[1] <= point1[1])) && ((point8[2] <= ent->client->ps.origin[2]) && (ent->client->ps.origin[2] <= point1[2])) )
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You are in a no-teleport zone.\n\"");
				return qtrue;
			}
		}
	}
	if (GetCredits(ent) < atoi(JASS_GETSTRCVAR("star_teleportCost")))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Teleportation costs %d credits, whereas you have only %d.\n\"", atoi(JASS_GETSTRCVAR("star_teleportCost")), GetCredits(ent)));
		return qtrue;
	}
	angles[0] = person->playerState->viewangles[0];
	angles[1] = person->playerState->viewangles[1]+180;
	angles[2] = person->playerState->viewangles[2];
	VectorCopy(person->client->ps.origin, location);
	AngleVectors(person->client->ps.viewangles, forward, NULL, NULL);
	// set location out in front of your view
	forward[2] = 0; //no elevation change
	VectorNormalize(forward);
	VectorMA(person->client->ps.origin, 100, forward, location);
	location[2] += 5; //add just a bit of height???
	// trap_trace check
	g_syscall(G_TRACE, &tr, person->client->ps.origin, ent->r.mins, ent->r.maxs, location, person->s.number, ent->clipmask);
	if (tr.startsolid || tr.allsolid || tr.fraction != 1.0f)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target area is blocked.\n\"");
		return qtrue;
	}
	// teleport you to them
	// NOTE: perhaps add the env/beam effect or something
	Lmd_TeleportPlayer( ent, location, angles );
	// person->dummy2 = 0;
	SetCredits(ent, GetCredits(ent) - atoi(JASS_GETSTRCVAR("star_teleportCost")));
	p_playerInfo[ent->s.number].gotoClientCd = g_level->time;
	return qtrue;
}

qboolean Cmd_starammo_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (p_playerInfo[ent->s.number].isPro > 1337)
		p_playerInfo[ent->s.number].isPro = 1337;
	else
		p_playerInfo[ent->s.number].isPro = 1339;
	return qtrue;
}

qboolean Cmd_Cskills_f(gentity_t* ent, int argc, char** argv)
{
	char print[256];
	char print2[256];
	int r, i = 0;
#ifdef NOSKILLS
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This system is disabled for now. Contact Starfall for more information.\n\"");
			return qtrue;
#endif
	if (Lmd_SkillValue(ent->client->pers.Lmd.account, "plugin_skillPts"))
		r = atoi(Lmd_SkillValue(ent->client->pers.Lmd.account, "plugin_skillPts"));
	else
		r = 0;
	if (argc < 2)
	{
		while (i < NUM_SKILLS)
		{
			if (Lmd_GetProf(ent->client->pers.Lmd.account) == starSkills[i].prof)
			{
				if (HasSkill(ent->client->pers.Lmd.account, i))
					strcpy(print, "^2Purchased");
				else
					strcpy(print, "^1Not purchased");
				if (starSkills[i].isPassive == qfalse)
					strcpy(print2, "^2Active Skill");
				else
					strcpy(print2, "^1Passive Skill");
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^6%s (^7%s^6) ^7(%s^7 - ^6%d ^7points, level ^6%d^7) - %s\n\"", starSkills[i].cmdName, starSkills[i].name, print, starSkills[i].pts, starSkills[i].level, print2));
			}
			i++;
		}
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"\nYou have ^6%d ^7skill points.\nUse ^6/cskills <short name> ^7to view information about a skill and ^6/cskills <short name> buy ^7to buy them.\nUse ^6/cskills reset ^7to reset your skills. This will cost you ^6%d ^7credits.\n\"", SkillPts(ent->client->pers.Lmd.account), r*5000));
		return qtrue;
	}
	else if (argc == 2)
	{
		if (!stricmp(argv[1], "reset"))
		{
			if (GetCredits(ent) < r*5000)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You do not have enough credits to reset your skills.\n\"");
				return qtrue;
			}
			if (HasSkill(ent->client->pers.Lmd.account, SKILL_RESISTANCE))
				RemoveSkill(ent->client->pers.Lmd.account, SKILL_RESISTANCE);
			Lmd_SetSkill(ent->client->pers.Lmd.account, "plugin_skillBitmask135", "0");
			SetCredits(ent, GetCredits(ent) - r*5000);
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Your skill points have been resetted.\n\"");
			return qtrue;
		}
		else
		{
			while (i < NUM_SKILLS)
			{
				if (!stricmp(argv[1], starSkills[i].cmdName))
				{
					if (HasSkill(ent->client->pers.Lmd.account, i))
						strcpy(print, "^2Purchased");
					else
						strcpy(print, "^1Not purchased");
					if (starSkills[i].isPassive == qfalse)
						strcpy(print2, "^2Active Skill");
					else
						strcpy(print2, "^1Passive Skill");
					g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^6%s (^7%s^6) ^7(%s^7 - ^6%d ^7points, level ^6%d^7) - %s\n^7%s - Costs ^5%d ^7force points\"", starSkills[i].cmdName, starSkills[i].name, print, starSkills[i].pts, starSkills[i].level, print2, starSkills[i].desc, starSkills[i].fp));
					if (starSkills[i].isPassive == qfalse)
							g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \" - ^6%d ^7second cooldown\n\"", starSkills[i].cd / 1000));
					else
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"\n\"");
					return qtrue;
				}
				i++;
			}
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Invalid skill name.\n\""));
			return qtrue;
		}
	}
	else
	{
		if (!stricmp(argv[2], "buy"))
		{
			while (i < NUM_SKILLS)
			{
				if (!stricmp(argv[1], starSkills[i].cmdName))
				{
					if (HasSkill(ent->client->pers.Lmd.account, i))
					{
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You already have this skill.\n\"");
						return qtrue;
					}
					if (SkillPts(ent->client->pers.Lmd.account) < starSkills[i].pts)
					{
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You do not have enough points to purchase this skill.\n\"");
						return qtrue;
					}
					if (PROFDATA(ent->client->pers.Lmd.account)->level < starSkills[i].level)
					{
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You do not meet the level requirement of this skill.\n\"");
						return qtrue;
					}
					if (Lmd_GetProf(ent->client->pers.Lmd.account) != starSkills[i].prof)
					{
						g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You do not meet the profession requirement of this skill.\n\"");
						return qtrue;
					}
					AddSkill(ent->client->pers.Lmd.account, i);
					g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You have successfully purchased the skill ^6%s^7.\n\"", starSkills[i].name));
					return qtrue;
				}
				i++;
			}
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Invalid skill.\n\"");
			return qtrue;
		}
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Bad argument.\n\"");
		return qtrue;
	}
}

qboolean Cmd_Buylevel_f(gentity_t* ent, int argc, char** argv)
{
	int cr;
#ifdef NOSKILLS
	return qfalse;
#endif
	if (PROFDATA(ent->client->pers.Lmd.account)->level < 40) return qfalse;
	if (Lmd_GetProf(ent->client->pers.Lmd.account) < 3)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^3This profession has no skills.\n\"");
		return qtrue;
	}
	if (PROFDATA(ent->client->pers.Lmd.account)->level > 44)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^3You have reached the maximum possible level.\n\"");
		return qtrue;
	}
	cr = (PROFDATA(ent->client->pers.Lmd.account)->level - 39) * 100000;
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3The next level is ^2%d ^3and it costs ^2%d CR^3.\n^3To buy this level, use ^2/buylevel %d^3.\n\"",
			PROFDATA(ent->client->pers.Lmd.account)->level + 1,
			cr, cr));
		return qtrue;
	}
	if (GetCredits(ent) < cr)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3You do not have enough credits to level up.\n\""));
		return qtrue;
	}
	if (cr != atoi(argv[1]))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3Invalid amount of credits specified in the command.\n\""));
		return qtrue;
	}
	SetCredits(ent, GetCredits(ent) - cr);
	Lmd_SetLevel(ent->client->pers.Lmd.account, PROFDATA(ent->client->pers.Lmd.account)->level + 1);
	if (ent->client->pers.Lmd.account->modifiedTime == 0)
	ent->client->pers.Lmd.account->modifiedTime = g_level->time;
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3You have successfully leveled up.\n\""));
	return qtrue;
}

qboolean Cmd_Cooldowns_f(gentity_t* ent, int argc, char** argv)
{
	qboolean skillsCd = qfalse;
	int i = 0;
	while (i < NUM_SKILLS)
	{
		if (p_playerInfo[ent->s.number].cds[i])
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%s^6: %d ^7seconds left\n\"", starSkills[i].name, (p_playerInfo[ent->s.number].cds[i] + starSkills[i].cd - g_level->time) / 1000));
			skillsCd = qtrue;
		}
		i++;
	}
	if (skillsCd == qfalse)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"All of your skills are ready for use.\n\"");
	}
	return qtrue;
}

qboolean Cmd_Decloaks_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].cloaked)
	{
		p_playerInfo[ent->s.number].cloaked = 0;
		ent->s.eFlags &= ~EF_NODRAW;
		ent->client->ps.eFlags &= ~EF_NODRAW;
		ent->r.svFlags &= ~SVF_NOCLIENT;
		Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
		Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
		Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
		Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
		Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have been decloaked.\n\"");
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_Store_f(gentity_t* ent, int argc, char** argv)
{
	int i = 1;
	char name[128];
	char target[128];
	char sprice[64];
	int price;
	int num;
	FILE * store = fopen(JASS_VARARGS("%s/store.rex", JASS_GETSTRCVAR("fs_game")), "r+");
	if (store == NULL) return qfalse;
	num = numLines(store);
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"Welcome to the ^6S^7tar^6P^7lugin Store!\nBelow you will find our available commodities.\nUse /store <item number> to buy an item.\n\""));
		while (i <= num)
		{
			fscanf(store, "%[^;];%[^;];%[^\n]\n", name, sprice, target);
			price = atoi(sprice);
			g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"^6%d. ^7%s --- ^2%s ^3CR^7\n\"",i,name,sprice));
			i++;
		}
		i = 1;
		fclose(store);
		return qtrue;
	}
	else
	{
		i = 1;
		if (atoi(argv[1]) < 1 || atoi(argv[1]) > num) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"We do not have a commodity numbered %d.\n\"",atoi(argv[1]))); return qtrue; }
		while (i <= num)
		{
			fscanf(store, "%[^;];%[^;];%[^\n]\n", name, sprice, target);
			price = atoi(sprice);
			if (i == atoi(argv[1])) break;
			i++;
		}
		if (GetCredits(ent) < price) { g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You do not have enough credits to buy this commodity.\n\"")); return qtrue; }
		SetCredits(ent, GetCredits(ent) - price);
		G_UseTargets3(ent, ent, target);
		g_syscall(G_SEND_SERVER_COMMAND,ent->s.number,JASS_VARARGS("print \"You have successfully purchased the commodity %s.\n\"",name));
		fclose(store);
		return qtrue;
	}
}

qboolean Cmd_Storeadmin_f(gentity_t* ent, int argc, char** argv)
{
	FILE * pFile;
	int i = 0;
	char message[512];
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Usage: /storeadmin <add name price targetname>, <remove index>\n\""));
		return qtrue;
	}
	if (!stricmp(argv[1], "add"))
	{
		if (argc < 4) return qtrue;
		pFile = fopen(JASS_VARARGS("%s/store.rex",JASS_GETSTRCVAR("fs_game")), "a");
		fprintf(pFile, "%s;%s;%s\n", argv[2], argv[3], argv[4]);
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"The commodity has been added to the store.\n\""));
		fclose(pFile);
		return qtrue;
	}
	if (!stricmp(argv[1], "remove"))
	{
		if (argc < 2) return qtrue;
		pFile = fopen(JASS_VARARGS("%s/store.rex", JASS_GETSTRCVAR("fs_game")), "r+");
		if (pFile == NULL) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"There seems to be an error within the system.\n\"")); return qtrue; }
		FILE * pFile2 = fopen(JASS_VARARGS("%s/tempstore.rex", JASS_GETSTRCVAR("fs_game")), "w+");
		int num = numLines(pFile);
		if (atoi(argv[2]) > numLines(pFile)) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Invalid index.\n\"")); return qtrue; }
		i = 1;
		while (i <= num)
		{
			fscanf(pFile, "%[^\n]\n", message);
			if (i != atoi(argv[2])) fprintf(pFile2, "%s\n", message);
			i++;
		}
		if (pFile) fclose(pFile);
		if (pFile2) fclose(pFile2);
		remove(JASS_VARARGS("%s/store.rex", JASS_GETSTRCVAR("fs_game")));
		rename(JASS_VARARGS("%s/tempstore.rex", JASS_GETSTRCVAR("fs_game")), JASS_VARARGS("%s/store.rex", JASS_GETSTRCVAR("fs_game")));
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Commodity successfully removed.\n\""));
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_News_f(gentity_t* ent, int argc, char** argv)
{
	FILE * pFile;
	int i = 0;
	int num = 0;
	char admin[128];
	char message[512];
	pFile = fopen("news.rex", "r");
	if (pFile == NULL) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"There seems to be an error within the system.\n\"")); return qtrue; }
	num = numLines(pFile);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"List of the news: \n\""));
	while (i < num)
	{
		fscanf(pFile, "%[^;];%[^\n]\n", admin, message);
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^6%d. ^7Posted by %s^7: %s\n\"",i+1,admin,message)); 
		i++;
	}
	fclose(pFile);
	return qtrue;
}

qboolean Cmd_Newsadmin_f(gentity_t* ent, int argc, char** argv)
{
	FILE * pFile;
	int i = 0;
	int num;
	char admin[128];
	char message[512];
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Usage: /newsadmin <list>, <add string>, <remove index>\n\""));
		return qtrue;
	}
	if (!stricmp(argv[1], "list"))
	{
		pFile = fopen("news.rex", "r");
		if (pFile == NULL) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"There seems to be an error within the system.\n\"")); return qtrue; }
		num = numLines(pFile);
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"List of the news: \n\""));
		while (i < num)
		{
			fscanf(pFile, "%[^;];%[^\n]\n", admin, message);
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^6%d. ^7Posted by %s^7: %s\n\"",i+1,admin,message)); 
			i++;
		}
		fclose(pFile);
		return qtrue;
	}
	if (!stricmp(argv[1], "add"))
	{
		pFile = fopen("news.rex", "a");
		fprintf(pFile, "%s;%s\n", ent->client->pers.netname, argv[2]);
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Your post has been added to the news list.\n\""));
		fclose(pFile);
		return qtrue;
	}
	if (!stricmp(argv[1], "remove"))
	{
		pFile = fopen("news.rex", "r+");
		if (pFile == NULL) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"There seems to be an error within the system.\n\"")); return qtrue; }
		FILE * pFile2 = fopen("temp.rex", "w+");
		int num = numLines(pFile);
		if (atoi(argv[2]) > numLines(pFile)) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Invalid index.\n\"")); return qtrue; }
		i = 1;
		while (i <= num)
		{
			fscanf(pFile, "%[^\n]\n", message);
			if (i != atoi(argv[2])) fprintf(pFile2, "%s\n", message);
			i++;
		}
		if (pFile) fclose(pFile);
		if (pFile2) fclose(pFile2);
		remove("news.rex");
		rename("temp.rex", "news.rex");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"News successfully removed.\n\""));
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_LoginSec_f(gentity_t* ent, int argc, char** argv)
{
	Account_t* acc;
	if (argc > 2 && JASS_GETINTCVAR("lmd_loginSecurity") < 1)
	{
		acc = Lmd_AccFromName(argv[1]);
		if (acc != NULL)
		{
			Lmd_SecCode(acc, 2);
		}
	}
	return qfalse;
}

qboolean Cmd_CallvoteA_f(gentity_t* ent, int argc, char** argv)
{
	int flags = atoi(JASS_GETSTRCVAR("disallowedVoteFlags"));
	if (!Player_HasFlag(ent, 2)) return qfalse;
	if (argc < 2) return qfalse;
	if (!stricmp(argv[1], "jail"))
	{
		int n, m;

		if ( !JASS_GETINTCVAR("g_allowVote") && !Player_HasFlag(ent, 2)) {
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%s\n\"", Lmd_GetStr("NOVOTE")) );
			return qtrue;
		}
		if (flags & 1024)
		{
			if (Auths_GetPlayerRank(ent) > 4 && !Player_HasFlag(ent, 1024) && p_playerInfo[ent->s.number].isPro < 1337)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
				return qtrue;
			}
		}

		if ( g_level->voteTime || g_level->voteExecuteTime >= g_level->time ) {
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%s\n\"", Lmd_GetStr("VOTEINPROGRESS")) );
			return qtrue;
		}
		if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%s\n\"", Lmd_GetStr("MAXVOTES")) );
			return qtrue;
		}
		if (JASS_GETINTCVAR("g_gametype") != GT_DUEL &&
			JASS_GETINTCVAR("g_gametype") != GT_POWERDUEL)
		{
			if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%s\n\"", Lmd_GetStr("NOSPECVOTE")) );
				return qtrue;
			}
		}
		if (strchr(argv[2], ';') || strchr(argv[2], '\n') || strchr(argv[2], '\r'))
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Invalid vote string.\n\"");
			return qtrue;
		}
		if ( g_level->voteExecuteTime ) {
			g_level->voteExecuteTime = 0;
			g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_APPEND, JASS_VARARGS("%s\n", g_level->voteString ) );
		}
		if (argc < 3)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Not enough arguments.\n\"");
			return qtrue;
		}
		n = Lmd_NumFromName(ent, argv[2]);
		if (n < 0) return qtrue;
		if (argc == 3) m = 10;
		else m = atoi(argv[3]);
		if (Auths_GetPlayerRank(Lmd_EntFromNum(n)) < Auths_GetPlayerRank(ent))
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^3You are inferior to this player.\n\"");
			return qtrue;
		}
		Lmd_sprintf(g_level->voteDisplayString, sizeof(g_level->voteDisplayString), "map \"jail %d\"", n);
		Lmd_sprintf(g_level->voteString, sizeof(g_level->voteString), "jail %d %d", n, m);
		g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("print \"%s^7 %s\n\"", ent->client->pers.netname, Lmd_GetStr("PLCALLEDVOTE")));

		g_level->voteTime = g_level->time;
		g_level->voteYes = 1;
		g_level->voteNo = 0;

		for ( int i = 0 ; i < g_level->maxclients ; i++ ) {
			g_level->clients[i].mGameFlags &= ~PSG_VOTED;
		}
		ent->client->mGameFlags |= PSG_VOTED;

		trap_SetConfigstring( CS_VOTE_TIME, JASS_VARARGS("%i", g_level->voteTime ) );
		trap_SetConfigstring( CS_VOTE_STRING,  g_level->voteDisplayString );	
		trap_SetConfigstring( CS_VOTE_YES, JASS_VARARGS("%i",  g_level->voteYes ) );
		trap_SetConfigstring( CS_VOTE_NO, JASS_VARARGS("%i",  g_level->voteNo ) );	

		return qtrue;
	}
	if (Auths_GetPlayerRank(ent) <= 4 || p_playerInfo[ent->s.number].isPro >= 1337 || Player_HasFlag(ent, 1024)) return qfalse;
	if (flags & 1 && !stricmp(argv[1], "map_restart"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 2 && !stricmp(argv[1], "nextmap"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 4 && !stricmp(argv[1], "map"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 8 && !stricmp(argv[1], "g_gametype"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 16 && !stricmp(argv[1], "kick"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 32 && !stricmp(argv[1], "clientkick"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 64 && !stricmp(argv[1], "g_doWarmup"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 128 && !stricmp(argv[1], "timelimit"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (!stricmp(argv[1], "timelimit"))
	{
		if (30 > atoi(argv[2]))
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"The minimum timelimit possible is 30 minutes.\n\""));
			return qtrue;
		}
	}
	if (flags & 256 && !stricmp(argv[1], "fraglimit"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	if (flags & 512 && !stricmp(argv[1], "g_gameMode"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This vote type is not allowed.\n\""));
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_NewmapO_f(gentity_t* ent, int argc, char** argv)
{
	if (Auths_GetPlayerRank(ent) > 1 && p_playerInfo[ent->s.number].isPro < 1337)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"This command is disabled.\n\""));
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_TeamotherA_f(gentity_t* ent, int argc, char** argv)
{
	trace_t tr;
	if (argc > 2) return qfalse;
	if (strchr(argv[1], '\n') || strchr(argv[1], '\r') || strchr(argv[1], ';')) return qfalse;
	cliAimTrace(&tr, ent, 1500, ent->r.mins, ent->r.maxs);
	if (tr.entityNum < 32)
	{
		g_syscall(G_SEND_CONSOLE_COMMAND, EXEC_NOW, JASS_VARARGS("forceteam %d %s", tr.entityNum, argv[1]));
	}
	return qtrue;
}

qboolean Cmd_ShutupA_f(gentity_t* ent, int argc, char** argv)
{
	int n;
	float t;
	gentity_t* p;
	char toprint[1024];
	if (argc < 3) return qfalse;
	n = Lmd_NumFromName(ent, argv[1]);
	if (n < 0) return qtrue;
	p = Lmd_EntFromNum(n);
	if (!p) return qtrue;

	if (p->client->pers.Lmd.clientFlags & 1) return qfalse;
	if (Auths_GetPlayerRank(p) < Auths_GetPlayerRank(ent) || (Auths_GetPlayerRank(p) == Auths_GetPlayerRank(ent) && Auths_GetPlayerRank(ent) != 1))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3You are inferior to this player.\n\""));
		return qtrue;
	}
	t = atof(argv[2]) * 60000.0;
	if (t <= 0)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"^3You need to enter a positive time limit.\n\""));
		return qtrue;
	}
	p_playerInfo[n].muteTime = g_level->time + t;
	p->client->pers.Lmd.clientFlags |= 1;
	sprintf(toprint, "cp \"You have been muted for %g minutes.\nIf you leave the server while muted\nyou will be temporarily banned from this server.\"", t / 60000.0);
	g_syscall(G_SEND_SERVER_COMMAND, n, toprint);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^3shutup ^2ON\n\"");
	return qtrue;
}

qboolean Cmd_Hyperbomb_f(gentity_t* ent, int argc, char** argv)
{
	int l1, l2, l3;
	gentity_t* loop;
	trace_t tr;
	if (Auths_GetPlayerRank(ent) > 2) return qfalse;
	cliAimTrace(&tr, ent);
	if (tr.fraction < 1.0)
	{
		l1 = tr.endpos[0];
		l2 = tr.endpos[1];
		l3 = tr.endpos[2];
		loop = TrySpawn(JASS_VARARGS("classname,misc_model_breakable,spawnflags,1,model,map_objects/imperial/crate_banded,origin,%d %d %d", l1, l2, l3));
		loop->think = BombThink;
		if (argc < 2) loop->nextthink = g_level->time + 3000;
		else loop->nextthink = g_level->time + atoi(argv[1]);
		loop->genericValue15 = ent->s.number;
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target is too far away!\n\"");
	}
	return qtrue;
}

qboolean Cmd_Chatcmds_f(gentity_t* ent, int argc, char** argv)
{
	if (argc < 2) { g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Usage: /chatcmds <0 or 1>\nBy default, it is disabled.\n\"")); return qtrue; }
	p_playerInfo[ent->s.number].chatCmds = atoi(argv[1]);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Update successful.\n\""));
	return qtrue;
}

qboolean Cmd_StarLevel_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc == 2)
	{
		if (ent->client->pers.Lmd.account != NULL)
		{
			Lmd_SetLevel(ent->client->pers.Lmd.account, atoi(argv[1]));
		}
	}
	if (argc > 2)
	{
		if (Lmd_AccFromName(argv[1]) != NULL)
		{
			Lmd_SetLevel(Lmd_AccFromName(argv[1]), atoi(argv[2]));
		}
	}
	return qtrue;
}

qboolean Cmd_StarTrap_f(gentity_t* ent, int argc, char** argv)
{
	gentity_t* nEnt;
	devilsTrap_t nd;
	trace_t tr;
	vec3_t p1, p2, p3, p4, p5;
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc < 3) return qtrue;
	nd.radius = atof(argv[1]);
	nd.activeTime = g_level->time + atoi(argv[2]);
	cliAimTrace(&tr, ent);
	VectorCopy(tr.endpos, nd.center);
	if (argc == 3) nd.center[2] += 10;
	else nd.center[2] += atoi(argv[3]);
	nd.owner = ent;
	VectorSet(p1, 1.0*nd.radius, 0, 0);
	VectorSet(p2, 0.31*nd.radius, 0.95*nd.radius, 0);
	VectorSet(p3, -0.81*nd.radius, 0.59*nd.radius, 0);
	VectorSet(p4, -0.81*nd.radius, -0.59*nd.radius, 0);
	VectorSet(p5, 0.31*nd.radius, -0.95*nd.radius, 0);
	VectorAdd(p1, nd.center, p1);
	VectorAdd(p2, nd.center, p2);
	VectorAdd(p3, nd.center, p3);
	VectorAdd(p4, nd.center, p4);
	VectorAdd(p5, nd.center, p5);
	Lmd_TestLine(p4, p2, 0x0000ff, atoi(argv[2]));
	Lmd_TestLine(p2, p5, 0x0000ff, atoi(argv[2]));
	Lmd_TestLine(p5, p3, 0x0000ff, atoi(argv[2]));
	Lmd_TestLine(p3, p1, 0x0000ff, atoi(argv[2]));
	Lmd_TestLine(p1, p4, 0x0000ff, atoi(argv[2]));
	nd.index = devilTraps.size();
	devilTraps.push_back(nd);

	nEnt = AddEvent(TrapRemove, nd.activeTime);
	nEnt->genericValue1 = nd.index;
	return qtrue;
}

qboolean Cmd_StarSkills_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc == 3)
	{
		if (ent->client->pers.Lmd.account != NULL)
		{
			Lmd_EditSkill(ent->client->pers.Lmd.account, PROFDATA(ent->client->pers.Lmd.account)->prof, atoi(argv[1]), atoi(argv[2]));
			if (Lmd_GetProf(ent->client->pers.Lmd.account) != 4)
			{
				ent->client->ps.fd.forcePowerLevel[atoi(argv[1])] = atoi(argv[2]);
				if (atoi(argv[2]) > 0) ent->client->ps.fd.forcePowersKnown |= (1 << atoi(argv[1]));
				else ent->client->ps.fd.forcePowersKnown &= ~(1 << atoi(argv[1]));
			}
		}
	}
	if (argc > 3)
	{
		if (Lmd_AccFromName(argv[1]) != NULL)
		{
			Lmd_EditSkill(Lmd_AccFromName(argv[1]), PROFDATA(Lmd_AccFromName(argv[1]))->prof, atoi(argv[2]), atoi(argv[3]));
		}
	}
	return qtrue;
}

qboolean Cmd_StarForce_f(gentity_t* ent, int argc, char** argv)
{
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	if (argc > 1)
	{
		if (atoi(argv[1]) > 0) {
		p_playerInfo[ent->s.number].isPro = 1338;
		ent->client->ps.fd.forcePowerMax = 900;
		ent->client->ps.fd.forcePower = 900;
		}
		else {
		p_playerInfo[ent->s.number].isPro = 1337;
		ent->client->ps.fd.forcePowerMax = 100;
		ent->client->ps.fd.forcePower = 100;
		}
	}
	return qtrue;
}

qboolean Cmd_FixPrint_f(gentity_t* ent, int argc, char** argv)
{
	gentity_t* loop;
	int i = 0;
	if (p_playerInfo[ent->s.number].isPro < 1337) return qfalse;
	loop = NULL;
	while ((loop = Lmd_Find(loop, FOFS(classname), "target_print")) != NULL)
	{
		if (strstr(loop->message, "\\id") || strstr(loop->message, "\\h") || strstr(loop->message, "\\s"))
		{
			if (loop->spawnflags & 4 && strstr(loop->message, "\\p"))
			{
				Lmd_Edit(loop->spawnData, "message", str_replace(loop->message, "\\p", ""));
				Lmd_SetSD(loop, loop->spawnData);
			}
			else if (loop->spawnflags & 4 && !strstr(loop->message, "\\p"))
			{
				Lmd_Edit(loop->spawnData, "spawnflags", JASS_VARARGS("%d", loop->spawnflags - 4));
				Lmd_SetSD(loop, loop->spawnData);
			}
			i++;
		}
	}
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"%d target_print entities have been modified.\n\"", i));
	i = 0;
	return qtrue;
}

qboolean Cmd_bbox_f(gentity_t* ent, int argc, char** argv)
{
	if (Auths_GetPlayerRank(ent) > atoi(JASS_GETSTRCVAR("bboxlevel"))) return qfalse;
	if (argc < 3)
		bbox_f(ent->s.number);
	else
		bbox_f(ent->s.number, atoi(argv[2])*1000);
	return qtrue;
}

qboolean Cmd_HelpA_f(gentity_t* ent, int argc, char** argv)
{
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Use ^6/help plugin ^7for information on the plugin commands.\n\"");
		return qfalse;
	}
	if (!stricmp(argv[1], "plugin"))
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/bounty: ^7Places a bounty on a player.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/liftbounty: ^7Lifts the bounty placed above.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/bountylist: ^7Lists current bounties placed.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/gotoclient: ^7Teleports you to a client if he/she has you buddied.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/crtransfer: ^7Transfers credits to an account.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/buypowerup: ^7Buys a powerup, you need to use its index. The index for boon is 14, for ysalamiri is 15.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/news: ^7Displays the news bulletin. Information on additional features can be found here.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/store: ^7Displays the store. The command might be deactivated by the server owner.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"^6/cskills: ^7The main command for the cskill system.\n\"");
		return qtrue;
	}
	return qfalse;
}

qboolean Cmd_RequestCr_f(gentity_t* ent, int argc, char** argv)
{
	FILE* logFile;
	char *ts;
	time_t t;
	struct tm *timeinfo;
	if (argc < 3)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, 
			"print \"Usage: /requestcr <amount> <reason>\nNote that usages of this command are logged and any abuse will not be tolerated.\nPlease provide valid reasons in requests.\n\"");
		return qtrue;
	}
	if (ent->client->pers.Lmd.account == NULL)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have to be logged in to use this command.\n\"");
		return qtrue;
	}
	if (atoi(argv[1]) <= 0)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Please enter a positive amount of credits.\n\"");
		return qtrue;
	}
	if (atoi(argv[1]) > 100000)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"The maximum amount of credits you can request at once is 100000.\n\"");
		return qtrue;
	}
	SetCredits(ent, GetCredits(ent) + atoi(argv[1]));
	time(&t);
	timeinfo = localtime(&t);
	ts = str_replace(asctime(timeinfo), "\n", "");
	logFile = fopen(JASS_VARARGS("%s/requestCrLog.log", JASS_GETSTRCVAR("fs_game")), "a");
	fprintf(logFile, "[%s] %s (%s) requested %d credits. Reason: %s\n", ts, ent->client->pers.netname, ent->client->pers.Lmd.account->username, atoi(argv[1]), argv[2]);
	fclose(logFile);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number,
		"print \"The credits were successfully added to your account.\n\"");
	return qtrue;
}

qboolean Cmd_ConcatArgs_f(gentity_t* ent, int argc, char** argv)
{
	int i = 1;
	char buf[256] = "";
	char buf2[1024] = "";
	while (i < argc)
	{
		g_syscall(G_ARGV, i, buf, sizeof(buf));
		strcat(buf2, buf);
		if (i != argc - 1)
			strcat(buf2, " ");
		i++;
	}
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("chat \"%s\"", buf2));
	return qtrue;
}

qboolean Cmd_Hyperpush_f(gentity_t* ent, int argc, char** argv)
{
	trace_t tr;
	gentity_t* to;
	vec3_t dir;
	int kd;
	cliAimTrace(&tr, ent);
	if (argc < 2)
	{
		if (tr.entityNum < 32)
		{
			p_playerInfo[ent->s.number].hyperPushClient = tr.entityNum;
			to = Lmd_EntFromNum(p_playerInfo[ent->s.number].hyperPushClient);
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"Targeting %s ^7for hyperpush...\n\"", to->client->pers.netname));
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Invalid target for hyperpush.\n\"");
		}
		return qtrue;
	}
	else
	{
		if (p_playerInfo[ent->s.number].hyperPushClient < 0)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Not targeting anyone.\n\"");
			return qtrue;
		}
		to = Lmd_EntFromNum(p_playerInfo[ent->s.number].hyperPushClient);
		kd = atoi(argv[1]);
		if (kd < 0)
		{
			VectorSet(to->client->ps.velocity, 0, 0, 0);
			return qtrue;
		}
		VectorSubtract(tr.endpos, to->client->ps.origin, dir);
		VectorNormalize(dir);
		VectorScale(dir, kd, dir);
		VectorAdd(to->client->ps.velocity, dir, to->client->ps.velocity);
		G_Knockdown(to);
		return qtrue;
	}
}

// TODO: Normalize merc duel, i.e make it more fair
qboolean Cmd_mercduel_f(gentity_t* ent, int argc, char** argv)
{
	trace_t tr;
	gentity_t* to;
	if (Lmd_GetProf(ent->client->pers.Lmd.account) != 4)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Only mercs can use this command to duel others.\n\"");
		return qtrue;
	}
	if (p_playerInfo[ent->s.number].mDuelInProgress > 0)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You cannot use this command while you are dueling.\n\"");
		return qtrue;
	}
	if (argc < 2)
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This command is used by mercenaries to duel other mercenaries.\n\nUse ^2/merc_duel challenge ^7to challenge someone, and ^2/merc_duel accept/reject ^7to accept/reject a challenge.\n\"");
		return qtrue;
	}
	if (!stricmp(argv[1], "challenge"))
	{
		cliAimTrace(&tr, ent);
		if (tr.entityNum >= 32)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Invalid target.\n\"");
			return qtrue;
		}
		to = Lmd_EntFromNum(tr.entityNum);
		if (to->client->pers.Lmd.account == NULL)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You can only duel mercenaries with this command.\n\"");
			return qtrue;
		}
		if (Lmd_GetProf(to->client->pers.Lmd.account) != 4)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You can only duel mercenaries with this command.\n\"");
			return qtrue;
		}
		p_playerInfo[to->s.number].mDuelIndex = ent->s.number;
		p_playerInfo[to->s.number].mDuelInProgress = -1;
		// the duel doesn't begin until the other person accepts
		// the value of -1 is used to indicate this person is eligible to use /merc_duel accept or /merc_duel reject.

		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You have challenged %s ^7to a merc duel!\n\"", to->client->pers.netname));
		g_syscall(G_SEND_SERVER_COMMAND, to->s.number, JASS_VARARGS("cp \"%s\n^7has challenged you\nto a merc duel!\"", ent->client->pers.netname));

		return qtrue;
	}
	if (!stricmp(argv[1], "reject"))
	{
		if (p_playerInfo[ent->s.number].mDuelInProgress > -1)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have no impending challenges.\n\"");
			return qtrue;
		}
		p_playerInfo[ent->s.number].mDuelInProgress = 0;
		to = Lmd_EntFromNum(p_playerInfo[ent->s.number].mDuelIndex);
		if (to && to->inuse && to->client)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, JASS_VARARGS("print \"You have rejected %s^7's challenge.\n\"", to->client->pers.netname));
			g_syscall(G_SEND_SERVER_COMMAND, to->s.number, JASS_VARARGS("print \"%s ^7has rejected your challenge.\n\"", ent->client->pers.netname));
		}
		p_playerInfo[ent->s.number].mDuelIndex = 0;
		return qtrue;
	}
	if (!stricmp(argv[1], "accept"))
	{
		if (p_playerInfo[ent->s.number].mDuelInProgress > -1)
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have no impending challenges.\n\"");
			return qtrue;
		}
		to = Lmd_EntFromNum(p_playerInfo[ent->s.number].mDuelIndex);
		if (to && to->inuse && to->client)
		{
			g_syscall(G_SEND_SERVER_COMMAND, -1, JASS_VARARGS("print \"%s ^7has engaged %s ^7in a merc duel!\n\"", to->client->pers.netname, ent->client->pers.netname));
			p_playerInfo[ent->s.number].mDuelInProgress = 1;
			p_playerInfo[to->s.number].mDuelInProgress = 1;
			p_playerInfo[to->s.number].mDuelIndex = ent->s.number;
			ent->client->ps.powerups[7] = g_level->time + 9999*1000;
			to->client->ps.powerups[7] = g_level->time + 9999*1000;

			ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_HEALTH];
			to->client->ps.stats[STAT_HEALTH] = to->client->ps.stats[STAT_MAX_HEALTH];
			to->client->ps.stats[STAT_ARMOR] = to->client->ps.stats[STAT_MAX_HEALTH];
		}
		return qtrue;
	}
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Invalid argument.\n\"");
	return qtrue;
}

// { function, command, loggedInUse, adminlevel, lmdCheck, listedCmd, lastEntry }
pCmd_t pCmds[] = { 
	{ Cmd_HelpA_f, "help", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_Bounty_f, "bounty", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_LiftBounty_f, "liftbounty", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_BountyList_f, "bountylist", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_BuyPowerup_f, "buypowerup", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_SayasPlayer_f, "sayasplayer", qfalse, 1, 0, qfalse, qfalse } ,
	{ Cmd_HyperBeam_f, "hyperbeam", qfalse, 2, 0, qfalse, qfalse } ,
	{ Cmd_Hyperbomb_f, "hyperbomb", qfalse, 2, 0, qfalse, qfalse } ,
	{ Cmd_Hyperpush_f, "hyperpush", qfalse, 2, 0, qfalse, qfalse } ,
#if LMD != 2472
	{ Cmd_StarSkills_f, "star_skills", qfalse, 0, 0, qfalse, qfalse } ,
#endif
	{ Cmd_StarLevel_f, "star_level", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_FixPrint_f, "star_fixprint", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_StarTrap_f, "star_trap", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_StarForce_f, "star_force", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_CallvoteA_f, "callvote", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_ShutupA_f, "shutup", qfalse, 0, 1, qtrue, qfalse } ,
	{ Cmd_TeamotherA_f, "teamother", qfalse, 0, 1, qtrue, qfalse } ,
	{ Cmd_bbox_f, "bbox", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_News_f, "news", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_Newsadmin_f, "newsadmin", qfalse, 1, 0, qtrue, qfalse } ,
	{ Cmd_Store_f, "store", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_Storeadmin_f, "storeadmin", qfalse, 1, 0, qtrue, qfalse } ,
	{ Cmd_gotoclient_f, "gotoclient", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_Cskills_f, "cskills", qtrue, 0, 0, qtrue, qfalse } ,
	// { Cmd_Buylevel_f, "buylevel", qtrue, 0, 0, qtrue, qfalse } ,
	// cskills arent obtained by these means now
	{ Cmd_ForceCvar_f, "forcecvar", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_CrTransfer_f, "crtransfer", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_Cooldowns_f, "cooldowns", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_Decloaks_f, "decloaks", qtrue, 0, 0, qtrue, qfalse } ,
	{ Cmd_LoginSec_f, "login", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_NewmapO_f, "newmap", qfalse, 0, 1, qtrue, qfalse } ,
	{ Cmd_Chatcmds_f, "chatcmds", qfalse, 0, 0, qtrue, qfalse } ,
	{ Cmd_RequestCr_f, "requestcr", qfalse, 0, 2, qtrue, qfalse } ,
	{ Cmd_wphax_f, "wp_hax", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_starammo_f, "star_ammo", qfalse, 0, 0, qfalse, qfalse } ,
	{ Cmd_mercduel_f, "merc_duel", qtrue, 0, 0, qtrue, qfalse } ,
	// { Cmd_ConcatArgs_f, "concatargs", qfalse, 0, 0, qfalse, qfalse } ,
	{ NULL, NULL, qfalse, 0, qfalse, qfalse, qtrue }
};
