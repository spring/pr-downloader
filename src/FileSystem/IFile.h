/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#ifndef _IFILE_H_
#define _IFILE_H_

class IFile
{
public:
	/**
	* general file abstraction, allows to write to pieces of a file and create hashes
	* of the complete fiele or pieces of it
	*/
	/**
	*	create a new file
	*	@param filename filename of the file
	*/
	virtual bool Open(const std::string& filename){return false;}
	/**
	*	close file
	*/
	virtual void Close(){}
	/**
	*	read buf from file, starting at restored piece pos, if piece>=0
	*/
	virtual int Read(char* buf, int bufsize){return 0;}
	/**
	*	write buf to file, starting at last pos restored from piece, if piece>=0
	*/
	virtual int Write(const char* buf, int bufsize){return 0;}
};

#endif
