/*
#include "templates.h"
#include "actor.h"
#include "info.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_local.h"
#include "p_enemy.h"
#include "gstrings.h"
#include "a_action.h"
#include "thingdef/thingdef.h"
#include "g_level.h"
*/

static FRandom pr_tracer ("Tracer", true);
static FRandom pr_skelfist ("SkelFist", true);

//
// A_SkelMissile
//
DEFINE_ACTION_FUNCTION(AActor, A_SkelMissile)
{		
	AActor *missile;
		
	// [BC] This is handled server-side.
	if (( NETWORK_GetState( ) == NETSTATE_CLIENT ) ||
		( CLIENTDEMO_IsPlaying( )))
	{
		return;
	}

	if (!self->target)
		return;
				
	A_FaceTarget (self);
	missile = P_SpawnMissileZ (self, self->z + 48*FRACUNIT,
		self->target, PClass::FindClass("RevenantTracer"));

	if (missile != NULL)
	{
		missile->x += missile->momx;
		missile->y += missile->momy;
		missile->tracer = self->target;

		// [BC] If we're the server, tell clients to spawn the missile.
		if ( NETWORK_GetState( ) == NETSTATE_SERVER )
			SERVERCOMMANDS_SpawnMissile( missile );
	}
}

#define TRACEANGLE (0xc000000)

DEFINE_ACTION_FUNCTION(AActor, A_Tracer)
{
	angle_t exact;
	fixed_t dist;
	fixed_t slope;
	AActor *dest;
	AActor *smoke;
				
	// killough 1/18/98: this is why some missiles do not have smoke
	// and some do. Also, internal demos start at random gametics, thus
	// the bug in which revenants cause internal demos to go out of sync.
	//
	// killough 3/6/98: fix revenant internal demo bug by subtracting
	// levelstarttic from gametic:
	//
	// [RH] level.time is always 0-based, so nothing special to do here.

	if (level.time & 3)
		return;
	
	// spawn a puff of smoke behind the rocket
	// [BC] Don't tell clients to spawn this puff.
	P_SpawnPuff (self, PClass::FindClass(NAME_BulletPuff), self->x, self->y, self->z, 0, 3, false, false);
		
	smoke = Spawn ("RevenantTracerSmoke", self->x - self->momx,
		self->y - self->momy, self->z, ALLOW_REPLACE);
	
	smoke->momz = FRACUNIT;
	smoke->tics -= pr_tracer()&3;
	if (smoke->tics < 1)
		smoke->tics = 1;
	
	// [BC] Server takes care of movement.
	if (( NETWORK_GetState( ) == NETSTATE_CLIENT ) ||
		( CLIENTDEMO_IsPlaying( )))
	{
		return;
	}


	// adjust direction
	dest = self->tracer;
		
	if (!dest || dest->health <= 0 || self->Speed == 0 || !self->CanSeek(dest))
		return;
	
	// change angle 	
	exact = R_PointToAngle2 (self->x, self->y, dest->x,  dest->y);

	if (exact != self->angle)
	{
		if (exact - self->angle > 0x80000000)
		{
			self->angle -= TRACEANGLE;
			if (exact - self->angle < 0x80000000)
				self->angle = exact;
		}
		else
		{
			self->angle += TRACEANGLE;
			if (exact - self->angle > 0x80000000)
				self->angle = exact;
		}
	}
		
	exact = self->angle>>ANGLETOFINESHIFT;
	self->momx = FixedMul (self->Speed, finecosine[exact]);
	self->momy = FixedMul (self->Speed, finesine[exact]);
	
	// change slope
	dist = P_AproxDistance (dest->x - self->x,
							dest->y - self->y);
	
	dist = dist / self->Speed;

	if (dist < 1)
		dist = 1;

	if (dest->height >= 56*FRACUNIT)
	{
		slope = (dest->z+40*FRACUNIT - self->z) / dist;
	}
	else
	{
		slope = (dest->z + self->height*2/3 - self->z) / dist;
	}

	if (slope < self->momz)
		self->momz -= FRACUNIT/8;
	else
		self->momz += FRACUNIT/8;

	// [BC] Update the thing's position, angle and momentum.
	if ( NETWORK_GetState( ) == NETSTATE_SERVER )
		SERVERCOMMANDS_MoveThingExact( self, CM_X|CM_Y|CM_Z|CM_ANGLE|CM_MOMX|CM_MOMY|CM_MOMZ );
}


DEFINE_ACTION_FUNCTION(AActor, A_SkelWhoosh)
{
	// [BC] This is handled server-side.
	if (( NETWORK_GetState( ) == NETSTATE_CLIENT ) ||
		( CLIENTDEMO_IsPlaying( )))
	{
		return;
	}

	if (!self->target)
		return;
	A_FaceTarget (self);
	S_Sound (self, CHAN_WEAPON, "skeleton/swing", 1, ATTN_NORM);

	// [BC] If we're the server, tell clients to play the sound.
	if ( NETWORK_GetState( ) == NETSTATE_SERVER )
		SERVERCOMMANDS_SoundActor( self, CHAN_WEAPON, "skeleton/swing", 1, ATTN_NORM );
}

DEFINE_ACTION_FUNCTION(AActor, A_SkelFist)
{
	// [BC] This is handled server-side.
	if (( NETWORK_GetState( ) == NETSTATE_CLIENT ) ||
		( CLIENTDEMO_IsPlaying( )))
	{
		return;
	}

	if (!self->target)
		return;
				
	A_FaceTarget (self);
		
	if (self->CheckMeleeRange ())
	{
		int damage = ((pr_skelfist()%10)+1)*6;
		S_Sound (self, CHAN_WEAPON, "skeleton/melee", 1, ATTN_NORM);
		P_DamageMobj (self->target, self, self, damage, NAME_Melee);
		P_TraceBleed (damage, self->target, self);

		// [BC] If we're the server, tell clients to play the sound.
		if ( NETWORK_GetState( ) == NETSTATE_SERVER )
			SERVERCOMMANDS_SoundActor( self, CHAN_WEAPON, "skeleton/melee", 1, ATTN_NORM );
	}
}
