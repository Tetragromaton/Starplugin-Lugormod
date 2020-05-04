#include "skills.h"
#include "lmdfuncs.h"

starSkill_t starSkills[NUM_SKILLS];

int FastRegen(gentity_t* ent)
{
	int num = ent->s.number;
	ent->client->ps.stats[STAT_HEALTH] -= 50;
	ent->health -= 50;
	if (ent->client->ps.stats[STAT_HEALTH] <= 0)
	{
		ent->client->ps.stats[STAT_HEALTH] = 0;
		ent->health = 0;
		Lmd_PlayerDie(ent, ent, ent, 50, MOD_SUICIDE);
		return 1;
	}
	p_playerInfo[num].fastRegen = 1;
	Lmd_PlayEffect(Lmd_EffectIndex("concussion/explosion"), ent->client->ps.origin, ent->client->ps.viewangles);
	g_syscall(G_SEND_SERVER_COMMAND, num, JASS_VARARGS("print \"Your force regeneration is doubled for %d seconds.\n\"", 30/**(PROFDATA(ent->client->pers.Lmd.account)->level - 40)*/));
	return 1;
}

int TreatInjury(gentity_t* ent)
{
	int num = ent->s.number;
	int hp = ent->client->ps.stats[STAT_HEALTH];
	ent->client->ps.stats[STAT_HEALTH] += 100; // (PROFDATA(ent->client->pers.Lmd.account)->level - 40) * 45;
	ent->health += 50; // (PROFDATA(ent->client->pers.Lmd.account)->level - 40) * 45;
	if (ent->client->ps.stats[STAT_HEALTH] > ent->client->ps.stats[STAT_MAX_HEALTH])
	{
		ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
	}
	Lmd_PlayEffect(Lmd_EffectIndex("concussion/explosion"), ent->client->ps.origin, ent->client->ps.viewangles);
	g_syscall(G_SEND_SERVER_COMMAND, num, JASS_VARARGS("print \"You have healed %d health points.\n\"", ent->client->ps.stats[STAT_HEALTH] - hp));
	return 1;
}

int SlowRegen(gentity_t* ent)
{
	Account_t* acc;
	int num = ent->s.number;
	trace_t tr;
	cliAimTrace(&tr, ent);
	if (tr.fraction < 1.0)
	{
		if (0 <= tr.entityNum && tr.entityNum < 32)
		{
			acc = Lmd_EntFromNum(tr.entityNum)->client->pers.Lmd.account;
			if (!acc) p_playerInfo[tr.entityNum].regenSlowed = g_level->time;
			if (Lmd_GetProf(acc) == 4) 
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This skill cannot be used on mercenaries.\n\"");
				return 0;
			}
			p_playerInfo[tr.entityNum].regenSlowed = g_level->time;
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Force regeneration of the target successfully slowed down for 10 seconds.\n\"");
			g_syscall(G_SEND_SERVER_COMMAND, tr.entityNum, "print \"Your force regeneration has been slowed down.\n\"");
			Lmd_PlayEffect(Lmd_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
			return 1;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
			return 0;
		}
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
		return 0;
	}
}

void ForceBeamTimer(gentity_t* to)
{
	vec3_t angles, storage;
	trace_t tr;
	gentity_t* ent = Lmd_EntFromNum(to->genericValue1);

	if (to->genericValue5 == 0)
	{
		S_SetAnim(ent, 1356);
		to->nextthink = g_level->time + 415;
		to->genericValue5 = 1;
	}
	else if (to->genericValue5 == 1)
	{
		VectorCopy(ent->client->ps.origin, storage);
		storage[2] += 25;
		cliAimTrace(&tr, ent, 730);
		if (tr.fraction < 1.0)
		{
			Lmd_PlayEffect(Lmd_EffectIndex("env/hevil_bolt"), storage, ent->client->ps.viewangles);
			VectorSet(angles, -90, 0, 0);
			Lmd_PlayEffect(Lmd_EffectIndex("ships/ship_explosion2"), tr.endpos, angles);
			Lmd_RadiusDamage(tr.endpos, ent, 200, 200, ent, ent, MOD_FORCE_DARK);
			to->nextthink = g_level->time + 415;
			to->genericValue5 = 2;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, to->genericValue1, "print \"Target is too far away!\n\"");
			p_playerInfo[to->genericValue1].cds[SKILL_FORCEBEAM] = 0;
			Lmd_FreeEntity(to);
			S_AnimReset(ent);
			return;
		}
	}
	else
	{
		S_AnimReset(ent);
		Lmd_FreeEntity(to);
	}
}

