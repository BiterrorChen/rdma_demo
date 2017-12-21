/*************************************************************************
	> File Name: server_write.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Mon 18 Dec 2017 04:12:28 PM CST
 ************************************************************************/


#include <stdlib.h>
#include <iostream>
#include <thread>
#include "RDMAWriteImmSocket.h"
#include "RDMAWriteImmServerSocket.h"
#include <glog/logging.h>
#include "RDMAWriteConnection.h"

void thr(RDMAWriteImmSocket *clientSocket) {
  RDMAWriteConnection connection(clientSocket);
  while(1) {
    int size = 0;
    char *buffer = NULL;
    connection.GetMessage(size, buffer);
    if (size == -1){
      return;
    }
    std::cout << "recv message:" << std::string(buffer) << std::endl;
  }
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  try {
    RDMAWriteImmServerSocket serverSocket(argv[1]);

    while (1) {
      RDMAWriteImmSocket *clientSocket = serverSocket.accept();
      std::thread(thr, clientSocket).detach();
    }
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  google::ShutdownGoogleLogging();
  return 0;
}
