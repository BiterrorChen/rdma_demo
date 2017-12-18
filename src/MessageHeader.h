#pragma once

#include <stddef.h>
#include "MessageType.h"

class MessageHeader {
 public:
  MessageType req_type;
  size_t body_size;
  int is_arrived;  // only used by RDMA Write

  MessageHeader() = default;
  MessageHeader(MessageType req_type, size_t body_size);
};
