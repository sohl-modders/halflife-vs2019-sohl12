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
#include "squadmonster.h"
#include "weapons.h"

#include "npcs/CMaleAssassin.h"

#include "CDeadMaleAssassin.h"

const char* CDeadMaleAssassin::m_szPoses[] = {
	"deadstomach",
	"deadside",
	"deadsitting"
};

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_massassin_dead, CDeadMaleAssassin);

//=========================================================
// KeyValue
//=========================================================
void CDeadMaleAssassin::KeyValue(KeyValueData* pkvd)
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
void CDeadMaleAssassin::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/massn.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	if ((m_iPose == -1))
		m_iPose = RANDOM_LONG(0, ARRAYSIZE(m_szPoses) - 1);

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);
	if (pev->sequence == -1)
	{
		ALERT(at_debug, "Dead massn with bad pose\n");
	}

	// Corpses have less health
	pev->health = 8;

	switch (pev->weapons)
	{
		case 0: // MP5
			SetBodygroup(MAssassinBodygroup::Weapons, MAssassinWeapon::MP5);
		break;
		case 1: // Sniper Rifle
			SetBodygroup(MAssassinBodygroup::Weapons, MAssassinWeapon::SniperRifle);
		break;
		case 2: // No gun
			SetBodygroup(MAssassinBodygroup::Weapons, MAssassinWeapon::None);
		break;
	}

	switch (pev->body)
	{
		case 0: // white
			SetBodygroup(MAssassinBodygroup::Heads, MAssassinHead::White);
		break;
		case 1: // black
			SetBodygroup(MAssassinBodygroup::Heads, MAssassinHead::Black);
		break;
		case 2: // thermal vision
			SetBodygroup(MAssassinBodygroup::Heads, MAssassinHead::ThermalVision);
		break;
		default: // RANDOM
			SetBodygroup(MAssassinBodygroup::Heads, RANDOM_LONG(0, 1));
		break;
	}

	MonsterInitDead();
}

//=========================================================
// Precache
//=========================================================
void CDeadMaleAssassin::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/massn.mdl");
}