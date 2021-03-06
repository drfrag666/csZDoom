// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//		Map Objects, MObj, definition and handling.
//
//-----------------------------------------------------------------------------


#ifndef __P_MOBJ_H__
#define __P_MOBJ_H__

// Basics.
#include "tables.h"

// We need the thinker_t stuff.
#include "dthinker.h"


// States are tied to finite states are
//	tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"

#include "doomdef.h"
#include "textures/textures.h"
#include "r_blend.h"
#include "s_sound.h"

struct subsector_t;
//
// NOTES: AActor
//
// Actors are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are almost always set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and sometimes z fields
// to do stereo positioning of any sound emitted by the actor.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when AActors are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The AActor->flags element has various bit flags
// used by the simulation.
//
// Every actor is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any actor that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable actor that has its origin contained.  
//
// A valid actor is an actor that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO* flags while a thing is valid.
//
// Any questions?
//

enum
{
// --- mobj.flags ---

	MF_SPECIAL			= 0x00000001,	// call P_SpecialThing when touched
	MF_SOLID			= 0x00000002,
	MF_SHOOTABLE		= 0x00000004,
	MF_NOSECTOR			= 0x00000008,	// don't use the sector links
										// (invisible but touchable)
	MF_NOBLOCKMAP		= 0x00000010,	// don't use the blocklinks
										// (inert but displayable)
	MF_AMBUSH			= 0x00000020,	// not activated by sound; deaf monster
	MF_JUSTHIT			= 0x00000040,	// try to attack right back
	MF_JUSTATTACKED		= 0x00000080,	// take at least one step before attacking
	MF_SPAWNCEILING		= 0x00000100,	// hang from ceiling instead of floor
	MF_NOGRAVITY		= 0x00000200,	// don't apply gravity every tic

// movement flags
	MF_DROPOFF			= 0x00000400,	// allow jumps from high places
	MF_PICKUP			= 0x00000800,	// for players to pick up items
	MF_NOCLIP			= 0x00001000,	// player cheat
	MF_INCHASE			= 0x00002000,	// [RH] used by A_Chase and A_Look to avoid recursion
	MF_FLOAT			= 0x00004000,	// allow moves to any height, no gravity
	MF_TELEPORT			= 0x00008000,	// don't cross lines or look at heights
	MF_MISSILE			= 0x00010000,	// don't hit same species, explode on block

	MF_DROPPED			= 0x00020000,	// dropped by a demon, not level spawned
	MF_SHADOW			= 0x00040000,	// actor is hard for monsters to see
	MF_NOBLOOD			= 0x00080000,	// don't bleed when shot (use puff)
	MF_CORPSE			= 0x00100000,	// don't stop moving halfway off a step
	MF_INFLOAT			= 0x00200000,	// floating to a height for a move, don't
										// auto float to target's height
	MF_INBOUNCE			= 0x00200000,	// used by Heretic bouncing missiles 

	MF_COUNTKILL		= 0x00400000,	// count towards intermission kill total
	MF_COUNTITEM		= 0x00800000,	// count towards intermission item total

	MF_SKULLFLY			= 0x01000000,	// skull in flight
	MF_NOTDMATCH		= 0x02000000,	// don't spawn in death match (key cards)

	MF_SPAWNSOUNDSOURCE	= 0x04000000,	// Plays missile's see sound at spawning object.
	MF_FRIENDLY			= 0x08000000,	// [RH] Friendly monsters for Strife (and MBF)
	MF_UNMORPHED		= 0x10000000,	// [RH] Actor is the unmorphed version of something else
	MF_NOLIFTDROP		= 0x20000000,	// [RH] Used with MF_NOGRAVITY to avoid dropping with lifts
	MF_STEALTH			= 0x40000000,	// [RH] Andy Baker's stealth monsters
	MF_ICECORPSE		= 0x80000000,	// a frozen corpse (for blasting) [RH] was 0x800000

// --- mobj.flags2 ---

	MF2_DONTREFLECT		= 0x00000001,	// this projectile cannot be reflected
	MF2_WINDTHRUST		= 0x00000002,	// gets pushed around by the wind specials
	MF2_DONTSEEKINVISIBLE=0x00000004,	// For seeker missiles: Don't home in on invisible/shadow targets
	MF2_BLASTED			= 0x00000008,	// actor will temporarily take damage from impact
	MF2_FLY				= 0x00000010,	// fly mode is active
	MF2_FLOORCLIP		= 0x00000020,	// if feet are allowed to be clipped
	MF2_SPAWNFLOAT		= 0x00000040,	// spawn random float z
	MF2_NOTELEPORT		= 0x00000080,	// does not teleport
	MF2_RIP				= 0x00000100,	// missile rips through solid targets
	MF2_PUSHABLE		= 0x00000200,	// can be pushed by other moving actors
	MF2_SLIDE			= 0x00000400,	// slides against walls
	MF2_ONMOBJ			= 0x00000800,	// actor is resting on top of another actor
	MF2_PASSMOBJ		= 0x00001000,	// Enable z block checking. If on,
										// this flag will allow the actor to
										// pass over/under other actors.
	MF2_CANNOTPUSH		= 0x00002000,	// cannot push other pushable mobjs
	MF2_THRUGHOST		= 0x00004000,	// missile will pass through ghosts [RH] was 8
	MF2_BOSS			= 0x00008000,	// mobj is a major boss

	MF2_DONTTRANSLATE	= 0x00010000,	// Don't apply palette translations
	MF2_NODMGTHRUST		= 0x00020000,	// does not thrust target when damaging
	MF2_TELESTOMP		= 0x00040000,	// mobj can stomp another
	MF2_FLOATBOB		= 0x00080000,	// use float bobbing z movement
	MF2_THRUACTORS		= 0x00100000,	// performs no actor<->actor collision checks
	MF2_IMPACT			= 0x00200000, 	// an MF_MISSILE mobj can activate SPAC_IMPACT
	MF2_PUSHWALL		= 0x00400000, 	// mobj can push walls
	MF2_MCROSS			= 0x00800000,	// can activate monster cross lines
	MF2_PCROSS			= 0x01000000,	// can activate projectile cross lines
	MF2_CANTLEAVEFLOORPIC=0x02000000,	// stay within a certain floor type
	MF2_NONSHOOTABLE	= 0x04000000,	// mobj is totally non-shootable, 
										// but still considered solid
	MF2_INVULNERABLE	= 0x08000000,	// mobj is invulnerable
	MF2_DORMANT			= 0x10000000,	// thing is dormant
	MF2_ARGSDEFINED		= 0x20000000,	// Internal flag used by DECORATE to signal that the 
										// args should not be taken from the mapthing definition
	MF2_SEEKERMISSILE	= 0x40000000,	// is a seeker (for reflection)
	MF2_REFLECTIVE		= 0x80000000,	// reflects missiles

