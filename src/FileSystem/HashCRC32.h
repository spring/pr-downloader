#ifndef _HASH_CRC32_H
#define _HASH_CRC32_H

#include "IHash.h"


class HashCRC32: public IHash
{
public:
	HashCRC32();
	void Init();
	void Final();
	void Update(const char* data,const int size);
	bool Set(const unsigned char* data, int size);
protected:
	int getSize() const;
	unsigned char get(int pos) const;
private:
	unsigned long crc;
};

#endif
