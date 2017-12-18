#include <stdlib.h>
#include <iostream>
#include <thread>
#include "RDMACMServerSocket.h"
#include "RDMACMSocket.h"

void thr(RDMACMSocket *clientSocket) {
  std::cout << "connect success" << std::endl;
  try {
    while (1) {
      Buffer readPacket = clientSocket->get_recv_buf();
      Buffer sendPacket = clientSocket->get_send_buf();
      memcpy(sendPacket.addr, readPacket.addr, readPacket.size);
      clientSocket->post_send(sendPacket);
      clientSocket->post_recv(readPacket);
    }
  } catch (std::exception &e) {
    std::cerr << "client exception: " << e.what() << std::endl;
  }
  delete clientSocket;
}

int main(int argc, char *argv[]) {
  try {
    RDMACMServerSocket serverSocket(argv[1]);

    while (1) {
      RDMACMSocket *clientSocket = serverSocket.accept();
      std::thread(thr, clientSocket).detach();
    }
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  return 0;
}
