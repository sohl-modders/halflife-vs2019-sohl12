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
#include "weapons.h"

#include "CTurret.h"

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_turret, CTurret);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION	CTurret::m_SaveData[] =
{
	DEFINE_FIELD(CTurret, m_iStartSpin, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTurret, CBaseTurret);

//=========================================================
// Spawn
//=========================================================
void CTurret::Spawn()
{
	Precache();

	SetModel("models/turret.mdl");
	
	if (!pev->health)
		pev->health = gSkillData.turretHealth;
	
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	
	UTIL_SetSize(pev, Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CTurret::Initialize);

	m_pEyeGlow = CSprite::SpriteCreate(TURRET_GLOW_SPRITE, pev->origin, FALSE);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 0, 0, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(edict(), 2);
	m_eyeBrightness = 0;

	SetNextThink(0.3);
}

//=========================================================
// Precache
//=========================================================
void CTurret::Precache()
{
	CBaseTurret::Precache();
	
	PrecacheModel(TURRET_GLOW_SPRITE);
}

//=========================================================
// SpinUpCall
//=========================================================
void CTurret::SpinUpCall()
{
	StudioFrameAdvance();
	SetNextThink(0.1);

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			SetNextThink(1.0); // spinup delay
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			m_iStartSpin = 1;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			SetNextThink(0.1); // retarget delay
			EMIT_SOUND(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			SetThink(&CTurret::ActiveThink);
			m_iStartSpin = 0;
			m_iSpin = 1;
		}
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CTurret::ActiveThink);
	}
}

//=========================================================
// SpinDownCall
//=========================================================
void CTurret::SpinDownCall()
{
	if (m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		if (pev->framerate == 1.0)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		}
		
		pev->framerate -= 0.02;
		
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = 0;
		}
	}
}
