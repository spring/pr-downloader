#include "CChecksum.h"
#include "md5/md5.h"

class CChecksumMD5: public CChecksum
{
public:
	CChecksumMD5() {}
	void Init();
	void Final();
	void Update(const char* data,const int size);
//		const std::string toString();
	int getSize();
	int get(int pos);
private:
	MD5_CTX mdContext;
};
