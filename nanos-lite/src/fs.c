#include "fs.h"
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin" , 0xffffffff, 0, 0, invalid_read, invalid_write},
  {"stdout", 0xffffffff, 0, 0, invalid_read, serial_write},
  {"stderr", 0xffffffff, 0, 0, invalid_read, serial_write},
  {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write},
  {"/dev/fb", 0xffffffff, 0, 0, invalid_read, fb_write },
  {"/dev/events", 0x7fffffff, 0, 0, events_read, invalid_write},
  {"/dev/tty", 0xffffffff, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  file_table[4].size = 4 * screen_width() * screen_height();
  assert(!strcmp(file_table[4].name, "/dev/fb"));
  for (int i = 7; i < NR_FILES; ++i) {
    file_table[i].write = ramdisk_write;
    file_table[i].read = ramdisk_read;
  }
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; ++i) {
    if(!strcmp(pathname, file_table[i].name))
      return i;
  }
  Log("error in open %s", pathname);
  //assert(0);
  return -1;
}

int fs_close(int fd) {
  file_table[fd].open_offset = 0;
  return 0;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

  //Log("fs_read%d %d %d %d", fd, file_table[fd].size, file_table[fd].open_offset, len);
  //printf("%d %d %d\n", file_table[fd].open_offset, len, file_table[fd].size);
  //if (file_table[fd].read) 
  //  ret = file_table[fd].read(buf, 0, len);
  //else 
  //  ret = ramdisk_read(buf, file_table[fd].disk_offset+file_table[fd].open_offset, len);
size_t fs_read(int fd, void *buf, size_t len) {
  size_t ret = 0;
  len = file_table[fd].open_offset + len > file_table[fd].size ? (file_table[fd].size - file_table[fd].open_offset) : len;
  ret = file_table[fd].read(buf, file_table[fd].disk_offset+file_table[fd].open_offset, len);
  file_table[fd].open_offset += ret;
  return ret;
}

  //Log("fs_write %d %d", fd, len);
size_t fs_write(int fd, const void *buf, size_t len) {
  len = file_table[fd].open_offset + len > file_table[fd].size ? (file_table[fd].size - file_table[fd].open_offset) : len;
  size_t ret = file_table[fd].write(buf, file_table[fd].disk_offset+file_table[fd].open_offset, len);
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  //assert(fd != 4);
  int pos = 0;
  switch(whence) {
    case SEEK_SET : pos = offset; break;
    case SEEK_CUR : pos = file_table[fd].open_offset + offset; break;
    case SEEK_END : pos = file_table[fd].size + offset; break;
  }
  if(pos < 0 || pos > file_table[fd].size) {
    Log("error in fs_lseek : fd = %d",fd);
    return -1;
  }
  file_table[fd].open_offset = pos;
  return pos;
}