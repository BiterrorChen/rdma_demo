#pragma once

#include <stdint.h>

class RemoteKeyAndAddr {
 public:
  uint32_t rkey;
  uint64_t remote_addr;

  RemoteKeyAndAddr() = default;
  RemoteKeyAndAddr(uint32_t rkey, uint64_t remote_addr) {
    this->rkey = rkey;
    this->remote_addr = remote_addr;
  }

  RemoteKeyAndAddr(uint32_t rkey, void *remote_addr) {
    this->rkey = rkey;
    this->remote_addr = (uint64_t)(uintptr_t)(remote_addr);
  }
};