int ForceBeam(gentity_t* ent)
{
	int num = ent->s.number;
	int s = 0;
	gentity_t* to;
	if (ent->client->pers.cmd.forwardmove || ent->client->pers.cmd.rightmove || ent->client->pers.cmd.upmove)
	{
		g_syscall(G_SEND_SERVER_COMMAND, num, "print \"You have to stand still when using this command.\n\"");
		return 0;
	}
	s = S_SetAnim(ent, 1355);
	if (s == ANIM_SUCCESS)
	{
		to = AddEvent(ForceBeamTimer, g_level->time + 750);
		to->genericValue1 = num;
		to->genericValue5 = 0;
		return 1;
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, num, "print \"This skill cannot be used while busy.\n\"");
		return 0;
	}
}

void BombThink(gentity_t* self)
{
	gentity_t* ent = Lmd_EntFromNum(self->genericValue15);
	Lmd_PlayEffect(Lmd_EffectIndex("explosions/wedge_explosion1"), self->s.origin, self->s.angles);
	Lmd_RadiusDamage(self->s.origin, ent, 133700, 500, ent, ent, MOD_THERMAL);
	Lmd_FreeEntity(self);
	return;
}

void TimeBombThink(gentity_t* self)
{
	gentity_t* ent = Lmd_EntFromNum(self->genericValue15);
	Lmd_PlayEffect(Lmd_EffectIndex("explosions/wedge_explosion1"), self->s.origin, self->s.angles);
	Lmd_RadiusDamage(self->s.origin, ent, 1000, 500, ent, ent, MOD_THERMAL);
	Lmd_FreeEntity(self);
	return;
}

int TimeBomb(gentity_t* ent)
{
		int l1 = 0;
		int l2 = 0;
		int l3 = 0;
		vec3_t mins;
		vec3_t maxs;
		int i = 0;
		gentity_t* loop;
		trace_t tr;
		VectorSet(mins, -13, -13, 0);
		VectorSet(maxs, 13, 13, 33);
		cliAimTrace(&tr, ent, 100, mins, maxs);
		if (tr.fraction < 1.0)
		{
			if (tr.contents & CONTENTS_BODY)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You can't place a time bomb on a player.\n\"");
				return 0;
			}
			l1 = tr.endpos[0];
			l2 = tr.endpos[1];
			l3 = tr.endpos[2];
			loop = TrySpawn(JASS_VARARGS("classname,misc_model_breakable,spawnflags,1,model,map_objects/imperial/crate_banded,origin,%d %d %d", l1, l2, l3));
			loop->think = TimeBombThink;
			loop->nextthink = g_level->time + 3000;
			loop->genericValue15 = ent->s.number;
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You placed a time bomb that will go off in 3 seconds.\n\"");
			return 1;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target is too far away!\n\"");
			return 0;
		}
}

int DisableJP(gentity_t* ent)
{
	int num = ent->s.number;
	trace_t tr;
	gentity_t* to;
	gentity_t* temp;
	cliAimTrace(&tr, ent);
	if (tr.fraction < 1.0)
	{
		if (0 <= tr.entityNum && tr.entityNum < 32)
		{
			to = Lmd_EntFromNum(tr.entityNum);
			if (!to) return 0;
			if (!to->client) return 0;
			if (to->client->ps.duelInProgress) return 0;
			if (Lmd_GetProf(to->client->pers.Lmd.account) < 4)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This skill can only be used on mercenaries.\n\"");
				return 0;
			}
			to->client->jetPackOn = qfalse;
			to->client->jetPackToggleTime = g_level->time + 10*1000;
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Jetpack of the target successfully disabled for 10 seconds.\n\"");
			g_syscall(G_SEND_SERVER_COMMAND, tr.entityNum, "print \"Your jetpack has been disabled for 10 seconds.\n\"");
			Lmd_PlayEffect(Lmd_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
			return 1;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
			return 0;
		}
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
		return 0;
	}
}

