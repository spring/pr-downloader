/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "FileSystem.h"
#include "Util.h"
#include "Downloader/IDownloader.h"
#include "HashMD5.h"
#include "HashSHA1.h"
#include "FileData.h"
#include "Logger.h"
#include "SevenZipArchive.h"
#include "ZipArchive.h"
#include "lib/bencode/bencode.h"

#include <zlib.h>
#include <string.h>
#include <list>
#include <string>
#include <sys/stat.h>
#include <dirent.h>

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <math.h>
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif
#endif

CFileSystem* CFileSystem::singleton = NULL;

bool CFileSystem::fileIsValid(const FileData* mod, const std::string& filename) const
{
	HashMD5 md5hash;
	int bytes;
	unsigned char data[IO_BUF_SIZE];
	gzFile inFile = gzopen (filename.c_str(), "rb");
	if (inFile == NULL) { //file can't be opened
		LOG_ERROR("Could not open file %s", filename.c_str());
		return false;
	}
	md5hash.Init();
//	unsigned long filesize=0;
	while ((bytes = gzread (inFile, data, IO_BUF_SIZE)) > 0) {
		md5hash.Update((char*)data, bytes);
//		filesize=filesize+bytes;
	}

	md5hash.Final();
	gzclose (inFile);
	/*	if (filesize!=mod->size){
			ERROR("File %s invalid, size wrong: %d but should be %d", filename.c_str(),filesize, mod->size);
			return false;
		}*/
	if (!md5hash.compare(mod->md5, sizeof(mod->md5) )) { //file is invalid
//		ERROR("Damaged file found: %s",filename.c_str());
//		unlink(filename.c_str());
		return false;
	}
	return true;
}

bool CFileSystem::parseSdp(const std::string& filename, std::list<FileData*>& files)
{
	char c_name[255];
	unsigned char c_md5[16];
	unsigned char c_crc32[4];
	unsigned char c_size[4];
	unsigned char length;

	gzFile in=gzopen(filename.c_str(), "r");
	if (in==Z_NULL) {
		LOG_ERROR("Could not open %s",filename.c_str());
		return NULL;
	}
	files.clear();
	while (true) {
		if (!gzread(in, &length, 1)) {
			if (gzeof(in)) {
				break;
			}
			LOG_ERROR("Unexpected eof in %s", filename.c_str());
			gzclose(in);
			return false;
		}
		if (!((gzread(in, &c_name, length)) &&
		      (gzread(in, &c_md5, 16)) &&
		      (gzread(in, &c_crc32, 4)) &&
		      (gzread(in, &c_size, 4)))) {
			LOG_ERROR("Error reading %s", filename.c_str());
			gzclose(in);
			return false;
		}
		FileData* f=new FileData;
		f->name = std::string(c_name, length);
		memcpy(f->md5, &c_md5, 16);
		memcpy(f->crc32, &c_crc32, 4);
		f->size = parse_int32(c_size);
		files.push_back(f);
	}
	gzclose(in);
	LOG_DEBUG("Parsed %s with %d files\n", filename.c_str(), (int)files.size());
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

bool CFileSystem::setWritePath(const std::string& path)
{
	if (!path.empty()) {
		if(!directoryExists(path)) {
			LOG_ERROR("filesystem-writepath doesn't exist: %s", path.c_str());
			return false;
		}
		springdir=path;
	} else {
#ifndef WIN32
		char* buf;
		buf=getenv("HOME");
		if (buf!=NULL) {
			springdir=buf;
			springdir.append("/.spring");
		} else { //no home: use cwd
			LOG_INFO("HOME isn't set, using CWD./spring");
			springdir=".spring";
		}
#else
		TCHAR pathMyDocs[MAX_PATH];
		SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, pathMyDocs);
		springdir=pathMyDocs;
		springdir.append("\\My Games\\Spring");
#endif
	}
	LOG_INFO("Using filesystem-writepath: %s", springdir.c_str());
	return true;

}

CFileSystem::CFileSystem()
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

const std::string& CFileSystem::getSpringDir()
{
	if (springdir.empty())
		(setWritePath(""));
	return springdir;
}

bool CFileSystem::directoryExists(const std::string& path) const
{
	struct stat fileinfo;
	int res=stat(path.c_str(),&fileinfo);
	return ((res == 0) && ((fileinfo.st_mode & S_IFDIR) != 0));
}

#ifdef WIN32
#define MKDIR(path) \
	mkdir(path)
