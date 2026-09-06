#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "image.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelOffset;
} BMPFileHeader;

typedef struct {
    uint32_t headerSize;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bitsperpixel;
    char ex[24];
} BMPInfoHeader;
#pragma pack(pop)

Image* create_image(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;
    Image *img = (Image*) malloc(sizeof(Image));
    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->pixels = (Pixel*) malloc(width * height * sizeof(Pixel));

    if (!img->pixels) {
        free(img);
        return NULL;
    }
    return img;
}

void free_image(Image *img) {
    if (img) {
        if (img->pixels) free(img->pixels);
        free(img);
    }
}

Image* copy_image(Image *img) {
    if (!img || !img->pixels) return NULL;
    Image *copy = create_image(img->width, img->height);
    if (!copy) return NULL;
    int total_pixels = img->width * img->height;
    
    for (int i = 0; i < total_pixels; i++) {
        copy->pixels[i].r = img->pixels[i].r;
        copy->pixels[i].g = img->pixels[i].g;
        copy->pixels[i].b = img->pixels[i].b;
    }
    
    return copy;
}

Image* load_image(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    if (fread(&fileHeader, sizeof(BMPFileHeader), 1, f) != 1 ||
        fread(&infoHeader, sizeof(BMPInfoHeader), 1, f) != 1) {
        fclose(f);
        return NULL;
    }

    if (fileHeader.type != 0x4D42) {
        fclose(f);
        return NULL;
    }

    if (infoHeader.bitsperpixel != 24) {
        fclose(f);
        return NULL;
    }

    int width = infoHeader.width;
    int height = infoHeader.height;

    Image *img = create_image(width, height);
    if (!img) {
        fclose(f);
        return NULL;
    }

    fseek(f, fileHeader.pixelOffset, SEEK_SET);

    int total_pixels = width * height;
    for (int i = 0; i < total_pixels; i++) {
        unsigned char bgr[3];
        if (fread(bgr, 3, 1, f) != 1) {
            free_image(img);
            fclose(f);
            return NULL;
        }
        img->pixels[i].b = bgr[0];
        img->pixels[i].g = bgr[1];
        img->pixels[i].r = bgr[2];
       
        
    }

    fclose(f);

    int count = 0;
    for (int i = 0; i < height / 2; i++) {
        for (int j = 0; j < width; j++) {
            int indx = (height * width) - (width * (i + 1)) + j;

            Pixel temp = img->pixels[count];
            img->pixels[count] = img->pixels[indx];
            img->pixels[indx] = temp;

            count++;
        }
    }
    
    return img;
}

int save_image(const char *filename, Image *img) {
    if (!img || !img->pixels) return 0;

    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    int width = img->width;
    int height = img->height;
    int image_size = width * height * 3;

    BMPFileHeader fileHeader;
    fileHeader.type = 0x4D42;
    fileHeader.fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.pixelOffset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader infoHeader;
    memset(&infoHeader, 0, sizeof(BMPInfoHeader));
    infoHeader.headerSize = sizeof(BMPInfoHeader);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsperpixel = 24;

    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, f);
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, f);

    for (int y = height - 1; y >= 0; y--) {
    //for(int y=0; y<height; y++){
        for (int x = 0; x < width; x++) {
            int indx = y * width + x;
            unsigned char bgr[3] = {
                img->pixels[indx].b,
                img->pixels[indx].g,
                img->pixels[indx].r
            };
            fwrite(bgr, 3, 1, f);
        }
    }

    fclose(f);
    return 1;
}