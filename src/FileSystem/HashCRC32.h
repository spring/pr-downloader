#include "IHash.h"

class HashCRC32: public IHash
{
public:
	HashCRC32() {}
	void Init();
	void Final();
	void Update(const char* data,const int size);
//		const std::string toString();
	int getSize();
	int get(int pos);
private:
	unsigned long crc;
};