#else
#define MKDIR(path) \
	mkdir(path, 0755)
#endif

bool CFileSystem::createSubdirs (const std::string& path) const
{
	std::string tmp=path;
	if (path[path.length()]!=PATH_DELIMITER) {
		tmp=tmp.substr(0,tmp.rfind(PATH_DELIMITER));
	}
	for (unsigned int i=2; i<tmp.size(); i++) {
		char c=tmp.at(i);
#ifdef WIN32
		if ((i==2) && (c == '\\'))
			continue;
#endif
		if (c==PATH_DELIMITER) {
			const std::string tocreate=tmp.substr(0,i);
			if (!fileSystem->directoryExists(tocreate)) {
				if (MKDIR(tmp.substr(0,i).c_str())!=0)
					return false;
			}
		}
	}

	if ((!directoryExists(tmp.c_str())) && (MKDIR(tmp.c_str()))!=0)
		return false;
	return true;
}
#undef MKDIR


void CFileSystem::getPoolFilename(const std::string& md5str, std::string& path)
{
	path = fileSystem->getSpringDir();
	path += PATH_DELIMITER;
	path += "pool";
	path += PATH_DELIMITER;
	path += md5str.at(0);
	path += md5str.at(1);
	path += PATH_DELIMITER;
	path +=	md5str.substr(2);
	path += ".gz";
}


