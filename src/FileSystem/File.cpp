/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "File.h"
#include "FileSystem.h"
#include "Logger.h"
#include "IHash.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef WIN32
#include <windows.h>
#endif

CFile::CFile():
	handle(NULL),
	piecesize(-1),
	size(-1),
	curpos(0),
	isnewfile(true),
	timestamp(0)
{
}

CFile::~CFile()
{
	//TODO: write buffered data
	Close();
}
void CFile::Close()
{
	if (handle!=0) {
		fclose(handle);
		if (IsNewFile()) {
			unlink(filename.c_str()); //delete possible existing destination file
			rename(tmpfile.c_str(), filename.c_str());
			isnewfile = false;
		}
		handle=0;
	}
}

bool CFile::Open(const std::string& filename, long size, int piecesize)
{
	LOG_DEBUG("%s %d %d", filename.c_str(), size, piecesize);
	this->filename=filename;
	this->size=size;
	fileSystem->createSubdirs(filename);
	SetPieceSize(piecesize);
//	fileSystem->createSubdirs(filename);
	if (handle!=NULL) {
		LOG_ERROR("file opened before old was closed");
		return false;
	}
	struct stat sb;
	int res=stat(filename.c_str(), &sb);
	timestamp = 0;
	isnewfile=res!=0;
	if (isnewfile) { //if file is new, create it, if not, open the existing one without truncating it
		tmpfile = filename + ".tmp";
		handle=fopen(tmpfile.c_str(), "wb+");
	} else {
		handle=fopen(filename.c_str(), "rb+");
		timestamp = sb.st_mtime;
	}
	if (handle==0) {
		LOG_ERROR("open(%s): %s",filename.c_str(), strerror(errno));
		return false;
	}

	if((!isnewfile) && (size>0) && (size!=sb.st_size)) { //truncate file if real-size != excepted file size
		int ret=ftruncate(fileno(handle), size);
		if (ret!=0) {
			LOG_ERROR("ftruncate failed");
		}
		LOG_ERROR("File already exists but file-size missmatched");
	} else if (size<=0) {
		//TODO: allocate disk space
	}
	LOG_INFO("opened %s", filename.c_str());
	return true;
}

bool CFile::Hash(IHash& hash, int piece)
{
//	LOG("Hash() piece %d", piece);
	char buf[IO_BUF_SIZE];
	SetPos(0, piece);
	hash.Init();
//	LOG("piece %d left: %d",piece,  GetPieceSize(piece));
	int read=0;
	long unsigned left=GetPieceSize(piece); //total bytes to hash
	if (left==0) {
		LOG_ERROR("tried to hash empty piece %d", piece);
		return false;
	}

	while(left>0) {
		int toread=std::min(left, (long unsigned)sizeof(buf));
		read=Read(buf, toread, piece);
		if(read<=0) {
			LOG_ERROR("EOF or read error on piece %d, left: %d toread: %d size: %d, GetPiecePos %d GetPieceSize(): %d read: %d", piece, left, toread, GetPieceSize(piece), GetPiecePos(piece), GetPieceSize(piece), read);
			LOG_ERROR("curpos: %d", curpos);
			return false;
		}
		hash.Update(buf, toread);
		left=left-toread;
	}
	hash.Final();
	SetPos(0, piece);
//	LOG("CFile::Hash(): %s piece:%d", hash.toString().c_str(), piece);
	return true;
}

int CFile::Read(char *buf, int bufsize, int piece)
{
	SetPos(GetPiecePos(piece), piece);
//	LOG("Read(%d) bufsize: %d GetPiecePos(): %d GetPieceSize() %d",piece, bufsize, GetPiecePos(piece), GetPieceSize(piece));
	clearerr(handle);
	int items=fread(buf, bufsize, 1, handle);
	if (items<=0) {
		if(ferror(handle)) {
			LOG_ERROR("read error %s bufsize: %d curpos: %d GetPieceSize: %d", strerror(errno), bufsize, curpos, GetPieceSize());
			SetPos(0, piece);
			return -1;
		}
		if(feof(handle)) {
			LOG_ERROR("EOF while Read: '%s' items: %d!", strerror(errno), items);
			LOG_ERROR("read error %s bufsize: %d curpos: %d GetPieceSize: %d", strerror(errno), bufsize, curpos, GetPieceSize());
			return -1;
		}
	}
	SetPos(GetPiecePos(piece)+bufsize, piece); //inc pos
	return bufsize;
}

