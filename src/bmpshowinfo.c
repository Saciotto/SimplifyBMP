#include "bmp.h"

#include <stdio.h>

void bmpShowInfo(const BMP *bmp)
{
    printf("Header Size: %u\n", bmp->header.headerSize);
    printf("Width      : %d\n", bmp->header.width);
    printf("Height     : %d\n", bmp->header.height);
    printf("Planes     : %u\n", bmp->header.planes);
    printf("Depth      : %u\n", bmp->header.depth);
    printf("Compression: %u\n", bmp->header.compression);
    printf("Image Size : %u\n", bmp->header.imageSize);
    printf("H. Resul.  : %d\n", bmp->header.hres);
    printf("V. Resul.  : %d\n", bmp->header.vres);
    printf("Colors used: %u\n", bmp->header.clrUsed);
    printf("Colors imp.: %u\n", bmp->header.clrImportant);
}