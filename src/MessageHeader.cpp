#include "MessageHeader.h"

MessageHeader::MessageHeader(MessageType req_type, size_t body_size) {
  this->req_type = req_type;
  this->body_size = body_size;
  this->is_arrived = 0xffffffff;
}
