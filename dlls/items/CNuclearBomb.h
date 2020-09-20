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

#ifndef CNUCLEARBOMB_H
#define CNUCLEARBOMB_H

#ifndef CNUCLEARBOMBTIMER_H
#include "CNuclearBombTimer.h"
#endif

#ifndef CNUCLEARBOMBBUTTON_H
#include "CNuclearBombButton.h"
#endif

//=========================================================
// Class definition of CNuclearBomb
//=========================================================
class CNuclearBomb : public CBaseToggle
{
public:
	int	Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int ObjectCaps() override
	{
		return CBaseToggle::ObjectCaps() | FCAP_IMPULSE_USE;
	}

	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	CNuclearBombTimer* m_pTimer;
	CNuclearBombButton* m_pButton;
	
	BOOL m_fOn;
	float m_flLastPush;
	int m_iPushCount;
};

#endif
