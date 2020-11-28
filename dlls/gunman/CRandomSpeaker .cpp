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
#include "CRandomSpeaker.h"

//=========================================================
// Link
//=========================================================
//LINK_ENTITY_TO_CLASS(monster_random_speaker, CRandomSpeaker);

//=========================================================
// KeyValue
//=========================================================
void CRandomSpeaker::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "random"))
	{
		random = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		volume = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rsnoise"))
	{
		rsnoise = pkvd->szValue;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		wait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}


//=========================================================
// Spawn
//=========================================================
void CRandomSpeaker::Spawn()
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = DAMAGE_NO;

	SetNextThink(0.1);
}

//=========================================================
// Precache
//=========================================================
void CRandomSpeaker::Precache()
{
	//string_t text = MAKE_STRING("sound/" + rsnois);
	
	//PRECACHE_SOUND((char*)STRING(text));
}