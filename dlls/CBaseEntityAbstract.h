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

#ifndef	CBASEENTITYABSTRACT_H
#define	CBASEENTITYABSTRACT_H

class CBaseEntityAbstract
{
public:
	// Constructor.  Set engine to use C/C++ callback functions
	virtual ~CBaseEntityAbstract() = default;

	// pointers to engine data
	entvars_t* pev; // Don't need to save/restore this pointer, the engine resets it

	/**
	*	@return This entity's edict.
	*/
	edict_t* edict() const { return ENT(pev); }

	/**
	*	@copydoc edict() const
	*/
	edict_t* edict() { return ENT(pev); }

	/**
	*	@return This entity's classname.
	*/
	const char* GetClassname() const { return STRING(pev->classname); }

	/**
	*	Sets this entity's classname.
	*	It is assumed that pszClassName is either a string in the program's string table or allocated using ALLOC_STRING,
	*	or otherwise has a lifetime that is at least as long as the rest of the map.
	*/
	void SetClassname(const char* pszClassName)
	{
		pev->classname = MAKE_STRING(pszClassName);
	}

	/**
	*	@return Whether this entity's classname matches the given classname.
	*/
	bool ClassnameIs(const char* const pszClassName) const
	{
		return FStrEq(GetClassname(), pszClassName);
	}

	/**
	*	@copydoc ClassnameIs( const char* const pszClassName ) const
	*/
	bool ClassnameIs(const string_t iszClassName) const
	{
		return ClassnameIs(STRING(iszClassName));
	}

	/**
	*	@return Whether this entity has a global name.
	*/
	bool HasGlobalName() const
	{
		return !!(*STRING(pev->globalname));
	}

	/**
	*	@return The global name.
	*/
	const char* GetGlobalName() const { return STRING(pev->globalname); }

	/**
	*	Sets the global name.
	*	@param iszGlobalName Name to set.
	*/
	void SetGlobalName(const string_t iszGlobalName)
	{
		pev->globalname = iszGlobalName;
	}

	/**
	*	Sets the global name.
	*	@param pszGlobalName Name to set.
	*/
	void SetGlobalName(const char* const pszGlobalName)
	{
		SetGlobalName(MAKE_STRING(pszGlobalName));
	}

	/**
	*	Clears the global name.
	*/
	void ClearGlobalName()
	{
		pev->globalname = iStringNull;
	}

	/**
	*	@return Whether this entity has a targetname.
	*/
	bool HasTargetname() const
	{
		return !!(*STRING(pev->targetname));
	}

	/**
	*	@return The targetname.
	*/
	const char* GetTargetname() const { return STRING(pev->targetname); }

	/**
	*	Sets the targetname.
	*	@param iszTargetName Name to set.
	*/
	void SetTargetname(const string_t iszTargetName)
	{
		pev->targetname = iszTargetName;
	}

	/**
	*	Sets the targetname.
	*	@param pszTargetName Name to set.
	*/
	void SetTargetname(const char* const pszTargetName)
	{
		SetTargetname(MAKE_STRING(pszTargetName));
	}

	/**
	*	Clears the targetname.
	*/
	void ClearTargetname()
	{
		pev->targetname = iStringNull;
	}

	/**
	*	@return Whether this entity has a target.
	*/
	bool HasTarget() const
	{
		return !!(*STRING(pev->target));
	}

	/**
	*	@return The target.
	*/
	const char* GetTarget() const { return STRING(pev->target); }

	/**
	*	Sets the target.
	*	@param iszTarget Target to set.
	*/
	void SetTarget(const string_t iszTarget)
	{
		pev->target = iszTarget;
	}

	/**
	*	Sets the target.
	*	@param pszTarget Target to set.
	*/
	void SetTarget(const char* const pszTarget)
	{
		SetTarget(MAKE_STRING(pszTarget));
	}

	/**
	*	Clears the target.
	*/
	void ClearTarget()
	{
		pev->target = iStringNull;
	}

	/**
	*	@return Whether this entity has a net name.
	*/
	bool HasNetName() const
	{
		return !!(*STRING(pev->netname));
	}

	/**
	*	@return The net name.
	*/
	const char* GetNetName() const { return STRING(pev->netname); }

	/**
	*	Sets the net name.
	*	@param iszNetName Net name to set.
	*/
	void SetNetName(const string_t iszNetName)
	{
		pev->netname = iszNetName;
	}

