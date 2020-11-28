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
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "weapons.h"
#include "CAllyMonster.h"
#include "CSquadTalkMonster.h"
#include "soundent.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"

#include "CHGrunt.h"
#include "CHGruntAllyTorch.h"

#include "Tasks.h"

int g_fTorchAllyQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int g_iSkillLevel;

LINK_ENTITY_TO_CLASS(monster_human_torch_ally, CHGruntAllyTorch);

TYPEDESCRIPTION	CHGruntAllyTorch::m_SaveData[] =
{
	DEFINE_FIELD(CHGruntAllyTorch, m_painTime, FIELD_TIME),
	DEFINE_FIELD(CHGruntAllyTorch, m_checkAttackTime, FIELD_TIME),
	DEFINE_FIELD(CHGruntAllyTorch, m_lastAttackCheck, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_flNextGrenadeCheck, FIELD_TIME),
	DEFINE_FIELD(CHGruntAllyTorch, m_flNextPainTime, FIELD_TIME),
	DEFINE_FIELD(CHGruntAllyTorch, m_vecTossVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CHGruntAllyTorch, m_fThrowGrenade, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_fStanding, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_fFirstEncounter, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_cClipSize, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAllyTorch, m_iSentence, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAllyTorch, m_iBlackOrWhite, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAllyTorch, m_fUseTorch, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_fGunHolstered, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_fTorchHolstered, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_fTorchActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAllyTorch, m_flLastShot, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CHGruntAllyTorch, CSquadTalkMonster);

const char* CHGruntAllyTorch::pTorchSentences[] =
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

enum
{
	TORCH_SENT_NONE = -1,
	TORCH_SENT_GREN = 0,
	TORCH_SENT_ALERT,
	TORCH_SENT_MONSTER,
	TORCH_SENT_COVER,
	TORCH_SENT_THROW,
	TORCH_SENT_CHARGE,
	TORCH_SENT_TAUNT,
} TORCH_ALLY_SENTENCE_TYPES;

//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after 
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has 
// started moving.
//=========================================================
void CHGruntAllyTorch::SpeakSentence(void)
{
	if (m_iSentence == TORCH_SENT_NONE)
	{
		// no sentence cued up.
		return;
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz(ENT(pev), pTorchSentences[m_iSentence], TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		JustSpoke();
	}
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CHGruntAllyTorch::GibMonster(void)
{
	if (m_hWaitMedic)
	{
		auto pMedic = m_hWaitMedic.Entity<CSquadTalkMonster>();

		if (pMedic->pev->deadflag != DEAD_NO)
			m_hWaitMedic = nullptr;
		else
			pMedic->HealMe(nullptr);
	}

	Vector	vecGunPos;
	Vector	vecGunAngles;

	//TODO: probably the wrong logic, but it was in the original
	if (GetBodygroup(TorchAllyBodygroup::Weapons) != TorchAllyWeapon::DesertEagle)
	{// throw a gun if the grunt has one
		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun = DropItem("weapon_eagle", vecGunPos, vecGunAngles);

		if (pGun)
		{
			pGun->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
			pGun->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
		}

		//Note: this wasn't in the original
		SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::None);
	}

	if (m_fTorchActive)
	{
		m_fTorchActive = false;
		UTIL_Remove(m_pTorchBeam);
		m_pTorchBeam = nullptr;
	}

	CBaseMonster::GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHGruntAllyTorch::ISoundMask(void)
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CHGruntAllyTorch::FOkToSpeak(void)
{
	// if someone else is talking, don't speak
	if (gpGlobals->time <= CSquadTalkMonster::g_talkWaitTime)
		return FALSE;

	if (pev->spawnflags & SF_MONSTER_GAG)
	{
		if (m_MonsterState != MONSTERSTATE_COMBAT)
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	// if player is not in pvs, don't speak
//	if (FNullEnt(FIND_CLIENT_IN_PVS(edict())))
//		return FALSE;

	return TRUE;
}

//=========================================================
//=========================================================
void CHGruntAllyTorch::JustSpoke(void)
{
	CSquadTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = TORCH_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHGruntAllyTorch::PrescheduleThink(void)
{
	if (InSquad() && m_hEnemy != NULL)
	{
		if (HasConditions(bits_COND_SEE_ENEMY))
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if (gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5)
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}
}

//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================
BOOL CHGruntAllyTorch::FCanCheckAttacks(void)
{
	if (!HasConditions(bits_COND_ENEMY_TOOFAR))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CHGruntAllyTorch::CheckMeleeAttack1(float flDot, float flDist)
{
	CBaseMonster* pEnemy;

	if (m_hEnemy != NULL)
	{
		pEnemy = m_hEnemy->MyMonsterPointer();

		if (!pEnemy)
		{
			return FALSE;
		}
	}

	if (flDist <= 64 && flDot >= 0.7 &&
		pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON)
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1 - overridden for HGrunt, cause 
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the HGrunt can attack when the enemy is
// occluded (throw grenade over wall, etc). We must 
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
BOOL CHGruntAllyTorch::CheckRangeAttack1(float flDot, float flDist)
{
	//Only if we have a weapon
	if (pev->weapons)
	{
		//Friendly fire is allowed
		if (!HasConditions(bits_COND_ENEMY_OCCLUDED) && flDist <= 1024 && flDot >= 0.5 /*&& NoFriendlyFire()*/)
		{
			TraceResult	tr;

			auto pEnemy = m_hEnemy.Entity<CBaseEntity>();

			//if( !pEnemy->IsPlayer() && flDist <= 64 )
			//{
			//	// kick nonclients, but don't shoot at them.
			//	return FALSE;
			//}

			//TODO: kinda odd that this doesn't use GetGunPosition like the original
			Vector vecSrc = pev->origin + Vector(0, 0, 55);

			//Fire at last known position, adjusting for target origin being offset from entity origin
			const auto targetOrigin = pEnemy->BodyTarget(vecSrc);

			const auto targetPosition = targetOrigin - pEnemy->pev->origin + m_vecEnemyLKP;

			// verify that a bullet fired from the gun will hit the enemy before the world.
			UTIL_TraceLine(vecSrc, targetPosition, dont_ignore_monsters, ENT(pev), &tr);

			m_lastAttackCheck = tr.flFraction == 1.0 ? true : tr.pHit && GET_PRIVATE(tr.pHit) == pEnemy;

			return m_lastAttackCheck;
		}
	}

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CHGruntAllyTorch::CheckRangeAttack2(float flDot, float flDist)
{
	if (m_fGunHolstered || !FBitSet(pev->weapons, TorchAllyWeaponFlag::HandGrenade))
	{
		return FALSE;
	}

	// if the grunt isn't moving, it's ok to check.
	if (m_flGroundSpeed != 0)
	{
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck)
	{
		return m_fThrowGrenade;
	}

	if (!FBitSet(m_hEnemy->pev->flags, FL_ONGROUND) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z)
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	Vector vecTarget;

	if (FBitSet(pev->weapons, TorchAllyWeaponFlag::HandGrenade))
	{
		// find feet
		if (RANDOM_LONG(0, 1))
		{
			// magically know where they are
			vecTarget = Vector(m_hEnemy->pev->origin.x, m_hEnemy->pev->origin.y, m_hEnemy->pev->absmin.z);
		}
		else
		{
			// toss it to where you last saw them
			vecTarget = m_vecEnemyLKP;
		}
		// vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		// vecTarget = vecTarget + m_hEnemy->pev->velocity * 2;
	}
	else
	{
		// find target
		// vecTarget = m_hEnemy->BodyTarget( pev->origin );
		vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget(pev->origin) - m_hEnemy->pev->origin);
		// estimate position
		if (HasConditions(bits_COND_SEE_ENEMY))
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.torchAllyGrenadeSpeed) * m_hEnemy->pev->velocity;
	}

	// are any of my squad members near the intended grenade impact area?
	if (InSquad())
	{
		if (SquadMemberInRange(vecTarget, 256))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
			m_fThrowGrenade = FALSE;
		}
	}

	if ((vecTarget - pev->origin).Length2D() <= 256)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}


	if (FBitSet(pev->weapons, TorchAllyWeaponFlag::HandGrenade))
	{
		Vector vecToss = VecCheckToss(pev, GetGunPosition(), vecTarget, 0.5);

		if (vecToss != g_vecZero)
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}
	else
	{
		Vector vecToss = VecCheckThrow(pev, GetGunPosition(), vecTarget, gSkillData.torchAllyGrenadeSpeed, 0.5);

		if (vecToss != g_vecZero)
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 0.3; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}



	return m_fThrowGrenade;
}


//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CHGruntAllyTorch::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// check for Torch fuel tank hit
	if (ptr->iHitgroup == 8)
	{
		//Make sure it kills this grunt
		bitsDamageType = DMG_ALWAYSGIB | DMG_BLAST;
		flDamage = pev->health;
		ExplosionCreate(ptr->vecEndPos, pev->angles, edict(), 100, true);
	}
	// check for helmet shot
	else if (ptr->iHitgroup == 11)
	{
		// make sure we're wearing one
		//TODO: disabled for ally
		if (/*GetBodygroup( HGruntAllyBodygroup::Head ) == HGruntAllyHead::GasMask &&*/ (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)))
		{
			// absorb damage
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, 1.0);
				flDamage = 0.01;
			}
		}
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}
	//PCV absorbs some damage types
	else if ((ptr->iHitgroup == HITGROUP_CHEST || ptr->iHitgroup == HITGROUP_STOMACH)
		&& (bitsDamageType & (DMG_BLAST | DMG_BULLET | DMG_SLASH)))
	{
		flDamage *= 0.5;
	}

	CSquadTalkMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CHGruntAllyTorch::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CSquadTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	if (!IsAlive() || pev->deadflag == DEAD_DYING)
		return ret;

	if (m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT))
	{
		Forget(bits_MEMORY_INCOVER);

		m_flPlayerDamage += flDamage;

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if (m_hEnemy == NULL)
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if (gpGlobals->time - m_flLastHitByPlayer < 4.0 && m_iPlayerHits > 2
				&& ((m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing(pevAttacker, pev->origin)))
			{
				// Alright, now I'm pissed!
				PlaySentence("FG_MAD", 4, VOL_NORM, ATTN_NORM);

				Remember(bits_MEMORY_PROVOKED);
				StopFollowing(TRUE);
				ALERT(at_console, "HGrunt Ally is now MAD!\n");
			}
			else
			{
				// Hey, be careful with that
				PlaySentence("FG_SHOT", 4, VOL_NORM, ATTN_NORM);
				Remember(bits_MEMORY_SUSPICIOUS);

				if (4.0 > gpGlobals->time - m_flLastHitByPlayer)
					++m_iPlayerHits;
				else
					m_iPlayerHits = 0;

				m_flLastHitByPlayer = gpGlobals->time;

				ALERT(at_console, "HGrunt Ally is now SUSPICIOUS!\n");
			}
		}
		else if (!m_hEnemy->IsPlayer())
		{
			PlaySentence("FG_SHOT", 4, VOL_NORM, ATTN_NORM);
		}
	}

	return ret;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHGruntAllyTorch::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 150;
		break;
	case ACT_RUN:
		ys = 150;
		break;
	case ACT_WALK:
		ys = 180;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 120;
		break;
	case ACT_RANGE_ATTACK2:
		ys = 120;
		break;
	case ACT_MELEE_ATTACK1:
		ys = 120;
		break;
	case ACT_MELEE_ATTACK2:
		ys = 120;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

