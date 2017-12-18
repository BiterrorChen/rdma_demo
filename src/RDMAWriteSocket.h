#pragma once

#include "MessageHeader.h"
#include "MessageType.h"
#include "RDMACMSocket.h"

class RDMAWriteSocket {
 private:
  void setup_write_buf();

 public:
  RDMACMSocket *rsock;
  struct ibv_mr *read_mr;
  Buffer write_buf;
  struct ibv_mr *write_mr;
  RemoteKeyAndAddr rka;

  RDMAWriteSocket(RDMACMSocket *rsock);
  ~RDMAWriteSocket();
  static RDMAWriteSocket *connect(const HostAndPort &hp);

  // void setup_read_buf();
  // int read(Item* item, RemoteKeyAndAddr rka);

  void send_msg(MessageHeader header, char *body);
  void recv_header(MessageHeader *header);
  char *get_body(size_t body_size);
  void clear_msg_buf();
  void send_close();
};
