#!/bin/sh


export PREFIX=$(pwd)/mingwlibs
export MINGWHOST=i686-w64-mingw32
export MINGW32CXX=${MINGWHOST}-g++
export MINGW32CC=${MINGWHOST}-gcc
export MINGW32CPP=${MINGWHOST}-cpp
export MINGW32RC=${MINGWHOST}-windres
export MINGW32AR=${MINGWHOST}-ar
export MINGW32RANLIB=${MINGWHOST}-ranlib
export MINGW32RC=${MINGWHOST}-windres
export DOWNLOAD="wget -c"
export PARALLEL="8"

echo "Using ${PREFIX} for mingwlibs"
if [ ! -s "$(pwd)/src/Downloader/IDownloader.h" ]; then
	echo "Please run this script from the root directory with tools/crosscompile.sh."
	exit
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

mkdir -p ${PREFIX}/include
mkdir -p ${PREFIX}/lib

if [ ! -s ${PREFIX}/lib/libz.a ]; then
	${DOWNLOAD} "http://prdownloads.sourceforge.net/libpng/zlib-1.2.5.tar.gz?download" -O zlib-1.2.5.tar.gz
	tar xifz zlib-1.2.5.tar.gz
	cd zlib-1.2.5
	export CC=${MINGW32CC}
	export CPP=${MINGW32CPP}
	export LDSHARED=${MINGW32CC}
	export AR="${MINGW32AR}"
	export RANLIB=${MINGW32RANLIB}
	./configure --prefix=${PREFIX}
	make install -j ${PARALLEL}
	cp zlib.h ../mingwlibs/include
	cp zconf.h ../mingwlibs/include
	cd ..
fi

if [ ! -s ${PREFIX}/lib/libcurl.a ]; then
	export CC=${MINGW32CC}
	export CXX=${MINGW32CXX}
	wget -c http://curl.haxx.se/download/curl-7.21.1.tar.bz2
	tar xifj curl-7.21.1.tar.bz2
	cd curl-7.21.1
	./configure --host=${MINGWHOST} --prefix=${PREFIX} \
		--disable-ftp --disable-dict --disable-imap --disable-pop3 \
		--disable-smtp --disable-telnet --disable-tftp --disable-ldap \
		--disable-rtsp --disable-manual --disable-shared
	make install -j ${PARALLEL}
	cd ..
fi

rm -f CMakeCache.txt

(
echo "SET(CMAKE_SYSTEM_NAME Windows)"
echo "SET(CMAKE_C_COMPILER $MINGW32CC)"
echo "SET(CMAKE_CXX_COMPILER $MINGW32CXX)"
echo "SET(CMAKE_FIND_ROOT_PATH /usr/$MINGWHOST)"
echo "SET(MINGWLIBS ./mingwlibs)"
echo "SET(CMAKE_INSTALL_PREFIX  ./dist)"
echo "SET(CMAKE_RC_COMPILER $MINGW32RC)"
) >win32.cmake

cmake . "-DCMAKE_TOOLCHAIN_FILE=./win32.cmake"
make install -j ${PARALELL}

