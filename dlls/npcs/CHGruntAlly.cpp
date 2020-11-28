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
#include "animation.h"
#include "weapons.h"
#include "soundent.h"
#include "CAllyMonster.h"
#include "CSquadTalkMonster.h"
#include "scripted.h"

#include "CHGrunt.h"
#include "CHGruntAlly.h"


#include "defaultai.h"
#include "Tasks.h"

int g_fGruntAllyQuestion; // true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int g_iSkillLevel;

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_human_grunt_ally, CHGruntAlly);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CHGruntAlly::m_SaveData[] =
{
	DEFINE_FIELD(CHGruntAlly, m_flNextGrenadeCheck, FIELD_TIME),
	DEFINE_FIELD(CHGruntAlly, m_flNextPainTime, FIELD_TIME),
	DEFINE_FIELD(CHGruntAlly, m_vecTossVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CHGruntAlly, m_fThrowGrenade, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAlly, m_fStanding, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAlly, m_fFirstEncounter, FIELD_BOOLEAN),
	DEFINE_FIELD(CHGruntAlly, m_cClipSize, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAlly, m_voicePitch, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAlly, m_iSentence, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAlly, m_iWeaponIdx, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAlly, m_iGruntHead, FIELD_INTEGER),
	DEFINE_FIELD(CHGruntAlly, m_iGruntTorso, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CHGruntAlly, CSquadTalkMonster);

const char* CHGruntAlly::pGruntSentences[] =
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

enum HGRUNT_ALLY_SENTENCE_TYPES
{
	HGRUNT_SENT_NONE = -1,
	HGRUNT_SENT_GREN = 0,
	HGRUNT_SENT_ALERT,
	HGRUNT_SENT_MONSTER,
	HGRUNT_SENT_COVER,
	HGRUNT_SENT_THROW,
	HGRUNT_SENT_CHARGE,
	HGRUNT_SENT_TAUNT,
};

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
void CHGruntAlly::SpeakSentence()
{
	if (m_iSentence == HGRUNT_SENT_NONE)
	{
		// no sentence cued up.
		return;
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[m_iSentence], 
			HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		JustSpoke();
	}
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CHGruntAlly::GibMonster()
{
	Vector vecGunPos;
	Vector vecGunAngles;

	if (m_iWeaponIdx != HGruntAllyWeapon::None)
	{// throw a gun if the grunt has one
		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun;
		if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Shotgun))
		{
			pGun = DropItem("weapon_shotgun", vecGunPos, vecGunAngles);
		}
		else if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
		{
			pGun = DropItem("weapon_m249", vecGunPos, vecGunAngles);
		}
		else
		{
			pGun = DropItem("weapon_9mmAR", vecGunPos, vecGunAngles);
		}
		if (pGun)
		{
			pGun->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
			pGun->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
		}

		if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::GrenadeLauncher))
		{
			pGun = DropItem("ammo_ARgrenades", vecGunPos, vecGunAngles);
			if (pGun)
			{
				pGun->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
				pGun->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
			}
		}

		m_iWeaponIdx = HGruntAllyWeapon::None;
	}

	CBaseMonster::GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHGruntAlly::ISoundMask()
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
BOOL CHGruntAlly::FOkToSpeak()
{
	// if someone else is talking, don't speak
	if (gpGlobals->time <= g_talkWaitTime)
		return FALSE;

	if (pev->spawnflags & SF_MONSTER_GAG)
	{
		if (m_MonsterState != MONSTERSTATE_COMBAT)
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
//=========================================================
void CHGruntAlly::JustSpoke()
{
	g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = HGRUNT_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHGruntAlly::PrescheduleThink()
{
	if (InSquad() && m_hEnemy != nullptr)
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
BOOL CHGruntAlly::FCanCheckAttacks()
{
	if (!HasConditions(bits_COND_ENEMY_TOOFAR))
		return TRUE;

	return FALSE;
}

//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CHGruntAlly::CheckMeleeAttack1(float flDot, float flDist)
{
	CBaseMonster* pEnemy;

	if (m_hEnemy != nullptr)
	{
		pEnemy = m_hEnemy->MyMonsterPointer();

		if (!pEnemy)
		{
			return FALSE;
		}
	}

	if (flDist <= 64 && flDot >= 0.7 &&
		pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON) {
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
BOOL CHGruntAlly::CheckRangeAttack1(float flDot, float flDist)
{
	//Only if we have a weapon
	if (pev->weapons)
	{
		const auto maxDistance = pev->weapons & HGruntAllyWeaponFlag::Shotgun ? 640 : 1024;

		//Friendly fire is allowed
		if (!HasConditions(bits_COND_ENEMY_OCCLUDED) && flDist <= maxDistance && flDot >= 0.5)
		{
			TraceResult	tr;

			auto pEnemy = m_hEnemy.Entity<CBaseEntity>();

			if (!pEnemy->IsPlayer() && flDist <= 64)
			{
				// kick nonclients, but don't shoot at them.
				return FALSE;
			}

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
BOOL CHGruntAlly::CheckRangeAttack2(float flDot, float flDist)
{
	if (!FBitSet(pev->weapons, (HGruntAllyWeaponFlag::HandGrenade | HGruntAllyWeaponFlag::GrenadeLauncher)))
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

	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::HandGrenade))
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
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.hgruntAllyGrenadeSpeed) * m_hEnemy->pev->velocity;
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


	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::HandGrenade))
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
		Vector vecToss = VecCheckThrow(pev, GetGunPosition(), vecTarget, gSkillData.hgruntAllyGrenadeSpeed, 0.5);

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
void CHGruntAlly::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
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
int CHGruntAlly::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Forget(bits_MEMORY_INCOVER);

	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CSquadTalkMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	if (pev->deadflag != DEAD_NO)
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
void CHGruntAlly::SetYawSpeed()
{
	switch (m_Activity)
	{
		case ACT_IDLE:
		case ACT_RUN:
			pev->yaw_speed = 150;
		break;
		case ACT_RANGE_ATTACK1:
		case ACT_RANGE_ATTACK2:
		case ACT_MELEE_ATTACK1:
		case ACT_MELEE_ATTACK2:
			pev->yaw_speed = 120;
		break;
		case ACT_WALK:
		case ACT_TURN_LEFT:
		case ACT_TURN_RIGHT:
			pev->yaw_speed = 180;
		break;
		case ACT_GLIDE:
		case ACT_FLY:
			pev->yaw_speed = 30;
		break;
		default:
			pev->yaw_speed = 90;
		break;
	}
}

//=========================================================
// IdleSound
//=========================================================
void CHGruntAlly::IdleSound()
{
	if (FOkToSpeak() && (g_fGruntAllyQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fGruntAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntAllyQuestion = 0;
		}

		JustSpoke();
	}
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CHGruntAlly::CheckAmmo()
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
int CHGruntAlly::Classify()
{
	return m_iClass ? m_iClass : CLASS_HUMAN_MILITARY_FRIENDLY;
}

//=========================================================
// Kick
//=========================================================
CBaseEntity* CHGruntAlly::Kick()
{
	TraceResult tr;

	UTIL_MakeVectors(pev->angles);
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = Instance(tr.pHit);
		return pEntity;
	}

	return nullptr;
}

//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================

Vector CHGruntAlly::GetGunPosition()
{
	if (m_fStanding)
		return pev->origin + Vector(0, 0, 60);

	return pev->origin + Vector(0, 0, 48);
}

//=========================================================
// Shoot
//=========================================================
void CHGruntAlly::Shoot()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr) //LRC - scripts may fire when you have no enemy
		return;

	const Vector vecShootOrigin = GetGunPosition();
	const Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	if (m_cAmmoLoaded > 0)
	{
		UTIL_MakeVectors(pev->angles);

		const Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) +
			gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
		EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
		FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5);
		// shoot +-5 degrees

		pev->effects |= EF_MUZZLEFLASH;

		m_cAmmoLoaded--; // take away a bullet!
	}

	const Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
}

//=========================================================
// Shoot
//=========================================================
void CHGruntAlly::Shotgun()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr)
		return;

	const Vector vecShootOrigin = GetGunPosition();
	const Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	UTIL_MakeVectors(pev->angles);

	const Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) +
		gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell,
	           TE_BOUNCE_SHOTSHELL);
	FireBullets(gSkillData.hgruntShotgunPellets, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048,
	            BULLET_PLAYER_BUCKSHOT, 0); // shoot +-7.5 degrees

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--; // take away a bullet!

	const Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHGruntAlly::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
		case HGRUNT_AE_DROP_GUN:
			{
				if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) 
					break; //LRC

				Vector	vecGunPos;
				Vector	vecGunAngles;

				GetAttachment(0, vecGunPos, vecGunAngles);

				// switch to body group with no gun.
				SetBodygroup(HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::None);

				// now spawn a gun.
				if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Shotgun))
				{
					DropItem("weapon_shotgun", vecGunPos, vecGunAngles);
				}
				else if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
				{
					DropItem("weapon_m249", vecGunPos, vecGunAngles);
				}
				else
				{
					DropItem("weapon_9mmAR", vecGunPos, vecGunAngles);
				}

				if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::GrenadeLauncher))
				{
					DropItem("ammo_ARgrenades", BodyTarget(pev->origin), vecGunAngles);
				}

				m_iWeaponIdx = HGruntAllyWeapon::None;
			}
		break;
		case HGRUNT_AE_RELOAD:
			if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
			{
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/saw_reload.wav", 1, ATTN_NORM);
			}
			else
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM);

			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;
		case HGRUNT_AE_GREN_TOSS:
			{
				UTIL_MakeVectors(pev->angles);
				// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
				//LRC - a bit of a hack. Ideally the grunts would work out in advance whether it's ok to throw.
				if (m_pCine)
				{
					Vector vecToss = g_vecZero;
					if (m_hTargetEnt != NULL && m_pCine->PreciseAttack())
					{
						vecToss = VecCheckToss(pev, GetGunPosition(), m_hTargetEnt->pev->origin, 0.5);
					}
					if (vecToss == g_vecZero)
					{
						vecToss = (gpGlobals->v_forward * 0.5 + gpGlobals->v_up * 0.5).Normalize() * gSkillData.
							hgruntGrenadeSpeed;
					}
					CGrenade::ShootTimed(pev, GetGunPosition(), vecToss, 3.5);
				}
				else
					CGrenade::ShootTimed(pev, GetGunPosition(), m_vecTossVelocity, 3.5);

				m_fThrowGrenade = FALSE;
				m_flNextGrenadeCheck = gpGlobals->time + 6;
				// wait six seconds before even looking again to see if a grenade can be thrown.
				// !!!LATER - when in a group, only try to throw grenade if ordered.
			}
		break;
		case HGRUNT_AE_GREN_LAUNCH:
			{
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
				//LRC: firing due to a script?
				if (m_pCine)
				{
					Vector vecToss;
					if (m_hTargetEnt != NULL && m_pCine->PreciseAttack())
						vecToss = VecCheckThrow(pev, GetGunPosition(), m_hTargetEnt->pev->origin,
						                        gSkillData.hgruntGrenadeSpeed, 0.5);
					else
					{
						// just shoot diagonally up+forwards
						UTIL_MakeVectors(pev->angles);
						vecToss = (gpGlobals->v_forward * 0.5 + gpGlobals->v_up * 0.5).Normalize() * gSkillData.
							hgruntGrenadeSpeed;
					}
					CGrenade::ShootContact(pev, GetGunPosition(), vecToss);
				}
				else
					CGrenade::ShootContact(pev, GetGunPosition(), m_vecTossVelocity);

				m_fThrowGrenade = FALSE;

				if (g_iSkillLevel == SKILL_HARD)
					m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT(2, 5);
					// wait a random amount of time before shooting again
				else
					m_flNextGrenadeCheck = gpGlobals->time + 6;
				// wait six seconds before even looking again to see if a grenade can be thrown.
			}
		break;
		case HGRUNT_AE_GREN_DROP:
			{
				UTIL_MakeVectors(pev->angles);
				CGrenade::ShootTimed(
					pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero,
					3);
			}
		break;
		case HGRUNT_AE_BURST1:
			{
				if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::MP5))
				{
					Shoot();

					// the first round of the three round burst plays the sound and puts a sound in the world sound list.
					if (RANDOM_LONG(0, 1))
					{
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "hgrunt/gr_mgun1.wav", 1, ATTN_NORM);
					}
					else
					{
						EMIT_SOUND(ENT(pev), CHAN_WEAPON, "hgrunt/gr_mgun2.wav", 1, ATTN_NORM);
					}
				}
				else if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
				{
					ShootSaw();
				}
				else
				{
					Shotgun();

					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, ATTN_NORM);
				}

				CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);
			}
		break;
		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
			if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::MP5))
			{
				Shoot();
			}
			else if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
			{
				ShootSaw();
			}
		break;
		case HGRUNT_AE_KICK:
			{
				CBaseEntity* pHurt = Kick();

				if (pHurt)
				{
					// SOUND HERE!
					UTIL_MakeVectors(pev->angles);
					pHurt->pev->punchangle.x = 15;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
					pHurt->TakeDamage(pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB);
				}
			}
		break;
		case HGRUNT_AE_CAUGHT_ENEMY:
			{
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz(ENT(pev), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
void CHGruntAlly::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/hgrunt_opfor.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;

	if (pev->health == 0)
		pev->health = gSkillData.hgruntAllyHealth;

	m_flFieldOfView = 0.2; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime = gpGlobals->time;
	m_iSentence = HGRUNT_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	m_fEnemyEluded = FALSE;
	m_fFirstEncounter = TRUE; // this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;
	m_iGruntTorso = HGruntAllyTorso::Normal;

	if (pev->weapons & HGruntAllyWeaponFlag::MP5)
	{
		m_iWeaponIdx = HGruntAllyWeapon::MP5;
		m_cClipSize = GRUNT_MP5_CLIP_SIZE;
	}
	else if (pev->weapons & HGruntAllyWeaponFlag::Shotgun)
	{
		m_cClipSize = GRUNT_SHOTGUN_CLIP_SIZE;
		m_iWeaponIdx = HGruntAllyWeapon::Shotgun;
		m_iGruntTorso = HGruntAllyTorso::Shotgun;
	}
	else if (pev->weapons & HGruntAllyWeaponFlag::Saw)
	{
		m_iWeaponIdx = HGruntAllyWeapon::Saw;
		m_cClipSize = GRUNT_SAW_CLIP_SIZE;
		m_iGruntTorso = HGruntAllyTorso::Saw;
	}
	else
	{
		m_iWeaponIdx = HGruntAllyWeapon::None;
		m_cClipSize = 0;
	}

	m_cAmmoLoaded = m_cClipSize;

	if (m_iGruntHead == HGruntAllyHead::Default)
	{
		if (pev->spawnflags & SF_SQUADMONSTER_LEADER)
		{
			m_iGruntHead = HGruntAllyHead::BeretWhite;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::Shotgun)
		{
			m_iGruntHead = HGruntAllyHead::OpsMask;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::Saw)
		{
			m_iGruntHead = RANDOM_LONG(0, 1) + HGruntAllyHead::BandanaWhite;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::MP5)
		{
			m_iGruntHead = HGruntAllyHead::MilitaryPolice;
		}
		else
		{
			m_iGruntHead = HGruntAllyHead::GasMask;
		}
	}

	SetBodygroup(HGruntAllyBodygroup::Head, m_iGruntHead);
	SetBodygroup(HGruntAllyBodygroup::Torso, m_iGruntTorso);
	SetBodygroup(HGruntAllyBodygroup::Weapons, m_iWeaponIdx);

	//TODO: probably also needs this for head HGruntAllyHead::BeretBlack
	if (m_iGruntHead == HGruntAllyHead::OpsMask || m_iGruntHead == HGruntAllyHead::BandanaBlack)
		m_voicePitch = 90;

	pev->skin = 0;

	m_iSawShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iSawLink = PRECACHE_MODEL("models/saw_link.mdl");

	g_talkWaitTime = 0;

	m_flMedicWaitTime = gpGlobals->time;

	MonsterInit();

	SetUse(&CHGruntAlly::FollowerUse);
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHGruntAlly::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/hgrunt_opfor.mdl");

	TalkInit();

	PRECACHE_SOUND("weapons/dryfire1.wav"); //LRC

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

	PRECACHE_SOUND("hgrunt/gr_reload1.wav");

	PRECACHE_SOUND("weapons/saw_fire1.wav");
	PRECACHE_SOUND("weapons/saw_fire2.wav");
	PRECACHE_SOUND("weapons/saw_fire3.wav");
	PRECACHE_SOUND("weapons/saw_reload.wav");

	PRECACHE_SOUND("weapons/glauncher.wav");

	PRECACHE_SOUND("weapons/sbarrel1.wav");

	PRECACHE_SOUND("fgrunt/medic.wav");

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	// get voice pitch
	m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	CSquadTalkMonster::Precache();
}

//=========================================================
// start task
//=========================================================
void CHGruntAlly::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
		case TASK_GRUNT_CHECK_FIRE:
			if (!NoFriendlyFire(true))
			{
				SetConditions(bits_COND_GRUNT_NOFIRE);
			}
			TaskComplete();
		break;
		case TASK_GRUNT_SPEAK_SENTENCE:
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
void CHGruntAlly::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
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
void CHGruntAlly::PainSound()
{
	if (gpGlobals->time > m_flNextPainTime)
	{
#if 0
		if (RANDOM_LONG(0, 99) < 5)
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "HG_PAIN", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
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
void CHGruntAlly::DeathSound()
{
	//TODO: these sounds don't exist, the gr_ prefix is wrong
	switch (RANDOM_LONG(0, 5))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death1.wav", 1, ATTN_IDLE);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death2.wav", 1, ATTN_IDLE);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death3.wav", 1, ATTN_IDLE);
		break;
	case 3:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death4.wav", 1, ATTN_IDLE);
		break;
	case 4:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death5.wav", 1, ATTN_IDLE);
		break;
	case 5:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death6.wav", 1, ATTN_IDLE);
		break;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
DEFINE_CUSTOM_SCHEDULES(CHGruntAlly)
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

IMPLEMENT_CUSTOM_SCHEDULES(CHGruntAlly, CSquadTalkMonster);

//=========================================================
// SetActivity 
//=========================================================
void CHGruntAlly::SetActivity(Activity NewActivity)
{
	int iSequence;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet(pev->weapons, HGruntWeaponFlag::MP5))
		{
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
		}
		else
		{
			if (m_fStanding)
			{
				// get aimable sequence
				iSequence = LookupSequence("standing_shotgun");
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence("crouching_shotgun");
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if (pev->weapons & HGruntWeaponFlag::Handgrenade)
		{
			// get toss anim
			iSequence = LookupSequence("throwgrenade");
		}
			// LRC: added a test to stop a marine without a launcher from firing.
		else if (pev->weapons & HGruntWeaponFlag::GrenadeLauncher)
		{
			// get launch anim
			iSequence = LookupSequence("launchgrenade");
		}
		else
		{
			ALERT(at_debug, "No grenades available. "); // flow into the error message we get at the end...
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

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}
}

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t* CHGruntAlly::GetSchedule()
{
	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if (pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE)
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType(SCHED_GRUNT_REPEL_LAND);
		}
		
		// repel down a rope, 
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return GetScheduleOfType(SCHED_GRUNT_REPEL_ATTACK);
		
		return GetScheduleOfType(SCHED_GRUNT_REPEL);
	}

	// grunts place HIGH priority on running away from danger sounds.
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = PBestSound();

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
					SENTENCEG_PlayRndSz(ENT(pev), "HG_GREN", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
			}
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
			if (HasConditions(bits_COND_NEW_ENEMY))
			{
				if (InSquad())
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;

					if (!IsLeader())
					{
						return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
					}
					//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
					// monster and has made it the squad's enemy. You
					// can check pev->flags for FL_CLIENT to determine whether this is the player
					// or a monster. He's going to immediately start
					// firing, though. If you'd like, we can make an alternate "first sight" 
					// schedule where the leader plays a handsign anim
					// that gives us enough time to hear a short sentence or spoken command
					// before he starts pluggin away.
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
							// player
							SENTENCEG_PlayRndSz(ENT(pev), "FG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0,
							                    m_voicePitch);
						else if ((m_hEnemy != NULL) &&
							(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) &&
							(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) &&
							(m_hEnemy->Classify() != CLASS_MACHINE))
							// monster
							SENTENCEG_PlayRndSz(ENT(pev), "FG_MONST", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0,
							                    m_voicePitch);

						JustSpoke();
					}

					if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
					{
						return GetScheduleOfType(SCHED_GRUNT_SUPPRESS);
					}
					
					return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
				}
			}
			// no ammo
			else if (HasConditions(bits_COND_NO_AMMO_LOADED))
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType(SCHED_GRUNT_COVER_AND_RELOAD);
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
						//SENTENCEG_PlayRndSz( ENT(pev), "FG_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						m_iSentence = HGRUNT_SENT_COVER;
						//JustSpoke();
					}
					return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
				}
				return GetScheduleOfType(SCHED_SMALL_FLINCH);
			}
			// can kick
			else if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
			{
				return GetScheduleOfType(SCHED_MELEE_ATTACK1);
			}
			// can grenade launch
			else if (FBitSet(pev->weapons, HGruntWeaponFlag::GrenadeLauncher) && HasConditions(bits_COND_CAN_RANGE_ATTACK2) &&
				OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				// shoot a grenade if you can
				return GetScheduleOfType(SCHED_RANGE_ATTACK2);
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
						return GetScheduleOfType(SCHED_GRUNT_FOUND_ENEMY);
					}
				}

				if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType(SCHED_RANGE_ATTACK1);
				}
				
				if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
				{
					// throw a grenade if can and no engage slots are available
					return GetScheduleOfType(SCHED_RANGE_ATTACK2);
				}
				// hide!
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			}
			// can't see enemy
			else if (HasConditions(bits_COND_ENEMY_OCCLUDED))
			{
				if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
				{
					//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz(ENT(pev), "FG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return GetScheduleOfType(SCHED_RANGE_ATTACK2);
				}
				
				if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
				{
					//!!!KELLY - grunt cannot see the enemy and has just decided to 
					// charge the enemy's position. 
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						//SENTENCEG_PlayRndSz( ENT(pev), "HG_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						m_iSentence = HGRUNT_SENT_CHARGE;
						//JustSpoke();
					}

					return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
				}
				//!!!KELLY - grunt is going to stay put for a couple seconds to see if
				// the enemy wanders back out into the open, or approaches the
				// grunt's covered position. Good place for a taunt, I guess?
				if (FOkToSpeak() && RANDOM_LONG(0, 1))
				{
					SENTENCEG_PlayRndSz(ENT(pev), "FG_TAUNT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				
				return GetScheduleOfType(SCHED_STANDOFF);
			}

			//Only if not following a player
			if (!m_hTargetEnt || !m_hTargetEnt->IsPlayer())
			{
				if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1))
				{
					return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
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
Schedule_t* CHGruntAlly::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if (InSquad())
			{
				if (g_iSkillLevel == SKILL_HARD && HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(
					bits_SLOTS_HGRUNT_GRENADE))
				{
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz(ENT(pev), "FG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return slGruntTossGrenadeCover;
				}
				return &slGruntTakeCover[0];
			}
			
			if (OccupySlot(bits_SLOTS_HGRUNT_GRENADE) && RANDOM_LONG(0, 1))
			{
				return &slGruntGrenadeCover[0];
			}
			
			return &slGruntTakeCover[0];
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slGruntTakeCoverFromBestSound[0];
		}
	case SCHED_GRUNT_TAKECOVER_FAILED:
		{
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK1) && OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
			{
				return GetScheduleOfType(SCHED_RANGE_ATTACK1);
			}

			return GetScheduleOfType(SCHED_FAIL);
		}
		break;
	case SCHED_GRUNT_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}
		break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
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
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
		{
			return &slGruntWaitInCover[0];
		}
	case SCHED_GRUNT_SWEEP:
		{
			return &slGruntSweep[0];
		}
	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slGruntHideReload[0];
		}
	case SCHED_GRUNT_FOUND_ENEMY:
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
	case SCHED_GRUNT_SUPPRESS:
		{
			if (m_hEnemy->IsPlayer() && m_fFirstEncounter)
			{
				m_fFirstEncounter = FALSE;
				// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slGruntSignalSuppress[0];
			}
			return &slGruntSuppress[0];
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
	case SCHED_GRUNT_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepel[0];
		}
	case SCHED_GRUNT_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepelAttack[0];
		}
	case SCHED_GRUNT_REPEL_LAND:
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


	default:
		{
			return CSquadTalkMonster::GetScheduleOfType(Type);
		}
	}
}

