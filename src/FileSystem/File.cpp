#include "File.h"
#include "FileSystem.h"
#include "Logger.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


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
	handle=open(filename.c_str(), O_CREAT);
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
	CPiece* ppiece;
	if (piece>=0) {
		ppiece=&pieces[piece];
		Seek(ppiece->pos, piece);
	}
	int bytes=read(handle, buf, bufsize);
	if (piece>=0) {
		ppiece.pos=ppiece.pos+bytes;
	}
	return bytes;
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	CPiece* ppiece;
	if (piece>=0) {
		ppiece=&pieces[piece];
		Seek(ppiece->pos, piece);
	}
	int bytes=write(handle, buf, bufsize);
	if (piece>=0) {
		ppiece->pos=ppiece->pos+bytes;
	}
	return bytes;
}


int CFile::Seek(int pos, int piece)
{
	if(piece>=0) { //adjust position relative to piece pos
		CPiece& ppiece=pieces[piece];
		pos=this->piecesize*piece+pos;
		ppiece.pos=pos;
	}
	lseek(handle, pos, SEEK_SET);
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

int CFile::getSize()
{
	return size;
}
