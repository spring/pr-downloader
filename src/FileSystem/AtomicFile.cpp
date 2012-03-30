/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "AtomicFile.h"
#include <sys/stat.h>
#include <unistd.h>
#include "Logger.h"

AtomicFile::AtomicFile(std::string filename)
{
	this->open(filename);
}

bool AtomicFile::open(std::string filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) == 0) {
		if (remove(filename.c_str())!=0)
			return false;
	}
	tmpname = filename + ".tmp";
	this->filename=filename;
	handle = fopen(tmpname.c_str(), "wb+");
	LOG_DEBUG("opened %s", filename.c_str());
	return true;
}

int AtomicFile::write(char* buf, int size)
{
	int res = fwrite(buf, size, 1, handle);
	LOG_DEBUG("writing %d %d\n", size, res);
	return res*size;
}

void AtomicFile::close()
{
	LOG_DEBUG("closing %s\n", filename.c_str() );
#ifndef WIN32
	fsync(fileno(handle));
#endif
	fclose(handle);
	rename(tmpname.c_str(), filename.c_str());
	handle = NULL;
}

AtomicFile::~AtomicFile()
{
	if (handle!=NULL) {
		LOG_ERROR("File %s wasn't closed, deleting it", tmpname.c_str());
		fclose(handle);
		remove(tmpname.c_str());
	}
}
