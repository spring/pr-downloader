

export PREFIX=/home/abma/Projects/downloader/mingwlibs
export MINGW32CXX=i586-mingw32msvc-g++
export MINGW32CC=i586-mingw32msvc-gcc
export MINGW32RC=i586-mingw32msvc-windres
export MINGW32AR=i586-mingw32msvc-ar
export MINGW32RANLIB=i586-mingw32msvc-ranlib
export MINGWHOST=i586-mingw32msvc
export DOWNLOAD="wget -c"
export PARALLEL="8"

cd ..

if [ ! -s ${PREFIX}/lib/libz.a ]; then
	${DOWNLOAD} "http://prdownloads.sourceforge.net/libpng/zlib-1.2.5.tar.gz?download" -O zlib-1.2.5.tar.gz
	tar xifz zlib-1.2.5.tar.gz
	cd zlib-1.2.5
	export CC=${MINGW32CC}
	export CPP=${MINGW32CXX}
	export LDSHARED=${MINGW32CC}
	export AR="${MINGW32AR}"
	export RANLIB=${MINGW32RANLIB}
	./configure --prefix=${PREFIX}
	make install -j ${PARALLEL}
	cd ..
fi

if [ ! -s ${PREFIX}/lib/libcurl.dll.a ]; then
	export CC=${MINGW32CC}
	export CXX=${MINGW32CXX}
	wget -c http://curl.haxx.se/download/curl-7.21.1.tar.bz2
	tar xifj curl-7.21.1.tar.bz2
	cd curl-7.21.1
	./configure --host=${MINGWHOST} --prefix=${PREFIX} \
		--disable-ftp --disable-dict --disable-imap --disable-pop3 \
		--disable-smtp --disable-telnet --disable-tftp --disable-ldap \
		--disable-rtsp --disable-manual --enable-static=no
	make install -j ${PARALLEL}
	cd ..
fi

if [ ! -s ${PREFIX}/lib/libboost_system-mt.dll ]; then
	wget -c -O boost_1_44_0.tar.bz2 "http://prdownloads.sourceforge.net/boost/boost/1.44.0/boost_1_44_0.tar.bz2?download"
	tar xifj boost_1_44_0.tar.bz2
	cd boost_1_44_0/
	./bootstrap.sh --without-icu
	#replace uppercase filename
	sed -i 's/WinError/winerror/g' ./boost/asio/error.hpp 
	echo "using gcc : : ${MINGW32CXX} -D_WIN32_WINNT=0x0501 : 
		<rc>${MINGW32RC} -D_WIN32_WINNT=0x0501
	        <archiver>${MINGW32AR} -D_WIN32_WINNT=0x0501
	;" > user-config.jam
	./bjam -d0 toolset=gcc target-os=windows variant=release threading=multi threadapi=win32\
		link=shared runtime-link=shared --prefix=$PREFIX --user-config=user-config.jam -j ${PARALLEL}\
		--without-mpi --without-python --without-test --without-serialization --without-filesystem -sNO_BZIP2=1 -sNO_ZLIB=1 --layout=tagged install
	cd ..
fi

if [ ! -s ${PREFIX}/lib/libtorrent-rasterbar.a ]; then
	${DOWNLOAD} http://libtorrent.googlecode.com/files/libtorrent-rasterbar-0.15.2.tar.gz
	tar xifz libtorrent-rasterbar-0.15.2.tar.gz
	#replace uppercase filename
	sed -i 's/Windows/windows/g' ./libtorrent-rasterbar-0.15.2/src/allocator.cpp

	cd libtorrent-rasterbar-0.15.2
	export CC="${MINGW32CC} -D_WIN32_WINNT=0x0501"
	export GCC="${MINGW32CC} -D_WIN32_WINNT=0x0501"
	export CXX="${MINGW32CXX} -D_WIN32_WINNT=0x0501"
	export CFLAGS="-I${PREFIX}/include -I${PREFIX}/include/boost"
	export LDFLAGS="-L${PREFIX}/lib"
	./configure --host=${MINGWHOST} --prefix=${PREFIX} --disable-encryption --disable-geoip --with-boost=${PREFIX}
	make install -j ${PARALLEL}
	cd ..
fi


if [ ! -s win32.cmake ]; then

	cat > win32.cmake <<EOF
SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
SET(CMAKE_FIND_ROOT_PATH /usr/i586-mingw32msvc)
SET(MINGWLIBS ./mingwlibs)
SET(CMAKE_INSTALL_PREFIX  ./dist)
EOF

fi

rm -f CMakeCache.txt
cmake . "-DCMAKE_TOOLCHAIN_FILE=./win32.cmake"
