#include "Logger.h"
#include <stdio.h>
#include <string.h>
#include <cstdarg>


void LOG(const std::string& format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format.c_str(), args);
	va_end(args);
}

void INFO(const std::string& format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[Info] ");
	vprintf(format.c_str(), args);
	va_end(args);
}

void DOWNLOAD(const std::string& filename)
{
	printf("[Download] %s\n",filename.c_str());
}

void PROGRESS(float done, float total)
{
	float percentage = done / total;
	printf("[Progress] %3.0f%% [",percentage * 100.0f);
	int totaldotz = 30;                           // how wide you want the progress meter to be
	int dotz = percentage * totaldotz;
	int ii=0;
	for ( ; ii < dotz; ii++) {
		printf("=");    //full
	}
	for ( ; ii < totaldotz; ii++) {
		printf(" ");    //blank
	}

	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	printf("] %d/%d ",(int)done,(int)total );
	printf("\r");
	fflush(stdout);
}

void ERROR(const std::string& format, ...)
{
	va_list args;
	va_start(args,format);
	printf("[Error] ");
	vprintf(format.c_str(),args);
	va_end(args);
}
