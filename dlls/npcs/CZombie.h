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
#ifndef CZOMBIE_H
#define CZOMBIE_H

#define	ZOMBIE_AE_ATTACK_RIGHT		0x01
#define	ZOMBIE_AE_ATTACK_LEFT		0x02
#define	ZOMBIE_AE_ATTACK_BOTH		0x03

#define ZOMBIE_FLINCH_DELAY			2		// at most one flinch every n secs

class CZombie : public CBaseMonster
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int Classify(void) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	int IgnoreConditions(void) override;

	float m_flNextFlinch;

	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;
	void AttackSound(void);

	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

	// No range attacks
	BOOL CheckRangeAttack1(float flDot, float flDist) override { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) override { return FALSE; }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
};

#endif
