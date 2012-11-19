/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include <string>

class IHash;
class FileData
{
public:
	FileData();
	~FileData();

	std::string name;
	unsigned char md5[16];
	unsigned char crc32[4];
	unsigned int size;
	unsigned int compsize; //compressed size
	bool download;
	int mode; //chmod
};

#endif
