#include "IHash.h"

bool IHash::compare(const IHash& checksum)
{
	assert(getSize()>0 && checksum.getSize()>0);
	if (checksum.getSize()!=getSize())
		return false;
	for (int i=0; i<getSize(); i++) {
		if (get(i)!=checksum.get(i))
			return false;
	}
	return true;
}
