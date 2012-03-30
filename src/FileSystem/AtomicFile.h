/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <stdio.h>
#include <string>

class AtomicFile
{
public:
	AtomicFile(std::string filename);
	~AtomicFile();
	bool open(std::string filename);
	int write(char* buf, int size);
	void close();
private:
	FILE* handle;
	std::string filename;
	std::string tmpname;
};
