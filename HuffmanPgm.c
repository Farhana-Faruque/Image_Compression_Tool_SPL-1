#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffpgm.h"
#include "utils.h"

#define MAX_SIZE 256

int compressHuffman(const char* inputFile, const char* outputFile) {
    FILE* input = fopen(inputFile, "rb");
    FILE* output = fopen(outputFile, "wb");
    if (!input || !output) {
        printf("Cannot open the file");
        return 1;
    }

    PGMHeader pgm;
    if (!readPGMHeader(input, &pgm)) {
        fclose(input);
        fclose(output);
        return 1;
    }

    long tP;
    unsigned char* iD = readPGMPixels(input, &pgm, &tP);
    if (!iD) {
        fclose(input);
        fclose(output);
        return 1;
    }

    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    fclose(input);

    unsigned int freq[MAX_SIZE] = {0};
    for (long i = 0; i < tP; i++) {
        freq[iD[i]]++;
    }

    Node* root = buildHuffmanTree(freq);
    if (!root) {
        printf("Failed to build Huffman tree during compression\n");
        free(iD);
        fclose(output);
        return 1;
    }

    char codes[MAX_SIZE][MAX_SIZE] = {0};
    int lengths[MAX_SIZE] = {0};
    char code[MAX_SIZE];
    generateCodes(root, code, 0, codes, lengths);

    if (fwrite(&pgm.width, sizeof(int), 1, output) != 1 ||
        fwrite(&pgm.height, sizeof(int), 1, output) != 1 ||
        fwrite(pgm.sign, sizeof(char), 2, output) != 2) {
        printf("Failed to write header\n");
        free(iD);
        freeHuffmanTree(root);
        fclose(output);
        return 1;
    }

    for (int i = 0; i < MAX_SIZE; i++) {
        if (freq[i] > 0) {
            if (fwrite(&i, sizeof(unsigned char), 1, output) != 1 ||
                fwrite(&freq[i], sizeof(unsigned int), 1, output) != 1) {
                printf("Failed to write frequency table\n");
                free(iD);
                freeHuffmanTree(root);
                fclose(output);
                return 1;
            }
        }
    }
    unsigned char zero = 0;
    if (fwrite(&zero, 1, 1, output) != 1) {
        printf("Failed to write frequency table end marker\n");
        free(iD);
        freeHuffmanTree(root);
        fclose(output);
        return 1;
    }

    unsigned int bitBuffer = 0;
    int bits = 0;
    for (long i = 0; i < tP; i++) {
        unsigned char pixel = iD[i];
        for (int j = 0; j < lengths[pixel]; j++) {
            bitBuffer = (bitBuffer << 1) | (codes[pixel][j] - '0');
            bits++;
            if (bits == 8) {
                unsigned char byte = (unsigned char)bitBuffer;
                if (fwrite(&byte, 1, 1, output) != 1) {
                    printf("Failed to write compressed data\n");
                    free(iD);
                    freeHuffmanTree(root);
                    fclose(output);
                    return 1;
                }
                bitBuffer = 0;
                bits = 0;
            }
        }
    }
    if (bits > 0) {
        bitBuffer <<= (8 - bits);
        unsigned char byte = (unsigned char)bitBuffer;
        if (fwrite(&byte, 1, 1, output) != 1) {
            printf("Failed to write final compressed byte\n");
            free(iD);
            freeHuffmanTree(root);
            fclose(output);
            return 1;
        }
    }

    fclose(output);
    free(iD);
    freeHuffmanTree(root);

    printCompressionStats(outputFile, size);

    return 0;
}

