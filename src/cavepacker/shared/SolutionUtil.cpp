#include <cavepacker/shared/SolutionUtil.h>
#include "common/Log.h"
#include "common/String.h"

namespace cavepacker {

std::string SolutionUtil::compress(const std::string& buffer) {
	std::string rle;
	const int bufLen = buffer.length();
	rle.reserve(bufLen);
	char lastC = '\0';
	int cnt = 1;
	for (int i = 0; i < bufLen; ++i) {
		const char c = buffer[i];
		if (c == lastC) {
			++cnt;
		} else {
			if (lastC != '\0') {
				if (cnt > 1) {
					rle.append(string::toString(cnt));
				}
				rle.push_back(lastC);
			}
			lastC = c;
			cnt = 1;
		}
	}
	if (lastC != '\0') {
		if (cnt > 1) {
			rle.append(string::toString(cnt));
		}
		rle.push_back(lastC);
	}

	return rle;
}

std::string SolutionUtil::decompress(const std::string& buffer) {
	std::string solution = buffer;
	for (std::string::const_iterator i = solution.begin(); i != solution.end(); ++i) {
		if (!isdigit(*i)) {
			Log::debug(LOG_GAMEIMPL, "Skip '%c'", *i);
			continue;
		}
		std::string digit;
		digit += *i;
		for (++i; i != solution.end(); ++i) {
			if (!isdigit(*i)) {
				Log::debug(LOG_GAMEIMPL, "Found '%s'", digit.c_str());
				break;
			}
			digit += *i;
		}
		const int n = string::toInt(digit);
		if (i == solution.end()) {
			Log::error(LOG_GAMEIMPL, "invalid rle encoded solution found");
			break;
		}
		if (n <= 0) {
			Log::error(LOG_GAMEIMPL, "invalid rle encoded solution found: %s", digit.c_str());
			break;
		}
		// single char repeat
		if (*i != '(') {
			std::string repeat;
			repeat += *i;
			const std::string r = repeat;
			for (int k = 1; k < n; ++k) {
				repeat += r;
			}
			solution = string::replaceAll(solution, digit + r, repeat);
			i = solution.begin();
			continue;
		}
		std::string repeat;
		int depth = 0;
		for (++i; i != solution.end(); ++i) {
			if (*i == '(') {
				++depth;
			} else if (*i == ')') {
				if (depth == 0) {
					++i;
					Log::debug(LOG_GAMEIMPL, "End of repeat (digit: %s) '%s'", digit.c_str(), repeat.c_str());
					break;
				}
				--depth;
			}
			repeat += *i;
		}
		const std::string r = repeat;
		for (int k = 1; k < n; ++k) {
			repeat += r;
		}
		Log::debug(LOG_GAMEIMPL, "Replace rle for '%s' times '%s'", digit.c_str(), repeat.c_str());
		solution = string::replaceAll(solution, digit + "(" + r + ")", repeat);
		i = solution.begin();
	}
	return string::toLower(solution);
}

}
