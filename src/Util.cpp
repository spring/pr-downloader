/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Util.h"
#include "FileSystem/FileSystem.h"
#include "Logger.h"

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <time.h>

void getStrByIdx(const std::string& str, std::string& res, char c, int idx)
{
	unsigned int i=0;
	if (idx==0) {
		for (i=0; i<str.size(); i++) {
			if (str[i]==c)
				break;
		}
		res.assign(str.substr(0,i));
		return;
	}
	int start=0;
	int end=0;
	int count=0;
	for (i=0; i<str.length(); i++) {
		if (str[i]==c) {
			count++;
			if (count>=idx) {
				if (start==0)
					start=i+1;
				else {
					end=i;
					break;
				}
			}
		}
	}
	res.assign(str.substr(start,end-start));
}

void gzip_str(const char* in, const int inlen,  char* out, int *outlen)
{
	z_stream zlibStreamStruct;
	zlibStreamStruct.zalloc    = Z_NULL; // Set zalloc, zfree, and opaque to Z_NULL so
	zlibStreamStruct.zfree     = Z_NULL; // that when we call deflateInit2 they will be
	zlibStreamStruct.opaque    = Z_NULL; // updated to use default allocation functions.
	zlibStreamStruct.total_out = 0; // Total number of output bytes produced so far
	zlibStreamStruct.next_in   = (Bytef*)in; // Pointer to input bytes
	zlibStreamStruct.avail_in  = inlen; // Number

	int res = deflateInit2(&zlibStreamStruct, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);
	if (res!= Z_OK) return;
	do {
		zlibStreamStruct.next_out = (Bytef*)out + zlibStreamStruct.total_out;
		zlibStreamStruct.avail_out = *outlen - zlibStreamStruct.total_out;
		res = deflate(&zlibStreamStruct, Z_FINISH);
	} while ( res == Z_OK );
	deflateEnd(&zlibStreamStruct);
	*outlen=zlibStreamStruct.total_out;
}

unsigned int parse_int32(unsigned char c[4])
{
	unsigned int i = 0;
	i = c[0] << 24 | i;
	i = c[1] << 16 | i;
	i = c[2] << 8  | i;
	i = c[3] << 0  | i;
	return i;
}

unsigned int intmin(int x, int y)
{
	if (x<y)
		return x;
	return y;
}

void urlEncode(std::string& url)
{
	for (int i=url.length()-1; i>=0; i--) {
		if (url.at(i)==' ') {
			url.replace(i,1,"%20");
		}
	}
}

bool urlToPath(const std::string& url, std::string& path)
{
	size_t pos=url.find("//");
	if (pos==std::string::npos) { //not found
		LOG_ERROR("urlToPath failed: %s",path.c_str());
		return false;
	}
	path=url.substr(pos+2);
	pos=path.find("/",pos+1);
	while (pos!=std::string::npos) { //replace / with "\\"
		path.replace(pos,1,1, PATH_DELIMITER);
		pos=path.find("/",pos+1);
	}
	return true;
}

unsigned long getTime()
{
	return time(NULL);
}

#ifdef WIN32
#include <windows.h>
std::wstring s2ws(const std::string& s)
{
	const size_t slength = s.length();
	const int len = MultiByteToWideChar(CP_UTF8 , 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf, len);
	delete[] buf;
	return r;
}

std::string ws2s(const std::wstring& s)
{
	const size_t slength = s.length();
	const int len = WideCharToMultiByte(CP_UTF8,0, s.c_str(),slength,NULL,0,NULL,NULL);
	char* buf = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, s.c_str(), slength, buf, len, NULL, NULL);
	std::string r(buf, len);
	delete [] buf;
	return r;
}


#endif


