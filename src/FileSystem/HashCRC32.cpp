#include "HashCRC32.h"
#include <zlib.h>

HashCRC32::HashCRC32()
{
	crc=0;
}

void HashCRC32::Init()
{
	crc=0;
}

void HashCRC32::Update(const char* data,const int size)
{
	crc=crc32(crc, (const Bytef*)data, size);
}

void HashCRC32::Final()
{
}

int HashCRC32::getSize() const
{
	return sizeof(crc);
}

unsigned char HashCRC32::get(int pos) const
{
	return crc;
}
