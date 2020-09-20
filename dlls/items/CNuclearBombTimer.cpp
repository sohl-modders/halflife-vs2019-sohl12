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

#include "CNuclearBombTimer.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(item_nuclearbombtimer, CNuclearBombTimer);

//=========================================================
// Spawn
//=========================================================
void CNuclearBombTimer::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/nuke_timer.mdl");

	pev->solid = SOLID_NOT;

	UTIL_SetSize(pev, { -16, -16, 0 }, { 16, 16, 32 });

	pev->movetype = MOVETYPE_NONE;

	UTIL_SetOrigin(this, pev->origin);

	if (DROP_TO_FLOOR(edict()) == 0)
	{
		ALERT(at_error, "Nuclear Bomb Timer fell out of level at %f,%f,%f", pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
	}
	else
	{
		pev->skin = 0;
		bPlayBombSound = true;
		bBombSoundPlaying = true;
	}
}

//=========================================================
// Precache
//=========================================================
void CNuclearBombTimer::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/nuke_timer.mdl");

	PRECACHE_SOUND("common/nuke_ticking.wav");
}

//=========================================================
// NuclearBombTimerThink
//=========================================================
void CNuclearBombTimer::NuclearBombTimerThink()
{
	if (pev->skin <= 1)
		++pev->skin;
	else
		pev->skin = 0;

	if (bPlayBombSound)
	{
		EMIT_SOUND(edict(), CHAN_BODY, "common/nuke_ticking.wav", 0.75, ATTN_IDLE);
		bBombSoundPlaying = true;
	}

	SetNextThink(0.1);
}

//=========================================================
// SetNuclearBombTimer
//=========================================================
void CNuclearBombTimer::SetNuclearBombTimer(bool fOn)
{
	if (fOn)
	{
		SetThink(&CNuclearBombTimer::NuclearBombTimerThink);
		SetNextThink(0.5);
		bPlayBombSound = true;
	}
	else
	{
		SetThink(nullptr);
		SetNextThink(0);

		pev->skin = 3;

		if (bBombSoundPlaying)
		{
			STOP_SOUND(edict(), CHAN_BODY, "common/nuke_ticking.wav");
			bBombSoundPlaying = false;
		}
	}
}