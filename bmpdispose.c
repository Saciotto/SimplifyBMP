#include "bmp.h"

#include <stdlib.h>

void bmpDispose(BMP *bmp)
{
    if (bmp == NULL) return;
    if (bmp->data != NULL) {
        size_t i, height;
        height = (size_t) abs(bmp->header.height);
        for (i = 0; i < height; i++)
            if (bmp->data[i] != NULL) free(bmp->data[i]);
        free(bmp->data);
    }
    free(bmp);
}
