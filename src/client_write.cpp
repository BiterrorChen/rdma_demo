/*************************************************************************
	> File Name: client_write.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Mon 18 Dec 2017 04:25:20 PM CST
 ************************************************************************/

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "HostAndPort.h"
#include "RDMAWriteImmSocket.h"

int main(int argc, char **argv) {
  HostAndPort host_port(argv[1], argv[2]);
  RDMAWriteImmSocket *clientSocket = RDMAWriteImmSocket::connect(host_port);
  const int count = atoi(argv[3]);

  for (int i = 0; i < count; ++i){
    std::string str = "Hello World!";
    MessageHeader header(MessageType::NORMAL, str.size() + 1);
    clientSocket->send_msg(header, (char *)str.c_str());

    clientSocket->recv_header(&header);
    if (header.req_type == MessageType::CLOSE){
      delete clientSocket;
      return 0;
    }
    char *message = clientSocket->get_body(header.body_size);
    std::cout << "client recv :" << std::string(message) << std::endl;
  }
  pause();
  clientSocket->send_close();
  delete clientSocket;
}
