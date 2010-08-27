#ifndef UTIL_H
#define UTIL_H

#include "FileSystem.h"

std::string getUrl(const CFileSystem::FileData* info, const std::string& path);
bool md5AtoI(const std::string& md5, unsigned char* dest);
bool md5ItoA(const unsigned char* source, std::string& md5);
std::string* getStrByIdx(std::string& str, char c, int idx);

#endif
