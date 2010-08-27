#include "FileSystem.h"
#include <stdio.h>
#include "Util.h"
#include <string.h>


/*
creates pool path:
for example:
http://mirror/pool/<first 2 hex of md5>/<rest of md5>.gz
/home/path/.spring/pool/...
*/

std::string getUrl(const CFileSystem::FileData* info, const std::string& path){
	int i;
	char md5[32];
	for (i = 0; i < 16; i++){
		sprintf(md5+i*2, "%02x", info->md5[i]);
	}
	std::string tmp=path;
	tmp.append("/");
	tmp.append(md5[0],1);
	tmp.append(md5[1],1);
	tmp.append("/");
	tmp.append(md5[2],30);
	tmp.append(".gz");
	return tmp;
}

int hex_digit(int c){
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}


bool md5AtoI(const std::string& md5, unsigned char* dest){
	const char* p=md5.data();
	int i;
	int d1;
	int d2;
	for (i = 0; i < 16; ++i) {
		if ((d1 = hex_digit(*p++)) == -1)
			return false;
		if ((d2 = hex_digit(*p++)) == -1)
			return false;
		*dest++ = d1*16 + d2;
	}
	return true;
}

bool md5ItoA(const unsigned char* source, std::string& md5){
	int i;
	char buf[33];
	for (i = 0; i < 16; i++){
		sprintf(buf+i*2, "%02x", source[i]);
	}
	md5.assign(buf);
	return true;
}


std::string* getStrByIdx(std::string& str, char c, int idx){
	unsigned int i=0;
	std::string* tmp=new std::string;
	if (idx==0){
		for(i=0;i<str.size();i++){
			if (str[i]==c)
				break;
		}
		tmp->assign(str.substr(0,i));
		return tmp;
	}
	int start=0;
	int end=0;
	int count=0;
	for(i=0;i<str.length();i++){
		if (str[i]==c){
			count++;
			if(count>=idx){
				if(start==0)
					start=i+1;
				else{
					end=i;
					break;
				}
			}
		}
	}
	tmp->assign(str.substr(start,end-start));
	return tmp;
}
