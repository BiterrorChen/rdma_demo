#pragma once

#include "RDMAWriteSocket.h"

class RDMAWriteImmSocket : public RDMAWriteSocket {
 private:
  Buffer msg_buf;

 public:
  RDMAWriteImmSocket(RDMACMSocket *rsock) : RDMAWriteSocket(rsock) {}
  ~RDMAWriteImmSocket() = default;
  static RDMAWriteImmSocket *connect(const HostAndPort &hp);

  void send_msg(MessageHeader header, char *body);
  void recv_header(MessageHeader *header);
  char *get_body(size_t body_size);
  void clear_msg_buf();
  void send_close();
};
