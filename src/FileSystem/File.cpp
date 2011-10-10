#include "File.h"
#include "FileSystem.h"
#include "Logger.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

CFile::CFile(const std::string& filename, int size, int piecesize)
{
	Open(filename);
	if (piecesize<=0)
		this->piecesize=1;
	else
		this->piecesize=piecesize;
	this->size=size;
	if(size>0)
		SetPieceSize(piecesize);
}

CFile::~CFile()
{
	//TODO: write buffered data
	Close();
}
void CFile::Close()
{
	if (handle!=0)
		close(handle);
	handle=0;
}

bool CFile::Open(const std::string& filename)
{
//	fileSystem->createSubdirs(filename);
	if (handle!=0) {
		LOG_ERROR("file opened before old was closed");
		return false;
	}
	handle=open(filename.c_str(), O_CREAT|O_RDWR);
	if (handle<0)
		LOG_ERROR("open(): %s\n",strerror(errno));
	return true;
}

bool CFile::Hash(std::list <IHash*> hashs, int piece)
{
	std::list <IHash*>::iterator it;
	char buf[1024];
	int bytes=0;
	for(it=hashs.begin(); it!=hashs.end(); ++it) {
		(*it)->Init();
	}
	int left=pieces[piece].size; //total bytes to hash
	while( (read>0) && (left>0) ) {
		int read=Read(buf, sizeof(buf), piece);
		left=left-read;
		for(it=hashs.begin(); it!=hashs.end(); ++it) {
			(*it)->Update(buf, bytes);
		}
	}
	for(it=hashs.begin(); it!=hashs.end(); ++it) {
		(*it)->Final();
	}
	return true;
}

int CFile::Read(char*buf, int bufsize, int piece)
{
	RestorePos(piece);
	int bytes=read(handle, buf, bufsize);
	IncPos(piece, bytes);
	return bytes;
}

void CFile::RestorePos(int piece)
{
	if (piece>=0) {
		assert(piece<=(int)pieces.size());
		Seek(pieces[piece].pos, piece);
	}
}

void CFile::IncPos(int piece, int pos)
{
	if (piece>=0) {
		pieces[piece].pos +=pos;
		assert(pos<=piecesize);
	} else {
		curpos += pos;
	}
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	RestorePos(piece);
	int bytes=write(handle, buf, bufsize);
	if(bytes<0) {
		LOG_ERROR("Error in write(): %s\n", strerror(errno));
		abort();
	}
	IncPos(piece, bytes);
	return bytes;
}


int CFile::Seek(int pos, int piece)
{
	if(piece>=0) { //adjust position relative to piece pos
		pos=this->piecesize*piece+pos;
	}

	if (curpos!=pos) {
		lseek(handle, pos, SEEK_SET);
		curpos=pos;
	}
	//TODO: error handling
	return pos;
}

bool CFile::SetPieceSize(int size)
{
	pieces.clear();
	if (this->size<=0) {
		LOG_ERROR("SetPieceSize(): FileSize<0\n");
		return false;
	}
	int count=this->size/size;
	if(count<=0) {
		LOG_ERROR("SetPieceSize(): count<0\n");
		return false;
	}
	for(int i=0; i<count; i++) {
		pieces[i]=CPiece(size);
	}
	piecesize=size;
	return true;
}
