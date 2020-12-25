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

#include "CDeadMonster.h"
#include "CDeadBarney.h"

//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iPose properly!
//
//=========================================================

const char* CDeadBarney::m_szPoses[] = {
	"lying_on_back",
	"lying_on_side",
	"lying_on_stomach"
};

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_barney_dead, CDeadBarney);

//=========================================================
// Spawn
//=========================================================
void CDeadBarney::Spawn()
{
	CreateInstace("models/barney.mdl", BLOOD_COLOR_RED, 8);
	MonsterInitDead();
}
