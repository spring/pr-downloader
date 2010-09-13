#!/bin/sh

cd ..

cd dist
FILES=`ls *.dll *.exe`
for i in $FILES; do
	echo Strip $i
	i586-mingw32msvc-strip $i
done

7z a -tzip -mx=9 rapid.zip $FILES
