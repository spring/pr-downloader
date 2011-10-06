#ifndef _FILE_H_
#define _FILE_H_

#include <string>
#include <map>

class IHash;


class CPiece
{
public:
	std::map <std::string, IHash*> hashs; //checksum for a part of a file
	bool validate(std::map <std::string, IHash*> hashs);
	int size;
	int written; //bytes written
	bool valid; //checksum validated
	int pos; //current read/write pos
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
	bool Validate(std::map <std::string, IHash> hashs, int piece=-1);
	int Read(char* buf, int bufsize, int piece=-1);
	int Write(const char* buf, int bufsize, int piece=-1);
	int Seek(int pos, int piece=-1);
	bool SetPieceSize(int size);
	const CPiece& getPiece(int piece);
	//int Open(const std::string& filename);
	//int Close();
	/**
	*	set the size of a pice
	* @return count of pieces
	*/
	int setpiecesieze(int size);

private:
	int handle; //file handle
	int piecesize; //size of a piece
	int size; //file size
	std::map <int, CPiece> pieces; //pieces of the file
	std::map <std::string, IHash*> hashs; //checksums for the complete file

};

#endif