	// The three types of bounciness are:
	// HERETIC - Missile will only bounce off the floor once and then enter
	//			 its death state. It does not bounce off walls at all.
	// HEXEN -	 Missile bounces off of walls and floors indefinitely.
	// DOOM -	 Like Hexen, but the bounce turns off if its vertical velocity
	//			 is too low.


// --- mobj.flags3 ---

	MF3_FLOORHUGGER		= 0x00000001,	// Missile stays on floor
	MF3_CEILINGHUGGER	= 0x00000002,	// Missile stays on ceiling
	MF3_NORADIUSDMG		= 0x00000004,	// Actor does not take radius damage
	MF3_GHOST			= 0x00000008,	// Actor is a ghost
	MF3_ALWAYSPUFF		= 0x00000010,	// Puff always appears, even when hit nothing
	MF3_SPECIALFLOORCLIP= 0x00000020,	// Actor uses floorclip for special effect (e.g. Wraith)
	MF3_DONTSPLASH		= 0x00000040,	// Thing doesn't make a splash
	MF3_NOSIGHTCHECK	= 0x00000080,	// Go after first acceptable target without checking sight
	MF3_DONTOVERLAP		= 0x00000100,	// Don't pass over/under other things with this bit set
	MF3_DONTMORPH		= 0x00000200,	// Immune to arti_egg
	MF3_DONTSQUASH		= 0x00000400,	// Death ball can't squash this actor
	MF3_EXPLOCOUNT		= 0x00000800,	// Don't explode until special2 counts to special1
	MF3_FULLVOLACTIVE	= 0x00001000,	// Active sound is played at full volume
	MF3_ISMONSTER		= 0x00002000,	// Actor is a monster
	MF3_SKYEXPLODE		= 0x00004000,	// Explode missile when hitting sky
	MF3_STAYMORPHED		= 0x00008000,	// Monster cannot unmorph
	MF3_DONTBLAST		= 0x00010000,	// Actor cannot be pushed by blasting
	MF3_CANBLAST		= 0x00020000,	// Actor is not a monster but can be blasted
	MF3_NOTARGET		= 0x00040000,	// This actor not targetted when it hurts something else
	MF3_DONTGIB			= 0x00080000,	// Don't gib this corpse
	MF3_NOBLOCKMONST	= 0x00100000,	// Can cross ML_BLOCKMONSTERS lines
	MF3_CRASHED			= 0x00200000,	// Actor entered its crash state
	MF3_FULLVOLDEATH	= 0x00400000,	// DeathSound is played full volume (for missiles)
	MF3_CANBOUNCEWATER	= 0x00800000,	// Missile can bounce on water
	MF3_NOWALLBOUNCESND = 0x01000000,	// Don't make noise when bouncing off a wall
	MF3_FOILINVUL		= 0x02000000,	// Actor can hurt MF2_INVULNERABLE things
	MF3_NOTELEOTHER		= 0x04000000,	// Monster is unaffected by teleport other artifact
	MF3_BLOODLESSIMPACT	= 0x08000000,	// Projectile does not leave blood
	MF3_NOEXPLODEFLOOR	= 0x10000000,	// Missile stops at floor instead of exploding
	MF3_WARNBOT			= 0x20000000,	// Missile warns bot
	MF3_PUFFONACTORS	= 0x40000000,	// Puff appears even when hit bleeding actors
	MF3_HUNTPLAYERS		= 0x80000000,	// Used with TIDtoHate, means to hate players too

// --- mobj.flags4 ---

	MF4_NOHATEPLAYERS	= 0x00000001,	// Ignore player attacks
	MF4_QUICKTORETALIATE= 0x00000002,	// Always switch targets when hurt
	MF4_NOICEDEATH		= 0x00000004,	// Actor never enters an ice death, not even the generic one
	MF4_BOSSDEATH		= 0x00000008,	// A_FreezeDeathChunks calls A_BossDeath
	MF4_RANDOMIZE		= 0x00000010,	// Missile has random initial tic count
	MF4_NOSKIN			= 0x00000020,	// Player cannot use skins
	MF4_FIXMAPTHINGPOS	= 0x00000040,	// Fix this actor's position when spawned as a map thing
	MF4_ACTLIKEBRIDGE	= 0x00000080,	// Pickups can "stand" on this actor
	MF4_STRIFEDAMAGE	= 0x00000100,	// Strife projectiles only do up to 4x damage, not 8x

