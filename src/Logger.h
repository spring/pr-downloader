#ifndef LOGGER_H
#define LOGGER_H

#include "FileSystem.h"

void INFO(const std::string& message, ...);
void DOWNLOAD(const std::string& filename);
void ERROR(const std::string& message, ...);

void PROGRESS(float done, float total);

#ifdef DEBUG
	#define DEBUG_LINE(fmt, ...) \
		printf( "%s:%d:%s(): " fmt "\n", __FILE__, \
									__LINE__, __FUNCTION__, __VA_ARGS__);
#else
	#define	DEBUG_LINE(fmt, ...)
#endif

#endif
