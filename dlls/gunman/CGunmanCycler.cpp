/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CCycler.h"
#include "CGunmanCycler.h"

/* FGD */
/*
@PointClass base(Monster) size(-16 - 16 0, 16 16 72) studio() = gunman_cycler : "Gunman Cycler"
[
	model(studio):"model"
	cyc_submodel1(integer) : "Body1" : 1
	cyc_submodel2(integer) : "Body2" : 1
	cyc_submodel3(integer) : "Body3" : 1
]
*/

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(gunman_cycler, CGunmanCycler);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION	CGunmanCycler::m_SaveData[] =
{
	DEFINE_FIELD(CGunmanCycler, m_animate, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CGunmanCycler, CCycler);

//=========================================================
// KeyValue
//=========================================================
void CGunmanCycler::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "cyc_submodel1"))
	{
		cyc_submodel1 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "cyc_submodel2"))
	{
		cyc_submodel2 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "cyc_submodel3"))
	{
		cyc_submodel3 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CCycler::KeyValue(pkvd);
}

//=========================================================
// Spawn
//=========================================================
void CGunmanCycler::Spawn(void)
{
	Precache();
	
	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/error.mdl");

	CCycler::Spawn();

	SetBodygroup(1, cyc_submodel1);
	
	SetBodygroup(2, cyc_submodel2);
	
	SetBodygroup(3, cyc_submodel3);

	pev->takedamage = DAMAGE_NO;
	pev->classname = MAKE_STRING("gmcycler");

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 72));

	UTIL_SetOrigin(this, pev->origin);
}

//=========================================================
// Precache
//=========================================================
void CGunmanCycler::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/error.mdl");
}