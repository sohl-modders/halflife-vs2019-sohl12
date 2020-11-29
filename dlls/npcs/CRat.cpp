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
#include "weapons.h"

#include "CRat.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_rat, CRat);

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CRat::Classify()
{
	return m_iClass ? m_iClass : CLASS_INSECT; //LRC- maybe someone needs to give them a basic biology lesson...
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CRat::SetYawSpeed()
{
	switch (m_Activity)
	{
		case ACT_IDLE:
		default:
			pev->yaw_speed = 45;
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CRat::Spawn()
{
	SetModel("models/bigrat.mdl");
	
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;

	SetHealth(8);
	
	pev->view_ofs = Vector(0, 0, 6);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}