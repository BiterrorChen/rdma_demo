#include <stdlib.h>
#include <iostream>
#include <thread>
#include "RDMAWriteImmSocket.h"
#include "RDMAWriteImmServerSocket.h"
#include "RDMASendConnection.h"

void thr(RDMACMSocket *clientSocket) {
  std::cout << "connect success" << std::endl;
  RDMASendConnection connection(clientSocket);
  while(1) {
    int size = 0;
    char *buffer = NULL;
    connection.GetMessage(size, buffer);
    if (size == -1){
      std::cout << "close " << std::endl;
      return;
    }
    std::cout << "recv message:" << std::string(buffer) << std::endl;
    free(buffer);
  }
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
