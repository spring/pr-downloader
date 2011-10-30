/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <string>

class CWidget
{
public:
	CWidget(const std::string& filename);
private:
	std::string name;
	std::string changelog;
	std::string author;
};
