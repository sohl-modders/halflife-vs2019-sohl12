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

#include "CDeadSkeleton.h"

const char* CDeadSkeleton::m_szPoses[] = {
	"s_onback",
	"s_sitting",
	"dead_against_wall",
	"dead_stomach"
};

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_skeleton_dead, CDeadSkeleton);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CDeadSkeleton::m_SaveData[] =
{
	DEFINE_FIELD(CDeadSkeleton, m_iPose, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CDeadSkeleton, CBaseMonster);

//=========================================================
// KeyValue
//=========================================================
void CDeadSkeleton::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

//=========================================================
// Spawn
//=========================================================
void CDeadSkeleton::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/skeleton.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	
	m_bloodColor = DONT_BLEED;

	if ((m_iPose == -1))
		m_iPose = RANDOM_LONG(0, ARRAYSIZE(m_szPoses) - 1);
	
	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead skeleton with bad pose\n");
		pev->sequence = 0;
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}

//=========================================================
// Precache
//=========================================================
void CDeadSkeleton::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/skeleton.mdl");
}