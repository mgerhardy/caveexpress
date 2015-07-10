#include "MiniRacerMapContext.h"
#include "common/FileSystem.h"
#include "common/Log.h"
#include "miniracer/shared/MiniRacerEntityType.h"

namespace miniracer {

MiniRacerMapContext::MiniRacerMapContext(const std::string& map) :
		IMapContext(map) {
	_title = map;
}

MiniRacerMapContext::~MiniRacerMapContext() {
}

void MiniRacerMapContext::onMapLoaded() {
}

bool MiniRacerMapContext::load(bool skipErrors) {
	return false;
}

}
