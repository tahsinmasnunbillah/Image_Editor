#include <iup.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "process.h"
#include "gui.h"

static Image *current_image = NULL;
static Image *undo_image = NULL;

static Ihandle *img_label = NULL;
static Ihandle *current_iup_img = NULL;

static void save_undo_state() {
    if (undo_image) free_image(undo_image);
    if (current_image) undo_image = copy_image(current_image);
    else undo_image = NULL;
}

static void refresh_display() {
    int max_w = 670;
    int max_h = 570;
    Ihandle *new_iup_img = NULL;

    if (!current_image) {
        unsigned char dummy_pixel[4] = {0, 0, 0, 0};
        new_iup_img = IupImageRGBA(1, 1, dummy_pixel);
    } else {
        float ratio_w = (float)max_w / current_image->width;
        float ratio_h = (float)max_h / current_image->height;
        float ratio = (ratio_w < ratio_h) ? ratio_w : ratio_h;

        int disp_w = (int)(current_image->width * ratio);
        int disp_h = (int)(current_image->height * ratio);

        if (disp_w < 1) disp_w = 1;
        if (disp_h < 1) disp_h = 1;

        unsigned char *buf = (unsigned char*) malloc(disp_w * disp_h * 3);
        if (!buf) return;

        for (int y = 0; y < disp_h; y++) {
            for (int x = 0; x < disp_w; x++) {
                int src_x = x * current_image->width / disp_w;
                int src_y = y * current_image->height / disp_h;
                int src_idx = src_y * current_image->width + src_x;
                int dst_idx = (y * disp_w + x) * 3;

                buf[dst_idx + 0] = current_image->pixels[src_idx].r;
                buf[dst_idx + 1] = current_image->pixels[src_idx].g;
                buf[dst_idx + 2] = current_image->pixels[src_idx].b;
            }
        }

        new_iup_img = IupImageRGB(disp_w, disp_h, buf);
        free(buf);
    }

    if (new_iup_img) {
        IupSetAttributeHandle(img_label, "IMAGE", new_iup_img);

        if (current_iup_img) {
            IupDestroy(current_iup_img);
        }
        current_iup_img = new_iup_img;
    }

    IupUpdate(img_label);
}