int PutYsalamiri(gentity_t* ent)
{
	int num = ent->s.number;
	trace_t tr;
	gentity_t* to;
	gentity_t* temp;
	cliAimTrace(&tr, ent);
	if (tr.fraction < 1.0)
	{
		if (0 <= tr.entityNum && tr.entityNum < 32)
		{
			to = Lmd_EntFromNum(tr.entityNum);
			if (!to) return 0;
			if (!to->client) return 0;
			if (to->client->ps.duelInProgress) return 0;
			if (Lmd_GetProf(to->client->pers.Lmd.account) == 4)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This skill cannot be used on mercenaries.\n\"");
				return 0;
			}
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"The target has been given a ysalamiri with a duration of five seconds.\n\"");
			if (to->client->ps.powerups[15] > g_level->time) to->client->ps.powerups[15] += 5000;
			else to->client->ps.powerups[15] = g_level->time + 5000;
			Lmd_PlayEffect(Lmd_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
			return 1;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
			return 0;
		}
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, num, "print \"No player targeted in the crosshair.\n\"");
		return 0;
	}
}

void CloakCheck(gentity_t* to)
{
	gentity_t* ent = Lmd_EntFromNum(to->genericValue1);
	if (p_playerInfo[ent->s.number].cloaked > g_level->time)
	{
			if (ent->client->ps.fd.forcePower - 30 < 0)
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
				ent->client->ps.fd.forcePower = 0;
				Lmd_FreeEntity(to);
			}
			else
			{
				ent->client->ps.fd.forcePower -= 30;
				to->nextthink = g_level->time + 1000;
			}
	}
	else
	{
		Lmd_FreeEntity(to);
	}
}

int CloakSkill(gentity_t* ent)
{
	gentity_t* to;
	ent->s.eFlags |= EF_NODRAW;
	ent->client->ps.eFlags |= EF_NODRAW;
	ent->r.svFlags |= SVF_NOCLIENT;
	p_playerInfo[ent->s.number].cloaked = g_level->time + 10*1000;
	Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
	Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
	Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
	Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
	Lmd_PlayEffect(Lmd_EffectIndex("ships/heavydmg"), ent->client->ps.origin, ent->client->ps.viewangles);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have been cloaked for 10 seconds.\n\"");
	to = AddEvent(CloakCheck, g_level->time + 1000);
	to->genericValue1 = ent->s.number;
	return 1;
}

int ForceCharge(gentity_t* ent)
{
	trace_t tr;
	cliAimTrace(&tr, ent, 730, ent->r.mins, ent->r.maxs);
	VectorCopy(tr.endpos, ent->client->ps.origin);
	VectorCopy(tr.endpos, ent->s.origin);
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have charged using the force.\n\"");
	return 1;
}

int ForceSlow(gentity_t* ent)
{
	trace_t tr;
	gentity_t* to;
	cliAimTrace(&tr, ent, 700);
	if (tr.fraction < 1.0 && tr.entityNum < 32)
	{
		to = Lmd_EntFromNum(tr.entityNum);
		if (to->client->ps.duelInProgress) return 0;
		Lmd_PlayEffect(Lmd_EffectIndex("repeater/concussion"), to->client->ps.origin, to->client->ps.viewangles);
		to->client->speed = JASS_GETINTCVAR("g_speed") * 0.5;
		to->client->speedTime = g_level->time + 10000;
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have slowed the enemy down for 10 seconds.\n\"");
		g_syscall(G_SEND_SERVER_COMMAND, to->s.number, "print \"You have been slowed down for 10 seconds.\n\"");
		return 1;
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You are not aiming at a valid player.\n\"");
		return 0;
	}
}

int MercSprint(gentity_t* ent)
{
	ent->client->speed = JASS_GETINTCVAR("g_speed") * 1.3;
	ent->client->speedTime = g_level->time + 10000;
	g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"You have used a stimulant to increase your agility for 10 seconds.\n\"");
	return 1;
}

void PoisonDamage(gentity_t* self)
{
	int dmg = irand(3, 8);
	gentity_t* to = Lmd_EntFromNum(self->genericValue1);
	gentity_t* ent = Lmd_EntFromNum(self->genericValue2);
	if (p_playerInfo[self->genericValue1].poisoned >= g_level->time)
	{
		to->client->ps.stats[STAT_HEALTH] -= dmg;
		to->health -= dmg;
		if (to->client->ps.stats[STAT_HEALTH] <= 0)
		{
			Lmd_PlayerDie(to, ent, ent, dmg, MOD_SLIME);
			Lmd_FreeEntity(self);
			return;
		}
	}
	else
	{
		Lmd_FreeEntity(self);
		return;
	}
	self->nextthink = g_level->time + 1000;
}

