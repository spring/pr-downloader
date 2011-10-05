#ifndef _FILE_H_
#define _FILE_H_

#include <list>

class CChecksum;


class CPiece
{
	CChecksum& checksum;
	//checksum;
	int size;
	int written; //bytes written
	bool valid; //checksum validated
};

class CFile
{
	CFile();



private:
	int handle;
	std::list<CPiece> pieces;


};

#endif
