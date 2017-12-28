#pragma once

#include <rdma/rdma_cma.h>
#include <rdma/rdma_verbs.h>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include "Buffer.h"
#include "HostAndPort.h"
#include "RemoteKeyAndAddr.h"
#include "common.h"
#include "MessageHeader.h"

#define VERBS_BUF_SIZE (PACKET_SIZE * PACKET_WINDOW_SIZE * 2)

class RDMACMSocket : boost::noncopyable {
 private:
  void setup_verbs_buf();

 public:
  struct rdma_cm_id *client_id;
  Buffer verbs_buf;
  struct ibv_mr *verbs_mr;
  boost::circular_buffer<Buffer> send_bufs;

  RDMACMSocket(struct rdma_cm_id *client_id);
  ~RDMACMSocket();
  static RDMACMSocket *connect(const HostAndPort &hp);
  int get_current_buffer_size() {
    return send_bufs.size();
  }

  int poll_send_cq(int num_entries, struct ibv_wc *wc);
  Buffer get_send_buf();
  void post_send(const Buffer &buf);

  int poll_recv_cq(int num_entries, struct ibv_wc *wc);
  Buffer get_recv_buf();
  void post_recv(const Buffer &buf);

  void post_write(const Buffer &buf, const RemoteKeyAndAddr &rka);
  void post_write_imm(const Buffer &buf, const RemoteKeyAndAddr &rka);
  void post_read(const Buffer &buf, const RemoteKeyAndAddr &rka);

  void send_msg_send(MessageHeader header, char *body);
  Buffer recv_msg_send();
  void clear_msg(Buffer buf);
  void send_close_send();
};

static inline int rdma_post_write_imm(struct rdma_cm_id *id, void *context,
                                      void *addr, size_t length,
                                      struct ibv_mr *mr, int flags,
                                      uint64_t remote_addr, uint32_t rkey) {
  struct ibv_sge sge;

  sge.addr = (uint64_t)(uintptr_t)addr;
  sge.length = (uint32_t)length;
  //sge.length = 5;
  sge.lkey = mr ? mr->lkey : 0;

  struct ibv_send_wr wr, *bad;

  wr.wr_id = (uintptr_t)context;
  wr.next = NULL;
  wr.sg_list = &sge;
  wr.num_sge = 1;
  wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  wr.send_flags = flags;
  wr.wr.rdma.remote_addr = remote_addr;
  wr.wr.rdma.rkey = rkey;
  wr.imm_data = 1855;

  return rdma_seterrno(ibv_post_send(id->qp, &wr, &bad));
}
