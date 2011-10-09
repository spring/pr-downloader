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
#include "bencode/bencode.h"


#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <math.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif
#endif

#include "FileSystem.h"
#include "Util.h"
#include "Downloader/IDownloader.h"
#include "FileSystem/HashMD5.h"
#include "Logger.h"


CFileSystem* CFileSystem::singleton = NULL;

bool CFileSystem::fileIsValid(const FileData& mod, const std::string& filename) const
{
	HashMD5 md5hash;
	int bytes;
	unsigned char data[4096];
	gzFile inFile = gzopen (filename.c_str(), "rb");
	if (inFile == NULL) { //file can't be opened
		LOG_ERROR("Could not open file %s\n", filename.c_str());
		return false;
	}
	md5hash.Init();
//	unsigned long filesize=0;
	while ((bytes = gzread (inFile, data, 4096)) > 0) {
		md5hash.Update((char*)data, bytes);
//		filesize=filesize+bytes;
	}
	md5hash.Final();
	gzclose (inFile);
	/*	if (filesize!=mod->size){
			ERROR("File %s invalid, size wrong: %d but should be %d\n", filename.c_str(),filesize, mod->size);
			return false;
		}*/

	if (!md5hash.compare(mod.md5, sizeof(mod.md5))) { //file is invalid
//		ERROR("Damaged file found: %s\n",filename.c_str());
//		unlink(filename.c_str());
		return false;
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
		LOG_ERROR("Could not open %s\n",filename.c_str());
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
			LOG_ERROR("Error reading %s\n", filename.c_str());
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


const std::string CFileSystem::getPoolFileName(const std::string& md5) const
{
	std::string name;

	name=getSpringDir();
	name += PATH_DELIMITER;
	name += "pool";
	name += PATH_DELIMITER;
	name += md5.at(0);
	name += md5.at(1);
	name += PATH_DELIMITER;
	if (!directoryExists(name)) {
		createSubdirs(name);
	}
	name += md5.substr(2);
	name += ".gz";
	return name;
}

int CFileSystem::validatePool(const std::string& path)
{

	unsigned long time=0;
	int res=0;
	std::list <std::string*>dirs;
	dirs.push_back(new std::string(path));
	int maxdirs=256; //FIXME: unknown dirs in pool will break bar
	int finished=0;
	while(!dirs.empty()) {
		struct dirent* dentry;
		DIR* d;
		std::string* dir=dirs.front();
		dirs.pop_front();
		d=opendir(dir->c_str());
		while ( (dentry=readdir(d))!=NULL) {
			unsigned long now=getTime();
			if (time<now) {
				LOG_PROGRESS(finished, maxdirs);
				fflush(stdout);
				time=now;
			}
			std::string absname=dir->c_str();
			absname += PATH_DELIMITER;
			absname += dentry->d_name;
			if (dentry->d_name[0]!='.') { //don't check hidden files / . / ..
				if ((dentry->d_type & DT_DIR)!=0) { //directory
					dirs.push_back(new std::string(absname));
				} else {
					FileData filedata;
					std::string md5;
					int len=absname.length();
					if (len<36) { //file length has at least to be <md5[0]><md5[1]>/<md5[2-30]>.gz
						LOG_ERROR("Invalid file: %s\n", absname.c_str());
					} else {
						md5="";
						md5.push_back(absname.at(len-36));
						md5.push_back(absname.at(len-35));
						md5.append(absname.substr(len-33, 30));
						if (!md5AtoI(md5,filedata.md5)) { //set md5 in filedata structure
							LOG_ERROR("Invalid filename %s\n", absname.c_str());
						}
						if (!fileIsValid(filedata, absname)) { //check if md5 in filename is the same as in filename
							LOG_ERROR("Invalid File in pool: %s\n",absname.c_str());
						} else {
							res++;
						}
					}
				}
			}
		}
		finished++;
		delete(dir);
		closedir(d);
	}
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

bool CFileSystem::parseTorrent(const char* data, int size, IDownload& dl)
{
	struct be_node* node=be_decoden(data, size);
#ifdef DEBUG
	be_dump(node);
#endif
	if (node->type!=BE_DICT) {
		LOG_ERROR("Error in torrent data\n");
		be_free(node);
		return false;
	}
	int i;
	struct be_node* infonode=NULL;
	for (i = 0; node->val.d[i].val; ++i) { //search for a dict with name info
		if ((node->type==BE_DICT) && (strcmp(node->val.d[i].key,"info")==0)) {
			infonode=node->val.d[i].val;
			break;
		}
	}
	if (infonode==NULL) {
		LOG_ERROR("couldn't find info node in be dict\n");
		be_free(node);
		return false;
	}
	for (i = 0; infonode->val.d[i].val; ++i) { //fetch needed data from dict and fill into dl
		struct be_node*datanode;
		datanode=infonode->val.d[i].val;
		switch(datanode->type) {
		case BE_STR: //current value is a string
//			if ((strcmp("name",infonode->val.d[i].key)==0) && (dl.name.empty())) { //set filename if not already set
//					dl.name=datanode->val.s;
//			} else
			if (!strcmp("pieces", infonode->val.d[i].key)) { //hash sum of a piece
				const int count = strlen(datanode->val.s)/6;
				for (int i=0; i<count; i++) {
					struct IDownload::piece piece;
					for(int j=0; j<5; j++) {
						piece.sha[j]=datanode->val.s[i*5+j];
						piece.state=IDownload::STATE_NONE;
					}
					dl.pieces.push_back(piece);
				}
			}
			break;
		case BE_INT: //current value is a int
			if (strcmp("length",infonode->val.d[i].key)==0) { //filesize
				dl.size=datanode->val.i;
			} else if (!strcmp("piece length",infonode->val.d[i].key)) { //length of a piece
				dl.piecesize=datanode->val.i;
			}
			break;
		default:
			break;
		}
	}
	DEBUG_LINE("Parsed torrent data: %s %d\n", dl.name.c_str(), dl.piecesize);
	be_free(node);
	return true;
}

bool CFileSystem::dumpSDP(const std::string& filename)
{
	std::list<CFileSystem::FileData> files;
	files.clear();
	if (!parseSdp(filename, files))
		return false;
	std::list<CFileSystem::FileData>::iterator it;
	LOG_INFO("filename	size	virtualname	crc32\n");
	for(it=files.begin(); it!=files.end(); ++it) {
		std::string md5;
		md5ItoA((const unsigned char*)(*it).name.c_str() ,md5);
		LOG_INFO("%s %8d %s %X\n",md5.c_str(), (*it).size, (*it).name.c_str(), (*it).crc32);
	}
	return true;
}

