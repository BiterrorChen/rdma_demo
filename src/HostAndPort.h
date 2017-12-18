#pragma once

#include <string.h>
#include <unistd.h>
#include <boost/functional/hash.hpp>

class HostAndPort {
 public:
  char hostname[64];
  char port_str[8];

  HostAndPort() = default;
  HostAndPort(char *hostname, char *port_str);
  bool operator==(const HostAndPort &other) const;
};
