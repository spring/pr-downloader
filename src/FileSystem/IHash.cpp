#include "IHash.h"
#include "Logger.h"
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
		unsigned char tmp=data[i];
		if (get(i)!=tmp) {
			LOG("compare failed(): %s %s\n", toString().c_str(), toString(data, size).c_str());
			return false;
		}
	}
	return true;
}

const std::string IHash::toString(const unsigned char* data, int size){
	std::string str;
	char buf[3];
	if (data==NULL){
		for(int i=0; i<getSize(); i++){
			snprintf(buf,sizeof(buf),"%.2x", get(i));
			str.append(buf);
		}
	}else{
		for(int i=0; i<size; i++){
			snprintf(buf,sizeof(buf),"%.2x", data[i]);
			str.append(buf);
		}
	}
	return str;
}
