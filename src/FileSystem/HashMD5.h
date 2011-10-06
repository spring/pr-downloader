#ifndef _HASH_MD5_H
#define _HASH_MD5_H

#include "IHash.h"
#include "md5/md5.h"

class HashMD5: public IHash
{
public:
	HashMD5() {}
	void Init();
	void Final();
	void Update(const char* data,const int size);
//		const std::string toString();
	int getSize();
	int get(int pos);
	bool Set(const void* data, int len);
	bool Set(const std::string& str);
	int getSize() const;
	unsigned char get(int pos) const;
private:
	MD5_CTX mdContext;
};

#endif
