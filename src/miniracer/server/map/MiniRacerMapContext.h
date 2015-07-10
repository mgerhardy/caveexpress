#pragma once

#include "common/IMapContext.h"
#include <string>

namespace miniracer {

class MiniRacerMapContext : public IMapContext {
private:
public:
	explicit MiniRacerMapContext(const std::string& map);
	virtual ~MiniRacerMapContext();

	// IMapContext
	void onMapLoaded () override;
	bool load (bool skipErrors) override;
	bool save () const override { return false; }
};

}
