#pragma once

#include <boost/noncopyable.hpp>
#include "RDMACMSocket.h"
#include "common.h"

class RDMACMServerSocket : boost::noncopyable {
 private:
  struct rdma_cm_id *listen_id;

 public:
  RDMACMServerSocket(char *port_str);
  ~RDMACMServerSocket();
  RDMACMSocket *accept();
};
