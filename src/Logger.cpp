/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Logger.h"
#include "Util.h"

#include <stdio.h>
#include <stdarg.h>

// Logging functions in standalone mode
#define WEAK __attribute__((weak))
// prdLogRaw is supposed to flush after printing (mostly to stdout/err
// for progress bars and such).
void WEAK prdLogRaw(const char* /*fileName*/, int /*line*/, const char* /*funcName*/,
                    const char* format, va_list args)
{
	vprintf(format, args);
	fflush(stdout);
}
// Normal logging
void WEAK prdLogError(const char* fileName, int line, const char* funcName,
                      const char* format, va_list args)
{
	fprintf(stderr, "[Error] %s:%d:%s():", fileName, line, funcName);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
}
void WEAK prdLogInfo(const char* fileName, int line, const char* funcName,
                     const char* format, va_list args)
{
	printf("[Info] %s:%d:%s():", fileName, line, funcName);
	vprintf(format, args);
	printf("\n");
}
void WEAK prdLogDebug(const char* fileName, int line, const char* funcName,
                      const char* format, va_list args)
{
	printf("[Debug] %s:%d:%s():", fileName, line, funcName);
	vprintf(format, args);
	printf("\n");
}


static bool logEnabled = true;

void LOG_DISABLE(bool disableLogging)
{
	logEnabled = !disableLogging;
}

void L_LOG(const char* fileName, int line, const char* funName,
           L_LEVEL level, const char* format...)
{
	if (!logEnabled) {
		return;
	}

	va_list args;
	va_start(args, format);
	switch (level) {
		case L_RAW:
			prdLogRaw(fileName, line, funName, format, args);
			break;
		default:
		case L_ERROR:
			prdLogError(fileName, line, funName, format, args);
			break;
		case L_INFO:
			prdLogInfo(fileName, line, funName, format, args);
			break;
		case L_DEBUG:
			prdLogDebug(fileName, line, funName, format, args);
			break;
	}
	va_end(args);
}

void LOG_PROGRESS(long done, long total, bool forceOutput)
{
	static unsigned long lastlogtime = 0;
	static float lastPercentage = 0.0f;

	if (!logEnabled) {
		return;
	}

	unsigned long now = getTime(); // needs to be here atm to avoid static link
				       // failure because of circular deps between
				       // libs

	if (lastlogtime < now) {
		lastlogtime = now;
	} else {
		if (!forceOutput)
			return;
	}
	if (total < 0) // if total bytes are unknown set to 50%
		total = done * 2;
	float percentage = 0;
	if (total > 0) {
		percentage = (float)done / total;
	}

	if (percentage == lastPercentage)
		return;
	lastPercentage = percentage;

	LOG("[Progress] %3.0f%% [", percentage * 100.0f);
	int totaldotz = 30; // how wide you want the progress meter to be
	int dotz = percentage * totaldotz;
	for (int i = 0; i < totaldotz; i++) {
		if (i >= dotz)
			LOG(" "); // blank
		else
			LOG("="); // full
	}
	// and back to line begin - do not forget the fflush to avoid output buffering
	// problems!
	LOG("] %ld/%ld ", done, total);
	LOG("\r");
}