void CHGruntAllyTorch::IdleSound(void)
{
	if (FOkToSpeak() && (g_fTorchAllyQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fTorchAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fTorchAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fTorchAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fTorchAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fTorchAllyQuestion = 0;
		}
		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CHGruntAllyTorch::CheckAmmo(void)
{
	if (m_cAmmoLoaded <= 0)
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CHGruntAllyTorch::Classify(void)
{
	return	CLASS_HUMAN_MILITARY_FRIENDLY;
}

//=========================================================
//=========================================================
CBaseEntity* CHGruntAllyTorch::Kick(void)
{
	TraceResult tr;

	UTIL_MakeVectors(pev->angles);
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		return pEntity;
	}

	return NULL;
}

//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================

Vector CHGruntAllyTorch::GetGunPosition()
{
	if (m_fStanding)
	{
		return pev->origin + Vector(0, 0, 60);
	}
	else
	{
		return pev->origin + Vector(0, 0, 48);
	}
}

//=========================================================
// Shoot
//=========================================================
void CHGruntAllyTorch::Shoot(void)
{
	//Limit fire rate
	if (m_hEnemy == NULL || gpGlobals->time - m_flLastShot <= 0.11)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	UTIL_MakeVectors(pev->angles);

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357, 0); // shoot +-5 degrees

	const auto random = RANDOM_LONG(0, 20);

	const auto pitch = random <= 10 ? random + 95 : 100;

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/desert_eagle_fire.wav", VOL_NORM, ATTN_NORM, 0, pitch);

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);

	m_flLastShot = gpGlobals->time;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHGruntAllyTorch::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;
	//TODO: add remaining torch grunt events
	switch (pEvent->event)
	{
	case TORCH_AE_DROP_GUN:
	{
		//If we don't have a gun equipped
		//TODO: why is it checking like this?
		if (GetBodygroup(TorchAllyBodygroup::Weapons) != TorchAllyWeapon::DesertEagle)
		{
			Vector	vecGunPos;
			Vector	vecGunAngles;

			GetAttachment(0, vecGunPos, vecGunAngles);

			// switch to body group with no gun.
			SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::None);

			// now spawn a gun.
			DropItem("weapon_eagle", vecGunPos, vecGunAngles);
		}
	}
	break;

	case TORCH_AE_RELOAD:
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/desert_eagle_reload.wav", 1, ATTN_NORM);

		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;

	case TORCH_AE_GREN_TOSS:
	{
		UTIL_MakeVectors(pev->angles);
		// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
		CGrenade::ShootTimed(pev, GetGunPosition(), m_vecTossVelocity, 3.5);

		m_fThrowGrenade = FALSE;
		m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		// !!!LATER - when in a group, only try to throw grenade if ordered.
	}
	break;

	case TORCH_AE_GREN_DROP:
	{
		UTIL_MakeVectors(pev->angles);
		CGrenade::ShootTimed(pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3);
	}
	break;

	case TORCH_AE_SHOOT:
	{
		Shoot();
	}
	break;

	case TORCH_AE_KICK:
	{
		CBaseEntity* pHurt = Kick();

		if (pHurt)
		{
			// SOUND HERE!
			UTIL_MakeVectors(pev->angles);
			pHurt->pev->punchangle.x = 15;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
			pHurt->TakeDamage(pev, pev, gSkillData.torchAllyDmgKick, DMG_CLUB);
		}
	}
	break;

	case TORCH_AE_HOLSTER_TORCH:
	{
		SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::DesertEagle);
		m_fGunHolstered = false;
		m_fTorchHolstered = true;
		break;
	}

	case TORCH_AE_HOLSTER_GUN:
	{
		SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::Torch);
		m_fGunHolstered = true;
		m_fTorchHolstered = false;
		break;
	}

	case TORCH_AE_HOLSTER_BOTH:
	{
		SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::None);
		m_fGunHolstered = true;
		m_fTorchHolstered = true;
		break;
	}

	case TORCH_AE_ACTIVATE_TORCH:
	{
		m_fTorchActive = true;
		m_pTorchBeam = CBeam::BeamCreate(TORCH_BEAM_SPRITE, 5);

		if (m_pTorchBeam)
		{
			Vector vecTorchPos, vecTorchAng;
			CBaseAnimating::GetAttachment(2, vecTorchPos, vecTorchAng);

			m_pTorchBeam->EntsInit(entindex(), entindex());

			m_pTorchBeam->SetStartAttachment(4);
			m_pTorchBeam->SetEndAttachment(3);

			m_pTorchBeam->SetColor(0, 0, 255);
			m_pTorchBeam->SetBrightness(255);
			m_pTorchBeam->SetWidth(5);
			m_pTorchBeam->SetFlags(BEAM_FSHADEIN);
			m_pTorchBeam->SetScrollRate(20);

			m_pTorchBeam->pev->spawnflags |= SF_BEAM_SPARKEND;
			m_pTorchBeam->DoSparks(vecTorchPos, vecTorchPos);
		}
		break;
	}

	case TORCH_AE_DEACTIVATE_TORCH:
	{
		if (m_pTorchBeam)
		{
			m_fTorchActive = false;
			UTIL_Remove(m_pTorchBeam);
			m_pTorchBeam = nullptr;
		}
		break;
	}

	case TORCH_AE_CAUGHT_ENEMY:
	{
		if (FOkToSpeak())
		{
			SENTENCEG_PlayRndSz(ENT(pev), "FG_ALERT", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			JustSpoke();
		}

	}

	default:
		CSquadTalkMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHGruntAllyTorch::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/hgrunt_torch.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;
	pev->health = gSkillData.torchAllyHealth;
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime = gpGlobals->time;
	m_iSentence = TORCH_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	m_fEnemyEluded = FALSE;
	m_fFirstEncounter = TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;

	m_fGunDrawn = false;
	m_fGunHolstered = false;
	m_fTorchHolstered = true;
	m_fTorchActive = false;

	if (!pev->weapons)
	{
		pev->weapons |= TorchAllyWeaponFlag::DesertEagle;
	}

	int weaponIndex = TorchAllyWeapon::None;

	if (pev->weapons & TorchAllyWeaponFlag::DesertEagle)
	{
		weaponIndex = TorchAllyWeapon::DesertEagle;
		m_cClipSize = TORCH_DEAGLE_CLIP_SIZE;
	}
	else
	{
		weaponIndex = TorchAllyWeapon::Torch;
		m_cClipSize = 0;
		m_fGunHolstered = true;
		m_fTorchHolstered = false;
	}

	SetBodygroup(TorchAllyBodygroup::Weapons, weaponIndex);

	m_cAmmoLoaded = m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	CSquadTalkMonster::g_talkWaitTime = 0;

	m_flMedicWaitTime = gpGlobals->time;

	MonsterInit();

	SetUse(&CHGruntAllyTorch::FollowerUse);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHGruntAllyTorch::Precache()
{
	PRECACHE_MODEL("models/hgrunt_torch.mdl");
	PRECACHE_MODEL(TORCH_BEAM_SPRITE);

	TalkInit();

	PRECACHE_SOUND("hgrunt/gr_mgun1.wav");
	PRECACHE_SOUND("hgrunt/gr_mgun2.wav");

	PRECACHE_SOUND("fgrunt/death1.wav");
	PRECACHE_SOUND("fgrunt/death2.wav");
	PRECACHE_SOUND("fgrunt/death3.wav");
	PRECACHE_SOUND("fgrunt/death4.wav");
	PRECACHE_SOUND("fgrunt/death5.wav");
	PRECACHE_SOUND("fgrunt/death6.wav");

	PRECACHE_SOUND("fgrunt/pain1.wav");
	PRECACHE_SOUND("fgrunt/pain2.wav");
	PRECACHE_SOUND("fgrunt/pain3.wav");
	PRECACHE_SOUND("fgrunt/pain4.wav");
	PRECACHE_SOUND("fgrunt/pain5.wav");
	PRECACHE_SOUND("fgrunt/pain6.wav");

	PRECACHE_SOUND("weapons/desert_eagle_fire.wav");
	PRECACHE_SOUND("weapons/desert_eagle_reload.wav");

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	PRECACHE_SOUND("fgrunt/torch_light.wav");
	PRECACHE_SOUND("fgrunt/torch_cut_loop.wav");
	PRECACHE_SOUND("fgrunt/medic.wav");

	// get voice pitch
	m_voicePitch = 95;

	CSquadTalkMonster::Precache();
}

//=========================================================
// start task
//=========================================================
void CHGruntAllyTorch::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_TORCH_ALLY_CHECK_FIRE:
		if (!NoFriendlyFire(true))
		{
			SetConditions(bits_COND_GRUNT_NOFIRE);
		}
		TaskComplete();
		break;

	case TASK_TORCH_ALLY_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;

	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget(bits_MEMORY_INCOVER);
		CSquadTalkMonster::StartTask(pTask);
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_TORCH_ALLY_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		CSquadTalkMonster::StartTask(pTask);
		if (pev->movetype == MOVETYPE_FLY)
		{
			m_IdealActivity = ACT_GLIDE;
		}
		break;

	default:
		CSquadTalkMonster::StartTask(pTask);
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CHGruntAllyTorch::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_TORCH_ALLY_FACE_TOSS_DIR:
	{
		// project a point along the toss vector and turn to face that point.
		MakeIdealYaw(pev->origin + m_vecTossVelocity * 64);
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			m_iTaskStatus = TASKSTATUS_COMPLETE;
		}
		break;
	}
	default:
	{
		CSquadTalkMonster::RunTask(pTask);
		break;
	}
	}
}