	MF4_CANUSEWALLS		= 0x00000200,	// Can activate 'use' specials
	MF4_MISSILEMORE		= 0x00000400,	// increases the chance of a missile attack
	MF4_MISSILEEVENMORE	= 0x00000800,	// significantly increases the chance of a missile attack
	MF4_FORCERADIUSDMG	= 0x00001000,	// if put on an object it will override MF3_NORADIUSDMG
	MF4_DONTFALL		= 0x00002000,	// Doesn't have NOGRAVITY disabled when dying.
	MF4_SEESDAGGERS		= 0x00004000,	// This actor can see you striking with a dagger
	MF4_INCOMBAT		= 0x00008000,	// Don't alert others when attacked by a dagger
	MF4_LOOKALLAROUND	= 0x00010000,	// Monster has eyes in the back of its head
	MF4_STANDSTILL		= 0x00020000,	// Monster should not chase targets unless attacked?
	MF4_SPECTRAL		= 0x00040000,
	MF4_SCROLLMOVE		= 0x00080000,	// momentum has been applied by a scroller
	MF4_NOSPLASHALERT	= 0x00100000,	// Splashes don't alert this monster
	MF4_SYNCHRONIZED	= 0x00200000,	// For actors spawned at load-time only: Do not randomize tics
	MF4_NOTARGETSWITCH	= 0x00400000,	// monster never switches target until current one is dead
	MF4_VFRICTION		= 0x00800000,	// Internal flag used by A_PainAttack to push a monster down
	MF4_DONTHURTSPECIES	= 0x01000000,	// Don't hurt one's own kind with explosions (hitscans, too?)
	MF4_SHIELDREFLECT	= 0x02000000,
	MF4_DEFLECT			= 0x04000000,	// different projectile reflection styles
	MF4_ALLOWPARTICLES	= 0x08000000,	// this puff type can be replaced by particles
	MF4_NOEXTREMEDEATH	= 0x10000000,	// this projectile or weapon never gibs its victim
	MF4_EXTREMEDEATH	= 0x20000000,	// this projectile or weapon always gibs its victim
	MF4_FRIGHTENED		= 0x40000000,	// Monster runs away from player
	MF4_NOBOUNCESOUND	= 0x80000000,	// Strife's grenades don't make a bouncing sound. 
	
	MF5_FASTER			= 0x00000001,	// moves faster when DF_FAST_MONSTERS or nightmare is on.
	MF5_FASTMELEE		= 0x00000002,	// has a faster melee attack when DF_FAST_MONSTERS or nightmare is on.
	MF5_NODROPOFF		= 0x00000004,	// cannot drop off under any circumstances.
	MF5_BOUNCEONACTORS	= 0x00000008,	// bouncing missile doesn't explode when it hits an actor 
	MF5_EXPLODEONWATER	= 0x00000010,	// bouncing missile explodes when hitting a water surface
	MF5_AVOIDINGDROPOFF = 0x00000020,	// Used to move monsters away from dropoffs
	MF5_NODAMAGE		= 0x00000040,	// Actor can be shot and reacts to being shot but takes no damage
	MF5_CHASEGOAL		= 0x00000080,	// Walks to goal instead of target if a valid goal is set.
	MF5_BLOODSPLATTER	= 0x00000100,	// Blood splatter like in Raven's games.
	MF5_OLDRADIUSDMG	= 0x00000200,	// Use old radius damage code (for barrels and boss brain)
	MF5_DEHEXPLOSION	= 0x00000400,	// Use the DEHACKED explosion options when this projectile explodes
	MF5_PIERCEARMOR		= 0x00000800,	// Armor doesn't protect against damage from this actor
	MF5_NOBLOODDECALS	= 0x00001000,	// Actor bleeds but doesn't spawn blood decals
	MF5_USESPECIAL		= 0x00002000,	// Actor executes its special when being 'used'.
	MF5_NOPAIN			= 0x00004000,	// If set the pain state won't be entered
	MF5_ALWAYSFAST		= 0x00008000,	// always uses 'fast' attacking logic
	MF5_NEVERFAST		= 0x00010000,	// never uses 'fast' attacking logic
	MF5_ALWAYSRESPAWN	= 0x00020000,	// always respawns, regardless of skill setting
	MF5_NEVERRESPAWN	= 0x00040000,	// never respawns, regardless of skill setting
	MF5_DONTRIP			= 0x00080000,	// Ripping projectiles explode when hittin this actor
	MF5_NOINFIGHTING	= 0x00100000,	// This actor doesn't switch target when it's hurt 
	MF5_NOINTERACTION	= 0x00200000,	// Thing is completely excluded from any gameplay related checks
	MF5_NOTIMEFREEZE	= 0x00400000,	// Actor is not affected by time freezer
	MF5_PUFFGETSOWNER	= 0x00800000,	// [BB] Sets the owner of the puff to the player who fired it
	MF5_SPECIALFIREDAMAGE=0x01000000,	// Special treatment of PhoenixFX1 turned into a flag to removr
										// dependence of main engine code of specific actor types.
	MF5_SUMMONEDMONSTER	= 0x02000000,	// To mark the friendly Minotaur. Hopefully to be generalized later.
	MF5_NOVERTICALMELEERANGE=0x04000000,// Does not check vertical distance for melee range
	MF5_BRIGHT			= 0x08000000,	// Actor is always rendered fullbright
	MF5_CANTSEEK		= 0x10000000,	// seeker missiles cannot home in on this actor
	MF5_INCONVERSATION	= 0x20000000,	// Actor is having a conversation
	MF5_PAINLESS		= 0x40000000,	// Actor always inflicts painless damage.
	MF5_MOVEWITHSECTOR	= 0x80000000,	// P_ChangeSector() will still process this actor if it has MF_NOBLOCKMAP

	MF6_NOBOSSRIP		= 0x00000001,	// For rippermissiles: Don't rip through bosses.
	MF6_THRUSPECIES		= 0x00000002,	// Actors passes through other of the same species.
	MF6_MTHRUSPECIES	= 0x00000004,	// Missile passes through actors of its shooter's species.

	// [BC] More object flags for Skulltag.

	// Object can only be picked up by blue team members.
	STFL_BLUETEAM		= 0x00000001,

	// Object can only be picked up by red team members.
	STFL_REDTEAM		= 0x00000002,

	// Object can be pulled by players.
	//STFL_PULLABLE		= 0x00000004,

	// Execute this object's special when player hits the use key in front of it.
	STFL_USESPECIAL		= 0x00000008,

	// Object impales players that fall on it.
	//STFL_IMPALE			= 0x00000010,

	// Execute this object's special when a players bumps into it.
	STFL_BUMPSPECIAL	= 0x00000020,

	// *** THE FOLLOWING FLAGS ARE IDENTIFERS FOR BOTS ***
	// ... eh, there's probably a better way to do this.
	// Object is a health item (stimpack, medkit, etc.).
	STFL_BASEHEALTH		= 0x00000020,

	// Object is a health item that can heal beyond the normal amount (soulsphere, etc.).
	STFL_SUPERHEALTH	= 0x00000040,

