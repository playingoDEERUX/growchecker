#pragma once
#include <iostream>
#include "essentials.h"
#include <vector>

namespace tool {
	const std::vector<std::string> explode(const std::string& s, const char& c);
	void LogMsg(std::string logtext);
}
