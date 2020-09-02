#include "bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

static uint32_t readLittleEndian(const unsigned char *buf, size_t len)
{
    uint32_t ret = 0;
    while (len-- > 0) {
        ret *= 0x100;
        ret += (uint32_t) buf[len];
    }
    return ret;
}

static int readUint16(FILE *fp, uint16_t *value)
{
    unsigned char buf[2];
    size_t len = fread(buf, 1, sizeof(buf), fp);
    if (len != sizeof(buf)) return BMP_FILE_ERROR;
    *value = (uint16_t) readLittleEndian(buf, sizeof(buf));
    return 0;
}

static int readUint32(FILE *fp, uint32_t *value)
{
    unsigned char buf[4];
    size_t len = fread((void *) buf, 1, sizeof(buf), fp);
    if (len != sizeof(buf)) return BMP_FILE_ERROR;
    *value = readLittleEndian(buf, sizeof(buf));
    return 0;
}

static int loadFileHeader(FILE *fp, uint32_t *offset)
{
    /* skip file size */
    int err = fseek(fp, 4, SEEK_CUR);
    if (err) return BMP_FILE_ERROR;

    /* skip reserved data */
    err = fseek(fp, 4, SEEK_CUR);
    if (err) return BMP_FILE_ERROR;

    err = readUint32(fp, offset);
    if (err) return err;

    return 0;
}

static int loadDibWin(FILE *fp, BMP *bmp)
{
    int err = readUint32(fp, &bmp->header.headerSize);
    if (err) return err;

    err = readUint32(fp, (uint32_t *) &bmp->header.width);
    if (err) return err;

    err = readUint32(fp, (uint32_t *) &bmp->header.height);
    if (err) return err;

    err = readUint16(fp, &bmp->header.planes);
    if (err) return err;

    err = readUint16(fp, &bmp->header.depth);
    if (err) return err;

    err = readUint32(fp, &bmp->header.compression);
    if (err) return err;

    err = readUint32(fp, &bmp->header.imageSize);
    if (err) return err;

    err = readUint32(fp, (uint32_t *) &bmp->header.hres);
    if (err) return err;

    err = readUint32(fp, (uint32_t *) &bmp->header.vres);
    if (err) return err;

    err = readUint32(fp, &bmp->header.clrUsed);
    if (err) return err;

    err = readUint32(fp, &bmp->header.clrImportant);
    if (err) return err;

    return 0;
}

static int allocData(BMP *bmp)
{
    size_t i, height, width;

    height = (size_t) abs(bmp->header.height);
    width = (size_t) abs(bmp->header.width);

    bmp->data = calloc(height, sizeof(bmp->data));
    if (bmp->data == NULL) return BMP_NO_MEMORY;

    for (i = 0; i < height; i++) {
        bmp->data[i] = calloc(width, sizeof(*bmp->data));
        if (bmp->data[i] == NULL) return BMP_NO_MEMORY;
    }
    return 0;
}

static int loadData(FILE *fp, BMP *bmp, uint32_t dataOffset)
{
    int err;
    size_t i, j, height, width, pos, offset, len, pixSize, rowSize, bmpRowSize;
    long int padding;
    char *data;

    err = fseek(fp, dataOffset, SEEK_SET);
    if (err) return BMP_FILE_ERROR;

    err = allocData(bmp);
    if (err) return err;

    height = (size_t) abs(bmp->header.height);
    width = (size_t) abs(bmp->header.width);
    offset = bmp->header.height > 0 ? height - 1 : 0;
    pixSize = bmp->header.depth / 8;
    rowSize = pixSize * width;
    bmpRowSize = rowSize;
    while (bmpRowSize % 4)
        bmpRowSize++;
    padding = (long int) (bmpRowSize - rowSize);

    data = malloc(rowSize);
    if (data == NULL) return BMP_NO_MEMORY;

    for (i = 0; i < height; i++) {
        char *pdata;

        len = fread((void *) data, 1, rowSize, fp);
        if (len != rowSize) {
            free(data);
            return BMP_FILE_ERROR;
        }

        pdata = data;
        pos = offset > i ? offset - i : i - offset;
        for (j = 0; j < width; j++) {
            if (pixSize == 4) {
                bmp->data[pos][j].r = *pdata++;
                bmp->data[pos][j].g = *pdata++;
                bmp->data[pos][j].b = *pdata++;
                bmp->data[pos][j].a = *pdata++;
            } else if (pixSize == 3) {
                bmp->data[pos][j].r = *pdata++;
                bmp->data[pos][j].g = *pdata++;
                bmp->data[pos][j].b = *pdata++;
                bmp->data[pos][j].a = 255;
            }
        }

        if (i + 1 < height) {
            err = fseek(fp, padding, SEEK_CUR);
            if (err) {
                free(data);
                return BMP_FILE_ERROR;
            }
        }
    }

    free(data);
    return 0;
}

static int loadFile(FILE *fp, BMP *bmp)
{
    int err;
    uint16_t id;
    uint32_t offset;

    err = readUint16(fp, &id);
    if (err) return err;

    if (id != BMP_ID_WINDOWS) return BMP_INVALID_BMP;

    err = loadFileHeader(fp, &offset);
    if (err) return err;

    err = loadDibWin(fp, bmp);
    if (err) return err;

    err = loadData(fp, bmp, offset);
    if (err) return err;

    return 0;
}

int bmpLoadFile(BMP **bmp, const char *fileName)
{
    int err;
    FILE *fp;

    *bmp = (BMP *) calloc(1, sizeof(**bmp));
    if (*bmp == NULL) return BMP_NO_MEMORY;

    fp = fopen(fileName, "rb");
    if (fp == NULL) {
        free(*bmp);
        return BMP_FILE_ERROR;
    }

    err = loadFile(fp, *bmp);
    if (err) {
        bmpDispose(*bmp);
        *bmp = NULL;
    }

    fclose(fp);
    return err;
}
