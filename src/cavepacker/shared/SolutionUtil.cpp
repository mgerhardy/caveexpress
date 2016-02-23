#include <cavepacker/shared/SolutionUtil.h>
#include "common/Log.h"
#include "common/String.h"

namespace cavepacker {

std::string SolutionUtil::compress(const std::string& buffer) {
	std::string rle;
	const int bufLen = static_cast<int>(buffer.length());
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
	for (std::size_t i = 0u; i < solution.length(); ++i) {
		char c = solution[i];
		if (!isdigit(c)) {
			Log::debug(LOG_GAMEIMPL, "Skip '%c'", c);
			continue;
		}
		std::string digit;
		digit += c;
		for (++i; i != solution.length(); ++i) {
			c = solution[i];
			if (!isdigit(c)) {
				Log::debug(LOG_GAMEIMPL, "Found '%s'", digit.c_str());
				break;
			}
			digit += c;
		}
		const int n = string::toInt(digit);
		if (i >= solution.length()) {
			Log::error(LOG_GAMEIMPL, "invalid rle encoded solution found");
			break;
		}
		if (n <= 0) {
			Log::error(LOG_GAMEIMPL, "invalid rle encoded solution found: %s", digit.c_str());
			break;
		}
		// single char repeat
		c = solution[i];
		if (c != '(') {
			std::string repeat;
			for (int k = 0; k < n; ++k) {
				repeat.push_back(c);
			}
			const std::string replace = digit + c;
			i -= replace.length() - 1;
			solution.replace(i, replace.length(), repeat);
			i += repeat.length() - 1;
			continue;
		}
		std::string repeat;
		int depth = 0;
		for (++i; i != solution.length(); ++i) {
			c = solution[i];
			if (c == '(') {
				++depth;
			} else if (c == ')') {
				if (depth == 0) {
					++i;
					Log::debug(LOG_GAMEIMPL, "End of repeat (digit: %s) '%s'", digit.c_str(), repeat.c_str());
					break;
				}
				--depth;
			}
			repeat.push_back(c);
		}
		const std::string r = repeat;
		for (int k = 1; k < n; ++k) {
			repeat += r;
		}
		Log::debug(LOG_GAMEIMPL, "Replace rle for '%s' times '%s'", digit.c_str(), repeat.c_str());
		const std::string replaceMulti = digit + "(" + r + ")";
		i -= replaceMulti.length();
		solution.replace(i, replaceMulti.length(), repeat);
		--i;
	}
	return string::toLower(solution);
}

}
