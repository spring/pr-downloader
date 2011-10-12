#ifndef _CCHECKSUMSHA1_H
#define _CCHECKSUMSHA1_H

#include "IHash.h"
#include "sha1/sha1.h"

class HashSHA1: public IHash
{
public:
	HashSHA1();
	void Init();
	void Final();
	void Update(const char* data,const int size);
protected:
	int getSize() const;
	unsigned char get(int pos) const;
private:
	SHA1Context sha1Context;
};

#endif
