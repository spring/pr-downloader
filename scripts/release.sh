#!/bin/sh

set -e
if [ ! -d src ]; then
	echo please run from the root source dir
	exit 1
fi
make -j8 install
cd dist
FILES=`ls *.dll *.exe`
for i in $FILES; do
	echo Strip $i
	i586-mingw32msvc-objcopy --only-keep-debug $i $i.dbg
	i586-mingw32msvc-strip --strip-unneeded $i
done
FILESDBG=`ls *.dbg`
ZIP="7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
VERSION=$(git describe)

${ZIP} pr-downloader-${VERSION}.7z $FILES
ls -lh pr-downloader-${VERSION}.7z
${ZIP} pr-downloader-${VERSION}_dbg.7z $FILESDBG
ls -lh pr-downloader-${VERSION}_dbg.7z


