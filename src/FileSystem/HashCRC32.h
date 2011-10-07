#include "IHash.h"

class HashCRC32: public IHash
{
public:
	HashCRC32() {
		crc=0;
	}
	void Init();
	void Final();
	void Update(const char* data,const int size);
//		const std::string toString();
	int getSize();
	unsigned char get(int pos) const;
private:
	unsigned long crc;
};
