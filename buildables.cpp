#include "lmdfuncs.h"
#include "buildables.h"

buildObj_t buildObj[NUM_BUILDABLES];

gentity_t* SP_Beam(gentity_t* ent)
{
	    int l1 = 0;
		int l2 = 0;
		int l3 = 0;
		int i = 0;
		gentity_t* loop;
		trace_t tr;
		cliAimTrace(&tr, ent, 75);
		if (tr.fraction < 1.0)
		{
			l1 = tr.endpos[0];
			l2 = tr.endpos[1];
			l3 = tr.endpos[2];
			loop = TrySpawn(JASS_VARARGS("classname,fx_runner,fxfile,env/beam,origin,%d %d %d", l1, l2, l3));
			return loop;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target is too far away!\n\"");
			return NULL;
		}
}

gentity_t* SP_Fire(gentity_t* ent)
{
	    int l1 = 0;
		int l2 = 0;
		int l3 = 0;
		int i = 0;
		gentity_t* loop;
		trace_t tr;
		cliAimTrace(&tr, ent, 75);
		if (tr.fraction < 1.0)
		{
			l1 = tr.endpos[0];
			l2 = tr.endpos[1];
			l3 = tr.endpos[2];
			loop = TrySpawn(JASS_VARARGS("classname,fx_runner,fxfile,env/fire,origin,%d %d %d", l1, l2, l3));
			return loop;
		}
		else
		{
			g_syscall(G_SEND_SERVER_COMMAND, ent->s.number, "print \"Target is too far away!\n\"");
			return NULL;
		}
}

void InitBuildables()
{
	// initialize buildables
	buildObj[0].name = "Beam";
	buildObj[0].desc = "A force beam.";
	buildObj[0].reqCrystal = 5;
	buildObj[0].reqIron = 0;
	buildObj[0].reqOrg = 0;
	buildObj[0].spawn = SP_Beam;

	buildObj[1].name = "Fire";
	buildObj[1].desc = "A piece of fire.";
	buildObj[1].reqCrystal = 0;
	buildObj[1].reqIron = 0;
	buildObj[1].reqOrg = 5;
	buildObj[1].spawn = SP_Fire;

	g_syscall(G_PRINT, "Buildables initialized.\n");
}