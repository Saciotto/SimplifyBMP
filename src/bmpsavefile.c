#include "bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int writeLittleEndian(FILE *fp, uint32_t value, size_t size)
{
    char buf[4];
    memset(buf, 0, sizeof(buf));
    if (size > sizeof(buf)) return BMP_INTERNAL;
    for (size_t idx = 0; idx < size; idx++) {
        buf[idx] = value % 0x100;
        value /= 0x100;
    }
    size_t len = fwrite(buf, 1, size, fp);
    if (len != size) return BMP_FILE_ERROR;
    return 0;
}

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
    bmp->header.imageSize = ((uint32_t) rowSize) * ((uint32_t) height);

    *offset = (uint32_t)(14L + bmp->header.headerSize);
    *fileSize = (uint32_t)(*offset + bmp->header.imageSize);
}

static int saveHeader(FILE *fp, uint32_t offset, uint32_t fileSize)
{
    int err = writeLittleEndian(fp, BMP_ID_WINDOWS, sizeof(uint16_t));
    if (err) return err;

    err = writeLittleEndian(fp, fileSize, sizeof(fileSize));
    if (err) return err;

    // Not used
    err = writeLittleEndian(fp, 0, sizeof(uint32_t));
    if (err) return err;

    err = writeLittleEndian(fp, offset, sizeof(offset));
    if (err) return err;

    return 0;
}

static int saveHeaderInfo(FILE *fp, BMP *bmp)
{
    int err = writeLittleEndian(fp, bmp->header.headerSize, sizeof(bmp->header.headerSize));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.width, sizeof(bmp->header.width));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.height, sizeof(bmp->header.height));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.planes, sizeof(bmp->header.planes));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.depth, sizeof(bmp->header.depth));
    if (err) return err;

    err = writeLittleEndian(fp, bmp->header.compression, sizeof(bmp->header.compression));
    if (err) return err;

    err = writeLittleEndian(fp, bmp->header.imageSize, sizeof(bmp->header.imageSize));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.hres, sizeof(bmp->header.hres));
    if (err) return err;

    err = writeLittleEndian(fp, (uint32_t) bmp->header.vres, sizeof(bmp->header.vres));
    if (err) return err;

    err = writeLittleEndian(fp, bmp->header.clrUsed, sizeof(bmp->header.clrUsed));
    if (err) return err;

    err = writeLittleEndian(fp, bmp->header.clrImportant, sizeof(bmp->header.clrImportant));
    if (err) return err;

    return 0;
}

static unsigned char applyAlpha(uint8_t color, uint8_t alpha)
{
    // Use white background.
    if (alpha == 255) return (unsigned char) color;
    return (unsigned char) (((float) color) * ((float) alpha) / 255 + 255 - alpha);
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
