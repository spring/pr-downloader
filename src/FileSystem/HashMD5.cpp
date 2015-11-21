/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "HashMD5.h"

#include <assert.h>
#include <string.h>

HashMD5::HashMD5()
{
	isset=false;
	memset(&mdContext,0,  sizeof(mdContext));
}

void HashMD5::Init()
{
	isset=false;
	MD5Init (&mdContext);
}
void HashMD5::Update(const char* data, const int size)
{
	MD5Update (&mdContext, (unsigned char*)data, size);
}
void HashMD5::Final()
{
	isset=true;
	MD5Final(&mdContext);
}

int HashMD5::getSize() const
{
	return sizeof(mdContext.digest);
}

unsigned char HashMD5::get(int pos) const
{
	assert(pos<(int)sizeof(mdContext.digest));
	return mdContext.digest[pos];
}

bool HashMD5::Set(const unsigned char* data, int size)
{
	if(size!=getSize())
		return false;
	for(int i=0; i<size; i++)
		mdContext.digest[i]=data[i];
	isset=true;
	return true;
}