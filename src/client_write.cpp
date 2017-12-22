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
#include <glog/logging.h>
#include "RDMAWriteConnection.h"

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  HostAndPort host_port(argv[1], argv[2]);
  RDMAWriteImmSocket *clientSocket = RDMAWriteImmSocket::connect(host_port);
  RDMAWriteConnection connect(clientSocket);

  const int count = atoi(argv[3]);

  for (int i = 0; i < count; ++i){
    //std::string str = "Hello World!";
    std::string str(996, 'a');
    connect.SendMsg(str, 0);
  }
  connect.SendClose();
  google::ShutdownGoogleLogging();
  return 0;
}
