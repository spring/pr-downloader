#include "IHash.h"
#include <stdio.h>

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

bool IHash::compare(const unsigned char* data, int size)
{
	assert(getSize()>0 && size>0);
	if (getSize()!=size)
		return false;
	for (int i=0; i<getSize(); i++) {
		if (get(i)!=data[i]) {
			printf("%d %d\n", get(i), data[i]);
			return false;
		}
	}
	return true;
}
