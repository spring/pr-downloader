/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _CCHECKSUMSHA1_H
#define _CCHECKSUMSHA1_H

#include "IHash.h"
#include "lib/sha1/sha1.h"

class HashSHA1 : public IHash
{
public:
	HashSHA1();
	void Init();
	void Final();
	void Update(const char* data, const int size);
	bool Set(const unsigned char* data, int size);

protected:
	int getSize() const;
	unsigned char get(int pos) const;

private:
	SHA1Context sha1Context;
};

#endif
