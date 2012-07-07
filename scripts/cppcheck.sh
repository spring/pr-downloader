#!/bin/sh
set +e

find src -name "*.cpp" -or -name "*.c" \
|egrep -v '(^src/lib/)' \
|cppcheck --force --enable=style,information,unusedFunction --quiet -I src --file-list=/dev/stdin --suppressions-list=scripts/cppcheck.supress $@
