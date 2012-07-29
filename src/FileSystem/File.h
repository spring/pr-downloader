/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _FILE_H_
#define _FILE_H_

#include <string>
#include <vector>
#include <map>

#include "IFile.h"

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


class CFile: public IFile
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
	CFile();
	~CFile();
	/**
	*	hashes a piece with given hashes (or complete file, if piece<=0)
	*/
	bool Hash(IHash& hash, int piece=-1);
	/**
	*	open file
	*/
	bool Open(const std::string& filename, long size=-1, int piecesize=-1);
	/**
	*	close file
	*/
	void Close();
	/**
	*	read buf from file, starting at restored piece pos, if piece>=0
	*/
    int Read(const char* buf, int bufsize, int piece=-1);
	/**
	*	write buf to file, starting at last pos restored from piece, if piece>=0
	*/
	int Write(const char* buf, int bufsize, int piece=-1);
	/**
	*	gets the size of the given pice, returns file size when piece<0. hint: first piece=0
	*	@return the size of a peace
	*/
	int GetPieceSize(int piece=-1) const;
	/**
	*	gets the read/write position of piece
	*/
	long GetPiecePos(int piece=-1) const;
	bool IsNewFile();
private:
	/**
	* seek to the (relative) position in the piece
	* @return the (relative) position in the piece, -1 means absolute
	*/
	int Seek(unsigned long pos, int piece=-1);
	/**
	* set the size of a pice
	* @return count of pieces
	*/
	bool SetPieceSize(int pieceSize);
	/**
	* inc position of piece after read/write
	*/
	void SetPos(long pos, int piece);
	/**
	* retrieves the size of the file handle
	*/
	long GetSizeFromHandle() const;
	std::string filename;
	FILE* handle; //file handle
	int piecesize; //size of a piece
	long size; //file size
	unsigned long curpos; //current file pointer absolute position
	std::vector <CFilePiece> pieces; //pieces of the file
	std::map <std::string, IHash*> hashs; //checksums for the complete file
	bool isnewfile;
};

#endif