int decompressHuffman(const char* inputFile, const char* outputFile) {
    FILE* input = fopen(inputFile, "rb");
    FILE* output = fopen(outputFile, "wb");
    if (!input || !output) {
        printf("Cannot open the file");
        return 1;
    }

    PGMHeader pgm;
    if (fread(&pgm.width, sizeof(int), 1, input) != 1 ||
        fread(&pgm.height, sizeof(int), 1, input) != 1 ||
        fread(pgm.sign, sizeof(char), 2, input) != 2) {
        printf("Failed to read header\n");
        fclose(input);
        fclose(output);
        return 1;
    }
    pgm.sign[2] = '\0';
    pgm.maxIntensity = 255;

    long tP = (long)pgm.width * pgm.height; // totalPixels
    unsigned char* dD = (unsigned char*)malloc(tP); // decompressedData
    if (!dD) {
        printf("Memory allocation failed\n");
        fclose(input);
        fclose(output);
        return 1;
    }

    unsigned int freq[MAX_SIZE] = {0};
    unsigned char value;
    int freqCount = 0;
    while (fread(&value, 1, 1, input) == 1) {
        if (value == 0) break; 
        unsigned int f;
        if (fread(&f, sizeof(unsigned int), 1, input) != 1) {
            printf("Error reading frequency value for byte %d\n", value);
            free(dD);
            fclose(input);
            fclose(output);
            return 1;
        }
        freq[value] = f;
        freqCount++;
    }
    if (freqCount == 0) {
        printf("No frequency data found in compressed file\n");
        free(dD);
        fclose(input);
        fclose(output);
        return 1;
    }

    Node* root = buildHuffmanTree(freq);
    if (!root) {
        printf("Failed to rebuild Huffman tree\n");
        free(dD);
        fclose(input);
        fclose(output);
        return 1;
    }

    Node* current = root;
    unsigned char byte;
    long pW = 0; // pixelsWritten
    int bits = 0;
    while (pW < tP) {
        if (bits == 0) {
            if (fread(&byte, 1, 1, input) != 1) {
                printf("Unexpected end of file at pixel %ld\n", pW);
                break;
            }
            bits = 8;
        }
        int bit = (byte >> (bits - 1)) & 1;
        bits--;

        if (bit) {
            if (!current->right) {
                printf("Invalid Huffman code at pixel %ld\n", pW);
                free(dD);
                freeHuffmanTree(root);
                fclose(input);
                fclose(output);
                return 1;
            }
            current = current->right;
        } else {
            if (!current->left) {
                printf("Invalid Huffman code at pixel %ld\n", pW);
                free(dD);
                freeHuffmanTree(root);
                fclose(input);
                fclose(output);
                return 1;
            }
            current = current->left;
        }

        if (!current->left && !current->right) {
            dD[pW++] = current->data;
            current = root;
        }
    }
    fclose(input);

    if (pW != tP) {
        printf("Error: Decompressed pixel count (%ld) doesn't match expected (%ld)\n",
               pW, tP);
        free(dD);
        freeHuffmanTree(root);
        fclose(output);
        return 1;
    }

    fprintf(output, "%s\n%d %d\n%d\n", pgm.sign, pgm.width, pgm.height, pgm.maxIntensity);
    if (strcmp(pgm.sign, "P2") == 0) {
        for (long i = 0; i < tP; i++) {
            if (fprintf(output, "%d", dD[i]) < 0) {
                printf("Error writing P2 data at pixel %ld\n", i);
                fclose(output);
                free(dD);
                freeHuffmanTree(root);
                return 1;
            }
            if ((i + 1) % pgm.width == 0) fprintf(output, "\n");
            else fprintf(output, " ");
        }
    } else {
        if (fwrite(dD, 1, tP, output) != tP) {
            printf("Error writing P5 data\n");
            fclose(output);
            free(dD);
            freeHuffmanTree(root);
            return 1;
        }
    }

    fclose(output);
    free(dD);
    freeHuffmanTree(root);
    return 0;
}

int huffman() {
    char inputFile[256];
    char compressedFile[256] = "compressed.bin";
    char decompressedFile[256];
    int yn;

    printf("What do you want to do??\n1.Compress an image.\n2.Decompress an image.\n");
    printf("Enter your choice in number: ");  
    scanf("%d", &yn);
    printf("\n");

    if(yn == 1){
        printf("Enter the input PGM file name:");
        scanf("%255s", inputFile);
        printf("\n");

        printf("Attempting to compress %s...\n", inputFile);
        printf("\n");

        if (compressHuffman(inputFile, compressedFile) == 0) {
            printf("Compression successful: %s -> %s\n", inputFile, compressedFile);

        } else {
            printf("Compression failed\n");
        }
    }
    else if(yn == 2){ 
        printf("Enter decompressed PGM file name: ");
        scanf("%255s", decompressedFile);
        printf("\n");

        printf("Attempting to decompress %s...\n", compressedFile);
        printf("\n");
        
        if (decompressHuffman(compressedFile, decompressedFile) == 0) {
            printf("Decompression successful: %s -> %s\n", compressedFile, decompressedFile);

            FILE* comp = fopen(compressedFile, "rb");
            fseek(comp, 0, SEEK_END);
            long compSize = ftell(comp);
            fclose(comp);
        
            FILE* decomp = fopen(decompressedFile, "rb");
            fseek(decomp, 0, SEEK_END);
            long decompSize = ftell(decomp);
            fclose(decomp);

            printf("Compressed size: %ld bytes\n", compSize);
            printf("Decompressed size: %ld bytes\n", decompSize);
        } else {
            printf("Decompression failed\n");
        }
    }
    else{
        printf("Invalid choice.\n");
    }
    
    return 0;
}
