#pragma once
#include "jka/game/g_local.h"
#include "jka/game/q_shared.h"
#include <Windows.h>

#define PRINTHOOK 1

//This structure is needed for replacing 
#pragma pack (push,1) 

struct jmp_far{ 
BYTE instr_push;    //assembler push instruction 
DWORD arg;  //argument - pointer to your function 
BYTE instr_ret;  //assembler ret instruction 
}; 
#pragma pack (pop) 

extern BYTE oldTargetLCUse[6]; //here the original function bytes will be stored 
extern BYTE oldLmdDropUse[6];
extern BYTE oldDelayUse[6];
extern BYTE oldFPRegen[6];
extern BYTE oldWP[6];
extern BYTE oldTP[6];
extern BYTE oldPowerupSP[6];
extern jmp_far jump;
extern DWORD flOldProtect;
extern bool shuttingDown;
extern bool gameStarted;

void S_ForcePowerRegenerate( gentity_t *self, int overrideAmt );
int Lmd_WPoints(gentity_t* ent);
void PCheckMode(gentity_t* self, gentity_t* activator);
void TargetLCUse(gentity_t* self, gentity_t* other, gentity_t* activator);
void Use_Target_Print(gentity_t *self, int clientNum);
void SP_target_level_change( gentity_t *self );
void TargetDelayThink(gentity_t* self);
void __cdecl TargetDelayUse(gentity_t* self, gentity_t* other, gentity_t* activator);
void target_powerup_use(gentity_t* self, gentity_t* other, gentity_t* activator);
void SP_target_powerup(gentity_t* self);
void InitHooks();
void RemoveHooks();