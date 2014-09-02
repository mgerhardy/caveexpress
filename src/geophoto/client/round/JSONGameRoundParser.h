#pragma once

#include "engine/common/File.h"
#include "geophoto/client/round/GameRoundData.h"
#include "engine/common/IProgressCallback.h"
#include <yajl/yajl_parse.h>

class JSONGameRoundParser {
private:
	GameRoundData _round;
	Location _currentLocation;

	enum State {
		STATE_ID, STATE_URL, STATE_THUMB_URL, STATE_LOCATION, STATE_LOCATION_PARENTS, STATE_LONGITUDE, STATE_LATITUDE, STATE_SCORE, STATE_CREATOR, STATE_NONE
	};

	State _state;
	int _depth;
	bool _result;
	IProgressCallback *_callback;

	bool readJson (const std::string& str);
	void printError (yajl_handle hand, const std::string& str);

	static int convert_string (void * ctx, const unsigned char * stringVal, size_t stringLen);
	static int convert_map_key (void * ctx, const unsigned char * stringVal, size_t stringLen);
	static int convert_start_map (void * ctx);
	static int convert_end_map (void * ctx);

public:
	JSONGameRoundParser (const FilePtr& file, IProgressCallback *callback);
	virtual ~JSONGameRoundParser ();

	bool wasSuccess () const;
	const Locations& getLocations () const;
};
