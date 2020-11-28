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

#ifndef CCYCLER_H
#define CCYCLER_H

class CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(const char* szModel, Vector vecMin, Vector vecMax);
	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void Spawn(void);
	void Think(void);
	//void Pain( float flDamage );
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	// Don't treat as a live target
	virtual BOOL IsAlive(void) { return FALSE; }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	int			m_animate;
};

#endif
