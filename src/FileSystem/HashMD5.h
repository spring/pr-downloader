/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _HASH_MD5_H
#define _HASH_MD5_H

#include "IHash.h"
#include "lib/md5/md5.h"

class HashMD5 : public IHash
{
public:
	void Init() override;
	void Final() override;
	void Update(const char* data, const int size) override;
	bool Set(const unsigned char* data, int size) override;
	unsigned char get(int pos) const override;
	static std::string CalculateHash(const char* data, const int size);
	const unsigned char* Data() const { return &mdContext.digest[0]; }

	int getSize() const override;

private:
	MD5_CTX mdContext = {};
};

#endif