	/**
	*	Sets the net name.
	*	@param pszNetName Name to set.
	*/
	void SetNetName(const char* const pszNetName)
	{
		SetNetName(MAKE_STRING(pszNetName));
	}

	/**
	*	Clears the net name.
	*/
	void ClearNetName()
	{
		pev->netname = iStringNull;
	}

	/**
	*	@return The absolute origin.
	*/
	const Vector& GetAbsOrigin() const { return pev->origin; }

	/**
	*	Sets the absolute origin.
	*	@param vecOrigin Origin to set.
	*/
	void SetAbsOrigin(const Vector& vecOrigin)
	{
		pev->origin = vecOrigin;
	}

	/**
	*	@return The old origin.
	*/
	const Vector& GetOldOrigin() const { return pev->oldorigin; }

	/**
	*	@return The old origin, in mutable form.
	*/
	Vector& GetMutableOldOrigin() { return pev->oldorigin; }

	/**
	*	Sets the old origin.
	*	@param vecOrigin Origin to set.
	*/
	void SetOldOrigin(const Vector& vecOrigin)
	{
		pev->oldorigin = vecOrigin;
	}

	/**
	*	@return The absolute velocity.
	*/
	const Vector& GetAbsVelocity() const { return pev->velocity; }

	/**
	*	Sets the absolute velocity.
	*	@param vecVelocity Velocity to set.
	*/
	void SetAbsVelocity(const Vector& vecVelocity)
	{
		pev->velocity = vecVelocity;
	}

	/**
	*	@return The base velocity.
	*/
	const Vector& GetBaseVelocity() const { return pev->basevelocity; }

	/**
	*	Sets the base velocity.
	*	@param vecVelocity Velocity to set.
	*/
	void SetBaseVelocity(const Vector& vecVelocity)
	{
		pev->basevelocity = vecVelocity;
	}

	/**
	*	@return This entity's move direction.
	*/
	const Vector& GetMoveDir() const { return pev->movedir; }

	/**
	*	Sets the move direction.
	*	@param vecMoveDir Move direction to set.
	*/
	void SetMoveDir(const Vector& vecMoveDir)
	{
		pev->movedir = vecMoveDir;
	}

	/**
	*	@return The absolute angles.
	*/
	const Vector& GetAbsAngles() const { return pev->angles; }

	/**
	*	Sets the absolute angles.
	*	@param vecAngles Angles to set.
	*/
	void SetAbsAngles(const Vector& vecAngles)
	{
		pev->angles = vecAngles;
	}

	/**
	*	@return The angular velocity.
	*/
	const Vector& GetAngularVelocity() const { return pev->avelocity; }

	/**
	*	Sets the angular velocity.
	*	@param vecAVelocity Angular velocity to set.
	*/
	void SetAngularVelocity(const Vector& vecAVelocity)
	{
		pev->avelocity = vecAVelocity;
	}

	/**
	*	@return The punch angle.
	*/
	const Vector& GetPunchAngle() const { return pev->punchangle; }

	/**
	*	Sets the punch angle.
	*	@param vecPunchAngle Punch angle to set.
	*/
	void SetPunchAngle(const Vector& vecPunchAngle)
	{
		pev->punchangle = vecPunchAngle;
	}

	/**
	*	@return The view angle.
	*/
	const Vector& GetViewAngle() const { return pev->v_angle; }

	/**
	*	Sets the view angle.
	*	@param vecViewAngle View angle to set.
	*/
	void SetViewAngle(const Vector& vecViewAngle)
	{
		pev->v_angle = vecViewAngle;
	}

	/**
	*	@return The ideal pitch.
	*/
	float GetIdealPitch() const { return pev->idealpitch; }

	/**
	*	Sets the ideal pitch.
	*	@param flIdealPitch Ideal pitch to set.
	*/
	void SetIdealPitch(const float flIdealPitch)
	{
		pev->idealpitch = flIdealPitch;
	}

	/**
	*	@return The pitch speed.
	*/
	float GetPitchSpeed() const { return pev->pitch_speed; }

	/**
	*	Sets the pitch speed.
	*	@param flPitchSpeed Pitch speed to set.
	*/
	void SetPitchSpeed(const float flPitchSpeed)
	{
		pev->pitch_speed = flPitchSpeed;
	}

