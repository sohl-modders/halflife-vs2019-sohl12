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

#ifndef CMALEASSASSIN_H
#define CMALEASSASSIN_H

#ifndef SCHEDULE_H
#include "schedule.h"
#endif

#ifndef CHGRUNT_H
#include "CHGrunt.h"
#endif

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	MASSASSIN_MP5_CLIP_SIZE			36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define MASSASSIN_SNIPER_CLIP_SIZE		1
#define MASSASSIN_NUM_HEADS				2 // how many grunt heads are there? 
#define	MASSASSIN_SENTENCE_VOLUME		(float)0.35 // volume of grunt sentences

namespace MAssassinBodygroup
{
	enum MAssassinBodygroup
	{
		Heads = 1,
		Weapons = 2
	};
}

namespace MAssassinHead
{
	enum MAssassinHead
	{
		Random = -1,
		White = 0,
		Black,
		ThermalVision
	};
}

namespace MAssassinWeapon
{
	enum MAssassinWeapon
	{
		MP5 = 0,
		SniperRifle,
		None
	};
}

namespace MAssassinWeaponFlag
{
	enum MAssassinWeaponFlag
	{
		MP5 = 1 << 0,
		HandGrenade = 1 << 1,
		GrenadeLauncher = 1 << 2,
		SniperRifle = 1 << 3,
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	MASSASSIN_AE_RELOAD			( 2 )
#define	MASSASSIN_AE_KICK			( 3 )
#define	MASSASSIN_AE_BURST1			( 4 )
#define	MASSASSIN_AE_BURST2			( 5 ) 
#define	MASSASSIN_AE_BURST3			( 6 ) 
#define	MASSASSIN_AE_GREN_TOSS		( 7 )
#define	MASSASSIN_AE_GREN_LAUNCH	( 8 )
#define	MASSASSIN_AE_GREN_DROP		( 9 )
#define	MASSASSIN_AE_DROP_GUN		( 11 )

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_MASSASSIN_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_MASSASSIN_ESTABLISH_LINE_OF_FIRE,
	// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_MASSASSIN_COVER_AND_RELOAD,
	SCHED_MASSASSIN_SWEEP,
	SCHED_MASSASSIN_FOUND_ENEMY,
	SCHED_MASSASSIN_REPEL,
	SCHED_MASSASSIN_REPEL_ATTACK,
	SCHED_MASSASSIN_REPEL_LAND,
	SCHED_MASSASSIN_WAIT_FACE_ENEMY,
	SCHED_MASSASSIN_TAKECOVER_FAILED,
	// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_MASSASSIN_ELOF_FAIL,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_MASSASSIN_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,
	TASK_MASSASSIN_SPEAK_SENTENCE,
	TASK_MASSASSIN_CHECK_FIRE,
};

//=========================================================
// Class definition of CMaleAssassin
//=========================================================
class CMaleAssassin : public CSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
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
	void PainSound() override {}
	void IdleSound() override {}
	
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

	int IRelationship(CBaseEntity* pTarget) override;

	BOOL FOkToSpeak();
	void JustSpoke();

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

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

	float m_flLastShot;
	BOOL m_fStandingGround;
	float m_flStandGroundRange;

	int m_voicePitch;

	int m_iBrassShell;
	int m_iShotgunShell;

	int m_iSentence;

	int m_iAssassinHead;

	static const char* pAssassinSentences[];
};

#endif // CMALEASSASSIN_H
