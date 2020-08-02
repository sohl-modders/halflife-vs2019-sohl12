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
#ifndef CHASSASSIN_H
#define CHASSASSIN_H

#define	ASSASSIN_AE_SHOOT1	1
#define	ASSASSIN_AE_TOSS1	2
#define	ASSASSIN_AE_JUMP	3

#define bits_MEMORY_BADJUMP		(bits_MEMORY_CUSTOM1)

class CHAssassin : public CBaseMonster
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int Classify(void) override;
	int ISoundMask(void) override;
	void Shoot(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	Schedule_t* GetSchedule(void) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override; // jump
	// BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1(float flDot, float flDist) override; // shoot
	BOOL CheckRangeAttack2(float flDot, float flDist) override; // throw grenade
	void StartTask(Task_t* pTask) override;
	void RunAI(void) override;
	void RunTask(Task_t* pTask) override;
	void DeathSound(void) override;
	void IdleSound(void) override;
	CUSTOM_SCHEDULES;

	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector m_vecTossVelocity;
	BOOL m_fThrowGrenade;

	int m_iTargetRanderamt;

	int m_iFrustration;

	int m_iShell;
};

#endif
