#ifndef BMP_H
#define BMP_H

#include <stdlib.h>
#include <stdint.h>

// Error codes
#define BMP_NO_MEMORY   -1001
#define BMP_FILE_ERROR  -1002
#define BMP_INVALID_BMP -1003

typedef enum { RGB_24, RGBA_32 } BMP_FORMAT;

struct Pixel {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct BMP_Header {
    uint32_t headerSize;     // The size, in bytes, of the Image Header
    int32_t width;           // This is the width of the image, in pixels
    int32_t height;          // This is the height of the image, in pixels
    uint16_t planes;         // This is the number of image planes in the image. It is always equal to 1
    uint16_t depth;          // The number of bits per pixel, which is the color depth of the image
    uint32_t compression;    // The compression method being used
    uint32_t imageSize;      // This is the size of the raw bitmap data
    int32_t hres;            // The horizontal resolution (pixels per meter)
    int32_t vres;            // The vertical resolution (pixels per meter)
    uint32_t clrUsed;        // The number of colors in the color palette, or 0 to default to 2n
    uint32_t clrImportant;   // The number of important colors used, or 0 when every color is important;
};

typedef struct {
    struct BMP_Header header;
    struct Pixel **data;
} BMP;

int bmpLoadFile(BMP **bmp, const char *fileName);

void bmpShowInfo(const BMP *bmp);

int bmpSaveFile(BMP *bmp, const char *fileName, BMP_FORMAT format);

void bmpDispose(BMP *bmp);

#endif
