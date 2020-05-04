int FastRegen(gentity_t* ent)
{
	int num = ent->s.number;
	p_playerInfo[num].fastRegen = 1;
	Lmd_PlayEffect(G_EffectIndex("concussion/explosion"), ent->client->ps.origin, ent->client->ps.viewangles);
	g_syscall(G_SEND_SERVER_COMMAND, num, JASS_VARARGS("print \"Your force regeneration is doubled for %d seconds.\n\"", 30/**(ent->client->pers.Lmd.account->profession->level - 40)*/));
	return 1;
}

int TreatInjury(gentity_t* ent)
{
	int num = ent->s.number;
	int hp = ent->client->ps.stats[STAT_HEALTH];
	ent->client->ps.stats[STAT_HEALTH] += 100; // (ent->client->pers.Lmd.account->profession->level - 40) * 45;
	ent->health += 50; // (ent->client->pers.Lmd.account->profession->level - 40) * 45;
	if (ent->client->ps.stats[STAT_HEALTH] > ent->client->ps.stats[STAT_MAX_HEALTH])
	{
		ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH];
		ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
	}
	Lmd_PlayEffect(G_EffectIndex("concussion/explosion"), ent->client->ps.origin, ent->client->ps.viewangles);
	g_syscall(G_SEND_SERVER_COMMAND, num, JASS_VARARGS("print \"You have healed %d health points.\n\"", ent->client->ps.stats[STAT_HEALTH] - hp));
	return 1;
}

int SlowRegen(gentity_t* ent)
{
	Account_t* acc;
	int num = ent->s.number;
	trace_t tr;
	vec3_t forward;
	vec3_t location;
	vec3_t storage;
	VectorCopy(ent->client->ps.origin, location);
	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorCopy(ent->client->ps.origin, storage);
	storage[2] += 25;
	// set location out in front of your view
	VectorNormalize(forward);
	VectorMA(ent->client->ps.origin, 26635, forward, location);
	g_syscall(G_TRACE, &tr, storage, 0, 0, location, ent->s.number, ent->clipmask);
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
			Lmd_PlayEffect(G_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
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
		vec3_t location;
		vec3_t forward;
		vec3_t storage;
		VectorSet(mins, -13, -13, 0);
		VectorSet(maxs, 13, 13, 33);
		VectorCopy(ent->client->ps.origin, location);
		VectorCopy(ent->client->ps.origin, storage);
		storage[2] += 25;
		AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
		VectorNormalize(forward);
		VectorMA(ent->client->ps.origin, 75, forward, location);
		g_syscall(G_TRACE, &tr, storage, mins, maxs, location, ent->s.number, ent->clipmask);
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
			loop = sp(JASS_VARARGS("classname,misc_model_breakable,spawnflags,1,model,map_objects/imperial/crate_banded,origin,%d %d %d", l1, l2, l3));
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
	vec3_t forward;
	vec3_t location;
	vec3_t storage;
	gentity_t* to;
	gentity_t* temp;
	VectorCopy(ent->client->ps.origin, location);
	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorCopy(ent->client->ps.origin, storage);
	storage[2] += 25;
	// set location out in front of your view
	VectorNormalize(forward);
	VectorMA(ent->client->ps.origin, 26635, forward, location);
	g_syscall(G_TRACE, &tr, storage, 0, 0, location, ent->s.number, ent->clipmask);
	if (tr.fraction < 1.0)
	{
		if (0 <= tr.entityNum && tr.entityNum < 32)
		{
			to = Lmd_EntFromNum(tr.entityNum);
			if (!to) return 0;
			if (!to->client) return 0;
			if (Lmd_GetProf(to->client->pers.Lmd.account) < 4)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This skill can only be used on mercenaries.\n\"");
				return 0;
			}
			to->client->jetPackOn = qfalse;
			to->client->ps.jetpackFuel = -1;
			p_playerInfo[tr.entityNum].jpDisabled = g_level->time;
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Jetpack of the target successfully disabled for 10 seconds.\n\"");
			g_syscall(G_SEND_SERVER_COMMAND, tr.entityNum, "print \"Your jetpack has been disabled for 10 seconds.\n\"");
			Lmd_PlayEffect(G_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
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
	vec3_t forward;
	vec3_t location;
	vec3_t storage;
	gentity_t* to;
	gentity_t* temp;
	VectorCopy(ent->client->ps.origin, location);
	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorCopy(ent->client->ps.origin, storage);
	storage[2] += 25;
	// set location out in front of your view
	VectorNormalize(forward);
	VectorMA(ent->client->ps.origin, 26635, forward, location);
	g_syscall(G_TRACE, &tr, storage, 0, 0, location, ent->s.number, ent->clipmask);
	if (tr.fraction < 1.0)
	{
		if (0 <= tr.entityNum && tr.entityNum < 32)
		{
			to = Lmd_EntFromNum(tr.entityNum);
			if (!to) return 0;
			if (!to->client) return 0;
			if (Lmd_GetProf(to->client->pers.Lmd.account) == 4)
			{
				g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"This skill cannot be used on mercenaries.\n\"");
				return 0;
			}
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"The target has been given a ysalamiri with a duration of three seconds.\n\"");
			if (to->client->ps.powerups[15] > g_level->time) to->client->ps.powerups[15] += 3000;
			else to->client->ps.powerups[15] = g_level->time + 3000;
			Lmd_PlayEffect(G_EffectIndex("repeater/concussion"), Lmd_EntFromNum(tr.entityNum)->client->ps.origin, Lmd_EntFromNum(tr.entityNum)->client->ps.viewangles);
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