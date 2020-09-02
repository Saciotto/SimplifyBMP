#include "bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void calculateHeader(BMP *bmp, BMP_FORMAT format, uint32_t *offset, uint32_t *fileSize)
{
    size_t rowSize, width, height, pixSize;

    switch (format) {
    case RGB_24:
        bmp->header.depth = 24;
        break;
    case RGBA_32:
        bmp->header.depth = 32;
        break;
    }

    width = (size_t) abs(bmp->header.width);
    height = (size_t) abs(bmp->header.height);
    pixSize = bmp->header.depth / 8;
    rowSize = pixSize * width;
    while (rowSize % 4)
        rowSize++;

    bmp->header.width = (int32_t) width;
    bmp->header.height = (int32_t) height;
    bmp->header.planes = 1;
    bmp->header.headerSize = sizeof(bmp->header);
    bmp->header.compression = 0;
    bmp->header.clrUsed = 0;
    bmp->header.clrImportant = 0;

    *offset = (uint32_t)(14L + bmp->header.headerSize);
    *fileSize = (uint32_t)(*offset + (rowSize * height));
}

static int saveHeader(FILE *fp, uint32_t offset, uint32_t fileSize)
{
    size_t len;
    const char id[] = "BM";

    len = fwrite((const void *) id, 1, 2, fp);
    if (len != 2) return BMP_FILE_ERROR;

    len = fwrite((const void *) &fileSize, 1, 4, fp);
    if (len != 4) return BMP_FILE_ERROR;

    const char notUsed[] = "\x00\x00\x00\x00";
    len = fwrite((const void *) notUsed, 1, 4, fp);
    if (len != 4) return BMP_FILE_ERROR;

    len = fwrite((const void *) &offset, 1, 4, fp);
    if (len != 4) return BMP_FILE_ERROR;

    return 0;
}

static int saveHeaderInfo(FILE *fp, BMP *bmp)
{
    size_t len = fwrite((void *) &bmp->header, bmp->header.headerSize, 1, fp);
    if (len != 1) return BMP_FILE_ERROR;
    return 0;
}

static unsigned char applyAlpha(uint8_t color, uint8_t alpha)
{
    // Use white background.
    if (alpha == 255) return (char) color;
    return (char)(((float) color) * ((float) alpha) / 255 + 255 - alpha);
}

static int saveData(FILE *fp, BMP *bmp, BMP_FORMAT format)
{
    int32_t i, j;
    size_t rowSize, pixSize, len;
    unsigned char *dataRow, *row;

    pixSize = bmp->header.depth / 8;
    rowSize = pixSize * (size_t)(bmp->header.width);
    while (rowSize % 4)
        rowSize++;

    dataRow = (unsigned char *) calloc(rowSize, 1);
    if (dataRow == NULL) return BMP_NO_MEMORY;

    for (i = bmp->header.height - 1; i >= 0; i--) {
        memset(dataRow, 0, rowSize);
        row = dataRow;

        for (j = 0; j < bmp->header.width; j++) {
            if (format == RGBA_32) {
                *row++ = (unsigned char) bmp->data[i][j].r;
                *row++ = (unsigned char) bmp->data[i][j].g;
                *row++ = (unsigned char) bmp->data[i][j].b;
                *row++ = (unsigned char) bmp->data[i][j].a;
            } else if (format == RGB_24) {
                *row++ = applyAlpha(bmp->data[i][j].r, bmp->data[i][j].a);
                *row++ = applyAlpha(bmp->data[i][j].g, bmp->data[i][j].a);
                *row++ = applyAlpha(bmp->data[i][j].b, bmp->data[i][j].a);
            }
        }

        len = fwrite(dataRow, 1, rowSize, fp);
        if (len != rowSize) {
            free(dataRow);
            return BMP_FILE_ERROR;
        }
    }

    free(dataRow);
    return 0;
}

static int saveFile(FILE *fp, BMP *bmp, BMP_FORMAT format)
{
    int err;
    uint32_t offset, fileSize;

    calculateHeader(bmp, format, &offset, &fileSize);

    err = saveHeader(fp, offset, fileSize);
    if (err) return err;

    err = saveHeaderInfo(fp, bmp);
    if (err) return err;

    err = saveData(fp, bmp, format);
    return err;
}

int bmpSaveFile(BMP *bmp, const char *fileName, BMP_FORMAT format)
{
    int err;
    FILE *fp;

    fp = fopen(fileName, "w+b");
    if (fp == NULL) {
        return BMP_FILE_ERROR;
    }

    err = saveFile(fp, bmp, format);

    fclose(fp);
    return err;
}
