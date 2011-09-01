#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>


#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <math.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif
#endif

#include "md5.h"
#include "FileSystem.h"
#include "Util.h"


CFileSystem* CFileSystem::singleton = NULL;

bool CFileSystem::fileIsValid(const FileData& mod, const std::string& filename) const
{
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	struct stat sb;
	if (stat(filename.c_str(),&sb)<0) {
		return false;
	}

	if (!sb.st_mode&S_IFREG) {
		printf("File is no file %s\n", filename.c_str());
		return false;
	}

	gzFile inFile = gzopen (filename.c_str(), "rb");
	if (inFile == NULL) { //file can't be opened
		return false;
	}
	MD5Init (&mdContext);
	unsigned long filesize=0;
	while ((bytes = gzread (inFile, data, 1024)) > 0) {
		MD5Update (&mdContext, data, bytes);
		filesize=filesize+bytes;
	}
	MD5Final (&mdContext);
	gzclose (inFile);
	/*	if (filesize!=mod->size){
			printf("File %s invalid, size wrong: %d but should be %d\n", filename.c_str(),filesize, mod->size);
			return false;
		}*/

	int i;
	for (i=0; i<16; i++) {
		if (mdContext.digest[i]!=mod.md5[i]) { //file is invalid
//			printf("Damaged file found: %s\n",filename.c_str());
//			unlink(filename.c_str());
			return false;
		}
	}
	return true;
}

bool CFileSystem::parseSdp(const std::string& filename, std::list<CFileSystem::FileData>& files)
{
	char c_name[255];
	unsigned char c_md5[16];
	unsigned char c_crc32[4];
	unsigned char c_size[4];

	gzFile in=gzopen(filename.c_str(), "r");
	if (in==Z_NULL) {
		printf("Could not open %s\n",filename.c_str());
		return NULL;
	}
	files.clear();
	FileData tmpfile;
	while (!gzeof(in)) {
		int length = gzgetc(in);
		if (length == -1) break;
		if (!((gzread(in, &c_name, length)) &&
		      (gzread(in, &c_md5, 16)) &&
		      (gzread(in, &c_crc32, 4)) &&
		      (gzread(in, &c_size, 4)))) {
			printf("Error reading %s\n", filename.c_str());
			gzclose(in);
			return false;
		}

		FileData f;
		f.name = std::string(c_name, length);
		std::memcpy(&f.md5, &c_md5, 16);
		f.crc32 = parse_int32(c_crc32);
		f.size = parse_int32(c_size);
		f.compsize = 0;
		files.push_back(f);
	}
	gzclose(in);
	return true;
}

CFileSystem::~CFileSystem()
{
	std::list<std::string>::iterator it;
	for (it = tmpfiles.begin(); it != tmpfiles.end(); ++it) {
		remove(it->c_str());
	}
	tmpfiles.clear();
}


CFileSystem::CFileSystem()
{
	tmpfiles.clear();
#ifndef WIN32
	char* buf;
	buf=getenv("HOME");
	springdir=buf;
	springdir.append("/.spring");
#else
	TCHAR pathMyDocs[MAX_PATH];
	SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, pathMyDocs);
	springdir=pathMyDocs;
	springdir.append("\\My Games\\Spring");
#endif
}

void CFileSystem::Initialize()
{
}

CFileSystem* CFileSystem::GetInstance()
{
	if (singleton==NULL)
		singleton=new CFileSystem();
	return singleton;
}

void CFileSystem::Shutdown()
{
	CFileSystem* tmpFileSystem=singleton;
	singleton=NULL;
	delete tmpFileSystem;
	tmpFileSystem=NULL;
}

const std::string& CFileSystem::getSpringDir() const
{
	return springdir;
}

bool CFileSystem::directoryExists(const std::string& path) const
{
	struct stat fileinfo;
	int res=stat(path.c_str(),&fileinfo);
	return (res==0);
}

void CFileSystem::createSubdirs (const std::string& path) const
{
	std::string tmp;
	tmp=path;
	if (path[path.length()]!=PATH_DELIMITER) {
		tmp=tmp.substr(0,tmp.rfind(PATH_DELIMITER));
	}
	for (unsigned int i=0; i<tmp.size(); i++) {
		char c=tmp.at(i);
		if (c==PATH_DELIMITER) {
#ifdef WIN32
			mkdir(tmp.substr(0,i).c_str());
#else
			mkdir(tmp.substr(0,i).c_str(),0777);
#endif
		}
	}
#ifdef WIN32
	mkdir(tmp.c_str());
#else
	mkdir(tmp.c_str(),0777);
#endif
}


const std::string CFileSystem::getPoolFileName(const CFileSystem::FileData& fdata) const
{
	std::string name;
	std::string md5;

	md5ItoA(fdata.md5, md5);
	name=getSpringDir();
	name += PATH_DELIMITER;
	name += "pool";
	name += PATH_DELIMITER;
	name += md5.at(0);
	name += md5.at(1);
	name += PATH_DELIMITER;
	createSubdirs(name);
	name += md5.substr(2);
	name += ".gz";
	return name;
}

int CFileSystem::validatePool(const std::string& path)
{
	DIR* d;
	d=opendir(path.c_str());
	int res=0;
	if (d!=NULL) {
		struct dirent* dentry;
		while ( (dentry=readdir(d))!=NULL) {
			struct stat sb;
			std::string tmp;
			if (dentry->d_name[0]!='.') {
				tmp=path+PATH_DELIMITER+dentry->d_name;
				stat(tmp.c_str(),&sb);
				if ((sb.st_mode&S_IFDIR)!=0) {
					res=res+validatePool(tmp);
					if (res%13==0) {
						printf("Valid files: %d\r",res);
						fflush(stdout);
					}
				} else {
					FileData filedata;
					std::string md5;
					int len=tmp.length();
					if (len<36) { //file length has at least to be <md5[0]><md5[1]>/<md5[2-30]>.gz
						printf("Invalid file: %s\n", tmp.c_str());
					} else {
						md5="";
						md5.push_back(tmp.at(len-36));
						md5.push_back(tmp.at(len-35));
						md5.append(tmp.substr(len-33, 30));
						md5AtoI(md5,filedata.md5);
						if (!fileIsValid(filedata,tmp)) {
							printf("Invalid File in pool: %s\n",tmp.c_str());
						} else {
							res++;
						}
					}
				}
			}

		}
	}
	closedir(d);
	return res;
}

bool CFileSystem::isOlder(const std::string& filename, int secs)
{
	struct stat sb;
	if (stat(filename.c_str(),&sb)<0) {
		return true;
	}
	time_t t;
#ifdef WIN32
	LARGE_INTEGER date;

	SYSTEMTIME pTime;
	FILETIME pFTime;
	GetSystemTime(&pTime);
	SystemTimeToFileTime(&pTime, &pFTime);

	date.HighPart = pFTime.dwHighDateTime;
	date.LowPart = pFTime.dwLowDateTime;

	date.QuadPart -= 11644473600000 * 10000;

	t = date.QuadPart / 10000000;
#else
	time(&t);
#endif
	return (t<sb.st_ctime+secs);
}

bool CFileSystem::fileExists(const std::string& filename)
{
	FILE* fp = NULL;
	fp = fopen(filename.c_str(), "r");
	if (fp == NULL) {
		return false;
	}
	fclose(fp);
	return true;
}
