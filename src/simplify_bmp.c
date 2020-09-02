#include "bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void ShowError(int err)
{
    switch (err) {
    case BMP_NO_MEMORY:
        fprintf(stderr, "insufficient memory.\n");
        break;
    case BMP_FILE_ERROR:
        fprintf(stderr, "File error.\n");
        break;
    case BMP_INVALID_BMP:
        fprintf(stderr, "Invalid Bitmap.\n");
        break;
    default:
        fprintf(stderr, "Defualt error.\n");
        break;
    }
}

static void showUsageErr()
{
    fprintf(stderr, "Invalid argument. Usage: simplify_bmp [-v] <img1.bmp> [img2.bmp ...].\n");
}

int main(int argc, char **argv)
{
    BMP *bmp;
    bool printDetails = false;
    int firstImage = 1;

    if (argc < 2) {
        showUsageErr();
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], "-v")) {
        printDetails = true;
        firstImage = 2;
        if (argc < 3) {
            showUsageErr();
            return EXIT_FAILURE;
        }
    }

    for (int idx = firstImage; idx < argc; idx++) {
        const char *filename = argv[idx];
        printf("Processing %s...\n", filename);
        int err = bmpLoadFile(&bmp, filename);
        if (err) {
            fprintf(stderr, "Error while reading file %s.\n", filename);
            ShowError(err);
            return EXIT_FAILURE;
        }

        if (printDetails) {
            printf("---------- Header (original) ----------\n");
            bmpShowInfo(bmp);
        }

        err = bmpSaveFile(bmp, filename, RGB_24);
        if (err) {
            fprintf(stderr, "Error while writing file %s.\n", filename);
            ShowError(err);
            bmpDispose(bmp);
            return EXIT_FAILURE;
        }

        if (printDetails) {
            printf("----------- Header (output) -----------\n");
            bmpShowInfo(bmp);
            printf("---------------------------------------\n\n");
        }

        bmpDispose(bmp);
    }
    printf("Done!\n");

    return EXIT_SUCCESS;
}