	// Object is an armor item (green armor, etc.).
	STFL_BASEARMOR		= 0x00000080,

	// Object is an armor item that gives armor beyond the normal amount (blue armor, etc.).
	STFL_SUPERARMOR		= 0x00000100,

	// Object is some kind of enemy. Kill it!
	//STFL_ENEMY			= 0x00000200,

	// Object is a rune.
	//STFL_RUNE			= 0x00000400,

	// Object is a powerup.
	//STFL_POWERUP		= 0x00000800,

	// Object is ammo.
	//STFL_AMMO			= 0x00001000,

	// Object is a score pillar in Skulltag.
	STFL_SCOREPILLAR	= 0x00002000,

	// Object is a weapon.
	//STFL_WEAPON			= 0x00004000,

	// Object is a key.
	//STFL_KEY			= 0x00008000,

	// Object is a node.
	STFL_NODE			= 0x00010000,

	// *** END OF IDENTIFIERS ***
	// [BB] Object uses "weapons/grbnce" as bounce sound.
	STFL_USESTBOUNCESOUND		= 0x00020000,

	// Object explodes on death.
	STFL_EXPLODEONDEATH		= 0x00040000,

	// This object was spawned when the map loaded.
	STFL_LEVELSPAWNED		= 0x00080000,

	// Has the object moved from its original position on the map?
	STFL_POSITIONCHANGED	= 0x00100000,

	// [BB] The actor is an obsolete spectator body, that should be deleted once the player is reborn.
	STFL_OBSOLETE_SPECTATOR_BODY		= 0x00200000,

// More flags for Skulltag... these having to do with the network.

	// This object does not have a network ID.
	NETFL_NONETID			= 0x00000001,

	// If this object is placed on a map, allow clients to spawn it on their own without
	// the server having to tell them to spawn it.
	NETFL_ALLOWCLIENTSPAWN	= 0x00000002,

	// Tell clients what this thing's arguments are, because they are important.
	NETFL_UPDATEARGUMENTS	= 0x00000004,

	// Handle the pickup of this item in a "special" way.
	NETFL_SPECIALPICKUP		= 0x00000008,

	// [BB] This actor was only spawned by the clients or is supposed to be.
	// The server doesn't spawn it at all.
	// Only use it on actors that don't affect the game in any way (visuals aside).
	NETFL_CLIENTSIDEONLY		= 0x00000010,

	// [BB] This actor was only spawned by the server or is supposed to be.
	// The clients don't spawn it at all.
	// Only use it on actors that are always invisible and don't block the movement of players.
	NETFL_SERVERSIDEONLY		= 0x00000020,

	// [BB] This actor just bounced off another actor and needs special treatment.
	NETFL_BOUNCED_OFF_ACTOR		= 0x00000040,

	// [BB] The clients have already destroyed this actor and thus it needs special treatment
	// (used for respawning barrels).
	NETFL_DESTROYED_ON_CLIENT		= 0x00000080,

	// [BC] End of new ST flags.

// --- mobj.renderflags ---

	RF_XFLIP			= 0x0001,	// Flip sprite horizontally
	RF_YFLIP			= 0x0002,	// Flip sprite vertically
	RF_ONESIDED			= 0x0004,	// Wall/floor sprite is visible from front only
	RF_RANDOMPOWERUPHACK	= 0x0008,	// [BC] This actor uses the random powerup hack: offsets are centered.
	RF_FULLBRIGHT		= 0x0010,	// Sprite is drawn at full brightness

	RF_RELMASK			= 0x0300,	// ---Relative z-coord for bound actors (these obey texture pegging)
	RF_RELABSOLUTE		= 0x0000,	// Actor z is absolute
	RF_RELUPPER			= 0x0100,	// Actor z is relative to upper part of wall
	RF_RELLOWER			= 0x0200,	// Actor z is relative to lower part of wall
	RF_RELMID			= 0x0300,	// Actor z is relative to middle part of wall

	RF_CLIPMASK			= 0x0c00,	// ---Clipping for bound actors
	RF_CLIPFULL			= 0x0000,	// Clip sprite to full height of wall
	RF_CLIPUPPER		= 0x0400,	// Clip sprite to upper part of wall
	RF_CLIPMID			= 0x0800,	// Clip sprite to mid part of wall
	RF_CLIPLOWER		= 0x0c00,	// Clip sprite to lower part of wall

	RF_DECALMASK		= RF_RELMASK|RF_CLIPMASK,

	RF_SPRITETYPEMASK	= 0x7000,	// ---Different sprite types, not all implemented
	RF_FACESPRITE		= 0x0000,	// Face sprite
	RF_WALLSPRITE		= 0x1000,	// Wall sprite
	RF_FLOORSPRITE		= 0x2000,	// Floor sprite
	RF_VOXELSPRITE		= 0x3000,	// Voxel object
	RF_INVISIBLE		= 0x8000,	// Don't bother drawing this actor

	RF_FORCEYBILLBOARD		= 0x10000,	// [BB] OpenGL only: draw with y axis billboard, i.e. anchored to the floor (overrides gl_billboard_mode setting)
	RF_FORCEXYBILLBOARD		= 0x20000,	// [BB] OpenGL only: draw with xy axis billboard, i.e. unanchored (overrides gl_billboard_mode setting)

// --- dummies for unknown/unimplemented Strife flags ---

	MF_STRIFEx8000000 = 0,		// seems related to MF_SHADOW
};

#define TRANSLUC25			(FRACUNIT/4)
#define TRANSLUC33			(FRACUNIT/3)
#define TRANSLUC50			(FRACUNIT/2)
#define TRANSLUC66			((FRACUNIT*2)/3)
#define TRANSLUC75			((FRACUNIT*3)/4)

// <wingdi.h> also #defines OPAQUE
#ifndef OPAQUE
#define OPAQUE				(FRACUNIT)
#endif

// This translucency value produces the closest match to Heretic's TINTTAB.
// ~40% of the value of the overlaid image shows through.
#define HR_SHADOW			(0x6800)

