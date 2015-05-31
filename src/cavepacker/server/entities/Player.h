#pragma once

#include "cavepacker/server/entities/IEntity.h"
#include "common/Direction.h"
#include "common/SoundType.h"
#include "common/ConfigVar.h"
#include "common/Logger.h"
#include "common/Pointers.h"
#include "common/network/IProtocolHandler.h"

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
	bool undo ();
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
