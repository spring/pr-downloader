#pragma once

#include "Md5.h"

#include <random>

namespace Rapid {

class StoreT
{
	private:
	std::string mRoot;
	std::default_random_engine mEngine;
	std::uniform_int_distribution<unsigned char> mDistribution;

	public:
	StoreT(std::string const & Root);

	void init();
	std::string getTempPath();
	std::string getSdpPath(DigestT const & Digest) const;
	std::string getPoolPath(DigestT const & Digest) const;
	std::string getVersionsPath() const;
	std::string getLastPath(std::string const & Prefix) const;
	std::string getLastGitPath(std::string const & Prefix) const;
	std::string getBuildPath(std::string const & Prefix, std::string Version) const;
};

}
