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

#ifndef COMMON_CBITSET_H
#include <CBitSet.h>
#endif

#ifndef COM_MODEL_H
#include <com_model.h>
#endif

#define DECLARE_CLASS_NOBASE( thisClass )	\
typedef thisClass ThisClass

#define DECLARE_CLASS( thisClass, baseClass )	\
DECLARE_CLASS_NOBASE( thisClass );				\
typedef baseClass BaseClass

using CEntBitSet = CBitSet<int>;

class CBaseEntityAbstract
{
public:
	int	m_iLFlags; // LRC- a new set of flags. (pev->spawnflags and pev->flags are full...)

public:
	CBaseEntityAbstract() {}

	virtual ~CBaseEntityAbstract() = default;

	/**
	*	Called when the entity is first created. - Solokiller
	*/
	virtual void OnCreate() {}

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
	*	@return The fixangle mode.
	*	@see FixAngleMode
	*/
	FixAngleMode GetFixAngleMode() const { return static_cast<FixAngleMode>(pev->fixangle); }

	/**
	*	Sets the fixangle mode.
	*	@param mode Mode.
	*	@see FixAngleMode
	*/
	void SetFixAngleMode(const FixAngleMode mode)
	{
		pev->fixangle = mode;
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
*	@brief Whether the weaponmodel name is set
*/
	bool HasWeaponModelName() const { return !!(*STRING(pev->weaponmodel)); }

	/**
	*	@return The third person weapon model name.
	*/
	const char* GetWeaponModelName() const { return STRING(pev->weaponmodel); }

	/**
	*	Sets the weapon model name.
	*	@param pszModelName Model name.
	*/
	void SetWeaponModelName(const char* const pszModelName)
	{
		pev->weaponmodel = MAKE_STRING(pszModelName);
	}

	/**
	*	Sets the weapon model name.
	*	@param iszModelName Model name.
	*/
	void SetWeaponModelName(const string_t iszModelName)
	{
		pev->weaponmodel = iszModelName;
	}

	/**
	*	Clears the weapon model name.
	*/
	void ClearWeaponModelName()
	{
		pev->weaponmodel = iStringNull;
	}

	/**
	*	@return The weapon animation.
	*/
	int GetWeaponAnim() const { return pev->weaponanim; }

	/**
	*	Sets the weapon animation.
	*	@param iWeaponAnim Weapon animation to set.
	*/
	void SetWeaponAnim(const int iWeaponAnim)
	{
		pev->weaponanim = iWeaponAnim;
	}

	/**
	*	@return Whether the player is ducking.
	*/
	bool IsDucking() const
	{
		return pev->bInDuck != 0;
	}

	/**
	*	Sets whether the player is ducking.
	*	@param bDucking Duck state to set.
	*/
	void SetDucking(const bool bDucking)
	{
		pev->bInDuck = bDucking;
	}

	/**
	*	@return The time at which a step sound was last played.
	*/
	int GetStepSoundTime() const { return pev->flTimeStepSound; }

	/**
	*	Sets the time at which a step sound was last played.
	*	@param iTime Time to set.
	*/
	void SetStepSoundTime(const int iTime)
	{
		pev->flTimeStepSound = iTime;
	}

	/**
	*	@return The time at which a swim sound was last played.
	*/
	int GetSwimSoundTime() const { return pev->flSwimTime; }

	/**
	*	Sets the time at which a swim sound was last played.
	*	@param iTime Time to set.
	*/
	void SetSwimSoundTime(const int iTime)
	{
		pev->flSwimTime = iTime;
	}

	/**
	*	@return The time at which a duck was last initiated
	*/
	int GetDuckTime() const { return pev->flDuckTime; }

	/**
	*	Sets the time at which a duck was last initiated
	*	@param iTime Time to set
	*/
	void SetDuckTime(const int iTime)
	{
		pev->flDuckTime = iTime;
	}

	/**
	*	@brief Whether the player is currently jumping out of water, and how much time is left until the jump completes (in milliseconds)
	*/
	float GetWaterJumpTime() const { return pev->teleport_time; }

	/**
	*	@see GetWaterJumpTime
	*/
	void SetWaterJumpTime(const float flTime)
	{
		pev->teleport_time = flTime;
	}

	/**
	*	@return Whether the last step was a left step sound.
	*/
	bool IsStepLeft() const
	{
		return pev->iStepLeft != 0;
	}

	/**
	*	Steps whether the last step was a left step sound.
	*	@param bStepLeft State to set.
	*/
	void SetStepLeft(const bool bStepLeft)
	{
		pev->iStepLeft = bStepLeft;
	}

	/**
	*	@return Fall velocity.
	*/
	float GetFallVelocity() const { return pev->flFallVelocity; }

	/**
	*	Sets the fall velocity.
	*	@param flFallVelocity Fall velocity to set.
	*/
	void SetFallVelocity(const float flFallVelocity)
	{
		pev->flFallVelocity = flFallVelocity;
	}

	/**
	*	@return The absolute minimum bounds.
	*/
	const Vector& GetAbsMin() const { return pev->absmin; }

	/**
	*	Sets the absolute minimum bounds.
	*	@param vecMin Minimum bounds to set.
	*/
	void SetAbsMin(const Vector& vecMin)
	{
		pev->absmin = vecMin;
	}

	/**
	*	@return The absolute maximum bounds.
	*/
	const Vector& GetAbsMax() const { return pev->absmax; }

	/**
	*	Sets the absolute maximum bounds.
	*	@param vecMax Maximum bounds to set.
	*/
	void SetAbsMax(const Vector& vecMax)
	{
		pev->absmax = vecMax;
	}

	/**
	*	@return The relative minimum bounds.
	*/
	const Vector& GetRelMin() const { return pev->mins; }

	/**
	*	Sets the relative minimum bounds.
	*	@param vecMin Minimum bounds to set.
	*/
	void SetRelMin(const Vector& vecMin)
	{
		pev->mins = vecMin;
	}

	/**
	*	@return The relative maximum bounds.
	*/
	const Vector& GetRelMax() const { return pev->maxs; }

	/**
	*	Sets the relative maximum bounds.
	*	@param vecMax Maximum bounds to set.
	*/
	void SetRelMax(const Vector& vecMax)
	{
		pev->maxs = vecMax;
	}

	/**
	*	@return The entity's bounds.
	*/
	const Vector& GetBounds() const { return pev->size; }

	/**
	*	Sets the size. The size is centered around the entity's origin.
	*	@param vecSize Size to set.
	*/
	void SetSize(const Vector& vecSize)
	{
		SetSize(-(vecSize / 2), vecSize / 2);
	}

	/**
	*	Sets the size.
	*	@param vecMin Minimum bounds.
	*	@param vecMax Maximum bounds.
	*/
	void SetSize(const Vector& vecMin, const Vector& vecMax)
	{
		UTIL_SetSize(pev, vecMin, vecMax);
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

	/**
	*	@return The skin.
	*/
	int GetSkin() const { return pev->skin; }

	/**
	*	Sets the skin.
	*	@param iSkin Skin to set.
	*/
	void SetSkin(const int iSkin)
	{
		pev->skin = iSkin;
	}

	/**
	*	@return The body.
	*/
	int GetBody() const { return pev->body; }

	/**
	*	Sets the body.
	*	@param iBody Body to set.
	*/
	void SetBody(const int iBody)
	{
		pev->body = iBody;
	}

	/**
	*	@return The entity's effects flags.
	*/
	const CBitSet<int>& GetEffects() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->effects);
	}

