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

enum class Status{
  Begining,
  StartThread,
  Working,
  Ending
};

class RDMAWriteConnection {
public:
  void SendMsg(std::string&, int level);
  void SendClose();
  void GetMessage(int &size, char *&buffer);
  RDMAWriteConnection(RDMAWriteImmSocket *clientSocket);
  ~RDMAWriteConnection(){
    mtx_.lock();
    status_ = Status::Ending;
    cv_.notify_all();
    mtx_.unlock();
    send_thr_.join();

    delete client_socket_;
  }

private:
  Status status_;
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
