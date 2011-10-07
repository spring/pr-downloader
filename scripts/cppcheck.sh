#!/bin/sh
set +e

find src -name *\.cpp -or -name *\.c \
|egrep -v '(^src/Downloader/Http/xmlrpc|^src/Downloader/Plasma/soap/|^src/Downloader/Widget/pugixml/|^src/FileSystem/bencode/)' \
|cppcheck --force --enable=style,information,unusedFunction --quiet -I src --file-list=/dev/stdin --suppressions-list=scripts/cppcheck.supress $@