// Hexen's TINTTAB is the same as Heretic's, just reversed.
#define HX_SHADOW			(0x9800)
#define HX_ALTSHADOW		(0x6800)

// This could easily be a bool but then it'd be much harder to find later. ;)
enum replace_t
{
	NO_REPLACE = 0,
	ALLOW_REPLACE = 1
};

enum EBounceType
{
	BOUNCE_None=0,
	BOUNCE_Doom=1,
	BOUNCE_Heretic=2,
	BOUNCE_Hexen=3,

	BOUNCE_TypeMask = 3,
	BOUNCE_UseSeeSound = 4,	// compatibility fallback. Thios will only be 
							// set by the compatibility handlers for the old bounce flags.

	// combined types
	BOUNCE_DoomCompat = BOUNCE_Doom | BOUNCE_UseSeeSound,
	BOUNCE_HereticCompat = BOUNCE_Heretic | BOUNCE_UseSeeSound,
	BOUNCE_HexenCompat = BOUNCE_Hexen | BOUNCE_UseSeeSound,
};

// [RH] Like msecnode_t, but for the blockmap
struct FBlockNode
{
	AActor *Me;						// actor this node references
	int BlockIndex;					// index into blocklinks for the block this node is in
	FBlockNode **PrevActor;			// previous actor in this block
	FBlockNode *NextActor;			// next actor in this block
	FBlockNode **PrevBlock;			// previous block this actor is in
	FBlockNode *NextBlock;			// next block this actor is in

	static FBlockNode *Create (AActor *who, int x, int y);
	void Release ();

	static FBlockNode *FreeBlocks;
};

class FDecalBase;
class AInventory;

inline AActor *GetDefaultByName (const char *name)
{
	return (AActor *)(PClass::FindClass(name)->Defaults);
}

inline AActor *GetDefaultByType (const PClass *type)
{
	return (AActor *)(type->Defaults);
}

template<class T>
inline T *GetDefault ()
{
	return (T *)(RUNTIME_CLASS(T)->Defaults);
}

struct line_t;
struct secplane_t;

// [BC] Prototype these classes here so they can be included in the actor structure.
class ABaseMonsterInvasionSpot;
class ABasePickupInvasionSpot;

struct FStrifeDialogueNode;

enum
{
	AMETA_BASE = 0x12000,

	AMETA_Obituary,			// string (player was killed by this actor)
	AMETA_HitObituary,		// string (player was killed by this actor in melee)
	AMETA_DeathHeight,		// fixed (height on normal death)
	AMETA_BurnHeight,		// fixed (height on burning death)
	AMETA_StrifeName,		// string (for named Strife objects)
	AMETA_BloodColor,		// colorized blood
	AMETA_GibHealth,		// negative health below which this monster dies an extreme death
	AMETA_WoundHealth,		// health needed to enter wound state
	AMETA_PoisonDamage,		// Amount of poison damage
	AMETA_FastSpeed,		// Speed in fast mode
	AMETA_RDFactor,			// Radius damage factor
	AMETA_CameraHeight,		// Height of camera when used as such
	AMETA_HowlSound,		// Sound being played when electrocuted or poisoned
	AMETA_BloodType,		// Blood replacement type
	AMETA_BloodType2,		// Bloodsplatter replacement type
	AMETA_BloodType3,		// AxeBlood replacement type
};

struct FDropItem 
{
	FName Name;
	int probability;
	int amount;
	FDropItem * Next;
};

class FDropItemPtrArray : public TArray<FDropItem *>
{
public:
	~FDropItemPtrArray();
};

extern FDropItemPtrArray DropItemList;

void FreeDropItemChain(FDropItem *chain);
int StoreDropItemChain(FDropItem *chain);



// Map Object definition.
class AActor : public DThinker
{
	DECLARE_CLASS (AActor, DThinker)
	HAS_OBJECT_POINTERS
public:
	AActor () throw();
	AActor (const AActor &other) throw();
	AActor &operator= (const AActor &other);
	void Destroy ();
	~AActor ();

	void Serialize (FArchive &arc);

	static AActor *StaticSpawn (const PClass *type, fixed_t x, fixed_t y, fixed_t z, replace_t allowreplacement, bool SpawningMapThing = false);

	inline AActor *GetDefault () const
	{
		return (AActor *)(RUNTIME_TYPE(this)->Defaults);
	}

	FDropItem *GetDropItems();

	// Return true if the monster should use a missile attack, false for melee
	bool SuggestMissileAttack (fixed_t dist);

	// Adjusts the angle for deflection/reflection of incoming missiles
	// Returns true if the missile should be allowed to explode anyway
	bool AdjustReflectionAngle (AActor *thing, angle_t &angle);

	// Returns true if this actor is within melee range of its target
	bool CheckMeleeRange ();

	// BeginPlay: Called just after the actor is created
	virtual void BeginPlay ();
	// LevelSpawned: Called after BeginPlay if this actor was spawned by the world
	virtual void LevelSpawned ();
	// Translates SpawnFlags into in-game flags.
	virtual void HandleSpawnFlags ();

	virtual void Activate (AActor *activator);
	virtual void Deactivate (AActor *activator);
	// [BC]
	virtual bool IsActive( void );

	virtual void Tick ();

	// Smallest yaw interval for a mapthing to be spawned with
	virtual angle_t AngleIncrements ();

	// Called when actor dies
	virtual void Die (AActor *source, AActor *inflictor);

	// Perform some special damage action. Returns the amount of damage to do.
	// Returning -1 signals the damage routine to exit immediately
	virtual int DoSpecialDamage (AActor *target, int damage);

	// Like DoSpecialDamage, but called on the actor receiving the damage.
	virtual int TakeSpecialDamage (AActor *inflictor, AActor *source, int damage, FName damagetype);

	// Centaurs and ettins squeal when electrocuted, poisoned, or "holy"-ed
	// Made a metadata property so no longer virtual
	void Howl ();

	// Actor just hit the floor
	virtual void HitFloor ();

	// plays bouncing sound
	void PlayBounceSound(bool onfloor);

	// Called when an actor with MF_MISSILE and MF2_FLOORBOUNCE hits the floor
	virtual bool FloorBounceMissile (secplane_t &plane);