int PoisonDart(gentity_t* ent)
{
	trace_t tr;
	gentity_t* ev;
	gentity_t* to;
	cliAimTrace(&tr, ent, 700);
	if (tr.fraction < 1.0 && tr.entityNum < 32)
	{
		to = Lmd_EntFromNum(tr.entityNum);
		if (to->client->ps.duelInProgress) return 0;
		ev = AddEvent(PoisonDamage, g_level->time + 1000);
		ev->genericValue1 = tr.entityNum;
		ev->genericValue2 = ent->s.number;
		p_playerInfo[tr.entityNum].poisoned = g_level->time + 10000;
		Lmd_PlayEffect(Lmd_EffectIndex("noghri_stick/gas_cloud"), to->client->ps.origin, to->client->ps.viewangles);
		Lmd_PlayEffect(Lmd_EffectIndex("noghri_stick/muzzle_flash"), ent->client->ps.origin, ent->client->ps.viewangles);
		g_syscall(G_SEND_SERVER_COMMAND, tr.entityNum, "print \"You have been poisoned for 10 seconds.\n\"");
		return 1;
	}
	else
	{
		g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"The target is too far away!\n\"");
		return 0;
	}
}

long SkillBitmask(Account_t* acc)
{
	if (!acc) return 0;
	if (Lmd_SkillValue(acc, "plugin_skillBitmask135"))
		return atol(Lmd_SkillValue(acc, "plugin_skillBitmask135"));
	else 
		return 0;
}

int HasSkill(Account_t* acc, int index, qboolean check)
{
	if (!acc) return 0;
	if (check)
	{
		if (PROFDATA(acc)->level < starSkills[index].level
		|| Lmd_GetProf(acc) != starSkills[index].prof) return 0;
	}
	if ( SkillBitmask(acc) & (1 << index) ) return 1;
	else return 0;
}

int SkillPts(Account_t* acc)
{
	int r;
	int i = 0;
	if (!acc) return 0;
	if (Lmd_SkillValue(acc, "plugin_skillPts"))
		r = atoi(Lmd_SkillValue(acc, "plugin_skillPts"));
	else
		r = 0;
	while (i < NUM_SKILLS)
	{
		if (HasSkill(acc, i))
			r -= starSkills[i].pts;
		i++;
	}
	if (r < 0) return 0;
	else return r;
}

void AddSkill(Account_t* acc, int index)
{
	char val[64];
	int i = 0;
	gentity_t* ent;
	if (!acc) return;
	sprintf(val, "%d", SkillBitmask(acc) | (1 << index));
	Lmd_SetSkill(acc, "plugin_skillBitmask135", val);

	if (index == SKILL_RESISTANCE)
	{
		while (i < 32)
		{
			ent = Lmd_EntFromNum(i);
			if (ent->client->pers.Lmd.account == acc)
			{
				ent->client->ps.fd.forcePowerLevel[FP_PUSH] = 5;
				ent->client->ps.fd.forcePowerLevel[FP_PULL] = 5;
				return;
			}
			i++;
		}
	}
}

void RemoveSkill(Account_t* acc, int index)
{
	char val[64];
	int i = 0;
	gentity_t* ent;
	if (!acc) return;
	sprintf(val, "%d", SkillBitmask(acc) & ~(1 << index));
	Lmd_SetSkill(acc, "plugin_skillBitmask135", val);

	if (index == SKILL_RESISTANCE)
	{
		while (i < 32)
		{
			ent = Lmd_EntFromNum(i);
			if (ent->client->pers.Lmd.account == acc)
			{
				ent->client->ps.fd.forcePowerLevel[FP_PUSH] = 0;
				ent->client->ps.fd.forcePowerLevel[FP_PULL] = 0;
				return;
			}
			i++;
		}
	}
}

