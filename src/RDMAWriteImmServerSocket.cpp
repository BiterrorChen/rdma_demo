#include "RDMAWriteImmServerSocket.h"

RDMAWriteImmSocket *RDMAWriteImmServerSocket::accept() {
  return new RDMAWriteImmSocket(this->rsock.accept());
}
