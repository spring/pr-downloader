#include "FileSystem.h"
#include <stdio.h>
#include "Util.h"
#include <string.h>
#include <zlib.h>


/*
creates pool path:
for example:
http://mirror/pool/<first 2 hex of md5>/<rest of md5>.gz
/home/path/.spring/pool/...
*/

std::string getUrl(const CFileSystem::FileData* info, const std::string& path)
{
	int i;
	char md5[32];
	for (i = 0; i < 16; i++) {
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

int hex_digit(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}


bool md5AtoI(const std::string& md5, unsigned char* dest)
{
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

bool md5ItoA(const unsigned char* source, std::string& md5)
{
	int i;
	char buf[33];
	for (i = 0; i < 16; i++) {
		sprintf(buf+i*2, "%02x", source[i]);
	}
	md5.assign(buf);
	return true;
}


std::string getStrByIdx(std::string& str, char c, int idx)
{
	unsigned int i=0;
	std::string tmp;
	if (idx==0) {
		for (i=0; i<str.size(); i++) {
			if (str[i]==c)
				break;
		}
		tmp.assign(str.substr(0,i));
		return tmp;
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
	tmp.assign(str.substr(start,end-start));
	return tmp;
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

bool match_download_name(const std::string &str1,const std::string& str2)
{
	if (str2=="") return true;
	if (str2=="*") return true;
	if (str1==str2) return true;
	return false;
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
		printf("urlToPath failed: %s\n",path.c_str());
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


/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

void base64_decode(const std::string& encoded_string, std::string& ret) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }
}
