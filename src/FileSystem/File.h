#ifndef _FILE_H_
#define _FILE_H_

#include <string>
#include <map>
#include <list>

class IHash;


class CPiece
{
public:
	CPiece();
	CPiece(int size) {
		this->size=size;
		this->written=0;
		this->valid=false;
		this->pos=0;
	};
	int size; //size of the piece, can be smaller than piecesize in the file
	int written; //bytes written
	bool valid; //checksum validated
	int pos; //current absolute read/write pos
};

class CFile
{
	/**
	* general file abstraction, allows to write to pieces of a file and create hashes
	* of the complete fiele or pieces of it
	*/
	/**
	*	create a new file
	*	@param filename filename of the file
	*	@param size of the filename, -1 will read it from file or create a new one
	*/
	CFile(const std::string& filename, int size=-1, int piecesize=-1);
	~CFile();
	/*
	* hashes a piece with given hashes (or complete file, if piece<=0)
	*/
	bool Hash(std::list <IHash*> hashs, int piece=-1);

	bool Open(const std::string& filename);
	void Close();
	int Read(char* buf, int bufsize, int piece=-1);
	int Write(const char* buf, int bufsize, int piece=-1);
	/**
	* seek to the (relative) position in the piece
	* @return the (relative) position in the piece, -1 means absolute
	*/
	int Seek(int pos, int piece=-1);
	int getSize();

private:
	/**
	* set the size of a pice
	* @return count of pieces
	*/
	bool SetPieceSize(int size);
	int handle; //file handle
	int piecesize; //size of a piece
	int size; //file size
	std::map <int, CPiece> pieces; //pieces of the file
	std::map <std::string, IHash*> hashs; //checksums for the complete file

};

#endif
