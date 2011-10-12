#include "File.h"
#include "FileSystem.h"
#include "Logger.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

CFile::CFile(const std::string& filename, int size, int piecesize)
{
	handle=NULL;
	Open(filename);
	if (piecesize<=0)
		this->piecesize=1;
	else
		this->piecesize=piecesize;
	this->size=size;
	if(size>0)
		SetPieceSize(piecesize);
	curpos=0;
}

CFile::~CFile()
{
	//TODO: write buffered data
	Close();
}
void CFile::Close()
{
	if (handle!=0)
		fclose(handle);
	handle=0;
}

bool CFile::Open(const std::string& filename)
{
//	fileSystem->createSubdirs(filename);
	if (handle!=0) {
		LOG_ERROR("file opened before old was closed\n");
		return false;
	}
	handle=fopen(filename.c_str(), "wb+");
	if (handle<=0)
		LOG_ERROR("open(%s): %s\n",filename.c_str(), strerror(errno));
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
	long unsigned left=GetPieceSize(piece); //total bytes to hash
//	LOG("piece %d left: %d\n",piece,  GetPieceSize(piece));
	pieces[piece].pos=0; //reset read/write pos
	int read=0;
	while(left>0) {
		int toread=std::min(left, (long unsigned)sizeof(buf));
		read=Read(buf, toread, piece);
		if(read<=0) {
			LOG_ERROR("EOF or read error on piece %d, left: %d toread: %d size: %d\n", piece, left, toread, GetPieceSize(piece));
			LOG_ERROR("%d\n", GetPieceSize(piece));
			return false;
		}
		left=left-read;
		for(it=hashs.begin(); it!=hashs.end(); ++it) {
			(*it)->Update(buf, bytes);
		}
	}
	if (left>0) {
		LOG_ERROR("Couldn't read all bytes for hashing, %d\n", left);
		return false;
	}
	for(it=hashs.begin(); it!=hashs.end(); ++it) {
		(*it)->Final();
	}
	return true;
}

int CFile::Read(char*buf, int bufsize, int piece)
{
	RestorePos(piece);
//	LOG("reading %d\n", bufsize);
	int items=fread(buf, bufsize, 1, handle);
	if(ferror(handle)!=0) {
		int piecepos=0;
		if (piece>0)
			piecepos=pieces[piece].pos;
		LOG_ERROR("read error %s bufsize: %d piecepos:%d curpos: %d\n", strerror(errno), bufsize, piecepos, curpos);
		return items;
	}
	IncPos(piece, bufsize);
	return bufsize;
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
		assert(pieces[piece].pos<=size+pos);
		assert(pos<=piecesize);
		pieces[piece].pos +=pos;
	} else {
		assert(curpos<=size);
		curpos += pos;
	}
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	RestorePos(piece);
	fwrite(buf, bufsize, 1, handle);
//	LOG("wrote bufsize %d\n", bufsize);
	if(ferror(handle)!=0) {
		LOG_ERROR("Error in write(): %s\n", strerror(errno));
		abort();
	}
	IncPos(piece, bufsize);
	/*
		if ((piece>=0) && (pieces[piece].pos==GetPieceSize(piece))) {
			LOG("piece finished: %d\n", piece);
		}
	*/
	return bufsize;
}


int CFile::Seek(int pos, int piece)
{
	if(piece>=0) { //adjust position relative to piece pos
		pos=this->piecesize*piece+pos;
	}

	if (curpos!=pos) { //only seek if needed
		int ret=fseek(handle, pos, SEEK_SET);
		if (ret!=0) {
			LOG_ERROR("seek error %d %d\n", ret, pos);
		}
		curpos=pos;
	}
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
	if(this->size%size>0)
		count++;
	if(count<=0) {
		LOG_ERROR("SetPieceSize(): count<0\n");
		return false;
	}
	for(int i=0; i<count; i++) {
		pieces.push_back(CPiece());
		pieces[i]=CPiece();
	}
	piecesize=size;
	LOG("SetPieceSize: %d\n", pieces.size());
	return true;
}

int CFile::GetPieceSize(int piece)
{
	if (piece>=0) {
		assert(piece<=(int)pieces.size());
		if (pieces.size()-1==piece) //last piece
			return size%piecesize;
		return piecesize;
	}
	return size;
}
