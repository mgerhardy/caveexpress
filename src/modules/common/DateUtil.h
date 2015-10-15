#pragma once

#include "common/String.h"
#include <time.h>

namespace dateutil {
inline bool isXmas() {
	const time_t t = time(0);
	struct tm *now = localtime(&t);
	const bool xmas = now->tm_mon == 11 && now->tm_mday >= 24
			&& now->tm_mday <= 26;
	return xmas;
}
inline std::string getDateString() {
	const time_t t = time(0);
	struct tm *now = localtime(&t);
	return string::format("%i/%02i/%02i-%02i:%02i:%02i", now->tm_year + 1900,
			now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
}
}
