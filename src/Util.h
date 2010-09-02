#ifndef UTIL_H
#define UTIL_H

#include "FileSystem.h"

/*creates a url from fileinfo, for example
* <path>/<first2chars of md5>/<last 30 chars of md5>.gz
*/
std::string getUrl(const CFileSystem::FileData* info, const std::string& path);
//converts md5 ascii to bin
bool md5AtoI(const std::string& md5, unsigned char* dest);
//converts md5 bin to ascii
bool md5ItoA(const unsigned char* source, std::string& md5);
//returns substring number idx split by c
std::string getStrByIdx(std::string& str, char c, int idx);

void gzip_str(const char* in, const int inlen,  char* out, int *outlen);
unsigned int parse_int32(unsigned char c[4]);
unsigned int intmin(int x, int y);
bool match_download_name(const std::string &str1,const std::string& str2);

#endif
