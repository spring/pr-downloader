#pr-downloader - tool to download maps and games for the Spring engine

##compile
to compile, you need libcurl and libz
	cmake .
	make
	make install

for cross-compilation run scripts/crosscompile.sh (tested on Ubuntu 11.04 / 64 bit with i586-mingw32msvc-g++ (GCC) 4.4.4)

That's it!

##usage

pr-downloader --download-game "Balanced Annihilation v7.20" --download-map "DeltaSiegeDry.sd7"

##coding style

please use the git-pre-commit hook:

	ln -sfv ../../git-hooks/pre-commit .git/hooks/pre-commit

##License

pr-downloader is GPL-2 as it contains xmlrpcpp (GPL-2) and gsoap (GPL-2) code.

