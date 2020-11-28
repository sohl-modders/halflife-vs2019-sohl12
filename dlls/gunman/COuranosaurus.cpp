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
#include "soundent.h"
#include "COuranosaurus.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_ourano, COuranosaurus);

//=========================================================
// Classify
//=========================================================
int COuranosaurus::Classify()
{
	return m_iClass ? m_iClass : CLASS_INSECT;
}

//=========================================================
// SetYawSpeed
//=========================================================
void COuranosaurus::SetYawSpeed()
{
	switch (m_Activity)
	{
	case ACT_IDLE:
	case ACT_RUN:
		pev->yaw_speed = 150;
		break;
	case ACT_EAT:
	case ACT_WALK:
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
	case ACT_THREAT_DISPLAY:
	case ACT_MELEE_ATTACK1:
	default:
		pev->yaw_speed = 120;
		break;
	}
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void COuranosaurus::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case 0:
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// ISoundMask - generic monster can't hear.
//=========================================================
int COuranosaurus::ISoundMask()
{
	return bits_SOUND_CARCASS | bits_SOUND_MEAT;
}

//=========================================================
// Spawn
//=========================================================
void COuranosaurus::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/Ourano.mdl");

	UTIL_SetSize(pev, Vector(-40, -40, -5), Vector(40, 40, 60));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;

	pev->effects = 0;
	pev->health = 50;

	m_flFieldOfView = 0.5; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_flDistLook = 750;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();

	pev->view_ofs = g_vecZero;
	pev->takedamage = DAMAGE_YES;

	//SetThink(&COuranosaurus::Think);
	SetUse(NULL);
	SetTouch(NULL);
	
	SetActivity(ACT_IDLE);
	SetState(MONSTERSTATE_IDLE);
	SetNextThink(1.0);
}

//=========================================================
// Precache
//=========================================================
void COuranosaurus::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/Ourano.mdl");

	PRECACHE_SOUND("ourano/Ourano_FootStep1.wav");
	PRECACHE_SOUND("ourano/Ourano_FootStep2.wav");
	PRECACHE_SOUND("ourano/Ourano_Chew_F0.wav");
	PRECACHE_SOUND("ourano/Ourano_Bite_F0.wav");
	PRECACHE_SOUND("ourano/Ourano_MoveAttack_Vocals.wav");
	PRECACHE_SOUND("ourano/Ourano_Roar_F0.wav");
	PRECACHE_SOUND("ourano/Ourano_QuickAttack_F0.wav");
	PRECACHE_SOUND("ourano/ourano_SmallFlinch_F0.wav");
	PRECACHE_SOUND("ourano/Ourano_BigFlinch_F0.wav");
	PRECACHE_SOUND("ourano/Ourano_DieSimple_F0.wav");
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
int COuranosaurus::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if (m_Activity == ACT_MELEE_ATTACK1)
	{
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	return iIgnore;
}