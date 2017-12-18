#pragma once

#include "RDMACMServerSocket.h"
#include "RDMAWriteImmSocket.h"

class RDMAWriteImmServerSocket{
 private:
  RDMACMServerSocket rsock;

 public:
  RDMAWriteImmServerSocket(char *port_str) : rsock(port_str) {}
  ~RDMAWriteImmServerSocket() = default;
  RDMAWriteImmSocket *accept();
};
