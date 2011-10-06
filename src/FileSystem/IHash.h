#ifndef _CCHECKSUM_H
#define _CCHECKSUM_H

#include <string>


class IHash
{
public:
	IHash() {}
	virtual void Init() {}
	virtual void Final() {}
	virtual void Update(const char* data,const int size) {}
	virtual const std::string toString() {
		std::string empty;
		return empty;
	}
	virtual bool compare(const IHash& checksum) {
		if (checksum.getSize()!=getSize())
			return false;
		for (int i=0; i<getSize(); i++) {
			if (get(i)!=checksum.get(i))
				return false;
		}
		return true;
	}
	virtual int getSize() const {
		return 0;
	}
	virtual int get(int pos) const {
		return 0;
	}
//	virtual const compare
};

#endif
