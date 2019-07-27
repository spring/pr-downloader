/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef UTIL_H
#define UTIL_H

#define HTTP_SEARCH_URL "https://api.springfiles.com/json.php"
#define MAX_PARALLEL_DOWNLOADS 10

#include <string>
#include <vector>

class FileData;

/**
        creates a url from fileinfo, for example
        <path>/<first2chars of md5>/<last 30 chars of md5>.gz
*/
std::string getUrl(const FileData* info, const std::string& path);

/**
        tokenizes a string into a vector split by c
        empty tokens aren't ignored
*/
std::vector<std::string> tokenizeString(const std::string& str, char c);

/**
* decompresses in to out
*/
int gzip_str(const char* in, const int inlen, char* out, int* outlen);

/**
        parses an int, read from file or network
*/
unsigned int parse_int32(unsigned char c[4]);

/**
        returns minimum
*/
unsigned int intmin(int x, int y);

/**
        creates a path from an url, for example:
        http://www.server.com/path/file.gz is translated to:
        www.server.com\path\file.gz
*/
bool urlToPath(const std::string& url, std::string& path);

/*
        convert std::wstring to std::string
*/
std::string ws2s(const std::wstring& s);
std::wstring s2ws(const std::string& s);

#endif
