#pragma once

#include "common/Enum.h"

class MapFailedReason : public Enum<MapFailedReason>
{
public:
	MapFailedReason(const std::string& _name = "") : Enum<MapFailedReason>(_name) {}
};
