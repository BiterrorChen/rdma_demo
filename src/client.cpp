#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "HostAndPort.h"
#include "MessageHeader.h"
#include "RDMASendConnection.h"
#include "RDMAWriteImmSocket.h"

using std::string;

uint64_t getnsecs(const struct timespec &in) {
  return in.tv_sec * 1000000000LL + in.tv_nsec;
}

int main(int argc, char *argv[]) {
  try {
    HostAndPort host_port(argv[1], argv[2]);
    RDMACMSocket *clientSocket = RDMACMSocket::connect(host_port);
    RDMASendConnection connect(clientSocket);

    struct timespec nbegin;
    struct timespec nend;
    const int count = atoi(argv[3]);
    clock_gettime(CLOCK_REALTIME, &nbegin);
    for (int i = 0; i < count; ++i) {
      string str = "Hello World!";
      connect.SendMsg(str, 1);
    }
    clock_gettime(CLOCK_REALTIME, &nend);
    const uint64_t nsecs = getnsecs(nend) - getnsecs(nbegin);
    std::cout << "wrote " << count << " in " << nsecs << " nsecs" << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  return 0;
}
