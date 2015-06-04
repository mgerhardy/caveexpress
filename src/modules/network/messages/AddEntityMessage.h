#pragma once

#include "network/IProtocolMessage.h"
#include "common/Animation.h"
#include "common/EntityType.h"
#include "common/EntityAlignment.h"
#include "common/SpriteDefinition.h"

class AddEntityMessage: public IProtocolMessage {
private:
	uint16_t _entityId;
	const Animation* _animation;
	const EntityType* _entityType;
	std::string _sprite;
	float _xpos;
	float _ypos;
	float _sizeX;
	float _sizeY;
	EntityAngle _angle;
	EntityAlignment _spriteAlign;
public:
	AddEntityMessage (uint16_t entityId, const EntityType& entityType, const Animation& animation,
			const std::string& sprite, float xpos, float ypos, float sizeX, float sizeY, EntityAngle angle,
			EntityAlignment spriteAlign) :
			IProtocolMessage(protocol::PROTO_ADDENTITY), _entityId(entityId), _entityType(&entityType), _animation(
					&animation), _sprite(sprite), _xpos(xpos), _ypos(ypos), _sizeX(sizeX), _sizeY(sizeY), _angle(angle), _spriteAlign(
					spriteAlign)
	{
	}

	PROTOCOL_CLASS_FACTORY(AddEntityMessage);

	explicit AddEntityMessage (ByteStream& input) :
			IProtocolMessage(protocol::PROTO_ADDENTITY)
	{
		_entityId = input.readShort();
		_entityType = &EntityType::get(input.readByte());
		_animation = &Animation::get(input.readByte());
		_sprite = input.readString();
		_xpos = input.readShortScaled();
		_ypos = input.readShortScaled();
		_sizeX = input.readShortScaled();
		_sizeY = input.readShortScaled();
		_angle = static_cast<EntityAngle>(input.readShort());
		_spriteAlign = static_cast<EntityAlignment>(input.readByte());
	}

	void serialize (ByteStream& out) const override
	{
		out.addByte(_id);
		out.addShort(_entityId);
		out.addByte(_entityType->id);
		out.addByte(_animation->id);
		out.addString(_sprite);
		out.addShortScaled(_xpos);
		out.addShortScaled(_ypos);
		out.addShortScaled(_sizeX);
		out.addShortScaled(_sizeY);
		out.addShort(_angle);
		out.addByte(_spriteAlign);
	}

	inline uint16_t getEntityId () const
	{
		return _entityId;
	}

	inline const EntityType& getEntityType () const
	{
		return *_entityType;
	}

	inline const Animation& getAnimation () const
	{
		return *_animation;
	}

	inline const std::string& getSprite () const
	{
		return _sprite;
	}

	inline float getX () const
	{
		return _xpos;
	}

	inline float getY () const
	{
		return _ypos;
	}

	inline float getWidth () const
	{
		return _sizeX;
	}

	inline float getHeight () const
	{
		return _sizeY;
	}

	inline EntityAngle getAngle () const
	{
		return _angle;
	}

	inline EntityAlignment getSpriteAlignment () const
	{
		return _spriteAlign;
	}
};
