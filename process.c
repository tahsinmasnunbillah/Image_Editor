#include <stdlib.h>
#include "image.h"
#include "process.h"

static unsigned char clamp(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (unsigned char)val;
}

void apply_grayscale(Image *img) {
    if (!img || !img->pixels) return;
    //  A commonly used formula is gray = 0.299R + 0.587G + 0.114B. 
    //  The resulting value should be assigned to all three color channels.
    int total_pixels = img->width * img->height;
    for (int i = 0; i < total_pixels; i++) {
        unsigned char gray = clamp((int)(0.299 * img->pixels[i].r + 0.587 * img->pixels[i].g + 0.114 * img->pixels[i].b));
        img->pixels[i].r = gray;
        img->pixels[i].g = gray;
        img->pixels[i].b = gray;
    }
}

void adjust_brightness(Image *img, int factor) {
    if (!img || !img->pixels) return;
    // A user-specified value can be added to each RGB component. 
    // Values must be restricted to the valid range of 0 to 255.
    int total_pixels = img->width * img->height;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i].r = clamp((int)img->pixels[i].r + factor);
        img->pixels[i].g = clamp((int)img->pixels[i].g + factor);
        img->pixels[i].b = clamp((int)img->pixels[i].b + factor);
    }
}

void apply_invert(Image *img) {
    if (!img || !img->pixels) return;
    //  The application should produce a negative-like image by calculating 
    //  R = 255 - R, G = 255 - G, and B = 255 - B for every pixel.
    int total_pixels = img->width * img->height;
    for (int i = 0; i < total_pixels; i++) {
        img->pixels[i].r = 255 - img->pixels[i].r;
        img->pixels[i].g = 255 - img->pixels[i].g;
        img->pixels[i].b = 255 - img->pixels[i].b;
    }
}

void flip_horizontal(Image *img) {
    if (!img || !img->pixels) return;
    // The application should mirror the image from left to right. 
    // A pixel at (x, y) should be exchanged with the pixel at (width - 1 - x, y).
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width / 2; x++) {
            int idx1 = y * img->width + x;
            int idx2 = y * img->width + (img->width - 1 - x);

            Pixel temp = img->pixels[idx1];
            img->pixels[idx1] = img->pixels[idx2];
            img->pixels[idx2] = temp;
        }
    }
}

void flip_vertical(Image *img) {
    if (!img || !img->pixels) return;
    // The application should mirror the image from top to bottom. 
    // A pixel at (x, y) should be exchanged with the pixel at (x, height - 1 - y).
    for (int y = 0; y < img->height / 2; y++) {
        for (int x = 0; x < img->width; x++) {
            int idx1 = y * img->width + x;
            int idx2 = ((img->height - 1 - y) * img->width) + x;

            Pixel temp = img->pixels[idx1];
            img->pixels[idx1] = img->pixels[idx2];
            img->pixels[idx2] = temp;
        }
    }
}

Image* rotate_90(Image *img) {
    if (!img || !img->pixels) return NULL;
    // Students should create a new image with the width and height
    // exchanged and copy the pixels to their new positions.
    Image *new_img = create_image(img->height, img->width);
    if (!new_img) return NULL;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int src_idx = y * img->width + x;
            int dst_x = img->height - 1 - y;
            int dst_y = x;
            int dst_idx = dst_y * new_img->width + dst_x;

            new_img->pixels[dst_idx] = img->pixels[src_idx];
        }
    }
    return new_img;
}

Image* crop_image(Image *img, int x, int y, int crop_w, int crop_h) {
    if (!img || !img->pixels) return NULL;
    //The application should allow the user to extract a rectangular region of the image. 
    // The program should ensure that the selected region remains within the image boundaries.
    if (x < 0 || y < 0 || crop_w <= 0 || crop_h <= 0 || x + crop_w > img->width || y + crop_h > img->height) {
        return NULL;
    }

    Image *new_img = create_image(crop_w, crop_h);
    if (!new_img) return NULL;

    for (int cy = 0; cy < crop_h; cy++) {
        for (int cx = 0; cx < crop_w; cx++) {
            int src_idx = (y + cy) * img->width + (x + cx);
            int dst_idx = cy * crop_w + cx;

            new_img->pixels[dst_idx] = img->pixels[src_idx];
        }
    }
    return new_img;
}

Image* apply_blur(Image *img) {
    if (!img || !img->pixels) return NULL;
    // The application should provide a blur operation using a 3 × 3 neighborhood. 
    // Each output pixel should be calculated by averaging the corresponding RGB components of the current pixel 
    // and its neighboring pixels.

    Image *new_img = create_image(img->width, img->height);
    if (!new_img) return NULL;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int sum_r = 0, sum_g = 0, sum_b = 0, count = 0;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;

                    if (nx >= 0 && nx < img->width && ny >= 0 && ny < img->height) {
                        int idx = ny * img->width + nx;
                        sum_r += img->pixels[idx].r;
                        sum_g += img->pixels[idx].g;
                        sum_b += img->pixels[idx].b;
                        count++;
                    }
                }
            }
            int dst_idx = y * img->width + x;
            new_img->pixels[dst_idx].r = clamp(sum_r / count);
            new_img->pixels[dst_idx].g = clamp(sum_g / count);
            new_img->pixels[dst_idx].b = clamp(sum_b / count);
        }
    }
      return new_img;
}
  
