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
	virtual const std::string toString() {
		std::string empty;
		return empty;
	}
	/**
	*	compare this hash
	*	@return true, when both hashes are identical
	*/
	virtual bool compare(const IHash& checksum);
	/**
	*	compare this hash
	*	@return true, when both hashes are identical
	*/
	virtual bool compare(const unsigned char* data, int size);
protected:
	/**
	*	returns the size of binary hash for comparison
	*/
	virtual int getSize() const=0;
	/**
	*	@return part of binary hash store for comparison
	*/
	virtual unsigned char get(int pos) const=0;
};

#endif
