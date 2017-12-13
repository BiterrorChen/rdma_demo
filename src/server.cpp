/*************************************************************************
	> File Name: server.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Tue 12 Dec 2017 08:57:02 PM CST
 ************************************************************************/

#include "RdmaSocket.hpp"
#include "debug.hpp"
#include "iostream"
using namespace std;

int main(int argc, char **argv){
  Configuration *config = new Configuration(argv[1]);
  void *mem = (void *)malloc(1024);
  RdmaSocket *socket = new RdmaSocket(2, (uint64_t)mem, 1024, config, true, 0);
  
  socket->RdmaListen();
  
  while(true) {
    struct ibv_wc wc[1];
    int ret = socket->PollOnce(0, 1, wc);
    if (ret <= 0){
      //cout << "poll ret error" << endl;
    }else {
      cout << "poll ret success" << endl;
      assert(wc[0].opcode == IBV_WC_RECV_RDMA_WITH_IMM);
      int NodeID = wc[0].imm_data;
      char message[20];
      ::memcpy(message, mem, 20);
      cout << "receive from NodeID: " << NodeID 
          << " message :"
          << message << endl;
      socket->RdmaReceive(NodeID, (uint64_t)mem, 0);
    }
  }
}
