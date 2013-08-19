/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */


#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <time.h>

#undef PR_DOWNLOADER_VERSION
#ifndef PR_DOWNLOADER_VERSION
#define QUOTEDVERSION "dev-" __DATE__
#else

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

#define QUOTEDVERSION QUOTEME(PR_DOWNLOADER_VERSION)
#endif


const char* getVersion();
const char* getAgent();