	// Called when an actor is to be reflected by a disc of repulsion.
	// Returns true to continue normal blast processing.
	virtual bool SpecialBlastHandling (AActor *source, fixed_t strength);

	// Called by RoughBlockCheck
	virtual bool IsOkayToAttack (AActor *target);

	// Plays the actor's ActiveSound if its voice isn't already making noise.
	void PlayActiveSound ();

	// Actor had MF_SKULLFLY set and rammed into something
	// Returns false to stop moving and true to keep moving
	virtual bool Slam (AActor *victim);

	// Called by PIT_CheckThing() and needed for some Hexen things.
	// Returns -1 for normal behavior, 0 to return false, and 1 to return true.
	// I'm not sure I like it this way, but it will do for now.
	virtual int SpecialMissileHit (AActor *victim);

	// Returns true if it's okay to switch target to "other" after being attacked by it.
	virtual bool OkayToSwitchTarget (AActor *other);

	// Something just touched this actor.
	virtual void Touch (AActor *toucher);

	// Adds the item to this actor's inventory and sets its Owner.
	virtual void AddInventory (AInventory *item);

	// Removes the item from the inventory list.
	virtual void RemoveInventory (AInventory *item);

	// Uses an item and removes it from the inventory.
	virtual bool UseInventory (AInventory *item);

	// Tosses an item out of the inventory.
	virtual AInventory *DropInventory (AInventory *item);

	// Returns true if this view is considered "local" for the player.
	bool CheckLocalView (int playernum) const;

	// Finds the first item of a particular type.
	AInventory *FindInventory (const PClass *type);
	AInventory *FindInventory (FName type);
	template<class T> T *FindInventory ()
	{
		return static_cast<T *> (FindInventory (RUNTIME_CLASS(T)));
	}

	// Adds one item of a particular type. Returns NULL if it could not be added.
	AInventory *GiveInventoryType (const PClass *type);

	// [BB] Adds one item of a particular type or its replacement. Returns NULL if it could not be added.
	AInventory *GiveInventoryTypeRespectingReplacements (const PClass *type);

	// Returns the first item held with IF_INVBAR set.
	AInventory *FirstInv ();

	// Tries to give the actor some ammo.
	bool GiveAmmo (const PClass *type, int amount);

	// Destroys all the inventory the actor is holding.
	void DestroyAllInventory ();

	// Set the alphacolor field properly
	void SetShade (DWORD rgb);
	void SetShade (int r, int g, int b);

	// Plays a conversation animation
	void ConversationAnimation (int animnum);

	// Make this actor hate the same things as another actor
	void CopyFriendliness (AActor *other, bool changeTarget);

	// Moves the other actor's inventory to this one
	void ObtainInventory (AActor *other);

	// Die. Now.
	virtual bool Massacre ();

	// Is the other actor on my team?
	bool IsTeammate (AActor *other);

	// Is the other actor my friend?
	bool IsFriend (AActor *other);

	// Do I hate the other actor?
	bool IsHostile (AActor *other);

	// What species am I?
	virtual FName GetSpecies();
	
	// Enter the crash state
	void Crash();

	// Check for monsters that count as kill but excludes all friendlies.
	bool CountsAsKill() const
	{
		return (flags & MF_COUNTKILL) && !(flags & MF_FRIENDLY);
	}

	bool intersects(AActor *other) const
	{
		fixed_t blockdist = radius + other->radius;
		return ( abs(x - other->x) < blockdist && abs(y - other->y) < blockdist);
	}

	// Calculate amount of missile damage
	virtual int GetMissileDamage(int mask, int add);

	bool CanSeek(AActor *target) const;

// info for drawing
// NOTE: The first member variable *must* be x.
	fixed_t	 		x,y,z;
	AActor			*snext, **sprev;	// links in sector (if needed)
	angle_t			angle;
	WORD			sprite;				// used to find patch_t and flip value
	BYTE			frame;				// sprite frame to draw
	fixed_t			scaleX, scaleY;		// Scaling values; FRACUNIT is normal size
	FRenderStyle	RenderStyle;		// Style to draw this actor with
	DWORD			renderflags;		// Different rendering flags
	FTextureID		picnum;				// Draw this instead of sprite if valid
	DWORD			effects;			// [RH] see p_effect.h
	fixed_t			alpha;
	DWORD			fillcolor;			// Color to draw when STYLE_Shaded

// interaction info
	fixed_t			pitch, roll;
	FBlockNode		*BlockNode;			// links in blocks (if needed)
	struct sector_t	*Sector;
	fixed_t			floorz, ceilingz;	// closest together of contacted secs
	fixed_t			dropoffz;		// killough 11/98: the lowest floor over all contacted Sectors.

	struct sector_t	*floorsector;
	FTextureID		floorpic;			// contacted sec floorpic
	struct sector_t	*ceilingsector;
	FTextureID		ceilingpic;			// contacted sec ceilingpic
	fixed_t			radius, height;		// for movement checking
	fixed_t			projectilepassheight;	// height for clipping projectile movement against this actor
	fixed_t			momx, momy, momz;	// momentums
	SDWORD			tics;				// state tic counter
	FState			*state;
	SDWORD			Damage;			// For missiles and monster railgun
	DWORD			flags;
	DWORD			flags2;			// Heretic flags
	DWORD			flags3;			// [RH] Hexen/Heretic actor-dependant behavior made flaggable
	DWORD			flags4;			// [RH] Even more flags!
	DWORD			flags5;			// OMG! We need another one.
	DWORD			flags6;			// Shit! Where did all the flags go?

	// [BB] If 0, not limited to any team, if > 0, limited to the team with number (ulLimitedToTeam-1).
	// [EP] TODO: remove the 'ul' prefix from this variable, it isn't ULONG anymore
	unsigned int	ulLimitedToTeam;

	// [BB] If 0, everybody can see the actor, if > 0, only members of team (ulVisibleToTeam-1) can see it.
	// [EP] TODO: remove the 'ul' prefix from this variable, it isn't ULONG anymore
	unsigned int	ulVisibleToTeam;

