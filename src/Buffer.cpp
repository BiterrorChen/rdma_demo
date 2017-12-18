#include "Buffer.h"

Buffer Buffer::allocate(size_t size) {
  Buffer buf;
  if (posix_memalign((void **)&buf.addr, getpagesize(), size) != 0) {
    perror("posix_memalign");
    exit(1);
  }

  buf.size = size;
  buf.clear();

  return buf;
}

Buffer &Buffer::write(void *src, size_t count) {
  if (this->write_pos + count <= this->addr + this->size) {
    memcpy(this->write_pos, src, count);
    this->write_pos += count;
    return *this;
  } else {
    std::cerr << "Buffer::write(): segmentation fault" << std::endl;
    exit(1);
  }
}

Buffer &Buffer::read(void *dst, size_t count) {
  if (this->read_pos + count <= this->addr + this->size) {
    memcpy(dst, this->read_pos, count);
    this->read_pos += count;
    return *this;
  } else {
    std::cerr << "Buffer::write(): segmentation fault" << std::endl;
    exit(1);
  }
}

void Buffer::free() { ::free(this->addr); }

void Buffer::clear() {
  memset(this->addr, 0, this->size);
  this->write_pos = this->addr;
  this->read_pos = this->addr;
}
