#pragma once

#include "cavepacker/server/entities/IEntity.h"
#include "engine/common/Direction.h"
#include "engine/common/SoundType.h"
#include "engine/common/ConfigVar.h"
#include "engine/common/Logger.h"
#include "engine/common/Pointers.h"
#include "engine/common/network/IProtocolHandler.h"

// forward decl
class Map;

class Player: public IEntity {
private:
	ClientId _clientId;
	std::string _name;
	std::string _solutionSave;
public:
	Player (Map& map, ClientId clientId);
	virtual ~Player ();

	ClientId getClientId () const;
	const std::string& getName () const;
	void setName (const std::string& name);
	void storeStep (char step);
	void undo ();
	const std::string& getSolution () const;
};

inline ClientId Player::getClientId () const
{
	return _clientId;
}

inline const std::string& Player::getName () const
{
	return _name;
}

inline void Player::setName (const std::string& name)
{
	_name = name;
}

inline const std::string& Player::getSolution () const
{
	return _solutionSave;
}