	/**
	*	@return The ideal yaw.
	*/
	float GetIdealYaw() const { return pev->ideal_yaw; }

	/**
	*	Sets the ideal yaw.
	*	@param flIdealYaw Ideal yaw to set.
	*/
	void SetIdealYaw(const float flIdealYaw)
	{
		pev->ideal_yaw = flIdealYaw;
	}

	/**
	*	@return The pitch speed.
	*/
	float GetYawSpeed() const { return pev->yaw_speed; }

	/**
	*	Sets the yaw speed.
	*	@param flYawSpeed Yaw speed to set.
	*/
	void SetYawSpeed(const float flYawSpeed)
	{
		pev->yaw_speed = flYawSpeed;
	}

	/**
	*	@return The model index.
	*/
	int GetModelIndex() const { return pev->modelindex; }

	/**
	*	Sets the model index.
	*	@param iModelIndex Model index to set.
	*/
	void SetModelIndex(const int iModelIndex)
	{
		pev->modelindex = iModelIndex;
	}

	/**
	*	@return Whether this entity has a model.
	*/
	bool HasModel() const { return !!(*STRING(pev->model)); }

	/**
	*	@return The model's name.
	*/
	const char* GetModelName() const { return STRING(pev->model); }

	/**
	*	Sets the model name. Does not set the model itself.
	*	@param pszModelName Name of the model.
	*/
	void SetModelName(const char* const pszModelName)
	{
		pev->model = MAKE_STRING(pszModelName);
	}

	/**
	*	Sets the model name. Does not set the model itself.
	*	@param iszModelName Name of the model.
	*/
	void SetModelName(const string_t iszModelName)
	{
		pev->model = iszModelName;
	}

	/**
	*	Sets the model.
	*	@param pszModelName Name of the model.
	*/
	void SetModel(const char* pszModelName);

	/**
	*	Sets the model.
	*	@param iszModelName Name of the model.
	*/
	void SetModel(const string_t iszModelName)
	{
		SetModel(STRING(iszModelName));
	}

	/**
	*	Precache the model.
	*	@param pszModelName Name of the model.
	*/
	int PrecacheModel(const char* pszModelName);

	/**
	*	Precache the model.
	*	@param iszModelName Name of the model.
	*/
	int PrecacheModel(const string_t iszModelName)
	{
		return PrecacheModel((char*)STRING(iszModelName));
	}

	/**
	*	Precache sounds.
	*	@param pszModelName Name of the model.
	*/
	int PrecacheSound(const char* pszSoundName);

	/**
	*	Precache sounds.
	*	@param iszModelName Name of the model.
	*/
	int PrecacheSound(const string_t iszSoundName)
	{
		return PrecacheSound((char*)STRING(iszSoundName));
	}

	/**
	*	Precache events.
	*	@param type
	*	@param psz
	*/
	unsigned short PrecacheEvent(int type, const char* psz);

	unsigned short PrecacheEvent(const char* psz)
	{
		return PrecacheEvent(1, psz);
	}

	/**
	*	Clears the model.
	*/
	void ClearModel()
	{
		pev->model = iStringNull;
		SetModelIndex(0);
	}

	/**
	*	@return View offset.
	*/
	const Vector& GetViewOffset() const { return pev->view_ofs; }

	/**
	*	@return View offset, in mutable form.
	*/
	Vector& GetMutableViewOffset() { return pev->view_ofs; }

	/**
	*	Sets the view offset.
	*	@param vecViewOffset View offset to set.
	*/
	void SetViewOffset(const Vector& vecViewOffset)
	{
		pev->view_ofs = vecViewOffset;
	}

	/**
*	@return The movetype.
*/
	MoveType GetMoveType() const { return static_cast<MoveType>(pev->movetype); }

	/**
	*	Sets the movetype.
	*	@param moveType Movetype to set.
	*/
	void SetMoveType(const MoveType moveType)
	{
		pev->movetype = moveType;
	}

	/**
	*	@return The solid type.
	*/
	Solid GetSolidType() const { return static_cast<Solid>(pev->solid); }

	/**
	*	Sets the solid type.
	*	@param solidType Solid type to set.
	*/
	void SetSolidType(const Solid solidType)
	{
		pev->solid = solidType;
	}
};

#endif
