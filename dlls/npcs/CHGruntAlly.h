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

#ifndef CHGRUNTALLY_H
#define CHGRUNTALLY_H

#ifndef SCHEDULE_H
#include "schedule.h"
#endif

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	GRUNT_MP5_CLIP_SIZE				36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GRUNT_SHOTGUN_CLIP_SIZE			8
#define GRUNT_SAW_CLIP_SIZE				36

namespace HGruntAllyWeaponFlag
{
	enum HGruntAllyWeaponFlag
	{
		MP5 = 1 << 0,
		HandGrenade = 1 << 1,
		GrenadeLauncher = 1 << 2,
		Shotgun = 1 << 3,
		Saw = 1 << 4
	};
}

namespace HGruntAllyBodygroup
{
	enum HGruntAllyBodygroup
	{
		Head = 1,
		Torso = 2,
		Weapons = 3
	};
}

namespace HGruntAllyHead
{
	enum HGruntAllyHead
	{
		Default = -1,
		GasMask = 0,
		BeretWhite,
		OpsMask,
		BandanaWhite,
		BandanaBlack,
		MilitaryPolice,
		Commander,
		BeretBlack,
	};
}

namespace HGruntAllyTorso
{
	enum HGruntAllyTorso
	{
		Normal = 0,
		Saw,
		Nothing,
		Shotgun
	};
}

namespace HGruntAllyWeapon
{
	enum HGruntAllyWeapon
	{
		MP5 = 0,
		Shotgun,
		Saw,
		None
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	HGRUNT_ALLY_AE_RELOAD		( 2 )
#define	HGRUNT_ALLY_AE_KICK			( 3 )
#define	HGRUNT_ALLY_AE_BURST1		( 4 )
#define	HGRUNT_ALLY_AE_BURST2		( 5 )
#define	HGRUNT_ALLY_AE_BURST3		( 6 )
#define	HGRUNT_ALLY_AE_GREN_TOSS	( 7 )
#define	HGRUNT_ALLY_AE_GREN_LAUNCH	( 8 )
#define	HGRUNT_ALLY_AE_GREN_DROP	( 9 )
#define	HGRUNT_ALLY_AE_CAUGHT_ENEMY	( 10 )
#define	HGRUNT_ALLY_AE_DROP_GUN		( 11 )

//=========================================================
// Class definition of CHGruntAlly
//=========================================================
class CHGruntAlly : public CSquadTalkMonster
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int Classify(void) override;
	int ISoundMask(void) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL FCanCheckAttacks(void) override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CheckAmmo(void) override;
	void SetActivity(Activity NewActivity) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void DeathSound(void) override;
	void PainSound(void) override;
	void IdleSound(void) override;
	Vector GetGunPosition(void) override;
	void Shoot(void);
	void Shotgun(void);
	void PrescheduleThink(void) override;
	void GibMonster(void) override;
	void SpeakSentence(void);

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	CBaseEntity* Kick(void);
	Schedule_t* GetSchedule(void) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr,
	                 int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	BOOL FOkToSpeak(void);
	void JustSpoke(void);

	int ObjectCaps() override;

	void TalkInit() override;

	void AlertSound() override;

	void DeclineFollowing() override;

	void ShootSaw();

	void KeyValue(KeyValueData* pkvd) override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

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

	int m_iBrassShell;
	int m_iShotgunShell;
	int m_iSawShell;
	int m_iSawLink;

	int m_iSentence;

	int m_iWeaponIdx;
	int m_iGruntHead;
	int m_iGruntTorso;

	static const char* pGruntSentences[];
};

#endif // CHGRUNTALLY_H
