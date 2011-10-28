/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "IHash.h"
#include "Logger.h"

#include <stdio.h>

bool IHash::compare(const IHash* checksum)
{
	assert(getSize()>0 && checksum->getSize()>0);
	if (checksum==NULL) //can't compare, so guess checksums are fine
		return true;
	if (checksum->getSize()!=getSize())
		return false;
	for (int i=0; i<getSize(); i++) {
		if (get(i)!=checksum->get(i))
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
			LOG_INFO("compare failed(): %s %s", toString().c_str(), toString(data, size).c_str());
			return false;
		}
	}
	return true;
}

const std::string IHash::toString(const unsigned char* data, int size)
{
	std::string str;
	char buf[3];
	if (data==NULL) {
		for(int i=0; i<getSize(); i++) {
			snprintf(buf,sizeof(buf),"%.2x", get(i));
			str.append(buf);
		}
	} else {
		for(int i=0; i<size; i++) {
			snprintf(buf,sizeof(buf),"%.2x", data[i]);
			str.append(buf);
		}
	}
	return str;
}

unsigned IHash::getVal(char c)
{
	if ((c>='0') && (c<='9'))
		return c-'0';
	if ((c>='a') && (c<='f'))
		return c-'a'+10;
	if ((c>='A') && (c<='F'))
		return c-'A'+10;
	return 0;
}

bool IHash::Set(const std::string& hash)
{
	unsigned char buf[256];
	if(hash.size()>sizeof(buf)) {
		LOG_ERROR("IHash::Set(): buffer to small");
		return false;
	}
	if(hash.size()%2!=0) {
		LOG_ERROR("IHash::Set(): buffer%2  != 0");
		return false;
	}
	for(unsigned i=0; i<hash.size()/2; i++) {
		buf[i]=getVal(hash.at((i*2)+1)) + getVal(hash.at(i*2))*16;
	}
	if(!Set(buf, hash.size()/2)) {
		LOG_ERROR("IHash:Set(): Error setting");
		return false;
	}
	isset=true;
	return true;
}

bool IHash::isSet()
{
	return isset;
}
