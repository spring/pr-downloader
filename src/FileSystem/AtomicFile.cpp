/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "AtomicFile.h"
#include <sys/stat.h>
#include <unistd.h>
#include "Logger.h"
#include "FileSystem.h"

AtomicFile::AtomicFile(std::string filename)
{
	this->Open(filename);
}

bool AtomicFile::Open(const std::string& filename):
	handle(NULL)
{
	tmpname = filename + ".tmp";
	this->filename=filename;
	const bool tmpexists = fileSystem->fileExists(tmpname);
	const bool fileexists = fileSystem->fileExists(filename);

	if (fileexists) {
		if (tmpexists) { //remove existing tempfile
			remove(tmpname.c_str());
		}
		//rename destination file to temp file
		if (rename(filename.c_str(), tmpname.c_str())<0) {
			LOG_ERROR("error renaming temp file %s", filename.c_str());
			return false;
		}
	}
	LOG_DEBUG("opened %s", filename.c_str());
	handle = fopen(tmpname.c_str(), "wb+");
	return (handle != NULL);
}

int AtomicFile::Write(const char *buf, int size)
{
	int res = fwrite(buf, size, 1, handle);
	LOG_DEBUG("writing %d %d", size, res);
	return res*size;
}

void AtomicFile::Close()
{
	LOG_DEBUG("closing %s", filename.c_str() );
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