	/**
	*	@copydoc GetEffects() const
	*/
	CBitSet<int>& GetEffects()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->effects);
	}

	/**
	*	@return The gravity multiplier.
	*/
	float GetGravity() const { return pev->gravity; }

	/**
	*	Sets the gravity multiplier.
	*	@param flGravity Gravity to set.
	*/
	void SetGravity(const float flGravity)
	{
		pev->gravity = flGravity;
	}

	/**
	*	@return The friction.
	*/
	float GetFriction() const { return pev->friction; }

	/**
	*	Sets the friction.
	*	@param flFriction Friction to set.
	*/
	void SetFriction(const float flFriction)
	{
		pev->friction = flFriction;
	}

	/**
	*	@return The light level.
	*/
	int GetLightLevel() const
	{
		//pev->lightlevel is not always the actual light level.
		return GETENTITYILLUM(const_cast<edict_t*>(edict()));
	}

	/**
	*	@return The sequence.
	*/
	int GetSequence() const { return pev->sequence; }

	/**
	*	Sets the sequence.
	*	@param iSequence Sequence to set.
	*/
	void SetSequence(const int iSequence)
	{
		pev->sequence = iSequence;
	}

	/**
	*	Movement animation sequence for player.
	*	@return The gait sequence.
	*/
	int GetGaitSequence() const { return pev->gaitsequence; }

	/**
	*	Sets the gait sequence.
	*	@param iGaitSequence Gait sequence to set.
	*/
	void SetGaitSequence(const int iGaitSequence)
	{
		pev->gaitsequence = iGaitSequence;
	}

	/**
	*	% playback position in animation sequences (0..255).
	*	@return The frame number, as a float.
	*/
	float GetFrame() const { return pev->frame; }

	/**
	*	Sets the frame number.
	*	@param flFrame Frame to set.
	*/
	void SetFrame(const float flFrame)
	{
		pev->frame = flFrame;
	}

	/**
	*	@return World time when the frame was changed.
	*/
	float GetAnimTime() const { return pev->animtime; }

	/**
	*	Sets the world time when the frame was changed.
	*	@param flAnimTime Time to set.
	*/
	void SetAnimTime(const float flAnimTime)
	{
		pev->animtime = flAnimTime;
	}

	/**
	*	@return The animation playback rate (-8x to 8x).
	*/
	float GetFrameRate() const { return pev->framerate; }

	/**
	*	Sets the animation playback rate.
	*	@param flFrameRate Frame rate to set.
	*/
	void SetFrameRate(const float flFrameRate)
	{
		pev->framerate = flFrameRate;
	}

	/**
	*	@return The sprite rendering scale (0..255).
	*/
	float GetScale() const { return pev->scale; }

	/**
	*	Sets the scale.
	*	@param flScale Scale to set.
	*/
	void SetScale(const float flScale)
	{
		pev->scale = flScale;
	}

	/**
	*	@return The render mode.
	*/
	RenderMode GetRenderMode() const
	{
		return static_cast<RenderMode>(pev->rendermode);
	}

	/**
	*	Sets the render mode.
	*	@param renderMode Render mode.
	*/
	void SetRenderMode(const RenderMode renderMode)
	{
		pev->rendermode = renderMode;
	}

	/**
	*	@return The render amount.
	*/
	float GetRenderAmount() const { return pev->renderamt; }

	/**
	*	Sets the render amount.
	*	@param flRenderAmount Render amount.
	*/
	void SetRenderAmount(const float flRenderAmount)
	{
		pev->renderamt = flRenderAmount;
	}

	/**
	*	@return The render color.
	*/
	const Vector& GetRenderColor() const { return pev->rendercolor; }

	/**
	*	Sets the render color.
	*	@param vecColor Render color to set.
	*/
	void SetRenderColor(const Vector& vecColor)
	{
		pev->rendercolor = vecColor;
	}

	/**
	*	@return The render FX.
	*/
	RenderFX GetRenderFX() const
	{
		return static_cast<RenderFX>(pev->renderfx);
	}

	/**
	*	Sets the render FX.
	*	@param renderFX Render FX to set.
	*/
	void SetRenderFX(const RenderFX renderFX)
	{
		pev->renderfx = renderFX;
	}

	/**
	*	@return Health amount.
	*/
	float GetHealth() const { return pev->health; }

	/**
	*	Sets the entity's health.
	*	@param flHealth Health amount to set.
	*/
	void SetHealth(const float flHealth)
	{
		if (pev->max_health == 0)
			pev->max_health = flHealth;
		
		pev->health = flHealth;
	}

	/**
	*	@return Maximum health.
	*/
	float GetMaxHealth() const { return pev->max_health; }

	/**
	*	Sets the maximum health.
	*	@param flMaxHealth Maximum health.
	*/
	void SetMaxHealth(const float flMaxHealth)
	{
		pev->max_health = flMaxHealth;
	}

	/**
	*	@return The armor amount.
	*/
	float GetArmorAmount() const { return pev->armorvalue; }

	/**
	*	Sets the armor amount.
	*	@param flArmorAmount Armor amount to set.
	*/
	void SetArmorAmount(const float flArmorAmount)
	{
		pev->armorvalue = flArmorAmount;
	}

	/**
	*	Never used in the SDK, but can be used to store different types of armor.
	*	@return The armor type.
	*/
	float GetArmorType() const { return pev->armortype; }

	/**
	*	Sets the armor type.
	*	@param flArmorType Armor type to set.
	*/
	void SetArmorType(const float flArmorType)
	{
		pev->armortype = flArmorType;
	}

	/**
	*	Player/monster score.
	*	@return Frags amount.
	*/
	float GetFrags() const { return pev->frags; }

	/**
	*	Sets the frags amount.
	*	@param flFrags Frags to set.
	*/
	void SetFrags(const float flFrags)
	{
		pev->frags = flFrags;
	}

	/**
	*	@return The entity's weapons flags.
	*/
	const CBitSet<int>& GetWeapons() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->weapons);
	}

	/**
	*	@copydoc GetWeapons() const
	*/
	CBitSet<int>& GetWeapons()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->weapons);
	}

	/**
	*	@return Takedamage mode.
	*/
	TakeDamageMode GetTakeDamageMode() const
	{
		return static_cast<TakeDamageMode>(static_cast<int>(pev->takedamage));
	}

	/**
	*	Sets the takedamage mode.
	*	@param takeDamageMode Takedamage mode to set.
	*/
	void SetTakeDamageMode(const TakeDamageMode takeDamageMode)
	{
		pev->takedamage = takeDamageMode;
	}

	/**
	*	@return DeadFlag.
	*/
	DeadFlag GetDeadFlag() const
	{
		return static_cast<DeadFlag>(pev->deadflag);
	}

	/**
	*	Sets the dead flag.
	*	@param deadFlag Dead flag to set.
	*/
	void SetDeadFlag(const DeadFlag deadFlag)
	{
		pev->deadflag = deadFlag;
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
	*	@return The entity's buttons flags.
	*/
	const CBitSet<int>& GetButtons() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->button);
	}

	/**
	*	@copydoc GetButtons() const
	*/
	CBitSet<int>& GetButtons()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->button);
	}

	/**
	*	@return The entity's old buttons flags.
	*/
	const CBitSet<int>& GetOldButtons() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->oldbuttons);
	}

	/**
	*	@copydoc GetOldButtons() const
	*/
	CBitSet<int>& GetOldButtons()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->oldbuttons);
	}

	/**
	*	@return The entity's impulse value
	*/
	int GetImpulse() const
	{
		return pev->impulse;
	}

	/**
	*	@see GetImpulse
	*/
	void SetImpulse(const int iImpulse)
	{
		pev->impulse = iImpulse;
	}

	/**
	*	@return The entity's spawn flags.
	*/
	const CBitSet<int>& GetSpawnFlags() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->spawnflags);
	}

	/**
	*	@copydoc GetSpawnFlags() const
	*/
	CBitSet<int>& GetSpawnFlags()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->spawnflags);
	}

	/**
	*	@return The entity's flags.
	*/
	const CBitSet<int>& GetFlags() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&pev->flags);
	}

	/**
	*	@copydoc GetFlags() const
	*/
	CBitSet<int>& GetFlags()
	{
		return *reinterpret_cast<CBitSet<int>*>(&pev->flags);
	}

	/**
	*	LRC
	*	@return The entity's desired flags.
	*/
	const CBitSet<int>& GetDesiredFlags() const
	{
		return *reinterpret_cast<const CBitSet<int>*>(&m_iLFlags);
	}

	/**
	*   LRC
	*	@copydoc GetDesiredFlags() const
	*/
	CBitSet<int>& GetDesiredFlags()
	{
		return *reinterpret_cast<CBitSet<int>*>(&m_iLFlags);
	}

	/**
	*	@return The color map.
	*/
	int GetColorMap() const { return pev->colormap; }

	/**
	*	Sets the color map.
	*	@param iColorMap Color map to set.
	*/
	void SetColorMap(const int iColorMap)
	{
		pev->colormap = iColorMap;
	}

	/**
	*	Gets the color map as its top and bottom values.
	*	@param[ out ] iTopColor Top color.
	*	@param[ out ] iBottomColor Bottom color.
	*/
	void GetColorMap(int& iTopColor, int& iBottomColor) const
	{
		iTopColor = GetColorMap() & 0xFF;
		iBottomColor = (GetColorMap() & 0xFF00) >> 8;
	}

	/**
	*	Sets the color map as its top and bottom values.
	*/
	void SetColorMap(const int iTopColor, const int iBottomColor)
	{
		SetColorMap((iTopColor & 0xFF) | ((iBottomColor << 8) & 0xFF00));
	}

	/**
	*	@return The team ID.
	*/
	int GetTeamID() const { return pev->team; }

	/**
	*	Sets the team ID.
	*/
	void SetTeamID(const int iTeamID)
	{
		pev->team = iTeamID;
	}

	/**
	*	@return The player class.
	*/
	int GetPlayerClass() const { return pev->playerclass; }

	/**
	*	Sets the player class.
	*	@param iPlayerClass Player class to set.
	*/
	void SetPlayerClass(const int iPlayerClass)
	{
		pev->playerclass = iPlayerClass;
	}

	/**
	*	@return The entity's water level.
	*/
	WaterLevel GetWaterLevel() const
	{
		return static_cast<WaterLevel>(pev->waterlevel);
	}

	/**
	*	Sets the water level.
	*	@param waterLevel Water level to set.
	*/
	void SetWaterLevel(const WaterLevel waterLevel)
	{
		pev->waterlevel = waterLevel;
	}

	/**
	*	@return Water type.
	*/
	Contents GetWaterType() const
	{
		return static_cast<Contents>(pev->watertype);
	}

	/**
	*	Sets the water type.
	*	@param waterType Water type to set.
	*/
	void SetWaterType(const Contents waterType)
	{
		pev->watertype = waterType;
	}

	/**
	*	@return Whether this entity has a message.
	*/
	bool HasMessage() const
	{
		return !!(*STRING(pev->message));
	}

	/**
	*	@return The message.
	*/
	const char* GetMessage() const { return STRING(pev->message); }

	/**
	*	Sets the message.
	*	@param iszMessage Message to set.
	*/
	void SetMessage(const string_t iszMessage)
	{
		pev->message = iszMessage;
	}

	/**
	*	Sets the message.
	*	@param pszMessage Message to set.
	*/
	void SetMessage(const char* const pszMessage)
	{
		SetMessage(MAKE_STRING(pszMessage));
	}

	/**
	*	Clears the message.
	*/
	void ClearMessage()
	{
		pev->message = iStringNull;
	}

	/**
	*	@return Speed.
	*/
	float GetSpeed() const { return pev->speed; }

	/**
	*	Sets the speed.
	*	@param flSpeed Speed to set.
	*/
	void SetSpeed(const float flSpeed)
	{
		pev->speed = flSpeed;
	}

	/**
	*	@return SMaximum seed.
	*/
	float GetMaxSpeed() const { return pev->maxspeed; }

	/**
	*	Sets the maximum speed.
	*	@param flSpeed Maximum speed to set.
	*/
	void SetMaxSpeed(const float flSpeed)
	{
		pev->maxspeed = flSpeed;
	}

	/**
	*	Time at which this entity runs out of air. Used while swimming.
	*	@return The air finished time.
	*/
	float GetAirFinishedTime() const { return pev->air_finished; }

	/**
	*	Sets the air finished time.
	*	@param flTime Time when this entity runs out of air.
	*/
	void SetAirFinishedTime(const float flTime)
	{
		pev->air_finished = flTime;
	}

	/**
	*	Time at which this entity can be hurt again while drowning.
	*	@return The pain finished time.
	*/
	float GetPainFinishedTime() const { return pev->pain_finished; }

	/**
	*	Sets the pain finished time.
	*	@param flTime Time when this entity should get hurt again.
	*/
	void SetPainFinishedTime(const float flTime)
	{
		pev->pain_finished = flTime;
	}

	/**
	*	@return Field of view.
	*/
	float GetFOV() const { return pev->fov; }

	/**
	*	Sets the field of view.
	*	@param flFOV Field of view to set.
	*/
	void SetFOV(const float flFOV)
	{
		pev->fov = flFOV;
	}

	/**
	*	@return The damage value.
	*/
	float GetDamage() const { return pev->dmg; }

	/**
	*	Sets the damage value.
	*	@param flDamage Damage value to set.
	*/
	void SetDamage(const float flDamage)
	{
		pev->dmg = flDamage;
	}

	/**
	*	@brief Gets the entity's damage time
	*/
	float GetDamageTime() const { return pev->dmgtime; }

	/**
	*	@see GetDamageTime
	*/
	void SetDamageTime(const float flDamageTime)
	{
		pev->dmgtime = flDamageTime;
	}

	/**
	*	@return Whether this entity has a noise.
	*/
	bool HasNoise() const
	{
		return !!(*STRING(pev->noise));
	}

	/**
	*	@return The noise.
	*/
	const char* GetNoise() const { return STRING(pev->noise); }

	/**
	*	Sets the noise.
	*	@param iszNoise Noise to set.
	*/
	void SetNoise(const string_t iszNoise)
	{
		pev->noise = iszNoise;
	}

	/**
	*	Sets the noise.
	*	@param pszNoise Noise to set.
	*/
	void SetNoise(const char* const pszNoise)
	{
		SetNoise(MAKE_STRING(pszNoise));
	}

	/**
	*	Clears the noise.
	*/
	void ClearNoise()
	{
		pev->noise = iStringNull;
	}

	/**
	*	@return Whether this entity has a noise1.
	*/
	bool HasNoise1() const
	{
		return !!(*STRING(pev->noise1));
	}

	/**
	*	@return The noise1.
	*/
	const char* GetNoise1() const { return STRING(pev->noise1); }

	/**
	*	Sets the noise1.
	*	@param iszNoise Noise1 to set.
	*/
	void SetNoise1(const string_t iszNoise)
	{
		pev->noise1 = iszNoise;
	}

	/**
	*	Sets the noise1.
	*	@param pszNoise Noise1 to set.
	*/
	void SetNoise1(const char* const pszNoise)
	{
		SetNoise1(MAKE_STRING(pszNoise));
	}

	/**
	*	Clears the noise1.
	*/
	void ClearNoise1()
	{
		pev->noise1 = iStringNull;
	}

	/**
	*	@return Whether this entity has a noise2.
	*/
	bool HasNoise2() const
	{
		return !!(*STRING(pev->noise2));
	}

	/**
	*	@return The noise2.
	*/
	const char* GetNoise2() const { return STRING(pev->noise2); }

	/**
	*	Sets the noise2.
	*	@param iszNoise Noise2 to set.
	*/
	void SetNoise2(const string_t iszNoise)
	{
		pev->noise2 = iszNoise;
	}

	/**
	*	Sets the noise2.
	*	@param pszNoise Noise2 to set.
	*/
	void SetNoise2(const char* const pszNoise)
	{
		SetNoise2(MAKE_STRING(pszNoise));
	}

	/**
	*	Clears the noise.
	*/
	void ClearNoise2()
	{
		pev->noise2 = iStringNull;
	}

	/**
	*	@return Whether this entity has a noise.
	*/
	bool HasNoise3() const
	{
		return !!(*STRING(pev->noise3));
	}

	/**
	*	@return The noise.
	*/
	const char* GetNoise3() const { return STRING(pev->noise3); }

	/**
	*	Sets the noise3.
	*	@param iszNoise Noise3 to set.
	*/
	void SetNoise3(const string_t iszNoise)
	{
		pev->noise3 = iszNoise;
	}

	/**
	*	Sets the noise3.
	*	@param pszNoise Noise3 to set.
	*/
	void SetNoise3(const char* const pszNoise)
	{
		SetNoise3(MAKE_STRING(pszNoise));
	}

	/**
	*	Clears the noise3.
	*/
	void ClearNoise3()
	{
		pev->noise3 = iStringNull;
	}
};

#endif
