#include "HostAndPort.h"

HostAndPort::HostAndPort(char *hostname, char *port_str) {
  strncpy(this->hostname, hostname, sizeof(this->hostname));
  strncpy(this->port_str, port_str, sizeof(this->port_str));
}

bool HostAndPort::operator==(const HostAndPort &other) const {
  return (strcmp(this->hostname, other.hostname) == 0) &&
         (strcmp(this->port_str, other.port_str) == 0);
}
