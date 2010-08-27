#include "md5.h"
#include "FileSystem.h"
#include "RapidDownloader.h"
#include "RepoMaster.h"
#include "Repo.h"
#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <cstring>


CFileSystem* CFileSystem::singleton = NULL;


bool CFileSystem::fileIsValid(FileData* mod, std::string& filename){
	gzFile inFile = gzopen (filename.c_str(), "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (inFile == NULL) {
		printf("error opening %s\n",filename.c_str());
		return false;
	}
	MD5Init (&mdContext);
	while ((bytes = gzread (inFile, data, 1024)) != 0)
		MD5Update (&mdContext, data, bytes);
	MD5Final (&mdContext);
	gzclose (inFile);
	int i;
	for(i=0; i<16;i++){
		if (mdContext.digest[i]!=mod->md5[i]){ //file is invalid, delete it
			printf("damaged file found, deleting it: %s",filename.c_str());
			unlink(filename.c_str());
			return false;
		}
	}
	return true;
}

static unsigned int parse_int32(unsigned char c[4]){
        unsigned int i = 0;
        i = c[0] << 24 | i;
        i = c[1] << 16 | i;
        i = c[2] << 8  | i;
        i = c[3] << 0  | i;
        return i;
}


/*
	parses the file for a mod and creates
*/
std::list<CFileSystem::FileData*>* CFileSystem::parseSdp(std::string& filename){
	char c_name[255];
	unsigned char c_md5[16];
	unsigned char c_crc32[4];
	unsigned char c_size[4];

	gzFile in=gzopen(filename.c_str(), "r");
	printf("parse_binary: %s\n",filename.c_str());
	if (in==Z_NULL){
        printf("Could not open %s\n",filename.c_str());
		return NULL;
	}
	std::list<FileData*>* tmp;
	tmp=new std::list<FileData*>;
	FileData tmpfile;
	while(!gzeof(in)){
		int length = gzgetc(in);
		if (length == -1) break;
		if (!gzread(in, &c_name, length)) break;
		if (!gzread(in, &c_md5, 16)) break;
		if (!gzread(in, &c_crc32, 4)) break;
		if (!gzread(in, &c_size, 4)) break;

		FileData *f = new FileData;
		f->name = std::string(c_name, length);
		std::memcpy(&f->md5, &c_md5, 16);
		f->crc32 = parse_int32(c_crc32);
		f->size = parse_int32(c_size);
		tmp->push_back(f);
	}
	return tmp;
}



const std::string* CFileSystem::createTempFile(){
	std::string *tmp=new std::string(tmpnam(NULL));
	tmpfiles.push_back(tmp);
	return tmp;
}

CFileSystem::~CFileSystem(){
	std::list<std::string*>::iterator it;
	for (it = tmpfiles.begin();it != tmpfiles.end(); ++it) {
		std::string* filename=(*it);
		remove(filename->c_str());
	}
}

CFileSystem::CFileSystem(){
	tmpfiles.clear();
	springdir="/home/matze/.spring";
}

void CFileSystem::Initialize(){
	singleton=new CFileSystem();
}
void CFileSystem::Shutdown(){
	CFileSystem* tmpFileSystem=singleton;
	singleton=NULL;
	delete tmpFileSystem;
	tmpFileSystem=NULL;
}

const std::string& CFileSystem::getSpringDir(){
	return springdir;
}


