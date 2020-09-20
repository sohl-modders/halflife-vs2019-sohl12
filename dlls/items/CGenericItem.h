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

#ifndef CGENERICITEM_H
#define CGENERICITEM_H

const auto SF_ITEMGENERIC_DROP_TO_FLOOR = 1 << 0;

//=========================================================
// Class definition of CGenericItem
//=========================================================
class CGenericItem : public CBaseAnimating
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;

	void EXPORT StartItem();
	void EXPORT AnimateThink();

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	float m_lastTime;
	int m_iSequence;
};

#endif // CGENERICITEM_H