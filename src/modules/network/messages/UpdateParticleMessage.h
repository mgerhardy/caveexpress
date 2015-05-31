#pragma once

#include "network/IProtocolMessage.h"
#include <vector>

struct UpdateParticleEntity {
	float x;
	float y;
	EntityAngle angle;
	uint8_t index;
	uint32_t lifetime;
};

class UpdateParticleMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	std::vector<UpdateParticleEntity> _bodies;
	const std::vector<UpdateParticleEntity>* _bodiesPtr;
	uint8_t _bodyCount;
	int _maxParticles;
	uint32_t _maxLifetime;

public:
	UpdateParticleMessage (uint16_t entityId, const std::vector<UpdateParticleEntity>& bodies, int maxParticles, uint32_t maxLifetime) :
			IProtocolMessage(protocol::PROTO_UPDATEPARTICLE), _entityId(entityId), _bodiesPtr(&bodies), _bodyCount(
					bodies.size()), _maxParticles(maxParticles), _maxLifetime(maxLifetime)
	{
	}

	UpdateParticleMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_UPDATEPARTICLE), _bodiesPtr(nullptr)
	{
		_entityId = input.readShort();
		_maxParticles = input.readByte();
		_maxLifetime = input.readByte() * 100;
		_bodyCount = input.readByte();
		_bodies.resize(_bodyCount);
		for (int i = 0; i < _bodyCount; ++i) {
			UpdateParticleEntity& e = _bodies[i];
			e.x = input.readShortScaled();
			e.y = input.readShortScaled();
			e.angle = static_cast<EntityAngle>(input.readByte()) * 2;
			e.index = input.readByte();
			e.lifetime = input.readByte() * 100;
		}
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addByte(_maxParticles);
		out.addByte(_maxLifetime / 100);
		out.addByte(_bodyCount);
		for (int i = 0; i < _bodyCount; ++i) {
			const UpdateParticleEntity& e = (*_bodiesPtr)[i];
			out.addShortScaled(e.x);
			out.addShortScaled(e.y);
			out.addByte(e.angle / 2);
			out.addByte(e.index);
			out.addByte(e.lifetime / 100);
		}
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline uint8_t getBodyCount () const
	{
		return _bodyCount;
	}

	inline int getMaxParticles () const
	{
		return _maxParticles;
	}

	// TODO: remove me - not needed, do it on the client side - too much traffic
	inline uint32_t getLifeTime (int bodyIndex) const
	{
		const UpdateParticleEntity& e = _bodies[bodyIndex];
		return e.lifetime;
	}

	inline uint32_t getMaxLifeTime () const
	{
		return _maxLifetime;
	}

	inline float getX (int bodyIndex) const
	{
		const UpdateParticleEntity& e = _bodies[bodyIndex];
		return e.x;
	}

	inline float getY (int bodyIndex) const
	{
		const UpdateParticleEntity& e = _bodies[bodyIndex];
		return e.y;
	}

	inline uint8_t getIndex (int bodyIndex) const
	{
		const UpdateParticleEntity& e = _bodies[bodyIndex];
		return e.index;
	}

	inline EntityAngle getAngle (int bodyIndex) const
	{
		const UpdateParticleEntity& e = _bodies[bodyIndex];
		return e.angle;
	}

	inline const std::string getSprite () const
	{
		return "particle-water";
	}
};
