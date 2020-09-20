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

#include "CDeadOtis.h"

//=========================================================
// DEAD OTIS PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iPose properly!
//
//=========================================================

const char* CDeadOtis::m_szPoses[] = {
	"lying_on_back",
	"lying_on_side",
	"lying_on_stomach",
	"stuffed_in_vent",
	"dead_sitting",
};

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_otis_dead, CDeadOtis);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CDeadOtis::m_SaveData[] =
{
	DEFINE_FIELD(CDeadOtis, m_iPose, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CDeadOtis, CBaseMonster);

//=========================================================
// KeyValue
//=========================================================
void CDeadOtis::KeyValue(KeyValueData* pkvd)
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
void CDeadOtis::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/otis.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	
	m_bloodColor = BLOOD_COLOR_RED;

	if ((m_iPose == -1))
		m_iPose = RANDOM_LONG(0, ARRAYSIZE(m_szPoses) - 1);

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead otis with bad pose\n");
		pev->sequence = 0;
	}
	
	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}

//=========================================================
// Precache
//=========================================================
void CDeadOtis::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/otis.mdl");
}