#pragma once

#include "ArchiveEntry.h"
#include "BitArray.h"
#include "Crc32.h"
#include "Md5.h"
#include "FileEntry.h"
#include "Store.h"
#include "Logger.h"

#include <map>
#include <string>
#include <stdexcept>
#include <vector>

namespace Rapid {

class PoolArchiveT
{
	private:
	StoreT & mStore;
	std::map<std::string, FileEntryT> mEntries;

	public:
	PoolArchiveT(StoreT & Store);

	void clear();
	void load(DigestT const & Digest);
	void add(std::string Name, FileEntryT const & Entry);
	void remove(std::string Name);
	void removePrefix(std::string Prefix);
	ArchiveEntryT save();
	DigestT getDigest();
	ChecksumT getChecksum();
	void makeZip(std::string const & Path);

	template<typename FunctorT>
	void iterate(BitArrayT const & Bits, FunctorT Functor)
	{
		if (Bits.size() < mEntries.size()) {
			LOG_ERROR("To few bits received: %d < %d", Bits.size(), mEntries.size());
			throw std::runtime_error{"Not enough bits"};
		}

		auto BitIndex = 0;
		for (auto & Pair : mEntries)
		{
			if (!Bits[BitIndex++]) continue;
			Functor(Pair.second);
		}
	}
};

}
