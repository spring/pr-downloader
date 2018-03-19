#include "Store.h"

#include "Hex.h"
#include "String.h"

#include <array>
#include <stdexcept>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

namespace Rapid {

StoreT::StoreT(std::string const & Root)
:
	mRoot{Root}
{}

namespace {

void touchDirectory(std::string const & Path)
{
	struct stat Stats;
	auto Error = stat(Path.c_str(), &Stats);

	if (Error == -1)
	{
		if (errno == ENOENT)
		{
			Error = mkdir(Path.c_str(), 0755);
			if (Error != 0) throw std::runtime_error{"Error creating directory: " + Path};
			return;
		}
		else throw std::runtime_error{"Error creating directory: " + Path};
	}

	if (!S_ISDIR(Stats.st_mode)) throw std::runtime_error{"Error creating directory: is a file:" + Path};
}

}

// This only needs to be called if the library user want to do writing to the pool
void StoreT::init()
{
	std::random_device Device;
	mEngine.seed(Device());

	std::string Scratch = mRoot;
	std::size_t RootSize = Scratch.size();

	touchDirectory(Scratch);
	touchDirectory(concatAt(Scratch, RootSize, "/temp"));
	touchDirectory(concatAt(Scratch, RootSize, "/pool"));
	touchDirectory(concatAt(Scratch, RootSize, "/packages"));
	touchDirectory(concatAt(Scratch, RootSize, "/last"));
	touchDirectory(concatAt(Scratch, RootSize, "/last-git"));
	touchDirectory(concatAt(Scratch, RootSize, "/builds"));

	concatAt(Scratch, RootSize, "/pool/  ");
	std::size_t PoolSize = Scratch.size();
	for (std::size_t I = 0; I < 16; ++I)
	{
		Scratch[PoolSize - 2] = Hex::EncodeTable[I];
		for (std::size_t J = 0; J < 16; ++J)
		{
			Scratch[PoolSize - 1] = Hex::EncodeTable[J];
			touchDirectory(Scratch);
		}
	}
}

std::string StoreT::getTempPath()
{
	unsigned char Random[8];
	for (std::size_t Index = 0; Index < 8; ++Index) Random[Index] = mDistribution(mEngine);
	std::array<char, 16> Hexed;
	Hex::encode(Hexed.data(), Random, 8);
	return concat(mRoot, "/temp/", Hexed);
}

std::string StoreT::getSdpPath(DigestT const & Digest) const
{
	std::array<char, 32> Hexed;
	Hex::encode(Hexed.data(), Digest.Buffer, 16);
	return concat(mRoot, "/packages/", Hexed, ".sdp");
}

std::string StoreT::getPoolPath(DigestT const & Digest) const
{
	std::array<char, 2> Prefix;
	std::array<char, 30> Hexed;
	Hex::encode(Prefix.data(), Digest.Buffer, 1);
	Hex::encode(Hexed.data(), Digest.Buffer + 1, 15);
	return concat(mRoot, "/pool/", Prefix, '/', Hexed, ".gz");
}

std::string StoreT::getVersionsPath() const
{
	return concat(mRoot, "/versions.gz");
}

std::string StoreT::getLastPath(std::string const & Prefix) const
{
	return concat(mRoot, "/last/", Prefix, ".gz");
}

std::string StoreT::getLastGitPath(std::string const & Prefix) const
{
	return concat(mRoot, "/last-git/", Prefix, ".gz");
}

std::string StoreT::getBuildPath(std::string const & Prefix, std::string Version) const
{
	// Replace spaces with underscores
	for (auto & Char : Version)
	{
		if (Char == ' ') Char = '_';
	}

	return concat(mRoot, "/builds/", Prefix, '-', Version, ".sdz");
}

}
