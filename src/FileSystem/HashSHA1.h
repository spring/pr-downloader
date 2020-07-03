/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _CCHECKSUMSHA1_H
#define _CCHECKSUMSHA1_H

#include "IHash.h"
#include "lib/sha1/sha1.h"

class HashSHA1 : public IHash
{
public:
	void Init() override;
	void Final() override;
	void Update(const char* data, const int size) override;
	bool Set(const unsigned char* data, int size) override;

protected:
	int getSize() const override;
	unsigned char get(int pos) const override;

private:
	SHA1Context sha1Context = {};
};

#endif
