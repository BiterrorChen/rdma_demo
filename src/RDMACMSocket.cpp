#include "RDMACMSocket.h"
#include <glog/logging.h>

RDMACMSocket::RDMACMSocket(struct rdma_cm_id *client_id)
    : send_bufs(PACKET_WINDOW_SIZE) {
  this->client_id = client_id;
  this->verbs_mr = NULL;
  setup_verbs_buf();
}

RDMACMSocket::~RDMACMSocket() {
  rdma_disconnect(this->client_id);
  rdma_dereg_mr(this->verbs_mr);
  rdma_destroy_ep(this->client_id);
  this->verbs_buf.free();
}

RDMACMSocket *RDMACMSocket::connect(const HostAndPort &hp) {
  struct rdma_cm_id *client_id = NULL;

  struct rdma_addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_port_space = RDMA_PS_TCP;
  res = NULL;

  char *hostname = const_cast<char *>(hp.hostname);
  char *port_str = const_cast<char *>(hp.port_str);

  if (rdma_getaddrinfo(hostname, port_str, &hints, &res) < 0) {
    perror("rdma_getaddrinfo");
    exit(1);
  }

  struct ibv_qp_init_attr attr;
  memset(&attr, 0, sizeof(attr));
  attr.cap.max_send_wr = PACKET_WINDOW_SIZE;
  attr.cap.max_recv_wr = PACKET_WINDOW_SIZE;
  attr.cap.max_send_sge = 1;
  attr.cap.max_recv_sge = 1;
  attr.cap.max_inline_data = 0;
  attr.sq_sig_all = 1;

  if (rdma_create_ep(&client_id, res, NULL, &attr) < 0) {
    rdma_freeaddrinfo(res);
    perror("rdma_create_ep");
    exit(1);
  }

  rdma_freeaddrinfo(res);

  if (rdma_connect(client_id, NULL) < 0) {
    rdma_destroy_ep(client_id);
    perror("rdma_connect");
    exit(1);
  }

  return new RDMACMSocket(client_id);
}

void RDMACMSocket::setup_verbs_buf() {
  this->verbs_buf = Buffer::allocate(PACKET_SIZE * PACKET_WINDOW_SIZE * 2);

  this->verbs_mr = rdma_reg_msgs(this->client_id, this->verbs_buf.addr,
                                 this->verbs_buf.size);
  if (this->verbs_mr == NULL) {
    this->verbs_buf.free();
    rdma_destroy_ep(this->client_id);
    perror("rdma_reg_msgs");
    exit(1);
  }

  char *send_buf_begin =
      this->verbs_buf.addr + PACKET_SIZE * PACKET_WINDOW_SIZE;
  for (int i = 0; i < PACKET_WINDOW_SIZE; ++i) {
    Buffer recv_buf(this->verbs_buf.addr + i * PACKET_SIZE, PACKET_SIZE);
    post_recv(recv_buf);
    Buffer send_buf(send_buf_begin + i * PACKET_SIZE, PACKET_SIZE);
    send_bufs.push_back(send_buf);
  }
}

int RDMACMSocket::poll_send_cq(int num_entries, struct ibv_wc *wc) {
  int ret;

  while (!(ret = ibv_poll_cq(this->client_id->send_cq, num_entries, wc)))
    ;
  if (ret < 0) {
    perror("ibv_poll_cq");
    exit(1);
  }

  return ret;
}

Buffer RDMACMSocket::get_send_buf() {
  if (send_bufs.empty()) {
    struct ibv_wc wc[PACKET_WINDOW_SIZE];
    int ret = poll_send_cq(PACKET_WINDOW_SIZE, wc);

    for (int i = 1; i < ret; ++i) {
      send_bufs.push_back(
          Buffer(reinterpret_cast<char *>(wc[i].wr_id), PACKET_SIZE));
    }

    Buffer buf(reinterpret_cast<char *>(wc[0].wr_id), PACKET_SIZE);
    buf.clear();
    return buf;
  }

  Buffer ret = send_bufs.front();
  send_bufs.pop_front();
  ret.clear();
  return ret;
}

void RDMACMSocket::post_send(const Buffer &buf) {
  if (rdma_post_send(this->client_id, buf.addr, buf.addr, buf.size,
                     this->verbs_mr, 0) < 0) {
    rdma_dereg_mr(this->verbs_mr);
    this->verbs_buf.free();
    rdma_destroy_ep(this->client_id);
    perror("rdma_post_send");
    exit(1);
  }
}

int RDMACMSocket::poll_recv_cq(int num_entries, struct ibv_wc *wc) {
  int ret;
  while (!(ret = ibv_poll_cq(this->client_id->recv_cq, num_entries, wc)))
    ;
  if (ret < 0) {
    perror("ibv_poll_cq");
    exit(1);
  }

  return ret;
}

Buffer RDMACMSocket::get_recv_buf() {
  struct ibv_wc wc;
  poll_recv_cq(1, &wc);
  if (wc.status != IBV_WC_SUCCESS) {
    LOG(WARNING) << "RDMACMSocket::get_recv_buf error status:" << wc.status;  
  }
  return Buffer(reinterpret_cast<char *>(wc.wr_id), wc.byte_len);
}

void RDMACMSocket::post_recv(const Buffer &buf) {
  if (rdma_post_recv(this->client_id, buf.addr, buf.addr, buf.size,
                     this->verbs_mr) < 0) {
    rdma_dereg_mr(this->verbs_mr);
    this->verbs_buf.free();
    rdma_destroy_ep(this->client_id);
    perror("rdma_post_recv");
    exit(1);
  }
}

void RDMACMSocket::post_write(const Buffer &buf, const RemoteKeyAndAddr &rka) {
  uint32_t rkey = rka.rkey;
  uint64_t raddr = rka.remote_addr;

  if (rdma_post_write(this->client_id, buf.addr, buf.addr, buf.GetCurrentSize(),
                      this->verbs_mr, 0, raddr, rkey) < 0) {
    this->send_bufs.push_front(buf);
    perror("rdma_post_write");
    exit(1);
  }
}

void RDMACMSocket::post_write_imm(const Buffer &buf,
                                  const RemoteKeyAndAddr &rka) {
  uint32_t rkey = rka.rkey;
  uint64_t raddr = rka.remote_addr;

  if (rdma_post_write_imm(this->client_id, buf.addr, buf.addr, buf.GetCurrentSize(),
                          this->verbs_mr, 0, raddr, rkey) < 0) {
    this->send_bufs.push_front(buf);
    perror("rdma_post_write_imm");
    exit(1);
  }
}

void RDMACMSocket::post_read(const Buffer &buf, const RemoteKeyAndAddr &rka) {
  uint32_t rkey = rka.rkey;
  uint64_t raddr = rka.remote_addr;

  if (rdma_post_read(this->client_id, buf.addr, buf.addr, buf.GetCurrentSize(),
                     this->verbs_mr, 0, raddr, rkey) < 0) {
    perror("rdma_post_read");
    exit(1);
  }
}
