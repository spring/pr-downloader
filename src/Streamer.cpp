#include "rapid/BitArray.h"
#include "rapid/Hex.h"
#include "rapid/Marshal.h"
#include "rapid/PoolArchive.h"
#include "rapid/Store.h"

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdio>

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

namespace {

using namespace Rapid;

struct StreamEntryT
{
	FileEntryT File;
	std::size_t Size;
};

void stream(std::string const & StorePath, std::string const & Hexed)
{
	// Read bit array
	auto File = gzdopen(fileno(stdin), "rb");
	BitArrayT Bits;
	char Buffer[4096];

	while (true)
	{
		auto Bytes = gzread(File, Buffer, 4096);
		if (Bytes < 0) throw std::runtime_error{"Error reading bit array"};
		if (Bytes == 0) {
			if (gzeof(File)) break;
			LOG_ERROR("Error calling gzread from POST data: %s", gzerror(File, nullptr));
		}
		Bits.append(Buffer, Bytes);
	}

	gzclose(File);

	// Load archive
	if (Hexed.size() != 32) throw std::runtime_error{"Hex must be 32 bytes"};
	DigestT Digest;
	Hex::decode(Hexed.data(), Digest.Buffer, 16);
	StoreT Store{StorePath};
	PoolArchiveT Archive{Store};
	Archive.load(Digest);

	// Accumulate marked files
	std::vector<StreamEntryT> Entries;
	std::size_t TotalSize = 0;

	Archive.iterate(Bits, [&](FileEntryT const & Entry)
	{
		auto Path = Store.getPoolPath(Entry.Digest);
		struct stat Stats;
		auto Error = stat(Path.c_str(), &Stats);
		if (Error == -1) throw std::runtime_error{"Error reading pool file"};

		Entries.push_back({Entry, static_cast<std::uint32_t>(Stats.st_size)});
		TotalSize += Stats.st_size;
		TotalSize += 4;
	});

	// Respond to request
	std::cout << "Content-Transfer-Encoding: binary\r\n";
	std::cout << "Content-Length: " << TotalSize << "\r\n";
	std::cout << "Content-Type: application/octet-stream\r\n";
	std::cout << "\r\n";
	std::cout.flush();

	for (auto & Entry: Entries)
	{
		auto Path = Store.getPoolPath(Entry.File.Digest);
		const int In = open(Path.c_str(), O_RDONLY);
		if (In < 0) throw std::runtime_error{"Error opening pool file"};
		std::uint8_t Bytes[4];
		Marshal::packLittle(Entry.Size, Bytes);
		std::cout.write(reinterpret_cast<char *>(Bytes), 4);
		std::cout.flush();
		for (std::size_t left = Entry.Size; left > 0;) {
			ssize_t written = sendfile(STDOUT_FILENO, In, 0, left);
			if (written < 0) {
				throw std::runtime_error{"Sendfile failed"};
			}
			left -= written;
		}
		close(In);
	}
}

}

int main(int argc, char const * const * argv, char const * const * env)
{
	umask(0002);

	const char* QueryString = getenv("QUERY_STRING");

	if (QueryString == nullptr)
	{
		std::cerr << "QUERY_STRING isn't set\n";
		return 1;
	}

	try
	{
		stream(".", QueryString);
	}
	catch (std::exception const & Exception)
	{
		std::cerr << Exception.what() << "\n";
		return 1;
	}
}