int CHGruntAlly::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}

void CHGruntAlly::TalkInit()
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

void CHGruntAlly::AlertSound()
{
	if (m_hEnemy && FOkToSpeak())
	{
		PlaySentence("FG_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
	}
}

void CHGruntAlly::DeclineFollowing()
{
	PlaySentence("FG_POK", 2, VOL_NORM, ATTN_NORM);
}

void CHGruntAlly::ShootSaw()
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	UTIL_MakeVectors(pev->angles);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
	{
		auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(75, 200) + gpGlobals->v_up * RANDOM_FLOAT(150, 200) + gpGlobals->v_forward * 25.0;
		EjectBrass(vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawLink, TE_BOUNCE_SHELL);
		break;
	}

	case 1:
	{
		auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(100, 250) + gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25.0;
		EjectBrass(vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawShell, TE_BOUNCE_SHELL);
		break;
	}
	}

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 8192, BULLET_PLAYER_556, 2); // shoot +-5 degrees

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/saw_fire1.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG(0, 15) + 94); break;
	case 1: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/saw_fire2.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG(0, 15) + 94); break;
	case 2: EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/saw_fire3.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG(0, 15) + 94); break;
	}

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
}

void CHGruntAlly::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iGruntHead = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CSquadTalkMonster::KeyValue(pkvd);
}

void CHGruntAlly::Killed(entvars_t* pevAttacker, int iGib)
{
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
	CSquadTalkMonster::Killed(pevAttacker, iGib);
}