#include "JSONGameRoundParser.h"
#include "engine/common/Logger.h"
#include "engine/common/System.h"

JSONGameRoundParser::JSONGameRoundParser (const FilePtr& file, IProgressCallback *callback) :
		_result(false), _depth(0), _state(STATE_NONE), _callback(callback)
{
	char *buffer;
	const int fileLen = file->read((void **) &buffer);
	if (fileLen <= 0) {
		error(LOG_CLIENT, "can't read the json round data");
		return;
	}
	const ScopedArrayPtr<char> p(buffer);
	const std::string json(buffer, fileLen);
	_result = readJson(json);
}

JSONGameRoundParser::~JSONGameRoundParser ()
{
}

bool JSONGameRoundParser::wasSuccess () const
{
	return _result;
}

void JSONGameRoundParser::printError (yajl_handle hand, const std::string& str)
{
	unsigned char * error = yajl_get_error(hand, 1, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	error(LOG_CLIENT, reinterpret_cast<const char *>(error));
	yajl_free_error(hand, error);
}

int JSONGameRoundParser::convert_string (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	std::string str;
	if (stringLen > 0)
		str = std::string(reinterpret_cast<const char *>(stringVal), stringLen);
	JSONGameRoundParser* p = reinterpret_cast<JSONGameRoundParser*>(ctx);
	if (p->_state == STATE_URL) {
		p->_currentLocation.url = str;
	} else if (p->_state == STATE_THUMB_URL) {
		p->_currentLocation.thumbUrl = str;
	} else if (p->_state == STATE_LOCATION) {
		p->_currentLocation.location = str;
	} else if (p->_state == STATE_LOCATION_PARENTS) {
		// TODO:
	} else if (p->_state == STATE_LATITUDE) {
		p->_currentLocation.latitude = strtod(str.c_str(), nullptr);
	} else if (p->_state == STATE_LONGITUDE) {
		p->_currentLocation.longitude = strtod(str.c_str(), nullptr);
	} else if (p->_state == STATE_SCORE) {
		p->_currentLocation.score = atoi(str.c_str());
	} else if (p->_state == STATE_CREATOR) {
		p->_currentLocation.creator = str;
	}
	return 1;
}

int JSONGameRoundParser::convert_map_key (void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	const std::string str(reinterpret_cast<const char *>(stringVal), stringLen);
	JSONGameRoundParser* p = reinterpret_cast<JSONGameRoundParser*>(ctx);
	if (str == "url") {
		p->_state = STATE_URL;
	} else if (p->_state == STATE_ID) {
		p->_currentLocation.id = atoi(str.c_str());
	} else if (str == "thumb_url") {
		p->_state = STATE_THUMB_URL;
	} else if (str == "location") {
		p->_state = STATE_LOCATION;
	} else if (str == "location_parents") {
		p->_state = STATE_LOCATION_PARENTS;
	} else if (str == "latitude") {
		p->_state = STATE_LATITUDE;
	} else if (str == "longitude") {
		p->_state = STATE_LONGITUDE;
	} else if (str == "score") {
		p->_state = STATE_SCORE;
	} else if (str == "creator") {
		p->_state = STATE_CREATOR;
	} else {
		p->_state = STATE_NONE;
	}
	return 1;
}

int JSONGameRoundParser::convert_start_map (void * ctx)
{
	JSONGameRoundParser* p = reinterpret_cast<JSONGameRoundParser*>(ctx);
	++p->_depth;
	if (p->_depth == 1) {
		p->_currentLocation = Location();
		p->_state = STATE_ID;
	}
	return 1;
}

int JSONGameRoundParser::convert_end_map (void * ctx)
{
	JSONGameRoundParser* p = reinterpret_cast<JSONGameRoundParser*>(ctx);
	--p->_depth;
	if (p->_depth == 1) {
		if (p->_currentLocation.id == 0)
			System.exit("Failed to parse the location id", 1);
		p->_round.addLocation(p->_currentLocation);
		if (p->_callback != nullptr)
			p->_callback->progressStep(p->_currentLocation.url);
		p->_currentLocation = Location();
		p->_state = STATE_ID;
	}
	return 1;
}

bool JSONGameRoundParser::readJson (const std::string& str)
{
	const yajl_callbacks callbacks = { nullptr, nullptr, nullptr, nullptr, nullptr, convert_string, convert_start_map,
			convert_map_key, convert_end_map, nullptr, nullptr };
	yajl_handle hand = yajl_alloc(&callbacks, nullptr, this);
	yajl_status stat = yajl_parse(hand, reinterpret_cast<const unsigned char*>(str.c_str()), str.length());
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	stat = yajl_complete_parse(hand);
	if (stat != yajl_status_ok && stat != yajl_status_client_canceled) {
		printError(hand, str);
		return false;
	}

	yajl_free(hand);

	return true;
}

const Locations& JSONGameRoundParser::getLocations () const
{
	return _round.getLocations();
}
