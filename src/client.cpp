/*************************************************************************
	> File Name: client.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Wed 13 Dec 2017 10:43:48 AM CST
 ************************************************************************/

#include "RdmaSocket.hpp"
#include "Configuration.hpp"

int main (int argc, char **argv){
  uint64_t mm = (uint64_t)malloc(1024);
  Configuration *config = new Configuration(argv[1]);
  RdmaSocket *socket = new RdmaSocket(1, mm, 1024, config, false, 0);
  socket->RdmaConnect(1);
  char message[20] = "Hello World!\n";
  ::memcpy((void *)mm, message, 20);
  socket->RdmaWrite(1, mm, 0, 20, socket->getNodeID(), 0);
}
