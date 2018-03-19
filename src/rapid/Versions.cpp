#include "Versions.h"

#include "Gzip.h"
#include "Hex.h"

#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

namespace Rapid {

VersionsT::VersionsT(StoreT & Store)
:
	mStore(Store)
{}

void VersionsT::clear()
{
	mEntries.clear();
}


namespace {

std::string readUntil(std::string::iterator & First, std::string::iterator Last, char Char)
{
	auto Iter = std::find(First, Last, Char);
	if (Iter == Last) throw std::runtime_error{"Error loading versions.gz"};
	std::string Result {First, Iter};
	First = Iter + 1;
	return Result;
}

std::vector<std::string> splitDepends(std::string const & String)
{
	std::vector<std::string> Depends;

	auto First = String.begin();
	auto Last = String.end();
	if (First != Last)
	{
		while (true)
		{
			auto Iter = std::find(First, Last, '|');
			Depends.push_back({First, Iter});
			if (Iter == Last) break;
			First = Iter + 1;
		}
	}

	return Depends;
}

}

void VersionsT::load()
{
	auto Path = mStore.getVersionsPath();

	struct stat Stats;
	auto Error = stat(Path.c_str(), &Stats);
	if (Error == -1)
	{
		// Its ok if versions.gz doesnt exist, we just begin empty
		if (errno == ENOENT) return;
		else throw std::runtime_error{"Error loading versions.gz"};
	}

	auto Buffer = GzipT::readFile(Path);
	auto First = Buffer.begin();
	auto Last = Buffer.end();

	// Read all entries
	while (First != Last)
	{
		auto Tag = readUntil(First, Last, ',');
		auto Hexed = readUntil(First, Last, ',');
		auto Depends = readUntil(First, Last, ',');
		auto Name = readUntil(First, Last, '\n');

		ArchiveEntryT Entry;
		if (Hexed.size() != 32) throw std::runtime_error{"Error loading versions.gz"};
		Hex::decode(Hexed.data(), Entry.Digest.Buffer, 16);
		Entry.Name = std::move(Name);

		// Split depends into vector
		Entry.Depends = splitDepends(Depends);

		mEntries.insert({std::move(Tag), std::move(Entry)});
	}
}

void VersionsT::save()
{
	auto Path = mStore.getVersionsPath();
	GzipT Out{Path, "wb"};

	for (auto & Pair : mEntries)
	{
		auto & Tag = Pair.first;
		auto & Entry = Pair.second;

		Out.write(Tag.data(), Tag.size());
		Out.write(',');
		char Hexed[32];
		Hex::encode(Hexed, Entry.Digest.Buffer, 16);
		Out.write(Hexed, 32);
		Out.write(',');
		auto Size = Entry.Depends.size();
		if (Size != 0)
		{
			Out.write(Entry.Depends[0].data(), Entry.Depends[0].size());
			for (std::size_t I = 1; I != Size; ++I)
			{
				Out.write('|');
				Out.write(Entry.Depends[I].data(), Entry.Depends[I].size());
			}
		}
		Out.write(',');
		Out.write(Entry.Name.data(), Entry.Name.size());
		Out.write('\n');
	}
	Out.close();
}

void VersionsT::add(std::string const & Tag, ArchiveEntryT const & Entry)
{
	auto Pair = mEntries.insert({Tag, Entry});
	// Overwrite the old entry if it already existed
	if (!Pair.second) Pair.first->second = Entry;
}

ArchiveEntryT const & VersionsT::findTag(std::string const & Tag)
{
	auto Iter = mEntries.find(Tag);
	if (Iter == mEntries.end()) throw std::runtime_error{"Error on unknown tag lookup"};
	return Iter->second;
}

}
