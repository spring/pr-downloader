#include <stdio.h>
#include "Logger.h"
#include <string.h>
#include <cstdarg>

void INFO(const std::string& message, ...)
{
	va_list args;
	va_start(args, message);
	printf("[Info] ");
	vprintf(message.c_str(), args);
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

void ERROR(const std::string& message, ...)
{
	va_list args;
	va_start(args,message);
	printf("[Error] ");
	vprintf(message.c_str(),args);
	va_end(args);
}
