#include "File.h"
#include "FileSystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


CFile::CFile(const std::string& filename, int size, int piecesize)
{
//	fileSystem->createSubdirs(filename);
	if (fileSystem->fileExists(filename))
		handle=open(filename.c_str(), O_CREAT);
	this->piecesize=piecesize;
	this->size=size;
	if ( (size>0) && (piecesize>0)) {
		const int pieces = size / piecesize;
		for (int i=0; i<pieces; i++) {
			this->pieces[i]=CPiece();
		}

	}
}

CFile::~CFile()
{
	//TODO: write buffered data
	close(handle);
}

bool CFile::Validate(std::map <std::string, IHash> hashs, int piece)
{
//	if (piece>=0)
	//FIXME
	return true;
}

int CFile::Read(char*buf, int bufsize, int piece)
{
//	if (piece>=0)
	int bytes=read(handle, buf, bufsize);
	return bytes;
}

int CFile::Write(const char*buf, int bufsize, int piece)
{
	if (piece>=0) {
		const CPiece& ppiece=getPiece(piece);
		Seek(ppiece.pos, piece);
	}
	int bytes=write(handle, buf, bufsize);
	return bytes;
}

int CFile::Seek(int pos, int piece)
{
//	if (piece>=0)
	int offset=lseek(handle, pos, SEEK_SET);
	return offset;
}

