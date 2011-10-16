#ifndef _FILE_H_
#define _FILE_H_

#include <string>
#include <vector>
#include <list>
#include <map>

class IHash;


class CFilePiece
{
public:
	CFilePiece() {
		this->valid=false;
		this->pos=0;
	};
	bool valid; //checksum validated
	int pos; //current relative read/write pos
};

class CFile
{
public:
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
	/**
	*	hashes a piece with given hashes (or complete file, if piece<=0)
	*/
	bool Hash(IHash& hash, int piece=-1);
	/**
	*	open file
	*/
	bool Open(const std::string& filename);
	/**
	*	close file
	*/
	void Close();
	/**
	*	read buf from file, starting at restored piece pos, if piece>=0
	*/
	int Read(char* buf, int bufsize, int piece=-1);
	/**
	*	write buf to file, starting at last pos restored from piece, if piece>=0
	*/
	int Write(const char* buf, int bufsize, int piece=-1);
	/**
	* seek to the (relative) position in the piece
	* @return the (relative) position in the piece, -1 means absolute
	*/
	int Seek(unsigned long pos, int piece=-1);
	/**
	*	gets the size of the given pice, returns file size when piece<0. hint: first piece=0
	*	@return the size of a peace
	*/
	int GetPieceSize(int piece=-1);
	/**
	*	gets the read/write position of piece
	*/
	int GetPiecePos(int piece=-1);
private:
	/**
	* set the size of a pice
	* @return count of pieces
	*/
	bool SetPieceSize(int size);
	/**
	* inc position of piece after read/write
	*/
	void IncPos(int piece, int pos);
	/**
	*	restore position of piece for read/write operations
	*/
	void RestorePos(int piece=-1);
	/**
	* reset given read/write pos of piece to 0
	*/
	void ResetPos(int piece=-1);
	FILE* handle; //file handle
	int piecesize; //size of a piece
	long size; //file size
	unsigned long curpos; //current file pointer absolute position
	std::vector <CFilePiece> pieces; //pieces of the file
	std::map <std::string, IHash*> hashs; //checksums for the complete file

};

#endif
