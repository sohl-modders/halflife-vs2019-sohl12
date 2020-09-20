/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "soundent.h"
#include "squadmonster.h"
#include "npcs/CHGrunt.h"
#include "npcs/CBarney.h"

#include "Tasks.h"

//=========================================================
// CHGrunt & CMaleAssassin
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t tlGruntFail[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_WAIT, static_cast<float>(2)},
	{TASK_WAIT_PVS, static_cast<float>(0)},
};

Schedule_t slGruntFail[] =
{
	{
		tlGruntFail,
		ARRAYSIZE(tlGruntFail),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"Grunt Fail"
	},
};

//=========================================================
// Grunt Combat Fail
//=========================================================
Task_t tlGruntCombatFail[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_WAIT_FACE_ENEMY, static_cast<float>(2)},
	{TASK_WAIT_PVS, static_cast<float>(0)},
};

Schedule_t slGruntCombatFail[] =
{
	{
		tlGruntCombatFail,
		ARRAYSIZE(tlGruntCombatFail),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Grunt Combat Fail"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t tlGruntVictoryDance[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_WAIT, static_cast<float>(1.5)},
	{TASK_GET_PATH_TO_ENEMY_CORPSE, static_cast<float>(0)},
	{TASK_WALK_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_VICTORY_DANCE)},
};

