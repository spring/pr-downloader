#pr-downloader - tool to download maps and games for the Spring engine

##compile
to compile, you need libtorrent-rasterbar,xmlrpc-c and boost
	cmake .
	make
	make install

That's it!

##usage

pr-downloader --download-game "Balanced Annihilation v7.20" --download-map "DeltaSiegeDry.sd7"

