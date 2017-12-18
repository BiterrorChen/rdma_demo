#include "RDMAWriteSocket.h"

RDMAWriteSocket::RDMAWriteSocket(RDMACMSocket *rsock) {
  this->rsock = rsock;
  this->write_mr = NULL;
  setup_write_buf();

  // send rkey and addr
  Buffer send_buf = this->rsock->get_send_buf();
  RemoteKeyAndAddr rka(this->write_mr->rkey, this->write_buf.addr);
  send_buf.write(rka);
  this->rsock->post_send(send_buf);

  // receive rkey and addr
  Buffer recv_buf = this->rsock->get_recv_buf();
  recv_buf.read(&this->rka);
  this->rsock->post_recv(recv_buf);
}

RDMAWriteSocket::~RDMAWriteSocket() {
  delete this->rsock;
  rdma_dereg_mr(this->write_mr);
  this->write_buf.free();
}

RDMAWriteSocket *RDMAWriteSocket::connect(const HostAndPort &hp) {
  return new RDMAWriteSocket(RDMACMSocket::connect(hp));
}

void RDMAWriteSocket::setup_write_buf() {
  this->write_buf = Buffer::allocate(PACKET_SIZE);

  this->write_mr = rdma_reg_write(this->rsock->client_id, this->write_buf.addr,
                                  this->write_buf.size);
  if (this->write_mr == NULL) {
    delete this->rsock;
    this->write_buf.free();
    perror("rdma_reg_write");
    exit(1);
  }
}

// void RDMAWriteSocket::setup_read_buf() {  // only called on server side

// Config& config = Config::get_config();

// this->read_mr = rdma_reg_read(this->rsock->client_id, config.cache_addr,
// config.cache_size);
// if (this->read_mr == NULL) {
// delete this->rsock;
// perror("rdma_reg_read");
// exit(1);
//}
//}

// int RDMAWriteSocket::read(Item* item, RemoteKeyAndAddr rka) {
// Buffer read_buf = this->rsock->get_send_buf();

// this->rsock->post_read(read_buf, rka);

//// read invalid flag + MessageHeader
// int is_invalid;
// MessageHeader header;
// volatile int* is_arrived =
//(int*)(read_buf.addr + sizeof(int) + offsetof(MessageHeader, is_arrived));
// while (true) {
// if (*is_arrived != 0) {
// read_buf.read(&is_invalid).read(&header);
// break;
//}
//}

// if (is_invalid != 0) {
// return -1;
//}

//// read body + is_arrived flag
// char* body;
// is_arrived = (int*)(read_buf.addr + sizeof(int) + sizeof(MessageHeader) +
// header.body_size);
// while (true) {
// if (*is_arrived != 0) {
// body = read_buf.addr + sizeof(int) + sizeof(MessageHeader);
// break;
//}
//}

// try {
//// deserialize
// msgpack::unpacked res;
// msgpack::unpack(&res, body, header.body_size);
// msgpack::object obj = res.get();
// obj.convert(item);
//} catch (msgpack::v1::insufficient_bytes e) {
// return -1;
//} catch (msgpack::v1::type_error e) {
// return -1;
//}

// return 0;
//}

void RDMAWriteSocket::send_msg(MessageHeader header, char *body) {
  Buffer send_buf = this->rsock->get_send_buf();
  int is_arrived = 0xffffffff;
  send_buf.write(header).write(body, header.body_size).write(is_arrived);
  this->rsock->post_write(send_buf, this->rka);
}

void RDMAWriteSocket::recv_header(MessageHeader *header) {
  // Config& config = Config::get_config();

  volatile int *is_arrived =
      (int *)(this->write_buf.addr + offsetof(MessageHeader, is_arrived));
  while (true) {
    if (*is_arrived != 0) {
      this->write_buf.read(header);
      return;
    }
  }
}

char *RDMAWriteSocket::get_body(size_t body_size) {
  // Config& config = Config::get_config();

  volatile int *is_arrived =
      (int *)(this->write_buf.addr + sizeof(MessageHeader) + body_size);
  while (true) {
    if (*is_arrived != 0) {
      return (this->write_buf.addr + sizeof(MessageHeader));
    }
  }
}

void RDMAWriteSocket::clear_msg_buf() { this->write_buf.clear(); }

void RDMAWriteSocket::send_close() {
  Buffer send_buf = this->rsock->get_send_buf();

  // clear send cq
  struct ibv_wc wc[PACKET_WINDOW_SIZE];
  // this->rsock->poll_send_cq(PACKET_WINDOW_SIZE, wc);

  //int ret =
      //ibv_poll_cq(this->rsock->client_id->send_cq, PACKET_WINDOW_SIZE, wc);
  //if (ret < 0) {
    //perror("ibv_poll_cq");
    //exit(1);
  //}

  // send close msg
  MessageHeader header(MessageType::CLOSE, 0);
  int is_arrived = 0xffffffff;
  send_buf.write(header).write(is_arrived);
  this->rsock->post_write(send_buf, this->rka);

  // check send
  //struct ibv_wc close_wc;
  int size = PACKET_WINDOW_SIZE - this->rsock->get_current_buffer_size();
  this->rsock->poll_send_cq(size, wc);
}
