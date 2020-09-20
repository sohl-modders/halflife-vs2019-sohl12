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
#include "squadmonster.h"
#include "weapons.h"
#include "talkmonster.h"
#include "soundent.h"
#include "customentity.h"
#include "Tasks.h"

#include "CMaleAssassin.h"

extern DLL_GLOBAL int g_iSkillLevel;

//=========================================================
// Link
//=========================================================
LINK_ENTITY_TO_CLASS(monster_male_assassin, CMaleAssassin);

//=========================================================
// Save and Restore
//=========================================================
TYPEDESCRIPTION CMaleAssassin::m_SaveData[] =
{
	DEFINE_FIELD(CMaleAssassin, m_flNextGrenadeCheck, FIELD_TIME),
	DEFINE_FIELD(CMaleAssassin, m_flNextPainTime, FIELD_TIME),
	DEFINE_FIELD(CMaleAssassin, m_vecTossVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CMaleAssassin, m_fThrowGrenade, FIELD_BOOLEAN),
	DEFINE_FIELD(CMaleAssassin, m_fStanding, FIELD_BOOLEAN),
	DEFINE_FIELD(CMaleAssassin, m_fFirstEncounter, FIELD_BOOLEAN),
	DEFINE_FIELD(CMaleAssassin, m_cClipSize, FIELD_INTEGER),
	DEFINE_FIELD(CMaleAssassin, m_voicePitch, FIELD_INTEGER),
	DEFINE_FIELD(CMaleAssassin, m_iSentence, FIELD_INTEGER),
	DEFINE_FIELD(CMaleAssassin, m_iAssassinHead, FIELD_INTEGER),
	DEFINE_FIELD(CMaleAssassin, m_flLastShot, FIELD_TIME),
	DEFINE_FIELD(CMaleAssassin, m_fStandingGround, FIELD_BOOLEAN),
	DEFINE_FIELD(CMaleAssassin, m_flStandGroundRange, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CMaleAssassin, CSquadMonster);

//=========================================================
// Sentences
//=========================================================
const char* CMaleAssassin::pAssassinSentences[] =
{
	"MSS_GREN", // grenade scared grunt
	"MSS_ALERT", // sees player
	"MSS_MONSTER", // sees monster
	"MSS_COVER", // running to cover
	"MSS_THROW", // about to throw grenade
	"MSS_CHARGE", // running out to get the enemy
	"MSS_TAUNT", // say rude things
};

enum MASSASSIN_SENTENCE_TYPES
{
	MASSASSIN_SENT_NONE = -1,
	MASSASSIN_SENT_GREN = 0,
	MASSASSIN_SENT_ALERT,
	MASSASSIN_SENT_MONSTER,
	MASSASSIN_SENT_COVER,
	MASSASSIN_SENT_THROW,
	MASSASSIN_SENT_CHARGE,
	MASSASSIN_SENT_TAUNT,
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
void CMaleAssassin::SpeakSentence()
{
	if (m_iSentence == MASSASSIN_SENT_NONE)
	{
		// no sentence cued up.
		return;
	}

	if (FOkToSpeak())
	{
		SENTENCEG_PlayRndSz(ENT(pev), pAssassinSentences[m_iSentence], MASSASSIN_SENTENCE_VOLUME, GRUNT_ATTN, 0,
		                    m_voicePitch);
		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are 
// Human Grunt's nemesis.
//=========================================================
int CMaleAssassin::IRelationship(CBaseEntity* pTarget)
{
	//LRC- only hate alien grunts if my behaviour hasn't been overridden
	if (!m_iClass && FClassnameIs(pTarget->pev, "monster_alien_grunt") ||
		(FClassnameIs(pTarget->pev, "monster_gargantua")))
	{
		return R_NM;
	}

	return CSquadMonster::IRelationship(pTarget);
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CMaleAssassin::GibMonster()
{
	Vector vecGunPos;
	Vector vecGunAngles;

	if (GetBodygroup(MAssassinBodygroup::Weapons) != MAssassinWeapon::None
		&& !(pev->spawnflags & SF_MONSTER_NO_WPN_DROP))
	{
		// throw a gun if the grunt has one
		GetAttachment(0, vecGunPos, vecGunAngles);

		CBaseEntity* pGun;
		if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
		{
			pGun = DropItem("weapon_9mmAR", vecGunPos, vecGunAngles);
		}
		else
		{
			pGun = DropItem("weapon_sniperrifle", vecGunPos, vecGunAngles);
		}

		if (pGun)
		{
			pGun->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
			pGun->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
		}

		if (FBitSet(pev->weapons, MAssassinWeaponFlag::GrenadeLauncher))
		{
			pGun = DropItem("ammo_ARgrenades", vecGunPos, vecGunAngles);
			if (pGun)
			{
				pGun->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
				pGun->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
			}
		}
	}

	CBaseMonster::GibMonster();
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CMaleAssassin::ISoundMask()
{
	return bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
BOOL CMaleAssassin::FOkToSpeak()
{
	// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkMonster::g_talkWaitTime)
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
void CMaleAssassin::JustSpoke()
{
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = MASSASSIN_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CMaleAssassin::PrescheduleThink()
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
BOOL CMaleAssassin::FCanCheckAttacks()
{
	if (!HasConditions(bits_COND_ENEMY_TOOFAR))
		return TRUE;

	return FALSE;
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CMaleAssassin::CheckMeleeAttack1(float flDot, float flDist)
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
BOOL CMaleAssassin::CheckRangeAttack1(float flDot, float flDist)
{
	if (pev->weapons)
	{
		if (m_fStandingGround && m_flStandGroundRange >= flDist)
		{
			m_fStandingGround = false;
		}

		if (!HasConditions(bits_COND_ENEMY_OCCLUDED) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire())
		{
			TraceResult tr;

			if (!m_hEnemy->IsPlayer() && flDist <= 64)
			{
				// kick nonclients, but don't shoot at them.
				return FALSE;
			}

			Vector vecSrc = GetGunPosition();

			// verify that a bullet fired from the gun will hit the enemy before the world.
			UTIL_TraceLine(vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

			if (tr.flFraction == 1.0)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack. 
//=========================================================
BOOL CMaleAssassin::CheckRangeAttack2(float flDot, float flDist)
{
	if (!FBitSet(pev->weapons, (MAssassinWeaponFlag::HandGrenade | MAssassinWeaponFlag::GrenadeLauncher)))
	{
		return FALSE;
	}

	// if the grunt isn't moving, it's ok to check.
	if (m_flGroundSpeed)
	{
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck)
	{
		return m_fThrowGrenade;
	}

	if (!FBitSet(m_hEnemy->pev->flags, FL_ONGROUND) && (m_hEnemy->pev->waterlevel == 0 || m_hEnemy->pev->watertype ==
		CONTENT_FOG) && m_vecEnemyLKP.z > pev->absmax.z)
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	Vector vecTarget;

	if (FBitSet(pev->weapons, MAssassinWeaponFlag::HandGrenade))
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
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.massassinGrenadeSpeed) * m_hEnemy->
				pev->velocity;
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

	if (FBitSet(pev->weapons, MAssassinWeaponFlag::HandGrenade))
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
		Vector vecToss = VecCheckThrow(pev, GetGunPosition(), vecTarget, gSkillData.massassinGrenadeSpeed, 0.5);

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
void CMaleAssassin::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
                                int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}

	CSquadMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CMaleAssassin::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Forget(bits_MEMORY_INCOVER);

	return CSquadMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMaleAssassin::SetYawSpeed()
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
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CMaleAssassin::CheckAmmo()
{
	if (pev->weapons && m_cAmmoLoaded <= 0)
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CMaleAssassin::Classify()
{
	return m_iClass ? m_iClass : CLASS_HUMAN_MILITARY;
}

//=========================================================
// Kick
//=========================================================
CBaseEntity* CMaleAssassin::Kick()
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
Vector CMaleAssassin::GetGunPosition()
{
	if (m_fStanding)
		return pev->origin + Vector(0, 0, 60);
	
	return pev->origin + Vector(0, 0, 48);
}

//=========================================================
// Shoot
//=========================================================
void CMaleAssassin::Shoot()
{
	if (m_hEnemy == nullptr && m_pCine == nullptr) //LRC - scripts may fire when you have no enemy
		return;

	if (FBitSet(pev->weapons, MAssassinWeaponFlag::SniperRifle) && gpGlobals->time - m_flLastShot <= 0.11)
		return;

	const Vector vecShootOrigin = GetGunPosition();
	const Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	if (m_cAmmoLoaded > 0)
	{
		UTIL_MakeVectors(pev->angles);

		if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
		{
			const Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) +
				gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
			EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
			FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5);
			// shoot +-5 degrees
		}
		else
		{
			//TODO: why is this 556? is 762 too damaging?
			//	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 2048, BULLET_PLAYER_556);
		}

		pev->effects |= EF_MUZZLEFLASH;

		m_cAmmoLoaded--; // take away a bullet!
	}

	const Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CMaleAssassin::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
		case MASSASSIN_AE_DROP_GUN:
			{
				Vector vecGunPos;
				Vector vecGunAngles;

				GetAttachment(0, vecGunPos, vecGunAngles);

				// switch to body group with no gun.
				SetBodygroup(MAssassinBodygroup::Weapons, MAssassinWeapon::None);

				// now spawn a gun.
				if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
				{
					DropItem("weapon_9mmAR", vecGunPos, vecGunAngles);
				}
				else
				{
					DropItem("weapon_sniperrifle", vecGunPos, vecGunAngles);
				}
				
				if (FBitSet(pev->weapons, MAssassinWeaponFlag::GrenadeLauncher))
				{
					DropItem("ammo_ARgrenades", BodyTarget(pev->origin), vecGunAngles);
				}
			}
		break;
		case MASSASSIN_AE_RELOAD:
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM);
			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
		break;
		case MASSASSIN_AE_GREN_TOSS:
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
		case MASSASSIN_AE_GREN_LAUNCH:
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
		case MASSASSIN_AE_GREN_DROP:
			{
				UTIL_MakeVectors(pev->angles);
				CGrenade::ShootTimed(
					pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero,
					3);
			}
		break;
		case MASSASSIN_AE_BURST1:
			{
				Shoot();

				if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
				{
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
				else
				{
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sniper_fire.wav", 1, ATTN_NORM);
				}

				CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);
			}
		break;
		case MASSASSIN_AE_BURST2:
		case MASSASSIN_AE_BURST3:
			Shoot();
		break;
		case MASSASSIN_AE_KICK:
			{
				CBaseEntity* pHurt = Kick();

				if (pHurt)
				{
					// SOUND HERE!
					UTIL_MakeVectors(pev->angles);
					pHurt->pev->punchangle.x = 15;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
					pHurt->TakeDamage(pev, pev, gSkillData.massassinDmgKick, DMG_CLUB);
				}
			}
		break;
		default:
			CSquadMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CMaleAssassin::Spawn()
{
	Precache();

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/massn.mdl");

	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;

	if (pev->health == 0)
		pev->health = gSkillData.massassinHealth;
	
	m_flFieldOfView = 0.2; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime = gpGlobals->time;
	m_iSentence = MASSASSIN_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded = FALSE;
	m_fFirstEncounter = TRUE; // this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	if (pev->weapons == 0)
	{
		// initialize to original values
		pev->weapons = MAssassinWeaponFlag::MP5 | MAssassinWeaponFlag::HandGrenade;
		// pev->weapons = HGRUNT_SHOTGUN;
		// pev->weapons = HGRUNT_9MMAR | HGRUNT_GRENADELAUNCHER;
	}

	if (m_iAssassinHead == MAssassinHead::Random)
	{
		m_iAssassinHead = RANDOM_LONG(MAssassinHead::White, MAssassinHead::ThermalVision);
	}

	int weaponModel;

	if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
	{
		weaponModel = MAssassinWeapon::MP5;
		m_cClipSize = MASSASSIN_MP5_CLIP_SIZE;
	}
	else if (FBitSet(pev->weapons, MAssassinWeaponFlag::SniperRifle))
	{
		weaponModel = MAssassinWeapon::SniperRifle;
		m_cClipSize = MASSASSIN_SNIPER_CLIP_SIZE;
	}
	else
	{
		weaponModel = MAssassinWeapon::None;
		m_cClipSize = 0;
	}

	SetBodygroup(MAssassinBodygroup::Heads, m_iAssassinHead);
	SetBodygroup(MAssassinBodygroup::Weapons, weaponModel);

	m_cAmmoLoaded = m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	m_fStandingGround = m_flStandGroundRange != 0;

	CTalkMonster::g_talkWaitTime = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMaleAssassin::Precache()
{
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	else
		PRECACHE_MODEL("models/massn.mdl");

	PRECACHE_SOUND("weapons/dryfire1.wav"); //LRC

	PRECACHE_SOUND("hgrunt/gr_mgun1.wav");
	PRECACHE_SOUND("hgrunt/gr_mgun2.wav");

	PRECACHE_SOUND("hgrunt/gr_die1.wav");
	PRECACHE_SOUND("hgrunt/gr_die2.wav");
	PRECACHE_SOUND("hgrunt/gr_die3.wav");

	PRECACHE_SOUND("hgrunt/gr_reload1.wav");

	PRECACHE_SOUND("weapons/glauncher.wav");

	PRECACHE_SOUND("weapons/sniper_fire.wav");

	PRECACHE_SOUND("zombie/claw_miss2.wav"); // because we use the basemonster SWIPE animation event

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;

	m_iBrassShell = PRECACHE_MODEL("models/shell.mdl"); // brass shell
}

//=========================================================
// KeyValue
//=========================================================
void CMaleAssassin::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq("head", pkvd->szKeyName))
	{
		m_iAssassinHead = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (FStrEq("standgroundrange", pkvd->szKeyName))
	{
		m_flStandGroundRange = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CSquadMonster::KeyValue(pkvd);
}

//=========================================================
// start task
//=========================================================
void CMaleAssassin::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
		case TASK_MASSASSIN_CHECK_FIRE:
			if (!NoFriendlyFire())
			{
				SetConditions(bits_COND_GRUNT_NOFIRE);
			}
			TaskComplete();
		break;
		case TASK_MASSASSIN_SPEAK_SENTENCE:
			//Assassins don't talk
			//SpeakSentence();
			TaskComplete();
		break;
		case TASK_WALK_PATH:
		case TASK_RUN_PATH:
			// grunt no longer assumes he is covered if he moves
			Forget(bits_MEMORY_INCOVER);
			CSquadMonster::StartTask(pTask);
		break;
		case TASK_RELOAD:
			m_IdealActivity = ACT_RELOAD;
		break;
		case TASK_FACE_IDEAL:
		case TASK_FACE_ENEMY:
			CSquadMonster::StartTask(pTask);
			if (pev->movetype == MOVETYPE_FLY)
			{
				m_IdealActivity = ACT_GLIDE;
			}
		break;
		default:
			CSquadMonster::StartTask(pTask);
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CMaleAssassin::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
		case TASK_MASSASSIN_FACE_TOSS_DIR:
			{
				// project a point along the toss vector and turn to face that point.
				MakeIdealYaw(pev->origin + m_vecTossVelocity * 64);
				ChangeYaw(pev->yaw_speed);

				if (FacingIdeal())

					m_iTaskStatus = TASKSTATUS_COMPLETE;
				break;
			}
		default:
			{
				CSquadMonster::RunTask(pTask);
				break;
			}
	}
}

//=========================================================
// DeathSound 
//=========================================================
void CMaleAssassin::DeathSound()
{
	switch (RANDOM_LONG(0, 2))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_IDLE);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_IDLE);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_IDLE);
		break;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
DEFINE_CUSTOM_SCHEDULES(CMaleAssassin)
{
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

IMPLEMENT_CUSTOM_SCHEDULES(CMaleAssassin, CSquadMonster);

//=========================================================
// SetActivity 
//=========================================================
void CMaleAssassin::SetActivity(Activity NewActivity)
{
	int iSequence;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		//Sniper uses the same set
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
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if (pev->weapons & MAssassinWeaponFlag::HandGrenade)
		{
			// get toss anim
			iSequence = LookupSequence("throwgrenade");
		}
		else
		{
			// get launch anim
			iSequence = LookupSequence("launchgrenade");
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
Schedule_t* CMaleAssassin::GetSchedule()
{
	// clear old sentence
	m_iSentence = MASSASSIN_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if (pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE)
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType(SCHED_MASSASSIN_REPEL_LAND);
		}
		
		// repel down a rope, 
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return GetScheduleOfType(SCHED_MASSASSIN_REPEL_ATTACK);
		
		return GetScheduleOfType(SCHED_MASSASSIN_REPEL);
	}

	// grunts place HIGH priority on running away from danger sounds.
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound = PBestSound();

		ASSERT(pSound != NULL);
		if (pSound)
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
		}
	}
	
	switch (m_MonsterState)
	{
		case MONSTERSTATE_COMBAT:
			{
				// dead enemy
				if (HasConditions(bits_COND_ENEMY_DEAD))
				{
					// call base class, all code to handle dead enemies is centralized there.
					return CBaseMonster::GetSchedule();
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
						
						if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
						{
							return GetScheduleOfType(SCHED_MASSASSIN_SUPPRESS);
						}
						
						return GetScheduleOfType(SCHED_MASSASSIN_ESTABLISH_LINE_OF_FIRE);
					}
				}
					// no ammo
				else if (HasConditions(bits_COND_NO_AMMO_LOADED))
				{
					//!!!KELLY - this individual just realized he's out of bullet ammo. 
					// He's going to try to find cover to run to and reload, but rarely, if 
					// none is available, he'll drop and reload in the open here. 
					return GetScheduleOfType(SCHED_MASSASSIN_COVER_AND_RELOAD);
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

				else if (FBitSet(pev->weapons, MAssassinWeaponFlag::GrenadeLauncher) &&
					HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
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
							return GetScheduleOfType(SCHED_MASSASSIN_FOUND_ENEMY);
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
						return GetScheduleOfType(SCHED_RANGE_ATTACK2);
					}
					
					if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
					{
						if (FOkToSpeak())
							m_iSentence = MASSASSIN_SENT_CHARGE;

						return GetScheduleOfType(SCHED_MASSASSIN_ESTABLISH_LINE_OF_FIRE);
					}
					return GetScheduleOfType(SCHED_STANDOFF);
				}

				if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1))
				{
					return GetScheduleOfType(SCHED_MASSASSIN_ESTABLISH_LINE_OF_FIRE);
				}
			}
	}

	// no special cases here, call the base class
	return CSquadMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CMaleAssassin::GetScheduleOfType(int Type)
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
					return slGruntTossGrenadeCover;
				}
				return &slGruntTakeCover[0];
			}
			if (RANDOM_LONG(0, 1))
			{
				return &slGruntTakeCover[0];
			}
			return &slGruntGrenadeCover[0];
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slGruntTakeCoverFromBestSound[0];
		}
	case SCHED_MASSASSIN_TAKECOVER_FAILED:
		{
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK1) && OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
			{
				return GetScheduleOfType(SCHED_RANGE_ATTACK1);
			}

			return GetScheduleOfType(SCHED_FAIL);
		}
		break;
	case SCHED_MASSASSIN_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}
		break;
	case SCHED_MASSASSIN_ESTABLISH_LINE_OF_FIRE:
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
	case SCHED_MASSASSIN_WAIT_FACE_ENEMY:
		{
			return &slGruntWaitInCover[0];
		}
	case SCHED_MASSASSIN_SWEEP:
		{
			return &slGruntSweep[0];
		}
	case SCHED_MASSASSIN_COVER_AND_RELOAD:
		{
			return &slGruntHideReload[0];
		}
	case SCHED_MASSASSIN_FOUND_ENEMY:
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
	case SCHED_MASSASSIN_SUPPRESS:
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
	case SCHED_MASSASSIN_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepel[0];
		}
	case SCHED_MASSASSIN_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepelAttack[0];
		}
	case SCHED_MASSASSIN_REPEL_LAND:
		{
			return &slGruntRepelLand[0];
		}
	default:
		{
			return CSquadMonster::GetScheduleOfType(Type);
		}
	}
}