Schedule_t slGruntVictoryDance[] =
{
	{
		tlGruntVictoryDance,
		ARRAYSIZE(tlGruntVictoryDance),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"GruntVictoryDance"
	},
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlGruntEstablishLineOfFire[] =
{
	{TASK_SET_FAIL_SCHEDULE, static_cast<float>(SCHED_GRUNT_ELOF_FAIL)},
	{TASK_GET_PATH_TO_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_SPEAK_SENTENCE, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
};

Schedule_t slGruntEstablishLineOfFire[] =
{
	{
		tlGruntEstablishLineOfFire,
		ARRAYSIZE(tlGruntEstablishLineOfFire),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntEstablishLineOfFire"
	},
};

//=========================================================
// GruntFoundEnemy - grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t tlGruntFoundEnemy[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, static_cast<float>(ACT_SIGNAL1)},
};

Schedule_t slGruntFoundEnemy[] =
{
	{
		tlGruntFoundEnemy,
		ARRAYSIZE(tlGruntFoundEnemy),
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntFoundEnemy"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t tlGruntCombatFace1[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_WAIT, static_cast<float>(1.5)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_GRUNT_SWEEP)},
};

Schedule_t slGruntCombatFace[] =
{
	{
		tlGruntCombatFace1,
		ARRAYSIZE(tlGruntCombatFace1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"Combat Face"
	},
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
Task_t tlGruntSignalSuppress[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_IDEAL, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, static_cast<float>(ACT_SIGNAL2)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
};

Schedule_t slGruntSignalSuppress[] =
{
	{
		tlGruntSignalSuppress,
		ARRAYSIZE(tlGruntSignalSuppress),
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_GRUNT_NOFIRE |
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SignalSuppress"
	},
};

Task_t tlGruntSuppress[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
};

Schedule_t slGruntSuppress[] =
{
	{
		tlGruntSuppress,
		ARRAYSIZE(tlGruntSuppress),
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_GRUNT_NOFIRE |
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"Suppress"
	},
};

//=========================================================
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t tlGruntWaitInCover[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_WAIT_FACE_ENEMY, static_cast<float>(1)},
};

Schedule_t slGruntWaitInCover[] =
{
	{
		tlGruntWaitInCover,
		ARRAYSIZE(tlGruntWaitInCover),
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"GruntWaitInCover"
	},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t tlGruntTakeCover1[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_SET_FAIL_SCHEDULE, static_cast<float>(SCHED_GRUNT_TAKECOVER_FAILED)},
	{TASK_WAIT, static_cast<float>(0.2)},
	{TASK_FIND_COVER_FROM_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_SPEAK_SENTENCE, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_REMEMBER, static_cast<float>(bits_MEMORY_INCOVER)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_GRUNT_WAIT_FACE_ENEMY)},
};

Schedule_t slGruntTakeCover[] =
{
	{
		tlGruntTakeCover1,
		ARRAYSIZE(tlGruntTakeCover1),
		0,
		0,
		"TakeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlGruntGrenadeCover1[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_FIND_COVER_FROM_ENEMY, static_cast<float>(99)},
	{TASK_FIND_FAR_NODE_COVER_FROM_ENEMY, static_cast<float>(384)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_SPECIAL_ATTACK1)},
	{TASK_CLEAR_MOVE_WAIT, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_GRUNT_WAIT_FACE_ENEMY)},
};

Schedule_t slGruntGrenadeCover[] =
{
	{
		tlGruntGrenadeCover1,
		ARRAYSIZE(tlGruntGrenadeCover1),
		0,
		0,
		"GrenadeCover"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlGruntTossGrenadeCover1[] =
{
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_RANGE_ATTACK2, static_cast<float>(0)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_TAKE_COVER_FROM_ENEMY)},
};

Schedule_t slGruntTossGrenadeCover[] =
{
	{
		tlGruntTossGrenadeCover1,
		ARRAYSIZE(tlGruntTossGrenadeCover1),
		0,
		0,
		"TossGrenadeCover"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t tlGruntTakeCoverFromBestSound[] =
{
	{TASK_SET_FAIL_SCHEDULE, static_cast<float>(SCHED_COWER)}, // duck and cover if cannot move from explosion
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_FIND_COVER_FROM_BEST_SOUND, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_REMEMBER, static_cast<float>(bits_MEMORY_INCOVER)},
	{TASK_TURN_LEFT, static_cast<float>(179)},
};

Schedule_t slGruntTakeCoverFromBestSound[] =
{
	{
		tlGruntTakeCoverFromBestSound,
		ARRAYSIZE(tlGruntTakeCoverFromBestSound),
		0,
		0,
		"GruntTakeCoverFromBestSound"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t tlGruntHideReload[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_SET_FAIL_SCHEDULE, static_cast<float>(SCHED_RELOAD)},
	{TASK_FIND_COVER_FROM_ENEMY, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_REMEMBER, static_cast<float>(bits_MEMORY_INCOVER)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_RELOAD)},
};

Schedule_t slGruntHideReload[] =
{
	{
		tlGruntHideReload,
		ARRAYSIZE(tlGruntHideReload),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GruntHideReload"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t tlGruntSweep[] =
{
	{TASK_TURN_LEFT, static_cast<float>(179)},
	{TASK_WAIT, static_cast<float>(1)},
	{TASK_TURN_LEFT, static_cast<float>(179)},
	{TASK_WAIT, static_cast<float>(1)},
};

Schedule_t slGruntSweep[] =
{
	{
		tlGruntSweep,
		ARRAYSIZE(tlGruntSweep),

		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD | // sound flags
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER,

		"Grunt Sweep"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack1A[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, static_cast<float>(ACT_CROUCH)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
};

Schedule_t slGruntRangeAttack1A[] =
{
	{
		tlGruntRangeAttack1A,
		ARRAYSIZE(tlGruntRangeAttack1A),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_HEAR_SOUND |
		bits_COND_GRUNT_NOFIRE |
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"Range Attack1A"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack1B[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, static_cast<float>(ACT_IDLE_ANGRY)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_GRUNT_CHECK_FIRE, static_cast<float>(0)},
	{TASK_RANGE_ATTACK1, static_cast<float>(0)},
};

Schedule_t slGruntRangeAttack1B[] =
{
	{
		tlGruntRangeAttack1B,
		ARRAYSIZE(tlGruntRangeAttack1B),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED |
		bits_COND_GRUNT_NOFIRE |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Range Attack1B"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlGruntRangeAttack2[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_GRUNT_FACE_TOSS_DIR, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_RANGE_ATTACK2)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_GRUNT_WAIT_FACE_ENEMY)},
	// don't run immediately after throwing grenade.
};

Schedule_t slGruntRangeAttack2[] =
{
	{
		tlGruntRangeAttack2,
		ARRAYSIZE(tlGruntRangeAttack2),
		0,
		0,
		"RangeAttack2"
	},
};

//=========================================================
// repel 
//=========================================================
Task_t tlGruntRepel[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_FACE_IDEAL, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_GLIDE)},
};

Schedule_t slGruntRepel[] =
{
	{
		tlGruntRepel,
		ARRAYSIZE(tlGruntRepel),
		bits_COND_SEE_ENEMY |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER,
		"Repel"
	},
};

//=========================================================
// repel 
//=========================================================
Task_t tlGruntRepelAttack[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_FACE_ENEMY, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_FLY)},
};

Schedule_t slGruntRepelAttack[] =
{
	{
		tlGruntRepelAttack,
		ARRAYSIZE(tlGruntRepelAttack),
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t tlGruntRepelLand[] =
{
	{TASK_STOP_MOVING, static_cast<float>(0)},
	{TASK_PLAY_SEQUENCE, static_cast<float>(ACT_LAND)},
	{TASK_GET_PATH_TO_LASTPOSITION, static_cast<float>(0)},
	{TASK_RUN_PATH, static_cast<float>(0)},
	{TASK_WAIT_FOR_MOVEMENT, static_cast<float>(0)},
	{TASK_CLEAR_LASTPOSITION, static_cast<float>(0)},
};

Schedule_t slGruntRepelLand[] =
{
	{
		tlGruntRepelLand,
		ARRAYSIZE(tlGruntRepelLand),
		bits_COND_SEE_ENEMY |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER,
		"Repel Land"
	},
};

//=========================================================
// CBarney & COtis
//=========================================================

//=========================================================
// Barney Follow
//=========================================================
Task_t tlBaFollow[] =
{
	{TASK_MOVE_TO_TARGET_RANGE, static_cast<float>(128)}, // Move within 128 of target ent (client)
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_TARGET_FACE)},
};

Schedule_t slBaFollow[] =
{
	{
		tlBaFollow,
		ARRAYSIZE(tlBaFollow),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Follow"
	},
};

//=========================================================
// BarneyDraw- much better looking draw schedule for when
// barney knows who he's gonna attack.
//=========================================================
Task_t tlBarneyEnemyDraw[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, 0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, static_cast<float>(ACT_ARM)},
};

Schedule_t slBarneyEnemyDraw[] =
{
	{
		tlBarneyEnemyDraw,
		ARRAYSIZE(tlBarneyEnemyDraw),
		0,
		0,
		"Barney Enemy Draw"
	}
};

Task_t tlBaFaceTarget[] =
{
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_FACE_TARGET, static_cast<float>(0)},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_SET_SCHEDULE, static_cast<float>(SCHED_TARGET_CHASE)},
};

Schedule_t slBaFaceTarget[] =
{
	{
		tlBaFaceTarget,
		ARRAYSIZE(tlBaFaceTarget),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FaceTarget"
	},
};

Task_t tlIdleBaStand[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, static_cast<float>(ACT_IDLE)},
	{TASK_WAIT, static_cast<float>(2)}, // repick IDLESTAND every two seconds.
	{TASK_TLK_HEADRESET, static_cast<float>(0)}, // reset head position
};

Schedule_t slIdleBaStand[] =
{
	{
		tlIdleBaStand,
		ARRAYSIZE(tlIdleBaStand),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT | // sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|

		bits_SOUND_DANGER |
		bits_SOUND_MEAT | // scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};