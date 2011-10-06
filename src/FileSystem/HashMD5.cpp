#ifndef _CCHECKSUMMD5_H
#define _CCHECKSUMMD5_H

#include <assert.h>

#include "HashMD5.h"



void HashMD5::Init()
{
	MD5Init (&mdContext);
}
void HashMD5::Update(const char* data, const int size)
{
	MD5Update (&mdContext, (unsigned char*)data, size);
}
void HashMD5::Final()
{
	MD5Final(&mdContext);
}

int HashMD5::getSize() const
{
	return sizeof(mdContext.digest);
}

unsigned char HashMD5::get(int pos) const
{
	assert( (pos>=0) && (pos<getSize()) );
	return mdContext.digest[pos];
}

bool HashMD5::Set(const void* data, int len)
{
	return true;
}

bool HashMD5::Set(const std::string& string)
{
	return true;
}


#endif
