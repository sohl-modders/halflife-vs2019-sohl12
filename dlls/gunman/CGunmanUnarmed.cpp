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
#include "basemonster.h"

#include "CGunmanUnarmed.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_human_unarmed, CGunmanUnarmed);

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CGunmanUnarmed::Classify()
{
	return m_iClass ? m_iClass : CLASS_NONE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CGunmanUnarmed::SetYawSpeed()
{
	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		pev->yaw_speed = 90;
	}
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CGunmanUnarmed::HandleAnimEvent(MonsterEvent_t* pEvent)
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
int CGunmanUnarmed::ISoundMask()
{
	return NULL;
}

//=========================================================
// Spawn
//=========================================================
void CGunmanUnarmed::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/gunmantrooper_ng.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = DONT_BLEED;

	if (pev->health == 0) //LRC
		pev->health = 100;

	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

//=========================================================
// Precache
//=========================================================
void CGunmanUnarmed::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/gunmantrooper_ng.mdl");
}