#ifndef PROCESS_H
#define PROCESS_H

#include "image.h"

void apply_grayscale(Image *img);
void adjust_brightness(Image *img, int factor);
void apply_invert(Image *img);
void flip_horizontal(Image *img);
void flip_vertical(Image *img);
Image* rotate_90(Image *img);
Image* crop_image(Image *img, int x, int y, int crop_w, int crop_h);
Image* apply_blur(Image *img);

#endif