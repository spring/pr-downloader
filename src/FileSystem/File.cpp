#include "File.h"
#include "FileSystem.h"
#include "Logger.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>


CFile::CFile(const std::string& filename, long size, int piecesize)
{
	handle=NULL;
	LOG("CFile() size: %d\n",size);
	this->size=size;
	this->piecesize=piecesize;
	this->curpos=0;
	this->filename=filename;
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
	SetPieceSize(piecesize);
//	fileSystem->createSubdirs(filename);
	if (handle!=NULL) {
		LOG_ERROR("file opened before old was closed\n");
		return false;
	}
	struct stat sb;
	int res=stat(filename.c_str(), &sb);
	if (res==0) { //check if file length is correct, if not set it
		handle=fopen(filename.c_str(), "r+");
	} else {
		handle=fopen(filename.c_str(), "w+");
	}
	if (handle<=0) {
		LOG_ERROR("open(%s): %s\n",filename.c_str(), strerror(errno));
		return false;
	}

	if((size>0) && (size!=res)) { //truncate file if real-size != excepted file size
		int ret=ftruncate(fileno(handle), size);
		if (ret!=0) {
			LOG_ERROR("ftruncate failed\n");
		}
		LOG_ERROR("File already exists but file-size missmatched\n");
	} else if (size<=0) {
		size=GetSize();
	}
	LOG_INFO("opened %s\n", filename.c_str());
	return true;
}

bool CFile::Hash(IHash& hash, int piece)
{
	LOG("Hash() piece %d\n", piece);
	if (handle==NULL)
		Open(filename);

	char buf[IO_BUF_SIZE];
	SetPos(0, piece);
	hash.Init();
	//	LOG("piece %d left: %d\n",piece,  GetPieceSize(piece));
	int read=0;
	long unsigned left=GetPieceSize(piece); //total bytes to hash

	while(left>0) {
		int toread=std::min(left, (long unsigned)sizeof(buf));
		read=Read(buf, toread, piece);
		if(read<0) {
			LOG_ERROR("EOF or read error on piece %d, left: %d toread: %d size: %d, GetPiecePos %d GetPieceSize(): %d read: %d\n", piece, left, toread, GetPieceSize(piece), GetPiecePos(piece), GetPieceSize(piece), read);
			LOG_ERROR("curpos: %d\n", curpos);
			return false;
		}
		left=left-read;
		hash.Update(buf, read);
	}
	hash.Final();
	SetPos(0, piece);
//	LOG("CFile::Hash(): %s\n", hash.toString().c_str());
	return true;
}

int CFile::Read(char*buf, int bufsize, int piece)
{
	LOG("Read() bufsize %d piece %d\n", bufsize, piece);
	if(handle==NULL)
		Open(filename);
//	LOG("read: %d %d %d total: %d\n", pieces[piece].pos, curpos, bufsize, pieces[piece].pos+bufsize);
	RestorePos(piece);
//	LOG("reading %d\n", bufsize);
//	clearerr(handle);
	int items=fread(buf, bufsize, 1, handle);
	if (items<=0) {
		if(feof(handle)) {
			LOG_ERROR("EOF while Read: '%s'!\n", strerror(errno));
			return -1;
		}
		if(ferror(handle)) {
			LOG_ERROR("read error %s bufsize: %d curpos: %d GetPieceSize: %d\n", strerror(errno), bufsize, curpos, GetPieceSize());
			SetPos(0, piece);
			return items;
		}
	}
	SetPos(GetPiecePos(piece)+bufsize, piece); //inc pos
	return bufsize;
}

void CFile::RestorePos(int piece)
{
	if (piece>=0) {
		assert(piece<=(int)pieces.size());
		Seek(GetPiecePos(piece), piece);
	}
}

void CFile::SetPos(long pos, int piece)
{
	LOG("IncPos() pos %d piece%d\n", pos, piece);
	if (piece>=0) {
		assert(pieces[piece].pos<=size+pos);
		assert(pos<=piecesize);
		pieces[piece].pos =pos;
	} else {
		assert(size<=0 || (long)curpos<=size);
		curpos = pos;
	}
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	LOG("Write() bufsize %d piece %d\n", bufsize, piece);
	if(handle==NULL)
		Open(filename);
	RestorePos(piece);
	int res=fwrite(buf, bufsize, 1, handle);
	if (res!=1)
		LOG_ERROR("write error %d\n", res);
//	LOG("wrote bufsize %d\n", bufsize);
	if(ferror(handle)!=0) {
		LOG_ERROR("Error in write(): %s\n", strerror(errno));
		abort();
	}
	if(feof(handle)) {
		LOG_ERROR("EOF in write(): %s\n", strerror(errno));
	}
	SetPos(GetPiecePos(piece)+bufsize, piece);
	if ((piece>=0) && (GetPiecePos(piece)==GetPieceSize(piece))) {
		LOG("piece finished: %d\n", piece);
	}
	return bufsize;
}


int CFile::Seek(unsigned long pos, int piece)
{
//	LOG("Seek() pos: %d piece: %d\n", pos, piece);
	if(handle==NULL)
		Open(filename);
	assert(piece<=(int)pieces.size());
	if(piece>=0) { //adjust position relative to piece pos
		pos=this->piecesize*piece+pos;
	}
	if (curpos!=pos) { //only seek if needed
		if (fseek(handle, pos, SEEK_SET)!=0) {
			LOG_ERROR("seek error %ld\n", pos);
		}
		curpos=pos;
	}
	return pos;
}


bool CFile::SetPieceSize(int pieceSize)
{
	pieces.clear();
	if ((size<=0) || (pieceSize<=0)) {
		LOG_DEBUG("SetPieceSize(): FileSize:%ld PieceSize: %d\n", size, pieceSize);
		return false;
	}
	int count=this->size/pieceSize;
	if(this->size%pieceSize>0)
		count++;
	if(count<=0) {
		LOG_ERROR("SetPieceSize(): count<0\n");
		return false;
	}
	for(int i=0; i<count; i++) {
		pieces.push_back(CFilePiece());
	}
	piecesize=pieceSize;
	curpos=0;
	LOG("SetPieceSize piecesize: %d filesize: %ld pieces count:%d\n", pieceSize, this->size, (int)pieces.size());
	return true;
}

int CFile::GetPieceSize(int piece)
{
	LOG("GetPieceSize piece %d, pieces.size() %d size %d \n", piece, pieces.size(), size);
	if (piece>=0) {
		assert(piece<=(int)pieces.size());
		if ((int)pieces.size()-1==piece) //last piece
			return size%piecesize;
		return piecesize;
	}
	return size;
}

long CFile::GetPiecePos(int piece)
{
	assert(piece<=(int)pieces.size());
	if (piece>=0)
		return pieces[piece].pos;
	return curpos;
}

long CFile::GetSize()
{
	if(handle==NULL) {
		LOG_ERROR("GetSize(): file isn't opened!\n");
		return -1;
	}

	struct stat sb;
	if (fstat(fileno(handle), &sb)!=0) {
		LOG_ERROR("CFile::SetSize(): fstat failed\n");
		return -1;
	}
	return sb.st_size;
}