void InitSkills()
{
	// mercenary skills
	starSkills[0].cd = 30*1000;
	starSkills[0].level = 1;
	starSkills[0].prof = 4;
	starSkills[0].function = TreatInjury;
	starSkills[0].pts = 2;
	starSkills[0].fp = 0;
	starSkills[0].name = "Treat Injury";
	starSkills[0].cmdName = "treatinjury";
	starSkills[0].desc = "Through an instant medpack, the mercenary heals 50 HP instantly.";
	starSkills[0].isPassive = qfalse;

	starSkills[2].cd = 60*1000;
	starSkills[2].level = 1;
	starSkills[2].prof = 4;
	starSkills[2].function = SlowRegen;
	starSkills[2].pts = 3;
	starSkills[2].fp = 0;
	starSkills[2].name = "Slow Regeneration";
	starSkills[2].cmdName = "slowregen";
	starSkills[2].desc = "The mercenary makes use of crystals that weaken one's connection to the Force, slowing the regeneration of their enemy down.";
	starSkills[2].isPassive = qfalse;

	starSkills[4].cd = 180*1000;
	starSkills[4].level = 1;
	starSkills[4].prof = 4;
	starSkills[4].function = TimeBomb;
	starSkills[4].pts = 5;
	starSkills[4].fp = 0;
	starSkills[4].name = "Time Bomb";
	starSkills[4].cmdName = "timebomb";
	starSkills[4].desc = "Using extensive electronic knowledge, the mercenary builds a devastating time bomb that goes off in 3 seconds.";
	starSkills[4].isPassive = qfalse;

	starSkills[6].level = 1;
	starSkills[6].prof = 4;
	starSkills[6].pts = 4;
	starSkills[6].fp = 0;
	starSkills[6].name = "Weapon Proficiency";
	starSkills[6].cmdName = "wpprof";
	starSkills[6].desc = "Gives extra weapon proficiency to the mercenary that possesses this feat. Provides 10 more additional weapon points.";
	starSkills[6].isPassive = qtrue;

	// force user skills
	starSkills[1].cd = 180*1000;
	starSkills[1].level = 1;
	starSkills[1].prof = 3;
	starSkills[1].function = FastRegen;
	starSkills[1].pts = 2;
	starSkills[1].fp = 0;
	starSkills[1].name = "Force Body";
	starSkills[1].cmdName = "forcebody";
	starSkills[1].desc = "The Force user enacts a ritual using his own blood, doubling his rate of regeneration for 30 seconds but also dealing damage to him.";
	starSkills[1].isPassive = qfalse;

	starSkills[3].cd = 90*1000;
	starSkills[3].level = 1;
	starSkills[3].prof = 3;
	starSkills[3].function = ForceBeam;
	starSkills[3].pts = 3;
	starSkills[3].fp = 0;
	starSkills[3].name = "Force Beam";
	starSkills[3].cmdName = "forcebeam";
	starSkills[3].desc = "Sends a force beam to the target, dealing radius damage in a radius of 200 units. The user is immune to the damage. Has a cooldown of 30 seconds.";
	starSkills[3].isPassive = qfalse;

	starSkills[5].cd = 60*1000;
	starSkills[5].level = 1;
	starSkills[5].prof = 3;
	starSkills[5].function = DisableJP;
	starSkills[5].name = "Jetpack Disabling";
	starSkills[5].pts = 3;
	starSkills[5].fp = 0;
	starSkills[5].cmdName = "disablejp";
	starSkills[5].desc = "Electronically scrambles the target's jetpack through the Force, disabling it for 10 seconds.";
	starSkills[5].isPassive = qfalse;

	starSkills[7].cd = 120*1000;
	starSkills[7].level = 1;
	starSkills[7].prof = 3;
	starSkills[7].function = PutYsalamiri;
	starSkills[7].pts = 4;
	starSkills[7].fp = 50;
	starSkills[7].name = "Force Breach";
	starSkills[7].cmdName = "forcebreach";
	starSkills[7].desc = "Creates a wound in the Force surrounding your enemy, making him unable to interact with the Force.";
	starSkills[7].isPassive = qfalse;

	starSkills[8].cd = 70*1000;
	starSkills[8].level = 1;
	starSkills[8].prof = 3;
	starSkills[8].function = CloakSkill;
	starSkills[8].pts = 4;
	starSkills[8].fp = 0;
	starSkills[8].name = "Force Cloak";
	starSkills[8].cmdName = "forcecloak";
	starSkills[8].desc = "With Force Power consumption, a Force user can alter the path of photons such that he becomes invisible unless he reveals himself.";
	starSkills[8].isPassive = qfalse;

	starSkills[SKILL_CHARGE].cd = 60*1000;
	starSkills[SKILL_CHARGE].level = 1;
	starSkills[SKILL_CHARGE].prof = 3;
	starSkills[SKILL_CHARGE].function = ForceCharge;
	starSkills[SKILL_CHARGE].pts = 3;
	starSkills[SKILL_CHARGE].fp = 50;
	starSkills[SKILL_CHARGE].name = "Force Charge";
	starSkills[SKILL_CHARGE].cmdName = "forcecharge";
	starSkills[SKILL_CHARGE].desc = "Uses the Force to grant you photon-like speed for a short duration, charging you for as far as 730 units.";
	starSkills[SKILL_CHARGE].isPassive = qfalse;

	starSkills[SKILL_RESISTANCE].level = 1;
	starSkills[SKILL_RESISTANCE].prof = 4;
	starSkills[SKILL_RESISTANCE].pts = 4;
	starSkills[SKILL_RESISTANCE].name = "Force Resistance";
	starSkills[SKILL_RESISTANCE].cmdName = "forceresistance";
	starSkills[SKILL_RESISTANCE].desc = "Increases a mercenary's resilience to the Force, granting Force user-like Force Push and Pull resistance.";
	starSkills[SKILL_RESISTANCE].isPassive = qtrue;

	starSkills[SKILL_LANDING].level = 1;
	starSkills[SKILL_LANDING].prof = 3;
	starSkills[SKILL_LANDING].pts = 2;
	starSkills[SKILL_LANDING].name = "Force Landing";
	starSkills[SKILL_LANDING].cmdName = "forcelanding";
	starSkills[SKILL_LANDING].desc = "Renders a Force user immune to unexpected falls by softening landings through the Force.";
	starSkills[SKILL_LANDING].isPassive = qtrue;

	starSkills[SKILL_SLOW].cd = 180*1000;
	starSkills[SKILL_SLOW].level = 1;
	starSkills[SKILL_SLOW].prof = 3;
	starSkills[SKILL_SLOW].pts = 3;
	starSkills[SKILL_SLOW].fp = 50;
	starSkills[SKILL_SLOW].function = ForceSlow;
	starSkills[SKILL_SLOW].name = "Force Slow";
	starSkills[SKILL_SLOW].cmdName = "forceslow";
	starSkills[SKILL_SLOW].desc = "The force user uses telekinesis to slow down an enemy for 10 seconds, sacrificing his own Force energy in the process.";
	starSkills[SKILL_SLOW].isPassive = qfalse;

	starSkills[SKILL_ADRENALINE].cd = 130*1000;
	starSkills[SKILL_ADRENALINE].level = 1;
	starSkills[SKILL_ADRENALINE].prof = 4;
	starSkills[SKILL_ADRENALINE].pts = 2;
	starSkills[SKILL_ADRENALINE].fp = 0;
	starSkills[SKILL_ADRENALINE].function = MercSprint;
	starSkills[SKILL_ADRENALINE].name = "Adrenaline Stimulation";
	starSkills[SKILL_ADRENALINE].cmdName = "adstimulation";
	starSkills[SKILL_ADRENALINE].desc = "The mercenary uses biofeedback techniques to unleash more adrenaline to his blood stream, increasing his overall agility for 10 seconds.";
	starSkills[SKILL_ADRENALINE].isPassive = qfalse;

	starSkills[SKILL_POISON].cd = 120*1000;
	starSkills[SKILL_POISON].level = 1;
	starSkills[SKILL_POISON].prof = 4;
	starSkills[SKILL_POISON].pts = 2;
	starSkills[SKILL_POISON].fp = 0;
	starSkills[SKILL_POISON].function = PoisonDart;
	starSkills[SKILL_POISON].name = "Poison Dart";
	starSkills[SKILL_POISON].cmdName = "poisondart";
	starSkills[SKILL_POISON].desc = "The mercenary makes use of a highly toxic dart to shoot the enemy, dealing damage over the course of 10 seconds.";
	starSkills[SKILL_POISON].isPassive = qfalse;

	g_syscall(G_PRINT, "Skills initialized.\n");
}