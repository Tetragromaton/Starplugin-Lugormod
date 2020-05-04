#pragma once
#include "jka/game/q_shared.h"
#include "jka/game/g_local.h"

int FastRegen(gentity_t* ent);
int TreatInjury(gentity_t* ent);
int SlowRegen(gentity_t* ent);
int ForceBeam(gentity_t* ent);
void BombThink(gentity_t* self);
void TimeBombThink(gentity_t* self);
int TimeBomb(gentity_t* ent);
int DisableJP(gentity_t* ent);
int PutYsalamiri(gentity_t* ent);

long SkillBitmask(Account_t* acc);
int HasSkill(Account_t* acc, int index, qboolean check = qfalse);
int SkillPts(Account_t* acc);
void AddSkill(Account_t* acc, int index);
void RemoveSkill(Account_t* acc, int index);

void InitSkills();