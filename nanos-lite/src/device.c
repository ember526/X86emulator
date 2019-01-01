#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  //_yield();
  //Log("serial_write %d %d", offset, len);
  int i = 0;
  for (i = 0; ((char *)buf)[i] != '\0' && i < len; ++i)
    _putc(((char *)buf)[i]);
  return i;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  //_yield();
  int key = read_key();
  bool key_down = false;
  if (key & 0x8000) {
    key ^= 0x8000;
    key_down = true;
  }
  //Log("key %d %d  %d", key, offset, len);
  size_t ret = 0;
  if (key != _KEY_NONE) {
    if (key_down) {
      //printf("-->*1\n");
      ret = sprintf((char*) buf, "kd %s\n", keyname[key]);
        extern int fg_pcb;
      if(key==_KEY_F1)      {fg_pcb = 1;Log("->Switching to procedure 1"); _yield();}
      else if(key==_KEY_F2) {fg_pcb = 2;Log("->Switching to procedure 2"); _yield();}
      else if(key==_KEY_F3) {fg_pcb = 3;Log("->Switching to procedure 3"); _yield();}
    } 
    else {
      //printf("-->*2\n");
      ret = sprintf((char*) buf, "ku %s\n", keyname[key]);
    }
  } 
  else {
      //printf("-->*3\n");
      ret = sprintf((char*) buf, "t %d\n", uptime());
  }
  //printf("-->%s %d\n", buf, ret);
  return ret;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
   memcpy(buf, dispinfo+offset, len);
   return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  //_yield();
  int x = (offset / sizeof(uint32_t) ) % screen_width();
  int y = (offset / sizeof(uint32_t) ) / screen_width();
  //printf("x:%d y:%d len%d\n", x, y, len/ sizeof(uint32_t));
  draw_rect((uint32_t *)buf, x, y, len / sizeof(uint32_t) ,1);
  return len;
  //int x = (offset >> 2 ) % screen_width();
  //int y = (offset >> 2 ) / screen_width();
  //printf("x:%d y:%d len%d\n", x, y, len>>2);
  //draw_rect((uint32_t *)buf, x, y, len >> 2 ,1);
  //return 0;
}
extern size_t video_read(uintptr_t reg, void *buf, size_t size);
void init_device() {
  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  Log("Initializing devices...");
  _ioe_init();

  _VideoInfoReg info;
  video_read(_DEVREG_VIDEO_INFO, (void *)&info, 0);
  Log("dis info : %d %d", info.width, info.height);
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", info.width, info.height);
  return;
}
