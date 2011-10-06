#ifndef _CCHECKSUM_H
#define _CCHECKSUM_H

#include <string>
#include <assert.h>


class IHash
{
public:
//	IHash() {}
	virtual bool Set(const void* data, int len)=0;
	virtual bool Set(const std::string& str)=0;
	virtual void Init()=0;
	virtual void Final()=0;
	virtual void Update(const char* data,const int size)=0;
	virtual const std::string toString() {
		std::string empty;
		return empty;
	}
	virtual bool compare(const IHash& checksum);
	virtual bool compare(const unsigned char* data, int size);
	virtual int getSize() const=0;
	virtual unsigned char get(int pos) const=0;
};

#endif
