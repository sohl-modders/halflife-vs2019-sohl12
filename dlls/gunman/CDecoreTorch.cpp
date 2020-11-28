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
#include "effects.h"
#include "CDecoreTorchFlame.h"
#include "CDecoreTorch.h"

/* FGD */
/*
@PointClass size(-8 -8 -8,  8  8 16) = decore_torch : "Torch with a Flame"[]
*/

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(decore_torch, CDecoreTorch);

//=========================================================
// Spawn
//=========================================================
void CDecoreTorch::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/Torch.mdl");

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_SLIDEBOX;
	pev->takedamage = DAMAGE_NO;
	pev->effects = 0;
	pev->health = 99999;
	pev->yaw_speed = 5;
	pev->ideal_yaw = pev->angles.y;

	UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 16));

	UTIL_SetOrigin(this, pev->origin);

	SetNextThink(0.1);
	pev->frame = RANDOM_FLOAT(0, 255);

	//Flame
	m_pFlame = CSprite::SpriteCreate("sprites/torch.spr", pev->origin + Vector(0, 0, 23),true);
	m_pFlame->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
	m_pFlame->SetScale(1);
	m_pFlame->SetFramerate(10.0);
	m_pFlame->TurnOn();
	m_pFlame->AbsoluteNextThink(m_pFlame->m_fNextThink + 0.1);
}

//=========================================================
// Precache
//=========================================================
void CDecoreTorch::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/Torch.mdl");

	PRECACHE_MODEL("sprites/torch.spr");
}
