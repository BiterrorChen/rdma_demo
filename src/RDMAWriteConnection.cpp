/*************************************************************************
	> File Name: RDMAWriteConnection.cpp
	> Author: chenchao
	> Mail: cqwzchenchao@163.com
	> Created Time: Thu 21 Dec 2017 01:49:06 PM CST
 ************************************************************************/

#include "RDMAWriteConnection.h"
#include <thread>

RDMAWriteConnection::RDMAWriteConnection(RDMAWriteImmSocket *client_socket):is_runing_(true),
  client_socket_(client_socket){
  send_thr_ = std::thread(&RDMAWriteConnection::SendThr, this);
}

void RDMAWriteConnection::SendThr(){
  while( 1 ){
    std::unique_lock<std::mutex> lk(mtx_);
    if(!is_runing_){
      return;
    }
    while(buffers_.empty() && buffer_higher_.empty()){
      cv_.wait(lk);
      if(!is_runing_){
        return;
      }
    }
    while(!buffer_higher_.empty()){
      std::string str = buffers_.front();
      buffers_.pop_front();
      DoSend(str);
    }
    while(!buffers_.empty()){
      std::string str = buffers_.front();
      buffers_.pop_front();
      DoSend(str);
    }
  }
}

void RDMAWriteConnection::DoSend(std::string &str){
  MessageHeader header(MessageType::NORMAL, str.size() + 1);
  client_socket_->send_msg(header, (char *)str.c_str());
  //std::cout << "send size : " << header.body_size << std::endl;

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
  
  cv_.notify_all();
}

void RDMAWriteConnection::SendClose(){
  client_socket_->send_close();
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
