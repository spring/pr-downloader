#ifndef LIBSPRINGLOBBY_HEADERGUARD_CRTPBASE_H
#define LIBSPRINGLOBBY_HEADERGUARD_CRTPBASE_H

#include <string>

namespace LSL
{

//! minimal interface for classes usable as Items for ContainerBase
template <class KeyImp>
struct HasKey
{
	typedef KeyImp
	    KeyType;
	//! This will be used a sthe index in std::map like container, needs to be unique
	virtual KeyImp key() const = 0;
	//! A readble string to identify the indexed class type by
	static std::string className();
};

} //namespace LSL {

#endif // LIBSPRINGLOBBY_HEADERGUARD_CRTPBASE_H
