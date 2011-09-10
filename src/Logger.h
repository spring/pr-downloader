#ifndef LOGGER_H
#define LOGGER_H

#include "FileSystem.h"

void INFO(std::string message, ...);

void DOWNLOAD(std::string filename);
void PROGRESS(float done, float total);
void ERROR(std::string message, ...);



#endif