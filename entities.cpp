#include "lmdfuncs.h"
#include "entities.h"

entLogicalData_t LTrue = Logical_True;

/* ** target_heal **
Heals or damages the user by the specified amount of health and armor.
spawnflags 1: Deals armor damage to health if armor is 0. */
void target_heal_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	activator->client->ps.stats[STAT_ARMOR] += atoi(self->target3);
	if (activator->client->ps.stats[STAT_MAX_HEALTH] < activator->client->ps.stats[STAT_ARMOR])
	{
		activator->client->ps.stats[STAT_ARMOR] = activator->client->ps.stats[STAT_MAX_HEALTH];
	}
	if (activator->client->ps.stats[STAT_ARMOR] < 0)
	{
		if (self->spawnflags & 1)
		{
			activator->health += activator->client->ps.stats[STAT_ARMOR];
			activator->client->ps.stats[STAT_HEALTH] += activator->client->ps.stats[STAT_ARMOR];
		}
		activator->client->ps.stats[STAT_ARMOR] = 0;
	}
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
	return;
}

void sp_target_heal(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "health", "", &s );
	self->target2 = Lmd_NewString(s);
	Lmd_SpawnString( "armor", "", &s );
	self->target3 = Lmd_NewString(s);
	Lmd_SpawnString("spawnflags", "0", &s);
	self->spawnflags = abs(atoi(s));

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_heal_use;
}

/* ** target_modify **
Modifies all entities on a map with the given targetname. It can change any specified key of the entity.
This has the same effect as /spawnstring edit, and results in the entity respawning. */
void target_modify_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	gentity_t* en = NULL;
	while ((en = Lmd_Find(en, FOFS(targetname), self->target)) != NULL)
	{
		Lmd_Edit(en->spawnData, self->target2, self->target3);
		Lmd_SetSD(en, en->spawnData);
	}
	return;
}

void sp_target_modify(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "target", "", &s );
	self->target = Lmd_NewString(s);
	Lmd_SpawnString( "key", "", &s );
	self->target2 = Lmd_NewString(s);
	Lmd_SpawnString( "value", "", &s);
	self->target3 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_modify_use;
}

/* ** target_execute **
Fires a given targetname for all players in a certain bounding box.
Not specifying these keys fires it for all players on the map.
By targeting a lmd_playercheck, you can filter the players and fire it for those with a special property.*/
void target_execute_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	bool bbox = true;
	vec3_t point1;
	vec3_t point8;
	int i = 0;
	gentity_t* en;
	if (strlen(self->target3) > 0 && strlen(self->target4) > 0)
	{
		sscanf(self->target3, "%f %f %f", &point8[0], &point8[1], &point8[2]);
		sscanf(self->target4, "%f %f %f", &point1[0], &point1[1], &point1[2]);
	}
	else bbox = false;
	while (i < 32)
	{
		en = Lmd_EntFromNum(i);
		if (!en) { i++; continue; }
		if (!en->client) { i++; continue; }
		if (!en->client->pers.connected) { i++; continue; }
		if (strlen(self->target3) > 0 && strlen(self->target4) > 0)
		{
			if (!bbox)
			{
				G_UseTargets3(en, en, self->target);
				i++; continue;
			}
			if ( ((point8[0] <= en->client->ps.origin[0]) && (en->client->ps.origin[0] <= point1[0])) && ((point8[1] <= en->client->ps.origin[1]) && (en->client->ps.origin[1] <= point1[1])) && ((point8[2] <= en->client->ps.origin[2]) && (en->client->ps.origin[2] <= point1[2])) )
				G_UseTargets3(en, en, self->target);
		}
		i++;
	}
}

void sp_target_execute(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "target", "", &s );
	self->target = Lmd_NewString(s);
	Lmd_SpawnString( "mins", "", &s );
	self->target3 = Lmd_NewString(s);
	Lmd_SpawnString( "maxs", "", &s);
	self->target4 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_execute_use;
}

/* ** target_oddball **
This lets the plugin know that the user now has an oddball.
By using this, the plugin can internally provide that player with bonuses, such as doubled credit rewards on killstreaks. */
void target_oddball_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (activator && activator->client)
	p_playerInfo[activator->s.number].oddball = atoi(self->target2);
	return;
}

void sp_target_oddball(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "value", "", &s );
	self->target2 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_oddball_use;
}

/* ** target_place **
This spawns a new entity on the map.
This new entity can be saved/loaded through mapfiles. Hence, use with care and do not save the map if this entity is in use. */
void target_place_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	char ss[1024];

	if (strlen(self->target2) > 0)
		sprintf(ss, "%sclassname,%s,", ss, self->target2);
	if (strlen(self->target3) > 0)
		sprintf(ss, "%sorigin,%s,", ss, self->target3);
	if (strlen(self->target4) > 0)
		sprintf(ss, "%s%s", ss, self->target4);
	TrySpawn(ss);
	return;
}

void sp_target_place(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "classname", "", &s );
	self->target2 = Lmd_NewString(s);
	Lmd_SpawnString( "origin", "0 0 0", &s );
	self->target3 = Lmd_NewString(s);
	Lmd_SpawnString( "string", "", &s);
	self->target4 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_place_use;
}

/* ** target_delete **
This deletes all targeted entities.
It can be used in conjunction with target_place to erase spawned entities at any time. */
void target_delete_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	gentity_t* en = NULL;
	while ((en = Lmd_Find(en, FOFS(targetname), self->target)) != NULL)
	{
		Lmd_FreeEntity(en);
	}
	return;
}

void sp_target_delete(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "target", "", &s );
	self->target = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_delete_use;
}

/* ** target_mapents **
Saves or loads a given mapents file.
Default is saving, if "load" key is set to > 0 then the given file is loaded instead. */
void target_mapents_use(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (strchr(self->target2, '\n') || strchr(self->target2, '\r') || strchr(self->target2, ';'))
	{
		g_syscall(G_PRINT, "WARNING: target_mapents_use encountered a rcon injection argument. It was filtered out.\n");
		return;
	}
	if (atoi(self->target3) > 0)
		g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("loadit %s", self->target2));
	else
		g_syscall(G_SEND_CONSOLE_COMMAND, NULL, JASS_VARARGS("saveit %s", self->target2));
	return;
}

void sp_target_mapents(gentity_t* self)
{
	char* s = new char[1024];

	Lmd_SpawnString( "load", "", &s );
	self->target3 = Lmd_NewString(s);
	Lmd_SpawnString( "file", "", &s );
	self->target2 = Lmd_NewString(s);

	Lmd_SetOrigin( self, self->s.origin );
	self->use = target_mapents_use;
}

void InitEntities()
{
	registerSpawnableEntity("target_heal", sp_target_heal, LTrue, NULL);
	registerSpawnableEntity("target_modify", sp_target_modify, LTrue, NULL);
	registerSpawnableEntity("target_execute", sp_target_execute, LTrue, NULL);
	registerSpawnableEntity("target_oddball", sp_target_oddball, LTrue, NULL);
	registerSpawnableEntity("target_place", sp_target_place, LTrue, NULL);
	registerSpawnableEntity("target_delete", sp_target_delete, LTrue, NULL);
	registerSpawnableEntity("target_mapents", sp_target_mapents, LTrue, NULL);

	g_syscall(G_PRINT, "Entities initialized.\n");
}
