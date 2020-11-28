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

#include "CDecoreMushroom.h"

/* FGD */
/*
@PointClass base(RenderFields, Appearflags, Angles) size(-16 -16 0, 16 16 16) studio("models/Mushroom.mdl") = decore_mushroom : "Decoration Mushroom"
[
	pose(Choices):"Pose" : 0 =
	[
		-1 : "Random Pose"
		0 : "Pose 1"
		1 : "Pose 2"
		2 : "Pose 3"
	]
]
@PointClass base(RenderFields, Appearflags, Angles) size(-16 -16 0, 16 16 16) studio("models/mushroom2.mdl") = decore_mushroom2 : "Decoration Mushroom 2"
[
	pose(Choices):"Pose" : -1 =
	[
		-1 : "Random Pose"
		0 : "Pose 1"
		1 : "Pose 2"
		2 : "Pose 3"
	]
]
*/

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(decore_mushroom, CDecoreMushroom);

const char* CDecoreMushroom::m_szPoses[] = {
	"pose1",
	"pose2",
	"pose3"
};

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CDecoreMushroom::m_SaveData[] =
{
	DEFINE_FIELD(CDecoreMushroom, m_iPose, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CDecoreMushroom, CDecore);

//=========================================================
// KeyValue
//=========================================================
void CDecoreMushroom::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CDecore::KeyValue(pkvd);
}

//=========================================================
// Spawn
//=========================================================
void CDecoreMushroom::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/Mushroom.mdl");

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_SLIDEBOX;
	pev->takedamage = DAMAGE_NO;
	pev->effects = 0;
	pev->health = 99999;
	pev->yaw_speed = 5;
	pev->ideal_yaw = pev->angles.y;

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));

	UTIL_SetOrigin(this, pev->origin);

	if ((m_iPose == -1))
		m_iPose = RANDOM_LONG(0, ARRAYSIZE(m_szPoses) - 1);

	SetSequenceByName(m_szPoses[m_iPose]);
}

//=========================================================
// Precache
//=========================================================
void CDecoreMushroom::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/Mushroom.mdl");
}