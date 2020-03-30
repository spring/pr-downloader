/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "HashSHA1.h"
#include "Logger.h"

#include <assert.h>
#include <string.h>

void HashSHA1::Init()
{
	isset = false;
	SHA1Reset(&sha1Context);
}

void HashSHA1::Update(const char* data, const int size)
{
	SHA1Input(&sha1Context, (unsigned char*)data, size);
}

void HashSHA1::Final()
{
	isset = true;
	SHA1Result(&sha1Context);
}

unsigned char HashSHA1::get(int pos) const
{
	return ((unsigned char*)&sha1Context.Message_Digest[pos / 4])[3 - pos % 4];
}

int HashSHA1::getSize() const
{
	return sizeof(sha1Context.Message_Digest);
}

bool HashSHA1::Set(const unsigned char* data, int size)
{
	if (size != getSize())
		return false;
	for (int i = 0; i < size; i++)
		((unsigned char*)&sha1Context.Message_Digest[i / 4])[3 - i % 4] = data[i];
	isset = true;
	return true;
}
