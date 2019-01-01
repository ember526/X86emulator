#include <am.h>
#include <x86.h>
#include <amdev.h>
//#include <klib.h>
#include "klib.h"
#define W 400
#define H 300
static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  uint32_t screen;
  //printf("screenadddddddddddsssssssssssssssssssssssssssssssssssssssss\n");
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      screen = inl(0x100);
      //printf("screen%d.\n", screen);
      info->width = screen >> 16;
      info->height = screen << 16 >> 16;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  //printf("screenadddddddddddsssssssssssssssssssssssssssssssssssssssss\n");
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;
      //int i;
      //int size = screen_width() * screen_height();
      //for (i = 0; i < size; i ++) fb[i] = i;
      //for(i = 0; i < ctl->h; i++) {
      //  memcpy(fb + (ctl->y + i)*screen_width() + ctl->x, ctl->pixels + i*ctl->w, ctl->w*4);
      //}
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int cp_bytes = sizeof(uint32_t) * (w < W - x?w:(W-x));
      for (int j = 0; j < h && y + j < H; j ++) {
        memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
        pixels += w;
      }
      if (ctl->sync) {
      // do nothing, hardware syncs.
      }
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
}