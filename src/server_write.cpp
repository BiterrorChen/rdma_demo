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

void thr(RDMAWriteImmSocket *clientSocket) {
  while(1) {
    MessageHeader header;
    clientSocket->recv_header(&header);
    if (header.req_type == MessageType::CLOSE) {
      delete clientSocket;
      return;
    }
    std::cout << "recv new message:";
    char *message = clientSocket->get_body(header.body_size);
    std::cout << std::string(message) << std::endl;
    clientSocket->clear_msg_buf();

    header.req_type = MessageType::NORMAL;
    std::string str = "server echo.";
    header.body_size = str.size() + 1;
    clientSocket->send_msg(header, (char *)str.c_str());
  }
}

int main(int argc, char *argv[]) {
  try {
    RDMAWriteImmServerSocket serverSocket(argv[1]);

    while (1) {
      RDMAWriteImmSocket *clientSocket = serverSocket.accept();
      std::thread(thr, clientSocket).detach();
    }
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  return 0;
}
