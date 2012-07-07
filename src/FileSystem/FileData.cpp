/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "FileData.h"
#include "IHash.h"
#include "HashCRC32.h"
#include "HashMD5.h"
#include "Logger.h"
#include <string.h>

FileData::FileData()
{
	name="";
	size=0;
	compsize=0;
	download=false;
	memset(md5, 0, sizeof(md5));
	memset(crc32, 0, sizeof(crc32));

}

FileData::~FileData()
{
}
