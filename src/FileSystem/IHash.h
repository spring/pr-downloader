#ifndef _CCHECKSUM_H
#define _CCHECKSUM_H

#include <string>
#include <assert.h>


class IHash
{
public:
//	IHash() {}
	virtual void Init() {}
	virtual void Final() {}
	virtual void Update(const char* data,const int size) {}
	virtual const std::string toString() {
		std::string empty;
		return empty;
	}
	bool compare(const IHash& checksum);
	virtual int getSize() const {
		return 0;
	}
	virtual int get(int pos) const {
		return 0;
	}
//	virtual const compare
};

#endif
