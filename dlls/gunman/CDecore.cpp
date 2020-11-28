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
#include "CDecore.h"

void CDecore::Spawn( void )
{
	Precache( );
	
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_SLIDEBOX;
	pev->takedamage = DAMAGE_NO;
	pev->effects = 0;
	pev->health = 99999;
    pev->yaw_speed = 5;
	pev->ideal_yaw = pev->angles.y;
	
	SET_MODEL(edict(), "models/null.mdl");
	
	UTIL_SetSize(pev, Vector( -16, -16, 0), Vector(16, 16, 36));
	
	UTIL_SetOrigin( this, pev->origin );

	SetTouch( &CDecore::Touch );
	
	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CDecore::Precache( void )
{
	PRECACHE_MODEL("models/null.mdl");
}

void CDecore::Animate( float frames )
{ 
	pev->frame += frames;
	if ( m_maxFrame > 0 )
		pev->frame = fmod( pev->frame, m_maxFrame );
}

void CDecore::Touch( CBaseEntity *pOther )
{
	if (pOther && pOther != this) {
		pOther->Touch(pOther);
	}
}