#include "tool.h"

const std::vector<std::string> tool::explode(const std::string& s, const char& c)
{
	std::string buff{ "" };
	std::vector<std::string> v;

	for (auto n : s)
	{
		if (n != c) buff += n; else
			if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if (buff != "") v.push_back(buff);

	return v;
}

void tool::LogMsg(std::string logtext)
{
	std::cout << "[ACCOUNT-CHECKER] (LOG): " << logtext << std::endl;
}
