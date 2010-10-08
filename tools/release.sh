#!/bin/sh

cd ..
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

${ZIP} rapid.7z $FILES
ls -lh rapid.7z
${ZIP} rapid_dbg.7z $FILESDBG
ls -lh rapid_dbg.7z

cd ../src
cppcheck --enable=all --quiet --quiet .

