/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _HASH_MD5_H
#define _HASH_MD5_H

#include "IHash.h"
#include "lib/md5/md5.h"

class HashMD5 : public IHash
{
public:
	void Init();
	void Final();
	void Update(const char* data, const int size);
	bool Set(const unsigned char* data, int size);
	unsigned char get(int pos) const;
	static std::string CalculateHash(const char* data, const int size);
	const unsigned char* Data() const { return &mdContext.digest[0]; }

	int getSize() const;

private:
	MD5_CTX mdContext = {};
};

#endif
