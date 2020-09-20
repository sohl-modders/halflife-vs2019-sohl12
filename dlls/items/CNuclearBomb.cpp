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

#include "CNuclearBomb.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(item_nuclearbomb, CNuclearBomb);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION	CNuclearBomb::m_SaveData[] =
{
	DEFINE_FIELD(CNuclearBomb, m_fOn, FIELD_BOOLEAN),
	DEFINE_FIELD(CNuclearBomb, m_flLastPush, FIELD_TIME),
	DEFINE_FIELD(CNuclearBomb, m_iPushCount, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CNuclearBomb, CBaseToggle);

//=========================================================
// KeyValue
//=========================================================
void CNuclearBomb::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq("initialstate", pkvd->szKeyName))
	{
		m_fOn = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = true;
	}
	else if (FStrEq("wait", pkvd->szKeyName))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
	{
		CBaseToggle::KeyValue(pkvd);
	}
}

//=========================================================
// Precache
//=========================================================
void CNuclearBomb::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/nuke_case.mdl");

	PRECACHE_SOUND("buttons/button4.wav");
	PRECACHE_SOUND("buttons/button6.wav");

	//The other entities are created here since a restore only calls Precache
	m_pTimer = GetClassPtr<CNuclearBombTimer>(nullptr);
	m_pTimer->pev->origin = pev->origin;
	m_pTimer->pev->angles = pev->angles;
	m_pTimer->Spawn();
	m_pTimer->SetNuclearBombTimer(m_fOn == 1);

	m_pButton = GetClassPtr<CNuclearBombButton>(nullptr);
	m_pButton->pev->origin = pev->origin;
	m_pButton->pev->angles = pev->angles;
	m_pButton->Spawn();
	m_pButton->pev->skin = m_fOn == 1;
}

//=========================================================
// Spawn
//=========================================================
void CNuclearBomb::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/nuke_case.mdl");

	UTIL_SetOrigin(this, pev->origin);
	UTIL_SetSize(pev, { -16, -16, 0 }, { 16, 16, 32 });

	pev->solid = SOLID_BBOX;

	pev->movetype = MOVETYPE_NONE;

	if (DROP_TO_FLOOR(edict()))
	{
		m_iPushCount = 0;
		m_flLastPush = gpGlobals->time;
	}
	else
	{
		ALERT(at_error, "Nuclear Bomb fell out of level at %f,%f,%f", pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
	}
}

//=========================================================
// Use
//=========================================================
void CNuclearBomb::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if ((m_flWait != -1.0 || m_iPushCount <= 0)
		&& m_flWait <= gpGlobals->time - m_flLastPush
		&& ShouldToggle(useType, m_fOn))
	{
		if (m_fOn)
		{
			m_fOn = false;
			EMIT_SOUND(edict(), CHAN_VOICE, "buttons/button4.wav", VOL_NORM, ATTN_NORM);
		}
		else
		{
			m_fOn = true;
			EMIT_SOUND(edict(), CHAN_VOICE, "buttons/button6.wav", VOL_NORM, ATTN_NORM);
		}

		SUB_UseTargets(pActivator, USE_TOGGLE, 0);

		if (m_pButton)
		{
			m_pButton->SetNuclearBombButton(m_fOn == 1);
		}

		if (m_pTimer)
		{
			m_pTimer->SetNuclearBombTimer(m_fOn == 1);
		}

		if (!m_pTimer || !m_pTimer->bBombSoundPlaying)
		{
			++m_iPushCount;
			m_flLastPush = gpGlobals->time;
		}
	}
}