//=========================================================
// PainSound
//=========================================================
void CHGruntAllyTorch::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
#if 0
		if (RANDOM_LONG(0, 99) < 5)
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_PAIN", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif 
		switch (RANDOM_LONG(0, 7))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain3.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain4.wav", 1, ATTN_NORM);
			break;
		case 2:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain5.wav", 1, ATTN_NORM);
			break;
		case 3:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain1.wav", 1, ATTN_NORM);
			break;
		case 4:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain2.wav", 1, ATTN_NORM);
			break;
		case 5:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain6.wav", 1, ATTN_NORM);
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CHGruntAllyTorch::DeathSound(void)
{
	//TODO: these sounds don't exist, the gr_ prefix is wrong
	switch (RANDOM_LONG(0, 5))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death1.wav", 1, ATTN_IDLE);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death2.wav", 1, ATTN_IDLE);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death3.wav", 1, ATTN_IDLE);
		break;
	case 3:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death4.wav", 1, ATTN_IDLE);
		break;
	case 4:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death5.wav", 1, ATTN_IDLE);
		break;
	case 5:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_death6.wav", 1, ATTN_IDLE);
		break;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

DEFINE_CUSTOM_SCHEDULES(CHGruntAllyTorch)
{
	slBaFollow,
	slBaFaceTarget,
	slIdleBaStand,
	slGruntFail,
	slGruntCombatFail,
	slGruntVictoryDance,
	slGruntEstablishLineOfFire,
	slGruntFoundEnemy,
	slGruntCombatFace,
	slGruntSignalSuppress,
	slGruntSuppress,
	slGruntWaitInCover,
	slGruntTakeCover,
	slGruntGrenadeCover,
	slGruntTossGrenadeCover,
	slGruntTakeCoverFromBestSound,
	slGruntHideReload,
	slGruntSweep,
	slGruntRangeAttack1A,
	slGruntRangeAttack1B,
	slGruntRangeAttack2,
	slGruntRepel,
	slGruntRepelAttack,
	slGruntRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES(CHGruntAllyTorch, CSquadTalkMonster);

//=========================================================
// SetActivity 
//=========================================================
void CHGruntAllyTorch::SetActivity(Activity NewActivity)
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (m_fStanding)
		{
			// get aimable sequence
			iSequence = LookupSequence("standing_mp5");
		}
		else
		{
			// get crouching shoot
			iSequence = LookupSequence("crouching_mp5");
		}
		break;
	case ACT_RUN:
		if (pev->health <= HGRUNT_LIMP_HEALTH)
		{
			// limp!
			iSequence = LookupActivity(ACT_RUN_HURT);
		}
		else
		{
			iSequence = LookupActivity(NewActivity);
		}
		break;
	case ACT_WALK:
		if (pev->health <= HGRUNT_LIMP_HEALTH)
		{
			// limp!
			iSequence = LookupActivity(ACT_WALK_HURT);
		}
		else
		{
			iSequence = LookupActivity(NewActivity);
		}
		break;
	case ACT_IDLE:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity(NewActivity);
		break;
	default:
		iSequence = LookupActivity(NewActivity);
		break;
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0;	// Set to the reset anim (if it's there)
	}
}

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t* CHGruntAllyTorch::GetSchedule(void)
{

	// clear old sentence
	m_iSentence = TORCH_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if (pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE)
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType(SCHED_TORCH_ALLY_REPEL_LAND);
		}
		else
		{
			// repel down a rope, 
			if (m_MonsterState == MONSTERSTATE_COMBAT)
				return GetScheduleOfType(SCHED_TORCH_ALLY_REPEL_ATTACK);
			else
				return GetScheduleOfType(SCHED_TORCH_ALLY_REPEL);
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound;
		pSound = PBestSound();

		ASSERT(pSound != NULL);
		if (pSound)
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
			{
				// dangerous sound nearby!

				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause 
				// this may only affect a single individual in a squad. 

				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(ENT(pev), "FG_GREN", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
			}
			/*
			if (!HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & (bits_SOUND_PLAYER | bits_SOUND_COMBAT) ))
			{
				MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			if (FOkToSpeak())
			{
				PlaySentence("FG_KILL", 4, VOL_NORM, ATTN_NORM);
			}

			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (m_hWaitMedic)
		{
			auto pMedic = m_hWaitMedic.Entity<CSquadTalkMonster>();

			if (pMedic->pev->deadflag != DEAD_NO)
				m_hWaitMedic = nullptr;
			else
				pMedic->HealMe(nullptr);

			m_flMedicWaitTime = gpGlobals->time + 5.0;
		}

		// new enemy
					//Do not fire until fired upon
		if (HasAllConditions(bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE))
		{
			if (InSquad())
			{
				MySquadLeader()->m_fEnemyEluded = FALSE;

				if (!IsLeader())
				{
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
				else
				{
					//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
					// monster and has made it the squad's enemy. You
					// can check pev->flags for FL_CLIENT to determine whether this is the player
					// or a monster. He's going to immediately start
					// firing, though. If you'd like, we can make an alternate "first sight" 
					// schedule where the leader plays a handsign anim
					// that gives us enough time to hear a short sentence or spoken command
					// before he starts pluggin away.
					if (FOkToSpeak())// && RANDOM_LONG(0,1))
					{
						if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
							// player
							SENTENCEG_PlayRndSz(ENT(pev), "FG_ALERT", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						else if ((m_hEnemy != NULL) &&
							(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) &&
							(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) &&
							(m_hEnemy->Classify() != CLASS_MACHINE))
							// monster
							SENTENCEG_PlayRndSz(ENT(pev), "FG_MONST", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

						JustSpoke();
					}

					if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
					{
						return GetScheduleOfType(SCHED_TORCH_ALLY_SUPPRESS);
					}
					else
					{
						return GetScheduleOfType(SCHED_TORCH_ALLY_ESTABLISH_LINE_OF_FIRE);
					}
				}
			}

			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		else if (HasConditions(bits_COND_HEAVY_DAMAGE))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		// no ammo
					//Only if the grunt has a weapon
		else if (pev->weapons && HasConditions(bits_COND_NO_AMMO_LOADED))
		{
			//!!!KELLY - this individual just realized he's out of bullet ammo. 
			// He's going to try to find cover to run to and reload, but rarely, if 
			// none is available, he'll drop and reload in the open here. 
			return GetScheduleOfType(SCHED_TORCH_ALLY_COVER_AND_RELOAD);
		}

		// damaged just a little
		else if (HasConditions(bits_COND_LIGHT_DAMAGE))
		{
			// if hurt:
			// 90% chance of taking cover
			// 10% chance of flinch.
			int iPercent = RANDOM_LONG(0, 99);

			if (iPercent <= 90 && m_hEnemy != NULL)
			{
				// only try to take cover if we actually have an enemy!

				//!!!KELLY - this grunt was hit and is going to run to cover.
				if (FOkToSpeak()) // && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "FG_COVER", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = TORCH_SENT_COVER;
					//JustSpoke();
				}
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			}
			else
			{
				return GetScheduleOfType(SCHED_SMALL_FLINCH);
			}
		}
		// can kick
		else if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		// can shoot
		else if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			if (InSquad())
			{
				// if the enemy has eluded the squad and a squad member has just located the enemy
				// and the enemy does not see the squad member, issue a call to the squad to waste a 
				// little time and give the player a chance to turn.
				if (MySquadLeader()->m_fEnemyEluded && !HasConditions(bits_COND_ENEMY_FACING_ME))
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;
					return GetScheduleOfType(SCHED_TORCH_ALLY_FOUND_ENEMY);
				}
			}

			if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
			{
				// try to take an available ENGAGE slot
				return GetScheduleOfType(SCHED_RANGE_ATTACK1);
			}
			else if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				// throw a grenade if can and no engage slots are available
				return GetScheduleOfType(SCHED_RANGE_ATTACK2);
			}
			else
			{
				// hide!
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			}
		}
		// can't see enemy
		else if (HasConditions(bits_COND_ENEMY_OCCLUDED))
		{
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(ENT(pev), "FG_THROW", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_RANGE_ATTACK2);
			}
			else if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
			{
				//!!!KELLY - grunt cannot see the enemy and has just decided to 
				// charge the enemy's position. 
				if (FOkToSpeak())// && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "FG_CHARGE", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = TORCH_SENT_CHARGE;
					//JustSpoke();
				}

				return GetScheduleOfType(SCHED_TORCH_ALLY_ESTABLISH_LINE_OF_FIRE);
			}
			else
			{
				//!!!KELLY - grunt is going to stay put for a couple seconds to see if
				// the enemy wanders back out into the open, or approaches the
				// grunt's covered position. Good place for a taunt, I guess?
				if (FOkToSpeak() && RANDOM_LONG(0, 1))
				{
					SENTENCEG_PlayRndSz(ENT(pev), "FG_TAUNT", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_STANDOFF);
			}
		}

		//Only if not following a player
		if (!m_hTargetEnt || !m_hTargetEnt->IsPlayer())
		{
			if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1))
			{
				return GetScheduleOfType(SCHED_TORCH_ALLY_ESTABLISH_LINE_OF_FIRE);
			}
		}

		//Don't fall through to idle schedules
		break;
	}

	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		//if we're not waiting on a medic and we're hurt, call out for a medic
		if (!m_hWaitMedic
			&& gpGlobals->time > m_flMedicWaitTime
			&& pev->health <= 20.0)
		{
			auto pMedic = MySquadMedic();

			if (!pMedic)
			{
				pMedic = FindSquadMedic(1024);
			}

			if (pMedic)
			{
				if (pMedic->pev->deadflag == DEAD_NO)
				{
					ALERT(at_aiconsole, "Injured Grunt found Medic\n");

					if (pMedic->HealMe(this))
					{
						ALERT(at_aiconsole, "Injured Grunt called for Medic\n");

						EMIT_SOUND_DYN(edict(), CHAN_VOICE, "fgrunt/medic.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

						JustSpoke();
						m_flMedicWaitTime = gpGlobals->time + 5.0;
					}
				}
			}
		}

		if (m_hEnemy == NULL && IsFollowing())
		{
			if (!m_hTargetEnt->IsAlive())
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing(FALSE);
				break;
			}
			else
			{
				if (HasConditions(bits_COND_CLIENT_PUSH))
				{
					return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}
				return GetScheduleOfType(SCHED_TARGET_FACE);
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH))
		{
			return GetScheduleOfType(SCHED_MOVE_AWAY);
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}

	// no special cases here, call the base class
	return CSquadTalkMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHGruntAllyTorch::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		if (InSquad())
		{
			if (g_iSkillLevel == SKILL_HARD && HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(ENT(pev), "FG_THROW", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return slGruntTossGrenadeCover;
			}
			else
			{
				return &slGruntTakeCover[0];
			}
		}
		else
		{
			//if ( RANDOM_LONG(0,1) )
			//{
			return &slGruntTakeCover[0];
			//}
			//else
			//{
			//	return &slGruntGrenadeCover[ 0 ];
			//}
		}
	}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
	{
		return &slGruntTakeCoverFromBestSound[0];
	}
	case SCHED_TORCH_ALLY_TAKECOVER_FAILED:
	{
		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1) && OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
		{
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}

		return GetScheduleOfType(SCHED_FAIL);
	}
	break;
	case SCHED_TORCH_ALLY_ELOF_FAIL:
	{
		// human grunt is unable to move to a position that allows him to attack the enemy.
		return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
	}
	break;
	case SCHED_TORCH_ALLY_ESTABLISH_LINE_OF_FIRE:
	{
		return &slGruntEstablishLineOfFire[0];
	}
	break;
	case SCHED_RANGE_ATTACK1:
	{
		// randomly stand or crouch
		if (RANDOM_LONG(0, 9) == 0)
			m_fStanding = RANDOM_LONG(0, 1);

		if (m_fStanding)
			return &slGruntRangeAttack1B[0];
		else
			return &slGruntRangeAttack1A[0];
	}
	case SCHED_RANGE_ATTACK2:
	{
		return &slGruntRangeAttack2[0];
	}
	case SCHED_COMBAT_FACE:
	{
		return &slGruntCombatFace[0];
	}
	case SCHED_TORCH_ALLY_WAIT_FACE_ENEMY:
	{
		return &slGruntWaitInCover[0];
	}
	case SCHED_TORCH_ALLY_SWEEP:
	{
		return &slGruntSweep[0];
	}
	case SCHED_TORCH_ALLY_COVER_AND_RELOAD:
	{
		return &slGruntHideReload[0];
	}
	case SCHED_TORCH_ALLY_FOUND_ENEMY:
	{
		return &slGruntFoundEnemy[0];
	}
	case SCHED_VICTORY_DANCE:
	{
		if (InSquad())
		{
			if (!IsLeader())
			{
				return &slGruntFail[0];
			}
		}

		return &slGruntVictoryDance[0];
	}

	case SCHED_TORCH_ALLY_SUPPRESS:
	{
		if (m_hEnemy->IsPlayer() && m_fFirstEncounter)
		{
			m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
			return &slGruntSignalSuppress[0];
		}
		else
		{
			return &slGruntSuppress[0];
		}
	}
	case SCHED_FAIL:
	{
		if (m_hEnemy != NULL)
		{
			// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
			return &slGruntCombatFail[0];
		}

		return &slGruntFail[0];
	}
	case SCHED_TORCH_ALLY_REPEL:
	{
		if (pev->velocity.z > -128)
			pev->velocity.z -= 32;
		return &slGruntRepel[0];
	}
	case SCHED_TORCH_ALLY_REPEL_ATTACK:
	{
		if (pev->velocity.z > -128)
			pev->velocity.z -= 32;
		return &slGruntRepelAttack[0];
	}
	case SCHED_TORCH_ALLY_REPEL_LAND:
	{
		return &slGruntRepelLand[0];
	}

	case SCHED_TARGET_CHASE:
		return slBaFollow;

	case SCHED_TARGET_FACE:
	{
		auto pSchedule = CSquadTalkMonster::GetScheduleOfType(SCHED_TARGET_FACE);

		if (pSchedule == slIdleStand)
			return slBaFaceTarget;
		return pSchedule;
	}

	case SCHED_IDLE_STAND:
	{
		auto pSchedule = CSquadTalkMonster::GetScheduleOfType(SCHED_IDLE_STAND);

		if (pSchedule == slIdleStand)
			return slIdleBaStand;
		return pSchedule;
	}

	//These exist in the original code, but are never used
	//schedule IDs are wrong because the ids are different
	/*
case 43:
	return &slGruntFail[ 0 ];
	*/

	default:
	{
		return CSquadTalkMonster::GetScheduleOfType(Type);
	}
	}
}

