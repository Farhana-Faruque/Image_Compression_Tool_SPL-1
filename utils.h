#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "image.h"

// PGM file reading utilities
int readPGMHeader(FILE* file, PGMHeader* pgm);
unsigned char* readPGMPixels(FILE* file, const PGMHeader* pgm, long* totalPixels);
void printCompressionStats(const char* outputFile, long originalSize);

#endif // UTILS_H
