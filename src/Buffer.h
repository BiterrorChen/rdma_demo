#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

class Buffer {
 public:
  char *addr;
  size_t size;
  char *write_pos;
  char *read_pos;

  Buffer() : addr(NULL), size(0), write_pos(NULL), read_pos(NULL) {}
  Buffer(char *addr, size_t size)
      : addr(addr), size(size), write_pos(addr), read_pos(addr) {}

  static Buffer allocate(size_t size);

  Buffer &write(void *src, size_t count);
  template <class T>
  Buffer &write(T src) {
    return write(&src, sizeof(T));
  }

  Buffer &read(void *dst, size_t count);
  template <class T>
  Buffer &read(T *dst) {
    return read(dst, sizeof(T));
  }

  void free();
  void clear();
};
