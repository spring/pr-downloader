/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include <string>

class FileData
{
public:
	std::string name;
	unsigned char md5[16] = {};
	unsigned char crc32[4] = {};
	unsigned int size = 0;
	unsigned int compsize = 0; // compressed size
	bool download = false;
	int mode = 0644; // chmod
};

#endif
