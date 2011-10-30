/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "FileData.h"
#include "IHash.h"
#include "HashCRC32.h"
#include "HashMD5.h"
#include "Logger.h"

FileData::FileData()
{
	name="";
	size=0;
	compsize=0;
	download=false;
	crc32=new HashCRC32();
	md5=new HashMD5();
}

FileData::~FileData()
{
	delete md5;
	delete crc32;
}
