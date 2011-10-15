#ifndef _CCHECKSUM_H
#define _CCHECKSUM_H

#include <string>
#include <assert.h>


class IHash
{
public:
	/**
	*	Init Hash
	*/
	virtual void Init()=0;
	/**
	*	Finalize Hash
	*/
	virtual void Final()=0;
	/**
	*	Update Hash with given data
	*/
	virtual void Update(const char* data,const int size)=0;
	/**
	*	return human readable hash string
	*/
	virtual const std::string toString(const unsigned char* data=NULL, int size=0);
	/**
	*	compare this hash
	*	@return true, when both hashes are identical
	*/
	virtual bool compare(const IHash* checksum);
	/**
	*	compare this hash
	*	@return true, when both hashes are identical
	*/
	virtual bool compare(const unsigned char* data, int size);
	/**
	* Set the md5 hash
	*/
	virtual bool Set(unsigned char* data, int size)=0;

	virtual bool Set(const std::string& hash);
	/**
	*	returns the size of binary hash for comparison
	*/
	virtual int getSize() const=0;
protected:
	/**
	*	@return part of binary hash store for comparison
	*/
	virtual unsigned char get(int pos) const=0;
private:
	/**
	* convert hex to int
	*/
	unsigned getVal(char c);

};

#endif