	// [BB] If NAME_None, all players can see the actor, else only players whose playerclass name is VisibleToPlayerClass can see it.
	FNameNoInit		VisibleToPlayerClass;

	// [BC] A new set of flags that ST uses.
	// [EP] TODO: remove the 'ul' prefix from this variable, it isn't ULONG anymore
	unsigned int	ulSTFlags;

	// [BC] A new set of flags that deal with network games.
	// [EP] TODO: remove the 'ul' prefix from this variable, it isn't ULONG anymore
	unsigned int	ulNetworkFlags;

	int				special1;		// Special info
	int				special2;		// Special info
	int 			health;
	BYTE			movedir;		// 0-7
	SBYTE			visdir;
	SWORD			movecount;		// when 0, select a new dir
	TObjPtr<AActor> target;			// thing being chased/attacked (or NULL)
									// also the originator for missiles
	TObjPtr<AActor>	lastenemy;		// Last known enemy -- killogh 2/15/98
	TObjPtr<AActor> LastHeard;		// [RH] Last actor this one heard
	SDWORD			reactiontime;	// if non 0, don't attack yet; used by
									// player to freeze a bit after teleporting
	SDWORD			threshold;		// if > 0, the target will be chased
									// no matter what (even if shot)
	player_t		*player;		// only valid if type of APlayerPawn
	TObjPtr<AActor>	LastLookActor;	// Actor last looked for (if TIDtoHate != 0)
	fixed_t			SpawnPoint[3]; 	// For nightmare respawn
	WORD			SpawnAngle;
	int				skillrespawncount;
	int				TIDtoHate;			// TID of things to hate (0 if none)
	FNameNoInit		Species;
	TObjPtr<AActor>	tracer;			// Thing being chased/attacked for tracers
	TObjPtr<AActor>	master;			// Thing which spawned this one (prevents mutual attacks)
	fixed_t			floorclip;		// value to use for floor clipping
	int				tid;			// thing identifier
	int				special;		// special
	BYTE			SavedSpecial;	// [BC] Saved actor special for when a map gets reset.
	int				args[5];		// special arguments
	int				SavedArgs[5];	// [Dusk] More map reset stuff
	int				SavedTID;		// [Dusk]

	AActor			*inext, **iprev;// Links to other mobjs in same bucket
	TObjPtr<AActor> goal;			// Monster's goal if not chasing anything
	int				waterlevel;		// 0=none, 1=feet, 2=waist, 3=eyes
	BYTE			boomwaterlevel;	// splash information for non-swimmable water sectors
	BYTE			MinMissileChance;// [RH] If a random # is > than this, then missile attack.
	SBYTE			LastLookPlayerNumber;// Player number last looked for (if TIDtoHate == 0)
	WORD			SpawnFlags;
	fixed_t			meleerange;		// specifies how far a melee attack reaches.
	fixed_t			meleethreshold;	// Distance below which a monster doesn't try to shoot missiles anynore
									// but instead tries to come closer for a melee attack.
									// This is not the same as meleerange
	fixed_t			maxtargetrange;	// any target farther away cannot be attacked
	int				bouncetype;		// which bouncing type?
	fixed_t			bouncefactor;	// Strife's grenades use 50%, Hexen's Flechettes 70.
	fixed_t			wallbouncefactor;	// The bounce factor for walls can be different.
	int				bouncecount;	// Strife's grenades only bounce twice before exploding
	fixed_t			gravity;		// [GRB] Gravity factor
	int 			FastChaseStrafeCount;
	int				DesignatedTeam;	// Allow for friendly fire cacluations to be done on non-players.

	AActor			*BlockingMobj;	// Actor that blocked the last move
	line_t			*BlockingLine;	// Line that blocked the last move

	// [KS] These temporary-use properties are needed to allow A_LookEx to pass its parameters to
	// LookFor*InBlock in P_BlockmapSearch so that friendly enemies and monsters that look for
	// other monsters can find their targets properly. If there's a cleaner way of doing this,
	// feel free to remove these and use that method instead.
	fixed_t			LookExMinDist;	// Minimum sight distance
	fixed_t			LookExMaxDist;	// Maximum sight distance
	angle_t			LookExFOV;		// Field of Vision

	// a linked list of sectors where this object appears
	struct msecnode_t	*touching_sectorlist;				// phares 3/14/98

	TObjPtr<AInventory>	Inventory;		// [RH] This actor's inventory
	DWORD			InventoryID;	// A unique ID to keep track of inventory items

	//Added by MC:
	SDWORD id;						// Player ID (for items, # in list.)

	BYTE smokecounter;
	BYTE FloatBobPhase;
	BYTE FriendPlayer;				// [RH] Player # + 1 this friendly monster works for (so 0 is no player, 1 is player 0, etc)
	DWORD Translation;

	// [RH] Stuff that used to be part of an Actor Info
	FSoundIDNoInit SeeSound;
	FSoundIDNoInit AttackSound;
	FSoundIDNoInit PainSound;
	FSoundIDNoInit DeathSound;
	FSoundIDNoInit ActiveSound;
	FSoundIDNoInit UseSound;		// [RH] Sound to play when an actor is used.
	FSoundIDNoInit BounceSound;
	FSoundIDNoInit WallBounceSound;

	fixed_t Speed;
	fixed_t FloatSpeed;
	fixed_t MaxDropOffHeight, MaxStepHeight;
	SDWORD Mass;
	SWORD PainChance;
	FNameNoInit DamageType;
	fixed_t DamageFactor;

	FState *SpawnState;
	FState *SeeState;
	FState *MeleeState;
	FState *MissileState;

	// [BC] This is the state that the actor is put into after being spawned on a map (this
	// isn't necessarily the spawn state).
	FState	*InitialState;

	// [RH] The dialogue to show when this actor is "used."
	FStrifeDialogueNode *Conversation;

	// [RH] Decal(s) this weapon/projectile generates on impact.
	FDecalBase *DecalGenerator;

	// [BC] Bunch of new stuff for ST.
	// Should this actor be drawn with a different colormap?
	// [EP] TODO: remove the 'l' prefix from this variable, it isn't LONG anymore
	int			lFixedColormap;

