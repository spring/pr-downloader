#include "HashSHA1.h"

#include <assert.h>
#include <string.h>

HashSHA1::HashSHA1()
{
	memset(&sha1Context,0,  sizeof(sha1Context));
}

void HashSHA1::Init()
{
	SHA1Reset(&sha1Context);
}

void HashSHA1::Update(const char* data, const int size)
{
	SHA1Input (&sha1Context, (unsigned char*)data, size);
}

void HashSHA1::Final()
{
	SHA1Result(&sha1Context);
}

unsigned char HashSHA1::get(int pos) const
{
	assert( (pos>=0) && (pos<getSize()) );
	return sha1Context.Message_Digest[pos];
}


int HashSHA1::getSize() const
{
	return sizeof(sha1Context.Message_Digest);
}
