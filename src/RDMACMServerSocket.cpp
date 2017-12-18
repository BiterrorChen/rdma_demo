#include "RDMACMServerSocket.h"

RDMACMServerSocket::RDMACMServerSocket(char *port_str) {
  this->listen_id = NULL;

  struct rdma_addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = RAI_PASSIVE;
  hints.ai_port_space = RDMA_PS_TCP;
  res = NULL;

  if (rdma_getaddrinfo(NULL, port_str, &hints, &res) < 0) {
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

  if (rdma_create_ep(&this->listen_id, res, NULL, &attr) < 0) {
    rdma_freeaddrinfo(res);
    perror("rdma_create_ep");
    exit(1);
  }

  rdma_freeaddrinfo(res);

  if (rdma_listen(this->listen_id, LISTEN_MAX) < 0) {
    rdma_destroy_ep(this->listen_id);
    perror("rdma_listen");
    exit(1);
  }
}

RDMACMServerSocket::~RDMACMServerSocket() { rdma_destroy_ep(this->listen_id); }

RDMACMSocket *RDMACMServerSocket::accept() {
  struct rdma_cm_id *client_id = NULL;
  if (rdma_get_request(this->listen_id, &client_id) < 0) {
    perror("rdma_get_request");
    exit(1);
  }

  if (rdma_accept(client_id, NULL) < 0) {
    perror("rdma_accept");
    exit(1);
  }
  std::cout << "accept success!" << std::endl;

  return new RDMACMSocket(client_id);
}
