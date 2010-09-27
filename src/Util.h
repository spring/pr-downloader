#ifndef UTIL_H
#define UTIL_H

#include "FileSystem.h"

/**
	creates a url from fileinfo, for example
	<path>/<first2chars of md5>/<last 30 chars of md5>.gz
*/
std::string getUrl(const CFileSystem::FileData* info, const std::string& path);

/**
	converts md5 ascii to bin
*/
bool md5AtoI(const std::string& md5, unsigned char* dest);

/**
	converts md5 bin to ascii
*/
bool md5ItoA(const unsigned char* source, std::string& md5);

/**
	returns substring number idx split by c
	str is for example "aaa,bbb,ccc"
	getStrByIdx(str,',',2) returns bbb
*/
std::string getStrByIdx(std::string& str, char c, int idx);

void gzip_str(const char* in, const int inlen,  char* out, int *outlen);

/**
	parses an int, read from file or network
*/
unsigned int parse_int32(unsigned char c[4]);

/**
	returns minimum
*/
unsigned int intmin(int x, int y);

/**
	compare str1 with str2
	if str2==* or "" it matches
	used for search in downloaders
*/
bool match_download_name(const std::string &str1,const std::string& str2);

/**
	replace " " with %20 in an url
	FIXME: this should handle all special chars
*/
void urlEncode(std::string& url);

/**
	creates a path from an url, for example:
	http://www.server.com/path/file.gz is translated to:
	www.server.com\path\file.gz
*/
bool urlToPath(const std::string& url, std::string& path);

#define DEBUG_LINE(string) \
	printf("%s:%d %s() %s\n", __FILE__, __LINE__, __FUNCTION__, string)

#endif
