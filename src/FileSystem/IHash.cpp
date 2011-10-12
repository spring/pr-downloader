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
			printf("%d %d\n", get(i), data[i]);
			return false;
		}
	}
	return true;

const std::string IHash::toString(){
	std::string str;
	const char* data=(const char*)get();
	char buf[4];
	if (getSize()%4!=0){
		LOG_ERROR("toString failed\n");
	}
	for(int i=0; i<getSize()/4; i++){
		unsigned int bla=data[i*4];
		sprintf(buf, "%x", bla);
		str.append(buf);
	}
	return str;
}
