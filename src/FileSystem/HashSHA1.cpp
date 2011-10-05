#include "HashSHA1.h"

#include <assert.h>


void CChecksumSHA1::Init()
{
	SHA1Reset(&sha1Context);
}

void CChecksumSHA1::Update(const char* data, const int size)
{
	SHA1Input (&sha1Context, (unsigned char*)data, size);
}

void CChecksumSHA1::Final()
{
	SHA1Result(&sha1Context);
}

int CChecksumSHA1::getSize()
{
	return sizeof( sha1Context.Message_Digest);
}

int CChecksumSHA1::get(int pos)
{
	assert( (pos>=0) && (pos<getSize()) );
	return sha1Context.Message_Digest[pos];
}
