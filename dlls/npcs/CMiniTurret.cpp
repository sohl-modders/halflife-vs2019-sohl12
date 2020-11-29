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

#include "CMiniTurret.h"

LINK_ENTITY_TO_CLASS(monster_miniturret, CMiniTurret);

void CMiniTurret::Spawn()
{
	Precache();
	
	SetModel("models/miniturret.mdl");
	
	if (!pev->health)
		pev->health = gSkillData.miniturretHealth;
	
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = 0;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();
	
	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	
	UTIL_SetSize(pev, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetThink(&CMiniTurret::Initialize);
	SetNextThink(0.3);
}

void CMiniTurret::Precache()
{
	CBaseTurret::Precache();
	
	PrecacheSound("weapons/hks1.wav");
	PrecacheSound("weapons/hks2.wav");
	PrecacheSound("weapons/hks3.wav");
}

void CMiniTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_9MM, 1);

	switch (RANDOM_LONG(0, 2))
	{
		case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}