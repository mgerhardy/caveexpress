#pragma once

#include "common/String.h"

namespace cavepacker {

// util class to deal with sokoban solutions
class SolutionUtil {
public:
	// transforms single step solution into rle encoded version
	static std::string compress(const std::string& buffer);

	// transforms rle encoded solution into uncompressed state with single steps
	static std::string decompress(const std::string& buffer);
};

}
