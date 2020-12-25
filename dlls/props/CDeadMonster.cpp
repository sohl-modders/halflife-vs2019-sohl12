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

#include "CDeadMonster.h"

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CDeadMonster::m_SaveData[] =
{
	DEFINE_FIELD(CDeadMonster, m_iPose, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CDeadMonster, CBaseMonster);

void CDeadMonster::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CDeadMonster::CreateInstace(const char* modelName, int bloodColor, int health)
{
	PrecacheModel((char*)modelName);
	
	SetModel(modelName);

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = bloodColor;

	const char* seqName = getPos(m_iPose);
	pev->sequence = LookupSequence(seqName);
	if (pev->sequence == -1)
	{
		ALERT(at_console, "%s with bad pose (no %s animation in %s)\n", 
			STRING(pev->classname), seqName, modelName);
		pev->sequence = 0;
	}
	
	pev->health = health;
}