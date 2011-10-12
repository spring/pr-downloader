#include "HashMD5.h"

#include <assert.h>
#include <string.h>

HashMD5::HashMD5()
{
	memset(&mdContext,0,  sizeof(mdContext));
}

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
	assert(pos<(int)sizeof(mdContext.digest[pos]));
	return mdContext.digest[pos];
}
