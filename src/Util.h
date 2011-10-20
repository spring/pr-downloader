#ifndef UTIL_H
#define UTIL_H

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define VERSION QUOTEME(PR_DOWNLOADER_VERSION)
#define USER_AGENT "pr-downloader/" VERSION
#define XMLRPC_METHOD "springfiles.search"
#define XMLRPC_HOST "springfiles.com"
#define XMLRPC_PORT 80
#define XMLRPC_URI "/xmlrpc.php"

#include "FileSystem/FileSystem.h"
#include "Logger.h"
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

/**
 * base64 decode a string
 */
void base64_decode(const std::string& encoded_string, std::string& ret);

/**
 *	returns the time
 */
unsigned long getTime();

#endif
