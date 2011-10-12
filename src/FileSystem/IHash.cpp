#include "IHash.h"
#include "Logger.h"
#include <stdio.h>
#include <string.h>

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
			printf("checksum different: pos:%d %d!=%d size:%d\n",i, get(i), data[i], size);
		}
	}
	return true;
}

const std::string IHash::toString(){
	std::string str;
	char buf[2];
	for(int i=0; i<getSize(); i++){
		sprintf(buf, "%.2x", get(i));
//		LOG("%d: output: %s\n",i, buf);
		str.append(buf);
	}
	return str;
}
