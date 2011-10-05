#ifndef _CCHECKSUMMD5_H
#define _CCHECKSUMMD5_H

#include <assert.h>

#include "CChecksumMD5.h"



void CChecksumMD5::Init()
{
	MD5Init (&mdContext);
}
void CChecksumMD5::Update(const char* data, const int size)
{
	MD5Update (&mdContext, (unsigned char*)data, size);
}
void CChecksumMD5::Final()
{
	MD5Final(&mdContext);
}

int CChecksumMD5::getSize()
{
	return sizeof(mdContext.digest);
}

int CChecksumMD5::get(int pos)
{
	assert( (pos>=0) && (pos<getSize()) );
	return mdContext.digest[pos];
}


#endif
