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

#ifndef CDECORETORCHFLAME_H
#define CDECORETORCHFLAME_H

class CDecoreTorchFlame : public CSprite
{
public:
	void Spawn() override;

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	static CDecoreTorchFlame* SpriteCreate(const Vector& origin);

private:
	float	m_lastTime;
	float	m_maxFrame;
};

#endif	// CDECORETORCHFLAME_H
