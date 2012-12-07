/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef UTIL_H
#define UTIL_H

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define VERSION QUOTEME(PR_DOWNLOADER_VERSION)
#define USER_AGENT "pr-downloader/" VERSION
#define XMLRPC_METHOD "springfiles.search"
#define XMLRPC_HOST "api.springfiles.com"
#define XMLRPC_PORT 80
#define XMLRPC_URI "/xmlrpc.php"
#define MAX_PARALLEL_DOWNLOADS 10

#include <string>

class FileData;

/**
	creates a url from fileinfo, for example
	<path>/<first2chars of md5>/<last 30 chars of md5>.gz
*/
std::string getUrl(const FileData* info, const std::string& path);

/**
	returns substring number idx split by c
	str is for example "aaa,bbb,ccc"
	getStrByIdx(str,',',2) returns bbb
*/
void getStrByIdx(const std::string& str, std::string& res, char c, int idx);

/**
* decompresses in to out
*/
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

/**
 *	returns the time
 */
unsigned long getTime();

#endif