	// ID used to identify this actor over network games.
	// [EP] TODO: remove the 'l' prefix from this variable, it isn't LONG anymore
	int			lNetID;

	// Pointer to the pickup spot this item was spawned from.
	ABaseMonsterInvasionSpot		*pMonsterSpot;
	ABasePickupInvasionSpot			*pPickupSpot;

	// What wave does this monster belong to in invasion mode?
	// [EP] TODO: remove the 'ul' prefix from this variable, it isn't ULONG anymore
	unsigned int ulInvasionWave;

	// [BC] End of ST stuff.

	// [RH] Used to interpolate the view to get >35 FPS
	fixed_t PrevX, PrevY, PrevZ;

	// [BB] Last tic in which the server sent a xyz-position / movedir update about this actor to the clients.
	int	lastNetXUpdateTic, lastNetYUpdateTic, lastNetZUpdateTic, lastNetMomXUpdateTic, lastNetMomYUpdateTic, lastNetMomZUpdateTic, lastNetMovedirUpdateTic;

	// [BB] Last xyz-position that was sent to the client.
	fixed_t lastX, lastY, lastZ;

	// [BB] Last xyz-momentum that was sent to the client.
	fixed_t lastMomx, lastMomy, lastMomz;

	// [BB] Last movedir that was sent to the client.
	BYTE lastMovedir;

	// ThingIDs
	static void ClearTIDHashes ();
	void AddToHash ();
	void RemoveFromHash ();

private:
	static AActor *TIDHash[128];
	static inline int TIDHASH (int key) { return key & 127; }

	friend class FActorIterator;

	sector_t *LinkToWorldForMapThing ();

public:
	void LinkToWorld (bool buggy=false);
	void LinkToWorld (sector_t *sector);
	void UnlinkFromWorld ();
	void AdjustFloorClip ();
	void SetOrigin (fixed_t x, fixed_t y, fixed_t z);
	bool InStateSequence(FState * newstate, FState * basestate);
	int GetTics(FState * newstate);
	bool SetState (FState *newstate);
	bool SetStateNF (FState *newstate);
	// [BB] Free the network ID of the actor.
	void FreeNetID ();
	// [BB] Completely hides the actor if it's still needed for a map reset, otherwise destroys the actor.
	void HideOrDestroyIfSafe ();
	// [BB] Called before being hidden by HideOrDestroyIfSafe().
	virtual void PrepareForHiding ();
	// [BC]
	bool InSpawnState( );
	bool InDeathState();
	// [BB]
	bool InState(FState *pState, unsigned int *pOffset = NULL, FState *pCurrentActorStateOverride = NULL ) const;
	bool InState(FName label) const;

	virtual bool UpdateWaterLevel (fixed_t oldz, bool splash=true);
	bool isFast();
	void SetIdle();

	FState *FindState (FName label) const
	{
		return GetClass()->ActorInfo->FindState(1, &label);
	}

	FState *FindState (FName label, FName sublabel, bool exact = false) const
	{
		FName names[] = { label, sublabel };
		return GetClass()->ActorInfo->FindState(2, names, exact);
	}


	bool HasSpecialDeathStates () const;

	// [GZDoom]
	TArray<TObjPtr<AActor> >		dynamiclights;
	void *				lightassociations;
	bool				hasmodel;
	subsector_t *		subsector;

	size_t PropagateMark();
};

class FActorIterator
{
public:
	FActorIterator (int i) : base (NULL), id (i)
	{
	}
	FActorIterator (int i, AActor *start) : base (start), id (i)
	{
	}
	AActor *Next ()
	{
		if (id == 0)
			return NULL;
		if (!base)
			base = AActor::TIDHash[id & 127];
		else
			base = base->inext;

		while (base && base->tid != id)
			base = base->inext;

		return base;
	}
private:
	AActor *base;
	int id;
};

template<class T>
class TActorIterator : public FActorIterator
{
public:
	TActorIterator (int id) : FActorIterator (id) {}
	T *Next ()
	{
		AActor *actor;
		do
		{
			actor = FActorIterator::Next ();
		} while (actor && !actor->IsKindOf (RUNTIME_CLASS(T)));
		return static_cast<T *>(actor);
	}
};

class NActorIterator : public FActorIterator
{
	const PClass *type;
public:
	NActorIterator (const PClass *cls, int id) : FActorIterator (id) { type = cls; }
	NActorIterator (FName cls, int id) : FActorIterator (id) { type = PClass::FindClass(cls); }
	NActorIterator (const char *cls, int id) : FActorIterator (id) { type = PClass::FindClass(cls); }
	AActor *Next ()
	{
		AActor *actor;
		if (type == NULL) return NULL;
		do
		{
			actor = FActorIterator::Next ();
		} while (actor && !actor->IsKindOf (type));
		return actor;
	}
};

inline AActor *Spawn (const PClass *type, fixed_t x, fixed_t y, fixed_t z, replace_t allowreplacement)
{
	return AActor::StaticSpawn (type, x, y, z, allowreplacement);
}

AActor *Spawn (const char *type, fixed_t x, fixed_t y, fixed_t z, replace_t allowreplacement);

template<class T>
inline T *Spawn (fixed_t x, fixed_t y, fixed_t z, replace_t allowreplacement)
{
	return static_cast<T *>(AActor::StaticSpawn (RUNTIME_CLASS(T), x, y, z, allowreplacement));
}

#define S_FREETARGMOBJ	1


// [BC] Network identification stuff for multiplayer.
void	ACTOR_ClearNetIDList( );
// [BB] Rebuild the global list of used / free NetIDs from scratch.
void	ACTOR_RebuildNetIDList( void );

#define	MAX_NETID				32768

// List of all possible network ID's for an actor. Slot is true if it available for use.
typedef struct
{
	// Is this node occupied, or free to be used by a new actor?
	bool	bFree;

	// If this node is occupied, this is the actor occupying it.
	AActor	*pActor;

} NETIDNODE_t;

extern	NETIDNODE_t		g_NetIDList[MAX_NETID];
#endif // __P_MOBJ_H__
