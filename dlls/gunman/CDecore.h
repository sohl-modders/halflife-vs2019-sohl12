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

#ifndef CDECORE_H
#define CDECORE_H

class CDecore : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void Animate( float frames );
	void Touch( CBaseEntity *pOther ) override;
	int Classify() override { return CLASS_NONE; }
	
	float m_maxFrame;
};

#endif	// CDECORE_H
