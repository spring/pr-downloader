/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include <string>

class IHash;
//FIXME: maybe not portable?
class FileData
{
public:
	FileData();
	~FileData();

	std::string name;
	IHash* crc32;
	IHash* md5;
	unsigned int size;
	unsigned int compsize; //compressed size
	bool download;
};

#endif
