/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <stdio.h>
#include <string>
#include "IFile.h"

class AtomicFile: public IFile
{
public:
	AtomicFile(std::string filename);
	~AtomicFile();
	bool Open(std::string filename);
	int Write(char* buf, int size);
	void Close();
private:
	FILE* handle;
	std::string filename;
	std::string tmpname;
};
