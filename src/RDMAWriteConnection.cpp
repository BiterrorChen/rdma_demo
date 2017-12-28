/*************************************************************************
	> File Name: RDMAWriteConnection.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Thu 21 Dec 2017 01:49:06 PM CST
 ************************************************************************/

#include "RDMAWriteConnection.h"
#include <thread>

RDMAWriteConnection::RDMAWriteConnection(RDMAWriteImmSocket *client_socket)
  :status_(Status::Begining),
   client_socket_(client_socket),
   closing_(false){
  send_thr_ = std::thread(&RDMAWriteConnection::SendThr, this);
  while(status_ != Status::StartThread);
  status_ = Status::Working;
}

void RDMAWriteConnection::SendThr(){
  std::unique_lock<std::mutex> lk(mtx_);
  std::cout << "thread run" << std::endl;
  status_ = Status::StartThread;
  while( 1 ){
    while(buffers_.empty() && buffer_higher_.empty() && closing_ == false){
      if(status_ == Status::Ending &&
          buffers_.empty() &&
          buffer_higher_.empty() &&
          closing_ == false){
        return;
      }
      cv_.wait(lk);
      std::cout << "wake!! " << std::endl;
      if(status_ == Status::Ending &&
          buffers_.empty() &&
          buffer_higher_.empty() &&
          closing_ == false){
        return;
      }
    }
    while(!buffer_higher_.empty()){
      std::string str = buffer_higher_.front();
      buffer_higher_.pop_front();
      lk.unlock();
      DoSend(str);
      lk.lock();
    }
    while(!buffers_.empty()){
      std::string str = buffers_.front();
      buffers_.pop_front();
      lk.unlock();
      DoSend(str);
      lk.lock();
    }
    while(buffer_higher_.empty() && buffers_.empty() && closing_){
      client_socket_->send_close();
      closing_ = false;
    }
  }
}

void RDMAWriteConnection::DoSend(std::string &str){
  MessageHeader header(MessageType::NORMAL, str.size() + 1);
  client_socket_->send_msg(header, (char *)str.c_str());
  std::cout << "send size : " << header.body_size << std::endl;

  client_socket_->recv_header(&header);
  if (header.req_type == MessageType::CLOSE){
    
  }
  client_socket_->clear_msg_buf();
}

void RDMAWriteConnection::SendMsg(std::string &str, int level){
  std::unique_lock<std::mutex> lk(mtx_);
  if (level == 1){
    buffer_higher_.push_back(str);
  }else {
    buffers_.push_back(str);
  }
  
  std::cout << "buffer size :" << buffer_higher_.size() + buffers_.size() << std::endl;
  cv_.notify_all();
}

void RDMAWriteConnection::SendClose(){
  std::unique_lock<std::mutex> lk(mtx_);
  closing_ = true;
  cv_.notify_one();
}

void RDMAWriteConnection::GetMessage(int &size, char *&buffer){
  MessageHeader header;
  client_socket_->recv_header(&header);
  std::cout << "get message success" << std::endl;
  if (header.req_type == MessageType::CLOSE){
    size = -1;
    return;
  }
  size = header.body_size;
  char *message = client_socket_->get_body(header.body_size);
  buffer = (char *)::malloc(size);
  ::memcpy(buffer, message, size);
  
  client_socket_->clear_msg_buf();
  std::string str("");
  MessageHeader header2(MessageType::NORMAL, str.size() + 1);
  client_socket_->send_msg(header, (char *)str.c_str());
}
