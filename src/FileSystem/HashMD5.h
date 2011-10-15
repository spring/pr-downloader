#ifndef _HASH_MD5_H
#define _HASH_MD5_H

#include "IHash.h"
#include "lib/md5/md5.h"

class HashMD5: public IHash
{
public:
	HashMD5();
	void Init();
	void Final();
	void Update(const char* data,const int size);
	bool Set(unsigned char* data, int size);
protected:
	int getSize() const;
	unsigned char get(int pos) const;

private:
	MD5_CTX mdContext;
};

#endif
