/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include <stdio.h>
#include <string>
#include "IFile.h"

class AtomicFile: public IFile
{
public:
	AtomicFile(std::string filename);
	virtual ~AtomicFile();
	bool Open(const std::string& filename) override;
	int Write(const char* buf, int size) override;
	void Close() override;
private:
	FILE* handle;
	std::string filename;
	std::string tmpname;
};