int CFileSystem::validatePool(const std::string& path)
{
	if(!directoryExists(path)) {
		LOG_ERROR("Pool directory doesn't exist: %s", path.c_str());
		return 0;
	}
	int res=0;
	std::list <std::string*>dirs;
	dirs.push_back(new std::string(path));
	int maxdirs=257; //FIXME: unknown dirs in pool will break bar
	int finished=0;
	IHash* md5=new HashMD5();
	while(!dirs.empty()) {
		struct dirent* dentry;
		DIR* d;
		std::string* dir=dirs.front();
		dirs.pop_front();
		d=opendir(dir->c_str());
		while ( (dentry=readdir(d))!=NULL) {
			LOG_PROGRESS(finished, maxdirs);
			std::string absname=dir->c_str();
			absname += PATH_DELIMITER;
			absname += dentry->d_name;
			if (dentry->d_name[0]!='.') { //don't check hidden files / . / ..
#ifndef WIN32
				if ((dentry->d_type & DT_DIR)!=0) { //directory
#else
				struct stat sb;
				stat(absname.c_str(), &sb);
				if((sb.st_mode & S_IFDIR)!=0) {
#endif
					dirs.push_back(new std::string(absname));
				} else {
					FileData filedata=FileData();
					int len=absname.length();
					if (len<36) { //file length has at least to be <md5[0]><md5[1]>/<md5[2-30]>.gz
						LOG_ERROR("Invalid file: %s", absname.c_str());
					} else {
						std::string md5str="";
						md5str.push_back(absname.at(len-36)); //get md5 from path + filename
						md5str.push_back(absname.at(len-35));
						md5str.append(absname.substr(len-33, 30));
						md5->Set(md5str);
						for(unsigned i=0; i<16; i++) {
							filedata.md5[i]=md5->get(i);
						}

						if (!fileIsValid(&filedata, absname)) { //check if md5 in filename is the same as in filename
							LOG_ERROR("Invalid File in pool: %s",absname.c_str());
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
	delete md5;
	LOG_PROGRESS(finished, maxdirs, true);
	LOG("");
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
	t =  (date.QuadPart / 10000000) - 11644473600LL;
#else
	time(&t);
#endif
	LOG_DEBUG("%s is %d seconds old, redownloading at %d",filename.c_str(), (int)(t - sb.st_ctime), secs);
	return (t<sb.st_ctime+secs);
}

bool CFileSystem::fileExists(const std::string& filename)
{
	struct stat buffer;
	return stat(filename.c_str(), &buffer) == 0;
}

bool CFileSystem::parseTorrent(const char* data, int size, IDownload* dl)
{
	struct be_node* node=be_decoden(data, size);
//#ifdef DEBUG
//	be_dump(node);
//#endif
	if(node==NULL) {
		LOG_ERROR("couldn't parse torrent");
		return false;
	}
	if (node->type!=BE_DICT) {
		LOG_ERROR("Error in torrent data");
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
		LOG_ERROR("couldn't find info node in be dict");
		be_free(node);
		return false;
	}
	for (i = 0; infonode->val.d[i].val; ++i) { //fetch needed data from dict and fill into dl
		struct be_node*datanode;
		datanode=infonode->val.d[i].val;
		switch(datanode->type) {
		case BE_STR: //current value is a string
			if ((strcmp("name",infonode->val.d[i].key)==0) && (dl->name.empty())) { //set filename if not already set
				dl->name=datanode->val.s;
			} else if (!strcmp("pieces", infonode->val.d[i].key)) { //hash sum of a piece
				const int count = be_str_len(datanode)/20; //one sha1 sum is 5 * 4 bytes long
				for (int i=0; i<count; i++) {
					struct IDownload::piece piece;
					const unsigned char* data=(unsigned char*)&datanode->val.s[i*20];
					piece.sha=new HashSHA1();
					if (!piece.sha->Set(data, 20)) {
						LOG_ERROR("Error setting sha1");
					}
					piece.state=IDownload::STATE_NONE;
					dl->pieces.push_back(piece);
				}
			}
			break;
		case BE_INT: //current value is a int
			if (strcmp("length",infonode->val.d[i].key)==0) { //filesize
				dl->size=datanode->val.i;
			} else if (!strcmp("piece length",infonode->val.d[i].key)) { //length of a piece
				dl->piecesize=datanode->val.i;
			}
			break;
		default:
			break;
		}
	}
	LOG_DEBUG("Parsed torrent data: %s %d", dl->name.c_str(), dl->piecesize);
	be_free(node);
	return true;
}

bool CFileSystem::dumpSDP(const std::string& filename)
{
	std::list<FileData*> files;
	files.clear();
	if (!parseSdp(filename, files))
		return false;
	LOG_INFO("md5 (filename in pool)           crc32        size filename");
	std::list<FileData*>::iterator it;
	HashMD5 md5;
	for(it=files.begin(); it!=files.end(); ++it) {
		md5.Set((*it)->md5, sizeof((*it)->md5));
		LOG_INFO("%s %.8X %8d %s",md5.toString().c_str(), (*it)->crc32, (*it)->size, (*it)->name.c_str());
	}
	return true;
}

bool CFileSystem::extract(const std::string& filename, const std::string& dstdir)
{
	LOG_INFO("%s %s", filename.c_str(), dstdir.c_str());
	const int len = filename.length();
	IArchive* archive;
	if ((len>4) && (filename.compare(len-3, 3,".7z") == 0 ) ) {
		archive = new CSevenZipArchive(filename);
	} else {
		archive = new CZipArchive(filename);
	}

	const unsigned int num = archive->NumFiles();
	for (unsigned int i=0; i<num; i++) {
		std::vector<unsigned char> buf;
		std::string name;
		int size;
		archive->FileInfo(i,name, size);
		if (!archive->GetFile(i, buf)) {
			LOG_ERROR("Error extracting %s from %s", name.c_str(), filename.c_str());
			delete archive;
			return false;
		}
#ifdef WIN32
		for(unsigned int i=0; i<name.length(); i++) { //replace / with \ on win32
			if (name[i] == '/')
				name[i]=PATH_DELIMITER;
		}
#endif
		std::string tmp = dstdir + PATH_DELIMITER;
		tmp += name.c_str(); //FIXME: concating UTF-16
		createSubdirs(tmp);
		LOG_INFO("extracting (%s)", tmp.c_str());
		FILE* f=fopen(tmp.c_str(), "wb+");
		if (f == NULL) {
			LOG_ERROR("Error creating %s", tmp.c_str());
			delete archive;
			return false;
		}
		fwrite(&buf[0], buf.size(), 1,f);
		const int err=ferror(f);
#ifndef WIN32
		fchmod(fileno(f), S_IXUSR|S_IWUSR|S_IRUSR|S_IXGRP|S_IRGRP|S_IXOTH|S_IROTH); //FIXME: use attributes from 7z archive
#endif
		if (err) {
			fclose(f);
			delete archive;
			return false;
		}
		fclose(f);
	}
	delete archive;
	LOG_INFO("done");
	return true;
}

bool CFileSystem::Rename(const std::string& source, const std::string& destination)
{
	int res = rename(source.c_str(), destination.c_str());
	return (res==0);
}


std::string CFileSystem::EscapePath(const std::string& path)
{
	std::string tmp;
	for (unsigned int i=0; i<path.size(); i++) {
		if ( (path[i] != '/')
		     && (path[i] != '\\')
		     && (path[i] != '.'))
			tmp += path[i];
	}
	return tmp;
}
