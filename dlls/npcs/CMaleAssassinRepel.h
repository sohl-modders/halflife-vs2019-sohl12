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

#ifndef CMALEASSASSINREPEL_H
#define CMALEASSASSINREPEL_H

//=========================================================
// Class definition of CMaleAssassinRepel
//=========================================================
class CMaleAssassinRepel : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	
	int m_iSpriteTexture; // Don't save, precache
};

#endif // CMALEASSASSINREPEL_H
