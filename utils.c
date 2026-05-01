#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define MAX_LINE 1024

// Reads PGM header (magic number, width, height, maxval)
int readPGMHeader(FILE* file, PGMHeader* pgm) {
    char line[MAX_LINE];
    
    if (!readLine(file, line, MAX_LINE) || sscanf(line, "%2s", pgm->sign) != 1) {
        printf("Failed to read magic number\n");
        return 0;
    }
    
    if (!readLine(file, line, MAX_LINE) || sscanf(line, "%d %d", &pgm->width, &pgm->height) != 2) {
        printf("Failed to read dimensions\n");
        return 0;
    }
    
    if (!readLine(file, line, MAX_LINE) || sscanf(line, "%d", &pgm->maxIntensity) != 1) {
        printf("Failed to read maxval\n");
        return 0;
    }
    
    // Validate PGM format
    if ((strcmp(pgm->sign, "P2") != 0 && strcmp(pgm->sign, "P5") != 0) || pgm->maxIntensity > 255) {
        printf("Unsupported PGM format: %s, maxval: %d\n", pgm->sign, pgm->maxIntensity);
        return 0;
    }
    
    return 1;
}

// Reads PGM pixel data (handles both P2 ASCII and P5 binary)
unsigned char* readPGMPixels(FILE* file, const PGMHeader* pgm, long* totalPixels) {
    *totalPixels = (long)pgm->width * pgm->height;
    unsigned char* imageData = (unsigned char*)malloc(*totalPixels);
    
    if (!imageData) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    if (strcmp(pgm->sign, "P2") == 0) {
        // ASCII format
        for (long i = 0; i < *totalPixels; i++) {
            int pixel;
            if (fscanf(file, "%d", &pixel) != 1) {
                printf("Error reading P2 data at pixel %ld\n", i);
                free(imageData);
                return NULL;
            }
            imageData[i] = (unsigned char)pixel;
        }
    } else {
        // Binary format (P5)
        if (fread(imageData, 1, *totalPixels, file) != *totalPixels) {
            printf("Error reading P5 data\n");
            free(imageData);
            return NULL;
        }
    }
    
    return imageData;
}

// Prints compression statistics
void printCompressionStats(const char* outputFile, long originalSize) {
    FILE* compFile = fopen(outputFile, "rb");
    if (!compFile) {
        printf("Warning: Could not open output file to calculate size\n");
        return;
    }
    
    fseek(compFile, 0, SEEK_END);
    long compressedSize = ftell(compFile);
    fclose(compFile);
    
    printf("Original size: %ld bytes\n", originalSize);
    printf("Compressed size: %ld bytes\n", compressedSize);
    printf("Compression ratio: %.2f%%\n", 
           (1.0 - ((float)compressedSize / originalSize)) * 100);
}
