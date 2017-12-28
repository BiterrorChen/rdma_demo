/*************************************************************************
	> File Name: RDMASendConnection.h
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Tue 26 Dec 2017 08:45:13 PM CST
 ************************************************************************/

#ifndef RDMASENDCONNECTION_H_
#define RDMASENDCONNECTION_H_

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

class RDMASendConnection {
public:
  void SendMsg(std::string&, int level);
  void SendClose();
  void GetMessage(int &size, char *&buffer);
  RDMASendConnection(RDMACMSocket *clientSocket);
  ~RDMASendConnection(){
    mtx_.lock();
    status_ = Status::Ending;
    cv_.notify_all();
    mtx_.unlock();
    send_thr_.join();

    delete client_socket_;
  }

private:
  Status status_;
  RDMACMSocket *client_socket_;
  std::list<std::string> buffers_ ;
  std::list<std::string> buffer_higher_;
  std::thread send_thr_;

  std::condition_variable cv_;
  std::mutex mtx_;

  void SendThr();
  void DoSend(std::string &);
};

#endif
