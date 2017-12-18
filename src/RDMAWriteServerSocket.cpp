#include "RDMAWriteServerSocket.h"

RDMAWriteSocket *RDMAWriteServerSocket::accept() {
  return new RDMAWriteSocket(this->rsock.accept());
}
