#pragma once

#include "engine/common/Enum.h"

class MapFailedReason : public Enum<MapFailedReason>
{
public:
	MapFailedReason(const std::string& _name = "") : Enum<MapFailedReason>(_name) {}
};
