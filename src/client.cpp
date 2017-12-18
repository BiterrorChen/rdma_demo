#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "HostAndPort.h"
#include "RDMACMSocket.h"

uint64_t getnsecs(const struct timespec &in) {
  return in.tv_sec * 1000000000LL + in.tv_nsec;
}

int main(int argc, char *argv[]) {
  try {
    HostAndPort host_port(argv[1], argv[2]);
    RDMACMSocket *clientSocket = RDMACMSocket::connect(host_port);
    struct timespec nbegin;
    struct timespec nend;
    const int count = atoi(argv[3]);
    clock_gettime(CLOCK_REALTIME, &nbegin);
    for (int i = 0; i < count; ++i) {
      Buffer sendPacket = clientSocket->get_send_buf();
      memset(sendPacket.addr, 'b', sendPacket.size);
      clientSocket->post_send(sendPacket);
      Buffer readPacket = clientSocket->get_recv_buf();
      clientSocket->post_recv(readPacket);
    }
    clock_gettime(CLOCK_REALTIME, &nend);
    const uint64_t nsecs = getnsecs(nend) - getnsecs(nbegin);
    std::cout << "wrote " << count << " in " << nsecs << " nsecs" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  return 0;
}
