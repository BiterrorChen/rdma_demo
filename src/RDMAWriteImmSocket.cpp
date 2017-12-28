#include "RDMAWriteImmSocket.h"

RDMAWriteImmSocket *RDMAWriteImmSocket::connect(const HostAndPort &hp) {
  return new RDMAWriteImmSocket(RDMACMSocket::connect(hp));
}

void RDMAWriteImmSocket::send_msg(MessageHeader header, char *body) {
  Buffer send_buf = this->rsock->get_send_buf();
  int is_arrived = 0xffffffff;
  send_buf.write(header).write(body, header.body_size).write(is_arrived);
  this->rsock->post_write_imm(send_buf, this->rka);
}

void RDMAWriteImmSocket::recv_header(MessageHeader *header) {
  this->msg_buf = this->rsock->get_recv_buf();
  this->write_buf.read(header);
}

char *RDMAWriteImmSocket::get_body(size_t body_size) {
  return (this->write_buf.addr + sizeof(MessageHeader));
}

void RDMAWriteImmSocket::clear_msg_buf() {
  this->write_buf.clear();
  this->rsock->post_recv(this->msg_buf);
}

void RDMAWriteImmSocket::send_close() {
  Buffer send_buf = this->rsock->get_send_buf();

  // clear send cq
  struct ibv_wc wc[PACKET_WINDOW_SIZE];
  //int ret;
  //ret = ibv_poll_cq(this->rsock->client_id->send_cq, PACKET_WINDOW_SIZE, wc);
  //if (ret < 0) {
    //perror("ibv_poll_cq");
    //exit(1);
  //}

  // send close msg
  MessageHeader header(MessageType::CLOSE, 0);
  int is_arrived = 0xffffffff;
  send_buf.write(header).write(is_arrived);
  this->rsock->post_write_imm(send_buf, this->rka);

  // check send
  //struct ibv_wc close_wc;
  int size = PACKET_WINDOW_SIZE - this->rsock->get_current_buffer_size();
  this->rsock->poll_send_cq(size, wc);
}

void RDMAWriteImmSocket::send_msg_send(MessageHeader header, char *body){
  Buffer sendPacket = rsock->get_send_buf();
  sendPacket.write(header).write(body, header.body_size);
  rsock->post_send(sendPacket);
}

Buffer RDMAWriteImmSocket::recv_msg_send(){
  return rsock->get_recv_buf();
}

void RDMAWriteImmSocket::clear_msg(Buffer buf){
  buf.clear();
  rsock->post_recv(buf);
}

void RDMAWriteImmSocket::send_close_send(){
  Buffer send_buf = this->rsock->get_send_buf();

  // clear send cq
  struct ibv_wc wc[PACKET_WINDOW_SIZE];
  //int ret;
  //ret = ibv_poll_cq(this->rsock->client_id->send_cq, PACKET_WINDOW_SIZE, wc);
  //if (ret < 0) {
    //perror("ibv_poll_cq");
    //exit(1);
  //}

  // send close msg
  MessageHeader header(MessageType::CLOSE, 0);
  int is_arrived = 0xffffffff;
  send_buf.write(header).write(is_arrived);
  this->rsock->post_send(send_buf);

  // check send
  //struct ibv_wc close_wc;
  int size = PACKET_WINDOW_SIZE - this->rsock->get_current_buffer_size();
  this->rsock->poll_send_cq(size, wc);
}
