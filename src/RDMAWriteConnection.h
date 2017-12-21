/*************************************************************************
	> File Name: RDMAWriteConnection.h
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Thu 21 Dec 2017 01:45:54 PM CST
 ************************************************************************/

#ifndef RDMAWRITECONNECTION_H_
#define RDMAWRITECONNECTION_H_

#include <list>
#include "RDMAWriteImmSocket.h"
#include "Buffer.h"
#include <string>
#include <condition_variable>
#include <mutex>
#include <thread>

class RDMAWriteConnection {
public:
  void SendMsg(std::string&, int level);
  void SendClose();
  void GetMessage(int &size, char *&buffer);
  int GetUnSendSize(){
    std::unique_lock<std::mutex> lk(mtx_);
    return buffers_.size() + buffer_higher_.size();
  }
  RDMAWriteConnection(RDMAWriteImmSocket *clientSocket);
  ~RDMAWriteConnection(){
    std::unique_lock<std::mutex> lk(mtx_);
    is_runing_ = false;
    cv_.notify_all();
    send_thr_.join();

    delete client_socket_;
  }

private:
  bool is_runing_;
  RDMAWriteImmSocket *client_socket_;
  std::list<std::string> buffers_ ;
  std::list<std::string> buffer_higher_;
  std::thread send_thr_;

  std::condition_variable cv_;
  std::mutex mtx_;

  void SendThr();
  void DoSend(std::string &);
};

#endif
