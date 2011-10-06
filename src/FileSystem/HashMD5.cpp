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

int HashMD5::getSize()
{
	return sizeof(mdContext.digest);
}

int HashMD5::get(int pos)
{
	assert( (pos>=0) && (pos<getSize()) );
	return mdContext.digest[pos];
}


#endif
