#include "PressurePlate.h"
#include "Gate.h"
#include "caveexpress/server/map/Map.h"
#include "caveexpress/shared/CaveExpressAnimation.h"

namespace caveexpress {

PressurePlate::PressurePlate (Map& map, const std::string& spriteID, gridCoord gridX, gridCoord gridY,
		const std::string& linkId, float requiredWeight, int holdMs) :
		MapTile(map, spriteID, gridX, gridY, EntityTypes::PRESSUREPLATE), _linkId(linkId), _requiredWeight(
				requiredWeight), _holdMs(holdMs), _linkedGate(nullptr), _holdUntil(0), _active(false)
{
	setAnimationType(Animations::ANIMATION_IDLE);
}

PressurePlate::~PressurePlate ()
{
}

bool PressurePlate::shouldCollide (const IEntity* entity) const
{
	// Solid ground for anything that can land or walk on regular tiles.
	return entity->isDynamic();
}

void PressurePlate::onContact (PhysicsContact contact, IEntity* entity)
{
	IEntity::onContact(contact, entity);
	if (entity != nullptr && entity->isDynamic())
		_contacts.insert(entity);
}

void PressurePlate::endContact (PhysicsContact contact, IEntity* entity)
{
	IEntity::endContact(contact, entity);
	if (entity != nullptr)
		_contacts.erase(entity);
}

bool PressurePlate::shouldActivate () const
{
	float mass = 0.0f;
	for (IEntity* e : _contacts) {
		if (e == nullptr || e->isRemove())
			continue;
		// Player or stone landing always triggers the pressed sprite / gate.
		if (e->isPlayer() || e->isStone())
			return true;
		// NPCs walk over the plate without activating it.
		if (e->isNpc())
			continue;
		mass += e->getMass();
	}
	return mass + 0.0001f >= _requiredWeight;
}

void PressurePlate::setActive (bool active)
{
	if (_active == active)
		return;
	_active = active;
	setAnimationType(active ? Animations::ANIMATION_ACTIVE : Animations::ANIMATION_IDLE);
	if (_linkedGate != nullptr)
		_linkedGate->setOpen(active);
}

void PressurePlate::update (uint32_t deltaTime)
{
	MapTile::update(deltaTime);

	for (auto i = _contacts.begin(); i != _contacts.end();) {
		if (*i == nullptr || (*i)->isRemove())
			i = _contacts.erase(i);
		else
			++i;
	}

	const bool weighted = shouldActivate();

	if (weighted) {
		if (_holdMs > 0)
			_holdUntil = _time + static_cast<uint32_t>(_holdMs);
		setActive(true);
	} else if (_holdMs > 0) {
		if (_holdUntil != 0 && _time < _holdUntil)
			setActive(true);
		else
			setActive(false);
	} else {
		setActive(false);
	}
}

}
