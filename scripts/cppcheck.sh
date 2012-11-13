#!/bin/sh
set +e

find src -name "*.cpp" -or -name "*.c" \
|egrep -v '(^src/lib/)' \
|cppcheck --force --enable=all --quiet -I src -I src/lib --file-list=/dev/stdin --suppressions-list=scripts/cppcheck.supress $@
