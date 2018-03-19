#pragma once

#include "Store.h"

namespace Rapid {

struct LastT
{
	std::uint32_t RevisionNum;
	DigestT Digest;

	static void save(LastT const & Last, StoreT & Store, std::string const & Prefix);
	static LastT load(StoreT & Store, std::string const & Prefix);
};

}
