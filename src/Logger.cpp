#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>

void LOG(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void LOG_INFO(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[Info] ");
	vprintf(format, args);
	va_end(args);
}

void LOG_ERROR(const char* format, ...)
{
	va_list args;
	va_start(args,format);
	printf("[Error] ");
	vprintf(format,args);
	va_end(args);
}

void LOG_DOWNLOAD(const char* filename)
{
	printf("[Download] %s\n",filename);
}

void LOG_PROGRESS(unsigned int done,unsigned  int total)
{
	float percentage = 0;
	if (total>0) {
		percentage = (float)done / total;
	}
	printf("[Progress] %3.0f%% [",percentage * 100.0f);
	int totaldotz = 30;                           // how wide you want the progress meter to be
	int dotz = percentage * totaldotz;
	for (int i=0; i < totaldotz; i++) {
		if (i>dotz)
			printf(" ");    //blank
		else
			printf("=");    //full
	}
	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	printf("] %d/%d ",(int)done,(int)total );
	printf("\r");
	fflush(stdout);
}