int CHGruntAllyTorch::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}

void CHGruntAllyTorch::TalkInit()
{
	CSquadTalkMonster::TalkInit();

	m_szGrp[TLK_ANSWER] = "FG_ANSWER";
	m_szGrp[TLK_QUESTION] = "FG_QUESTION";
	m_szGrp[TLK_IDLE] = "FG_IDLE";
	m_szGrp[TLK_STARE] = "FG_STARE";
	m_szGrp[TLK_USE] = "FG_OK";
	m_szGrp[TLK_UNUSE] = "FG_WAIT";
	m_szGrp[TLK_STOP] = "FG_STOP";

	m_szGrp[TLK_NOSHOOT] = "FG_SCARED";
	m_szGrp[TLK_HELLO] = "FG_HELLO";

	m_szGrp[TLK_PLHURT1] = "!FG_CUREA";
	m_szGrp[TLK_PLHURT2] = "!FG_CUREB";
	m_szGrp[TLK_PLHURT3] = "!FG_CUREC";

	m_szGrp[TLK_PHELLO] = NULL;	//"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = NULL;	//"BA_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "FG_PQUEST";		// UNDONE

	m_szGrp[TLK_SMELL] = "FG_SMELL";

	m_szGrp[TLK_WOUND] = "FG_WOUND";
	m_szGrp[TLK_MORTAL] = "FG_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

void CHGruntAllyTorch::AlertSound()
{
	if (m_hEnemy && FOkToSpeak())
	{
		PlaySentence("FG_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
	}
}

void CHGruntAllyTorch::DeclineFollowing()
{
	PlaySentence("FG_POK", 2, VOL_NORM, ATTN_NORM);
}

void CHGruntAllyTorch::Killed(entvars_t* pevAttacker, int iGib)
{
	if (m_hTargetEnt != nullptr)
	{
		m_hTargetEnt.Entity<CSquadTalkMonster>()->m_hWaitMedic = nullptr;
	}

	if (m_MonsterState != MONSTERSTATE_DEAD)
	{
		if (HasMemory(bits_MEMORY_SUSPICIOUS) || IsFacing(pevAttacker, pev->origin))
		{
			Remember(bits_MEMORY_PROVOKED);

			StopFollowing(true);
		}
	}

	if (m_hWaitMedic)
	{
		auto v4 = m_hWaitMedic.Entity<CSquadTalkMonster>();
		if (v4->pev->deadflag)
			m_hWaitMedic = nullptr;
		else
			v4->HealMe(nullptr);
	}

	SetUse(nullptr);

	if (m_fTorchActive)
	{
		m_fTorchActive = false;
		UTIL_Remove(m_pTorchBeam);
		m_pTorchBeam = nullptr;
	}

	CSquadTalkMonster::Killed(pevAttacker, iGib);
}

void CHGruntAllyTorch::MonsterThink()
{
	if (m_fTorchActive && m_pTorchBeam)
	{
		Vector vecTorchPos;
		Vector vecTorchAng;
		Vector vecEndPos;
		Vector vecEndAng;

		GetAttachment(2, vecTorchPos, vecTorchAng);
		GetAttachment(3, vecEndPos, vecEndAng);

		TraceResult tr;
		UTIL_TraceLine(vecTorchPos, (vecEndPos - vecTorchPos).Normalize() * 4 + vecTorchPos, ignore_monsters, edict(), &tr);

		if (tr.flFraction != 1.0)
		{
			m_pTorchBeam->pev->spawnflags &= ~SF_BEAM_SPARKSTART;
			//TODO: looks like a bug to me, shouldn't be bitwise inverting
			m_pTorchBeam->pev->spawnflags |= ~SF_BEAM_SPARKEND;

			UTIL_DecalTrace(&tr, RANDOM_LONG(0, 4));
			m_pTorchBeam->DoSparks(tr.vecEndPos, tr.vecEndPos);
		}

		m_pTorchBeam->SetBrightness(RANDOM_LONG(192, 255));
	}

	CSquadTalkMonster::MonsterThink();
}

//=========================================================
// CHGruntAllyTorchRepel - when triggered, spawns a monster_human_grunt_ally
// repelling down a line.
//=========================================================

class CHGruntAllyTorchRepel : public CBaseMonster
{
public:
	void KeyValue(KeyValueData* pkvd) override;

	void Spawn(void);
	void Precache(void);
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture;	// Don't save, precache

	//TODO: needs save/restore (not in op4)
	int m_iszUse;
	int m_iszUnUse;
};

LINK_ENTITY_TO_CLASS(monster_torch_ally_repel, CHGruntAllyTorchRepel);

void CHGruntAllyTorchRepel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "UseSentence"))
	{
		m_iszUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "UnUseSentence"))
	{
		m_iszUnUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CHGruntAllyTorchRepel::Spawn(void)
{
	Precache();
	pev->solid = SOLID_NOT;

	SetUse(&CHGruntAllyTorchRepel::RepelUse);
}

void CHGruntAllyTorchRepel::Precache(void)
{
	//TODO: needs to precache torch
	UTIL_PrecacheOther("monster_human_grunt_ally");
	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");
}

void CHGruntAllyTorchRepel::RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP)
		return NULL;
	*/

	//TODO: needs to be torch
	CBaseEntity* pEntity = Create("monster_human_grunt_ally", pev->origin, pev->angles);
	auto pGrunt = static_cast<CHGruntAllyTorch*>(pEntity->MySquadTalkMonsterPointer());

	if (pGrunt)
	{
		pGrunt->pev->weapons = pev->weapons;
		pGrunt->pev->netname = pev->netname;

		//Carry over these spawn flags
		pGrunt->pev->spawnflags |= pev->spawnflags
			& (SF_MONSTER_WAIT_TILL_SEEN
				| SF_MONSTER_GAG
				| SF_MONSTER_HITMONSTERCLIP
				| SF_MONSTER_PRISONER
				| SF_SQUADMONSTER_LEADER
				| SF_MONSTER_PREDISASTER);

		pGrunt->m_iszUse = m_iszUse;
		pGrunt->m_iszUnUse = m_iszUnUse;

		//Run logic to set up body groups (Spawn was already called once by Create above)
		pGrunt->Spawn();

		pGrunt->pev->movetype = MOVETYPE_FLY;
		pGrunt->pev->velocity = Vector(0, 0, RANDOM_FLOAT(-196, -128));
		pGrunt->SetActivity(ACT_GLIDE);
		// UNDONE: position?
		pGrunt->m_vecLastPosition = tr.vecEndPos;

		CBeam* pBeam = CBeam::BeamCreate("sprites/rope.spr", 10);
		pBeam->PointEntInit(pev->origin + Vector(0, 0, 112), pGrunt->entindex());
		pBeam->SetFlags(BEAM_FSOLID);
		pBeam->SetColor(255, 255, 255);
		pBeam->SetThink(&CBeam::SUB_Remove);
		pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

		UTIL_Remove(this);
	}
}
