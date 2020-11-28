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

/* FGD */
/*
@PointClass size(-4 -4 -8,  4  4 16) = decore_torchflame : "Flame Like Torch Flame"[]
*/

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(decore_torchflame, CDecoreTorchFlame);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION	CDecoreTorchFlame::m_SaveData[] =
{
	DEFINE_FIELD(CDecoreTorchFlame, m_lastTime, FIELD_TIME),
	DEFINE_FIELD(CDecoreTorchFlame, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CDecoreTorchFlame, CSprite);

//=========================================================
// Spawn
//=========================================================
void CDecoreTorchFlame::Spawn()
{
	if(!pev->model)
		pev->model = MAKE_STRING("sprites/torch.spr");

	Precache();

	SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);

	SetScale(1);
	
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;
	pev->classname = MAKE_STRING("torchflame");

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	
	TurnOn();

	// Worldcraft only sets y rotation, copy to Z
	if (pev->angles.y != 0 && pev->angles.z == 0)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}

	SetFramerate(10.0);
	AbsoluteNextThink(0.1);
}

//=========================================================
// SpriteCreate
//=========================================================
CDecoreTorchFlame* CDecoreTorchFlame::SpriteCreate(const Vector& origin)
{
	CDecoreTorchFlame* pSprite = GetClassPtr((CDecoreTorchFlame*)NULL);
	pSprite->SpriteInit("sprites/torch.spr", origin);
	return pSprite;
}