void CFile::SetPos(long pos, int piece)
{
//	LOG("SetPos() pos %d piece%d", pos, piece);
	if (piece>=0) {
		//LOG_DEBUG("pos: %d piecesize: %d", pos, piecesize);
		assert(pieces[piece].pos<=size+pos);
		assert(pos<=piecesize);
		pieces[piece].pos =pos;
	} else {
		assert(size<=0 || pos<=size);
		curpos = pos;
	}
	Seek(pos, piece);
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	SetPos(GetPiecePos(piece), piece);
	clearerr(handle);
//	LOG("Write() bufsize %d piece %d handle %d", bufsize, piece, fileno(handle));
	int res=fwrite(buf, bufsize, 1, handle);
	if (res!=1)
		LOG_ERROR("write error %d", res);
//	LOG("wrote bufsize %d", bufsize);
	if(ferror(handle)!=0) {
		LOG_ERROR("Error in write(): %s", strerror(errno));
		abort();
	}
	if(feof(handle)) {
		LOG_ERROR("EOF in write(): %s", strerror(errno));
	}
	SetPos(GetPiecePos(piece)+bufsize, piece);
	/*	if ((piece>=0) && (GetPiecePos(piece)==GetPieceSize(piece))) {
			LOG("piece finished: %d", piece);
		}
	*/
	return bufsize;
}


int CFile::Seek(unsigned long pos, int piece)
{
	assert(piece<=(int)pieces.size());
	if(piece>=0) { //adjust position relative to piece pos
		pos=this->piecesize*piece+pos;
	}
//	LOG("Seek() pos: %d piece: %d", pos, piece);
	clearerr(handle);
	if (fseek(handle, pos, SEEK_SET)!=0) {
		LOG_ERROR("seek error %ld", pos);
	}
	return pos;
}

bool CFile::SetPieceSize(int pieceSize)
{
	assert(handle==NULL); //this function has to be called before the file is opened
	pieces.clear();
	if ((size<=0) || (pieceSize<=0)) {
		LOG_DEBUG("SetPieceSize(): FileSize:%ld PieceSize: %d", size, pieceSize);
		return false;
	}
	if (size<pieceSize) {
		pieceSize = size;
		LOG_DEBUG("SetPieceSize(): forcing lower pieceSize: %d", pieceSize);
	}
	unsigned count=this->size/pieceSize;
	if (count==pieces.size()) //check if size is already correct
		return true;
	pieces.clear();
	if(this->size%pieceSize>0)
		count++;
	if(count==0) {
		LOG_ERROR("SetPieceSize(): count==0");
		return false;
	}
	for(unsigned i=0; i<count; i++) {
		pieces.push_back(CFilePiece());
	}
	piecesize=pieceSize;
	curpos=0;
//	LOG("SetPieceSize piecesize: %d filesize: %ld pieces count:%d", pieceSize, this->size, (int)pieces.size());
	return true;
}

int CFile::GetPieceSize(int piece) const
{
	if (piece>=0) {
		assert(piece<=(int)pieces.size());
		if ((int)pieces.size()-1==piece) //last piece
			return size-(piecesize*((int)pieces.size()-1));
//		LOG("GetPieceSize piece %d, pieces.size() %d piecesize: %d size %d ", piece, pieces.size(),piecesize, size);
		return piecesize;
	}
	if (size<0) {
		return GetSizeFromHandle();
	}
	return size;
}

long CFile::GetPiecePos(int piece) const
{
	assert(piece<=(int)pieces.size());
	if (piece>=0)
		return pieces[piece].pos;
	return curpos;
}

long CFile::GetSizeFromHandle() const
{
	if(handle==NULL) {
		LOG_ERROR("GetSize(): file isn't opened!");
		return -1;
	}

	struct stat sb;
	if (fstat(fileno(handle), &sb)!=0) {
		LOG_ERROR("CFile::SetSize(): fstat failed");
		return -1;
	}
//	LOG("GetSizeFromHandle: %d blocks: %d", sb.st_size, sb.st_blocks);
	return sb.st_size;
}

bool CFile::IsNewFile()
{
	return isnewfile;
}

long CFile::GetTimestamp()
{
	return timestamp;
}

bool CFile::SetTimestamp(long timestamp)
{
#ifdef WIN32
	FILETIME ftime;
	HANDLE h;
	bool close = false;
	if (handle==NULL) {
		h = CreateFile(filename.c_str() , GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		close = true;
	} else {
		h = (HANDLE)_get_osfhandle(fileno(handle));
	}
	if (h == NULL) {
		return false;
	}
	fileSystem->TimestampToFiletime(timestamp, ftime);
	bool ret = SetFileTime(h, NULL, NULL, &ftime);
	if (close) { //close opened file
		CloseHandle(h);
	}
	return ret;
#else
	struct timeval tv = {0, 0};
	tv.tv_sec = timestamp;
	if (handle==NULL) {
		return lutimes(filename.c_str(), &tv) == 0;
	} else {
		return futimes(fileno(handle), &tv) == 0;
	}
#endif
}
