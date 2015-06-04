#pragma once

#include "caveexpress/server/entities/IEntity.h"
#include "caveexpress/shared/constants/NPCState.h"

class NPC: public IEntity {
protected:
	float _initialWalkingSpeed;
	b2Vec2 _targetPos;
	b2Vec2 _initialPosition;
	bool _lastDirectionRight;
	int _triggerMovement;
	// the time at which the npc was dazed
	uint32_t _dazedTime;
	// the time that a npc is dazed
	uint32_t _dazedTimeout;
	TimerID _idleTimer;
	TimerID _moveTimer;
	// time at which the npc is going to die
	uint32_t _swimmingTime;
	uint16_t _swimmingTimeDelay;
	float _initialSwimmingSpeed;
	b2Vec2 _currentSwimmingSpeed;

	int handleTurnAnimation (const b2Vec2& targetPos, const Animation& left, const Animation& right);
	void move ();

	virtual b2BodyType getBodyType () const;

public:
	NPC (const EntityType &type, Map& map);
	virtual ~NPC ();

	virtual void setSwimming (const b2Vec2& pos);
	virtual void setSwimmingIdle ();
	virtual void setFalling ();
	virtual void setMoving (gridCoord x);
	virtual void setMoving (const b2Vec2& targetPos = b2Vec2_zero);
	virtual void setIdle ();
	virtual void setDone ();
	// @param[in] entity The entity that caused the dying
	virtual void setDying (const IEntity* entity);
	// @param[in] entity The entity that caused the dazed state
	virtual void setDazed (const IEntity* entity);

	inline void setDazedTimeout (uint32_t dazedTimeout)
	{
		_dazedTimeout = dazedTimeout;
	}

	inline void resetTriggerMovement() {
		_triggerMovement = 0;
	}

	virtual const Animation& getFallingAnimation () const;
	virtual const Animation& getIdleAnimation () const;

	virtual b2Body* createBody (const b2Vec2 &pos, bool setOnGround = true, bool fixedRotation = true);

	inline bool isIdle () const
	{
		return _state == NPCState::NPC_IDLE;
	}

	// moving towards the player if the player landed
	inline bool isMoving () const
	{
		return _state == NPCState::NPC_MOVING;
	}

	inline bool isAttacking () const
	{
		return _state == NPCState::NPC_ATTACKING;
	}

	inline bool isArrived () const
	{
		return _state == NPCState::NPC_ARRIVED;
	}

	// on board of the player's vehicle
	inline bool isCollected () const
	{
		return _state == NPCState::NPC_COLLECTED;
	}

	// dying in the water (sinking)
	inline bool isDying () const
	{
		return _state == NPCState::NPC_DYING;
	}

	// swimming in the water and waiting for rescue
	inline bool isSwimming () const
	{
		return _state == NPCState::NPC_SWIMMING;
	}

	inline bool isStruggle () const
	{
		return _state == NPCState::NPC_STRUGGLE;
	}

	// dying in the water (sinking)
	inline bool isDazed () const
	{
		return _state == NPCState::NPC_DAZED;
	}

	// npc is going to walk into the cave and disappears
	inline bool isDone () const
	{
		return _state == NPCState::NPC_DONE;
	}

	// falling down, because the npc was hit by the player
	inline bool isFalling () const
	{
		return _state == NPCState::NPC_FALLING;
	}

	// returns true if the npc has arrived its initial position again
	virtual bool returnToInitialPosition ();

	// IEntity
	virtual float getDensity () const override;
	virtual void update (uint32_t deltaTime) override;
	void setState (int state) override;
	virtual void onContact (b2Contact* contact, IEntity* entity) override;
	virtual bool shouldCollide (const IEntity* entity) const override;
	virtual void onSpawn () override;
	virtual void setPos (const b2Vec2& pos) override;
};

class SetOnGroundRayCastCallback: public b2RayCastCallback {
private:
	NPC* _npc;

public:
	SetOnGroundRayCastCallback (NPC* npc) :
			_npc(npc)
	{
	}

	float32 ReportFixture (b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
		IEntity *e = static_cast<IEntity *>(fixture->GetBody()->GetUserData());
		// the border fixture
		if (e == nullptr)
			return -1;
		if (!_npc->shouldCollide(e))
			return -1;
		const b2Vec2 newPos(point.x, point.y - _npc->getSize().y / 2.0f);
		_npc->setPos(newPos);
		return 0;
	}
};
