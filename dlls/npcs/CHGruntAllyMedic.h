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

#ifndef CHGRUNTALLYMEDIC_H
#define CHGRUNTALLYMEDIC_H

#ifndef SCHEDULE_H
#include "schedule.h"
#endif

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	MEDIC_DEAGLE_CLIP_SIZE			9 // how many bullets in a clip?
#define	MEDIC_GLOCK_CLIP_SIZE			9 // how many bullets in a clip?

#define	MEDIC_SENTENCE_VOLUME	(float)0.35 // volume of grunt sentences
#define TORCH_BEAM_SPRITE "sprites/xbeam3.spr"

namespace MedicAllyBodygroup
{
	enum MedicAllyBodygroup
	{
		Head = 2,
		Weapons = 3
	};
}

namespace MedicAllyHead
{
	enum MedicAllyHead
	{
		Default = -1,
		White = 0,
		Black
	};
}

namespace MedicAllyWeapon
{
	enum MedicAllyWeapon
	{
		DesertEagle = 0,
		Glock,
		Needle,
		None
	};
}

namespace MedicAllyWeaponFlag
{
	enum MedicAllyWeaponFlag
	{
		DesertEagle = 1 << 0,
		Glock = 1 << 1,
		Needle = 1 << 2,
		HandGrenade = 1 << 3,
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	MEDIC_AE_RELOAD			( 2 )
#define	MEDIC_AE_KICK			( 3 )
#define	MEDIC_AE_SHOOT			( 4 )
#define	MEDIC_AE_GREN_TOSS		( 7 )
#define	MEDIC_AE_GREN_DROP		( 9 )
#define	MEDIC_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define	MEDIC_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define MEDIC_AE_HOLSTER_GUN	15
#define MEDIC_AE_EQUIP_NEEDLE	16
#define MEDIC_AE_HOLSTER_NEEDLE	17
#define MEDIC_AE_EQUIP_GUN		18

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_MEDIC_ALLY_SUPPRESS = LAST_TALKMONSTER_SCHEDULE + 1,
	SCHED_MEDIC_ALLY_ESTABLISH_LINE_OF_FIRE,
	// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_MEDIC_ALLY_COVER_AND_RELOAD,
	SCHED_MEDIC_ALLY_SWEEP,
	SCHED_MEDIC_ALLY_FOUND_ENEMY,
	SCHED_MEDIC_ALLY_REPEL,
	SCHED_MEDIC_ALLY_REPEL_ATTACK,
	SCHED_MEDIC_ALLY_REPEL_LAND,
	SCHED_MEDIC_ALLY_WAIT_FACE_ENEMY,
	SCHED_MEDIC_ALLY_TAKECOVER_FAILED,
	// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_MEDIC_ALLY_HEAL_ALLY,
	SCHED_MEDIC_ALLY_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_MEDIC_ALLY_FACE_TOSS_DIR = LAST_TALKMONSTER_TASK + 1,
	TASK_MEDIC_ALLY_SPEAK_SENTENCE,
	TASK_MEDIC_ALLY_CHECK_FIRE,
};

//=========================================================
// Class definition of CHGruntAllyMedic
//=========================================================
class CHGruntAllyMedic : public CSquadTalkMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	int ISoundMask() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL FCanCheckAttacks() override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CheckAmmo() override;
	void SetActivity(Activity NewActivity) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void DeathSound() override;
	void PainSound() override;
	void IdleSound() override;
	Vector GetGunPosition() override;
	void Shoot();
	void PrescheduleThink() override;
	void GibMonster() override;
	void SpeakSentence();

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	CBaseEntity* Kick();
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
	                 int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	BOOL FOkToSpeak();
	void JustSpoke();

	int ObjectCaps() override { return FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE; };

	void TalkInit() override;

	void AlertSound() override;

	void DeclineFollowing() override;

	void KeyValue(KeyValueData* pkvd) override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void MonsterThink() override;

	BOOL HealMe(CSquadTalkMonster* pTarget);

	void HealOff();

	void HealerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void HealerActivate(CBaseMonster* pTarget);

	MONSTERSTATE GetIdealState() override
	{
		return CSquadTalkMonster::GetIdealState();
	}

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector m_vecTossVelocity;

	BOOL m_fThrowGrenade;
	BOOL m_fStanding;
	BOOL m_fFirstEncounter; // only put on the handsign show in the squad's first encounter.
	int m_cClipSize;

	int m_iHealCharge;
	BOOL m_fUseHealing;
	BOOL m_fHealing;

	float m_flLastUseTime;

	EHANDLE m_hNewTargetEnt;

	BOOL m_fQueueFollow;
	BOOL m_fHealAudioPlaying;

	float m_flFollowCheckTime;
	BOOL m_fFollowChecking;
	BOOL m_fFollowChecked;

	float m_flLastRejectAudio;

	int m_iBlackOrWhite;

	BOOL m_fGunHolstered;
	BOOL m_fHypoHolstered;
	BOOL m_fHealActive;

	int m_iWeaponIdx;

	float m_flLastShot;

	int m_iBrassShell;

	int m_iSentence;

	static const char* pMedicSentences[];
};

#endif // CHGRUNTALLYMEDIC_H
