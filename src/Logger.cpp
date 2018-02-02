/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "Logger.h"
#include "Util.h"

#include <stdio.h>
#include <stdarg.h>

#define WEAK __attribute__((weak))
// Logging functions in standalone mode

// prdlograw is supposed to flush after printing (mostly to stdout/err
// for progress bars and such).
void WEAK prdlograw(const char* format, va_list args)
{
	vprintf(format, args);
	fflush(stdout);
}
// Normal logging
void WEAK prdlogerror(const char* format, va_list args)
{
	fprintf(stderr, "[Error] ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
}
void WEAK prdloginfo(const char* format, va_list args)
{
	printf("[Info] ");
	vprintf(format, args);
	printf("\n");
}
void WEAK prdlogdebug(const char* format, va_list args)
{
	printf("[Debug] ");
	vprintf(format, args);
	printf("\n");
}


static bool logEnabled = true;

void LOG_DISABLE(bool disableLogging)
{
	logEnabled = !disableLogging;
}

void L_LOG(L_LEVEL level, const char* format...)
{
	if (!logEnabled) {
		return;
	}

	va_list args;
	va_start(args, format);
	switch (level) {
		case L_RAW:
			prdlograw (format, args);
			break;
		default:
		case L_ERROR:
			prdlogerror (format, args);
			break;
		case L_INFO:
			prdloginfo (format, args);
			break;
		case L_DEBUG:
			prdlogdebug (format, args);
			break;
	}
	va_end(args);
}

void LOG_DOWNLOAD(const char* filename)
{
	L_LOG(L_RAW, "[Download] %s\n", filename);
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

	L_LOG(L_RAW, "[Progress] %3.0f%% [", percentage * 100.0f);
	int totaldotz = 30; // how wide you want the progress meter to be
	int dotz = percentage * totaldotz;
	for (int i = 0; i < totaldotz; i++) {
		if (i >= dotz)
			L_LOG(L_RAW, " "); // blank
		else
			L_LOG(L_RAW, "="); // full
	}
	// and back to line begin - do not forget the fflush to avoid output buffering
	// problems!
	L_LOG(L_RAW, "] %ld/%ld ", done, total);
	L_LOG(L_RAW, "\r");
}