static int btn_open_cb(Ihandle *self) {
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(filedlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");

    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(filedlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(filedlg, "VALUE");
        Image *img = load_image(filename);
        if (img) {
            if (current_image) free_image(current_image);
            if (undo_image) free_image(undo_image);
            current_image = img;
            undo_image = NULL;
            refresh_display();
        } else {
            IupMessage("Error", "Could not load image! Make sure it is a valid 24-bit uncompressed BMP file.");
        }
    }
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

static int btn_save_cb(Ihandle *self) {
    if (!current_image) {
        IupMessage("Warning", "No image loaded to save!");
        return IUP_DEFAULT;
    }
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(filedlg, "EXTFILTER", "BMP Files (*.bmp)|*.bmp|");

    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(filedlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(filedlg, "VALUE");
        if (save_image(filename, current_image)) {
            IupMessage("Success", "BMP Image saved successfully!");
        } else {
            IupMessage("Error", "Failed to save image!");
        }
    }
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

static int btn_grayscale_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    apply_grayscale(current_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int dialog_ok_cb(Ihandle *self) {
    Ihandle *dlg = IupGetDialog(self);
    IupSetAttribute(dlg, "STATUS", "1");
    return IUP_CLOSE;
}

static int dialog_cancel_cb(Ihandle *self) {
    Ihandle *dlg = IupGetDialog(self);
    IupSetAttribute(dlg, "STATUS", "0");
    return IUP_CLOSE;
}

static int btn_brightness_cb(Ihandle *self) {
    if (!current_image) { 
        IupMessage("Warning", "Load a BMP image first!"); 
        return IUP_DEFAULT; 
    }

    Ihandle *txt_factor = IupText(NULL);
    IupSetAttribute(txt_factor, "VALUE", "0");
    IupSetAttribute(txt_factor, "VISIBLECOLUMNS", "8");
    IupSetAttribute(txt_factor, "EXPAND", "NO");

    Ihandle *lbl = IupLabel("Adjustment Value (-255 to 255):");

    Ihandle *row = IupHbox(lbl, txt_factor, NULL);
    IupSetAttribute(row, "GAP", "7");
    IupSetAttribute(row, "ALIGNMENT", "ACENTER");
    IupSetAttribute(row, "EXPAND", "HORIZONTAL");

    Ihandle *btn_ok = IupButton("OK", NULL);
    Ihandle *btn_cancel = IupButton("Cancel", NULL);
    IupSetAttribute(btn_ok, "PADDING", "17x7");
    IupSetAttribute(btn_cancel, "PADDING", "17x7");

    IupSetCallback(btn_ok, "ACTION", (Icallback)dialog_ok_cb);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)dialog_cancel_cb);

    Ihandle *box_btn = IupHbox(IupFill(), btn_ok, btn_cancel, NULL);
    IupSetAttribute(box_btn, "GAP", "5");
    IupSetAttribute(box_btn, "EXPAND", "HORIZONTAL");

    Ihandle *vbox = IupVbox(row, box_btn, NULL);
    IupSetAttribute(vbox, "MARGIN", "30x30");
    IupSetAttribute(vbox, "GAP", "1");

    Ihandle *dlg = IupDialog(vbox);
    IupSetAttribute(dlg, "TITLE", "Brightness Adjustment");
    IupSetAttribute(dlg, "DIALOGFRAME", "YES");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "MINBOX", "NO");
    IupSetAttribute(dlg, "MAXBOX", "NO");
    
    IupSetAttribute(dlg, "RASTERSIZE", "500x320");
    
    IupSetAttributeHandle(dlg, "DEFAULTENTER", btn_ok);
    IupSetAttributeHandle(dlg, "DEFAULTESC", btn_cancel);

    IupPopup(dlg, IUP_CENTER, IUP_CENTER);

    char *status = IupGetAttribute(dlg, "STATUS");
    int is_ok = (status && strcmp(status, "1") == 0);
    int factor = atoi(IupGetAttribute(txt_factor, "VALUE"));

    IupDestroy(dlg);

    if (!is_ok) return IUP_DEFAULT; 

    if (factor < -255 || factor > 255) {
        IupMessage("Error", "Invalid Value! Enter a number between -255 and 255.");
        return IUP_DEFAULT;
    }

    save_undo_state();
    adjust_brightness(current_image, factor);
    refresh_display();

    return IUP_DEFAULT;
}

static int btn_invert_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    apply_invert(current_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int btn_hflip_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    flip_horizontal(current_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int btn_vflip_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    flip_vertical(current_image);
    refresh_display();
    return IUP_DEFAULT;
}

static int btn_rotate_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    Image *rotated = rotate_90(current_image);
    if (rotated) {
        free_image(current_image);
        current_image = rotated;
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int btn_crop_cb(Ihandle *self) {
    if (!current_image) { 
        IupMessage("Warning", "Load a BMP image first!"); 
        return IUP_DEFAULT; 
    }

    int orig_w = current_image->width;
    int orig_h = current_image->height;

    char str_w[32], str_h[32];
    sprintf(str_w, "%d", orig_w);
    sprintf(str_h, "%d", orig_h / 2);

    Ihandle *txt_x = IupText(NULL);
    Ihandle *txt_y = IupText(NULL);
    Ihandle *txt_w = IupText(NULL);
    Ihandle *txt_h = IupText(NULL);

    IupSetAttribute(txt_x, "VALUE", "0");
    IupSetAttribute(txt_y, "VALUE", "0");
    IupSetAttribute(txt_w, "VALUE", str_w);
    IupSetAttribute(txt_h, "VALUE", str_h);

    IupSetAttribute(txt_x, "VISIBLECOLUMNS", "5");
    IupSetAttribute(txt_y, "VISIBLECOLUMNS", "5");
    IupSetAttribute(txt_w, "VISIBLECOLUMNS", "5");
    IupSetAttribute(txt_h, "VISIBLECOLUMNS", "5");

    IupSetAttribute(txt_x, "EXPAND", "NO");
    IupSetAttribute(txt_y, "EXPAND", "NO");
    IupSetAttribute(txt_w, "EXPAND", "NO");
    IupSetAttribute(txt_h, "EXPAND", "NO");

    char lbl_x_str[64], lbl_y_str[64];
    sprintf(lbl_x_str, "Start X (0 to %d):", orig_w - 1);
    sprintf(lbl_y_str, "Start Y (0 to %d):", orig_h - 1);

    Ihandle *lbl_x = IupLabel(lbl_x_str);
    Ihandle *lbl_y = IupLabel(lbl_y_str);
    Ihandle *lbl_w = IupLabel("New Width (px):");
    Ihandle *lbl_h = IupLabel("New Height (px):");

    IupSetAttribute(lbl_x, "RASTERSIZE", "200x");
    IupSetAttribute(lbl_y, "RASTERSIZE", "200x");
    IupSetAttribute(lbl_w, "RASTERSIZE", "200x");
    IupSetAttribute(lbl_h, "RASTERSIZE", "200x");

    Ihandle *row_x = IupHbox(lbl_x, txt_x, NULL);
    Ihandle *row_y = IupHbox(lbl_y, txt_y, NULL);
    Ihandle *row_w = IupHbox(lbl_w, txt_w, NULL);
    Ihandle *row_h = IupHbox(lbl_h, txt_h, NULL);

    IupSetAttribute(row_x, "GAP", "7");
    IupSetAttribute(row_y, "GAP", "7");
    IupSetAttribute(row_w, "GAP", "7");
    IupSetAttribute(row_h, "GAP", "7");

    IupSetAttribute(row_x, "ALIGNMENT", "ACENTER");
    IupSetAttribute(row_y, "ALIGNMENT", "ACENTER");
    IupSetAttribute(row_w, "ALIGNMENT", "ACENTER");
    IupSetAttribute(row_h, "ALIGNMENT", "ACENTER");

    IupSetAttribute(row_x, "EXPAND", "HORIZONTAL");
    IupSetAttribute(row_y, "EXPAND", "HORIZONTAL");
    IupSetAttribute(row_w, "EXPAND", "HORIZONTAL");
    IupSetAttribute(row_h, "EXPAND", "HORIZONTAL");

    Ihandle *btn_ok = IupButton("OK", NULL);
    Ihandle *btn_cancel = IupButton("Cancel", NULL);

    IupSetAttribute(btn_ok, "PADDING", "17x7");
    IupSetAttribute(btn_cancel, "PADDING", "17x7");

    IupSetCallback(btn_ok, "ACTION", (Icallback)dialog_ok_cb);
    IupSetCallback(btn_cancel, "ACTION", (Icallback)dialog_cancel_cb);

    Ihandle *box_btn = IupHbox(IupFill(), btn_ok, btn_cancel, NULL);
    IupSetAttribute(box_btn, "GAP", "5");
    IupSetAttribute(box_btn, "EXPAND", "HORIZONTAL");

    Ihandle *vbox = IupVbox(row_x, row_y, row_w, row_h, box_btn, NULL);
    IupSetAttribute(vbox, "MARGIN", "30x30"); 
    IupSetAttribute(vbox, "GAP", "1");

    Ihandle *dlg = IupDialog(vbox);
    char dlg_title[128];
    sprintf(dlg_title, "Crop Image (%dx%d)", orig_w, orig_h);
    IupSetAttribute(dlg, "TITLE", dlg_title);
    IupSetAttribute(dlg, "DIALOGFRAME", "YES");
    IupSetAttribute(dlg, "RESIZE", "NO");
    IupSetAttribute(dlg, "MINBOX", "NO");
    IupSetAttribute(dlg, "MAXBOX", "NO");

    IupSetAttribute(dlg, "RASTERSIZE", "480x600");
    
    IupSetAttributeHandle(dlg, "DEFAULTENTER", btn_ok);
    IupSetAttributeHandle(dlg, "DEFAULTESC", btn_cancel);

    IupPopup(dlg, IUP_CENTER, IUP_CENTER);

    char *status = IupGetAttribute(dlg, "STATUS");
    int is_ok = (status && strcmp(status, "1") == 0);

    int start_x = atoi(IupGetAttribute(txt_x, "VALUE"));
    int start_y = atoi(IupGetAttribute(txt_y, "VALUE"));
    int new_w = atoi(IupGetAttribute(txt_w, "VALUE"));
    int new_h = atoi(IupGetAttribute(txt_h, "VALUE"));

    IupDestroy(dlg);

    if (!is_ok) return IUP_DEFAULT;

    if (start_x < 0 || start_y < 0 || new_w <= 0 || new_h <= 0 || 
        (start_x + new_w > orig_w) || (start_y + new_h > orig_h)) {
        IupMessage("Error", "Invalid Crop Area! Selection goes outside the image boundary.");
        return IUP_DEFAULT;
    }

    save_undo_state();
    Image *cropped = crop_image(current_image, start_x, start_y, new_w, new_h);
    if (cropped) {
        free_image(current_image);
        current_image = cropped;
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int btn_blur_cb(Ihandle *self) {
    if (!current_image) { IupMessage("Warning", "Load a BMP image first!"); return IUP_DEFAULT; }
    save_undo_state();
    Image *blurred = apply_blur(current_image);
    if (blurred) {
        free_image(current_image);
        current_image = blurred;
        refresh_display();
    }
    return IUP_DEFAULT;
}

static int btn_undo_cb(Ihandle *self) {
    if (!undo_image) {
        IupMessage("Info", "Nothing to undo!");
        return IUP_DEFAULT;
    }
    Image *temp = current_image;
    current_image = undo_image;
    undo_image = NULL;
    if (temp) free_image(temp);
    refresh_display();
    return IUP_DEFAULT;
}

void start_gui(int argc, char **argv) {
    IupOpen(&argc, &argv);

    Ihandle *btn_open = IupButton("Open BMP", NULL);
    Ihandle *btn_save = IupButton("Save BMP", NULL);
    Ihandle *btn_gray = IupButton("Grayscale", NULL);
    Ihandle *btn_bright = IupButton("Brightness", NULL);
    Ihandle *btn_invert = IupButton("Invert Color", NULL);
    Ihandle *btn_hflip = IupButton("Flip Horizontal", NULL);
    Ihandle *btn_vflip = IupButton("Flip Vertical", NULL);
    Ihandle *btn_rotate = IupButton("Rotate 90°", NULL);
    Ihandle *btn_crop = IupButton("Crop Image", NULL);
    Ihandle *btn_blur = IupButton("Blur", NULL);
    Ihandle *btn_undo = IupButton("Undo", NULL);

    IupSetCallback(btn_open, "ACTION", (Icallback)btn_open_cb);
    IupSetCallback(btn_save, "ACTION", (Icallback)btn_save_cb);
    IupSetCallback(btn_gray, "ACTION", (Icallback)btn_grayscale_cb);
    IupSetCallback(btn_bright, "ACTION", (Icallback)btn_brightness_cb);
    IupSetCallback(btn_invert, "ACTION", (Icallback)btn_invert_cb);
    IupSetCallback(btn_hflip, "ACTION", (Icallback)btn_hflip_cb);
    IupSetCallback(btn_vflip, "ACTION", (Icallback)btn_vflip_cb);
    IupSetCallback(btn_rotate, "ACTION", (Icallback)btn_rotate_cb);
    IupSetCallback(btn_crop, "ACTION", (Icallback)btn_crop_cb);
    IupSetCallback(btn_blur, "ACTION", (Icallback)btn_blur_cb);
    IupSetCallback(btn_undo, "ACTION", (Icallback)btn_undo_cb);

    Ihandle *button_row = IupHbox(
        IupFill(),
        btn_open, btn_save, btn_gray, btn_bright, 
        btn_invert, btn_hflip, btn_vflip, btn_rotate, 
        btn_crop, btn_blur, btn_undo,
        IupFill(),
        NULL
    );

    IupSetAttribute(button_row, "GAP", "3");
    IupSetAttribute(button_row, "ALIGNMENT", "ACENTER");
    IupSetAttribute(button_row, "EXPAND", "HORIZONTAL");

    img_label = IupLabel(NULL);
    IupSetAttribute(img_label, "RASTERSIZE", "800x650");
    IupSetAttribute(img_label, "EXPAND", "NO");
    IupSetAttribute(img_label, "ALIGNMENT", "ACENTER:ACENTER");

    refresh_display();

    Ihandle *img_hbox = IupHbox(IupFill(), img_label, IupFill(), NULL);

    Ihandle *main_vbox = IupVbox(button_row, IupFill(), img_hbox, IupFill(), NULL);
    IupSetAttribute(main_vbox, "GAP", "10");
    IupSetAttribute(main_vbox, "MARGIN", "10x10");
    IupSetAttribute(main_vbox, "ALIGNMENT", "ACENTER");

    Ihandle *dlg = IupDialog(main_vbox);
    IupSetAttribute(dlg, "TITLE", "Image Editor");

    int scr_w = 0, scr_h = 0;
    char *fullsize = IupGetGlobal("FULLSIZE");
    if (!fullsize) fullsize = IupGetGlobal("SCREENSIZE");

    if (fullsize && sscanf(fullsize, "%dx%d", &scr_w, &scr_h) == 2) {
        char rastersize[64];
        sprintf(rastersize, "1280x%d", scr_h);
        IupSetAttribute(dlg, "RASTERSIZE", rastersize);
    } else {
        IupSetAttribute(dlg, "RASTERSIZE", "1280x820");
    }

    IupSetAttribute(dlg, "RESIZE", "YES");
    IupSetAttribute(dlg, "MAXBOX", "YES");

    IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
    IupMainLoop();

    if (current_image) free_image(current_image);
    if (undo_image) free_image(undo_image);
    if (current_iup_img) IupDestroy(current_iup_img);

    IupClose();
}