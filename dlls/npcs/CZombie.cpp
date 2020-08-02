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
//=========================================================
// Zombie
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"CZombie.h"

LINK_ENTITY_TO_CLASS(monster_zombie, CZombie);

const char* CZombie::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CZombie::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CZombie::pAttackSounds[] =
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

const char* CZombie::pIdleSounds[] =
{
	"zombie/zo_idle1.wav",
	"zombie/zo_idle2.wav",
	"zombie/zo_idle3.wav",
	"zombie/zo_idle4.wav",
};

const char* CZombie::pAlertSounds[] =
{
	"zombie/zo_alert10.wav",
	"zombie/zo_alert20.wav",
	"zombie/zo_alert30.wav",
};

const char* CZombie::pPainSounds[] =
{
	"zombie/zo_pain1.wav",
	"zombie/zo_pain2.wav",
};

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CZombie::Classify()
{
	return HasClassify() ? GetClassify() : CLASS_ALIEN_MONSTER;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CZombie::SetYawSpeed()
{
	CBaseEntity::SetYawSpeed(120);
}

int CZombie::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Take 30% damage from bullets
	if (bitsDamageType == DMG_BULLET)
	{
		Vector vecDir = GetAbsOrigin() - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce(flDamage);
		SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
		flDamage *= 0.3;
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound();
	
	return BaseClass::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CZombie::PainSound()
{
	const int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0,ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0,
		               pitch);
}

void CZombie::AlertSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0,ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM, 0,
	               pitch);
}

void CZombie::IdleSound()
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0,ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM, 0,
	               pitch);
}

void CZombie::AttackSound()
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random attack sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0,
	               pitch);
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CZombie::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
		{
			// do stuff for this event.
			//		ALERT( at_console, "Slash right!\n" );
			CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->GetFlags().Any(FL_MONSTER | FL_CLIENT))
				{
					pHurt->SetPunchAngle(Vector(5, 0, -18));
					pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_right * 100);
				}
				// Play a random attack hit sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds) - 1)],
				               1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON,
				               pAttackMissSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0,
				               100 + RANDOM_LONG(-5, 5));

			if (RANDOM_LONG(0, 1))
				AttackSound();
		}
		break;

	case ZOMBIE_AE_ATTACK_LEFT:
		{
			// do stuff for this event.
			//		ALERT( at_console, "Slash left!\n" );
			CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgOneSlash, DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->GetFlags().Any(FL_MONSTER | FL_CLIENT))
				{
					pHurt->SetPunchAngle(Vector(5, 0, 18));
					pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_right * 100);
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds) - 1)],
				               1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON,
				               pAttackMissSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0,
				               100 + RANDOM_LONG(-5, 5));

			if (RANDOM_LONG(0, 1))
				AttackSound();
		}
		break;

	case ZOMBIE_AE_ATTACK_BOTH:
		{
			// do stuff for this event.
			CBaseEntity* pHurt = CheckTraceHullAttack(70, gSkillData.zombieDmgBothSlash, DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->GetFlags().Any(FL_MONSTER | FL_CLIENT))
				{
					pHurt->SetPunchAngle(Vector(5, 0, 0));
					pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_forward * -120);
					
				}
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds) - 1)],
				               1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			}
			else
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON,
				               pAttackMissSounds[RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0,
				               100 + RANDOM_LONG(-5, 5));

			if (RANDOM_LONG(0, 1))
				AttackSound();
		}
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CZombie::Spawn()
{
	Precache();

	if (HasModel())
		SetModel(GetModelName());
	else
		SetModel("models/zombie.mdl");

	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(SOLID_SLIDEBOX);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_GREEN);
	
	if (!GetHealth())
		SetHealth(gSkillData.zombieHealth);
	
	SetViewOffset(VEC_VIEW);// position of the eyes relative to monster's origin.
	SetFieldOfView(0.5); // indicates the width of this monster's forward view cone ( as a dotproduct result )
	
	SetMonsterState(MONSTERSTATE_NONE);
	SetCapability(bits_CAP_DOORS_GROUP);
	
	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CZombie::Precache()
{
	int i;

	if (HasModel())
		PrecacheModel(GetModelName()); //LRC
	else
		PrecacheModel("models/zombie.mdl");

	for (i = 0; i < ARRAYSIZE(pAttackHitSounds); i++)
		PrecacheSound((char*)pAttackHitSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAttackMissSounds); i++)
		PrecacheSound((char*)pAttackMissSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAttackSounds); i++)
		PrecacheSound((char*)pAttackSounds[i]);

	for (i = 0; i < ARRAYSIZE(pIdleSounds); i++)
		PrecacheSound((char*)pIdleSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAlertSounds); i++)
		PrecacheSound((char*)pAlertSounds[i]);

	for (i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PrecacheSound((char*)pPainSounds[i]);
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================


int CZombie::IgnoreConditions()
{
	int iIgnore = BaseClass::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
}
