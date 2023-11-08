#include "io/udp/udp_socket.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

namespace keylogger {

UdpSocket::~UdpSocket() {
  if (sock_info_.fd) {
    close(sock_info_.fd);
  }
}

void UdpSocket::SetupSender(const std::string& addr, int port, int flags) {
  if (!IsValidIpv4Address(addr)) {
    throw std::invalid_argument("invalid IP addr -> " + addr);
  }

  if (port <= 0) {
    throw std::invalid_argument("port must be a positive integer");
  }

  if (flags < 0) {
    throw std::invalid_argument(
        "flags must be 0 or the bitwise OR of one or more socket option flags");
  }

  /* File descriptor initalization. */
  sock_info_.port = port;
  sock_info_.fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_info_.fd < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Initialize the remote target's sockaddr_in info. */
  sock_info_.remote.sin_family = AF_INET;
  sock_info_.remote.sin_port = ::htons(static_cast<uint16_t>(port));
  int status = ::inet_aton(addr.c_str(), &sock_info_.remote.sin_addr);
  if (!status) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Set the socket to be non blocking. */
  int sock_flags = fcntl(sock_info_.fd, F_GETFL);
  status = fcntl(sock_info_.fd, F_SETFL, sock_flags | flags);
  if (status < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = ::htons(0);
  sock_info_.local.sin_addr.s_addr = ::htonl(INADDR_ANY);
}

void UdpSocket::SetupReceiver(int port, int flags) {
  if (port <= 0) {
    throw std::invalid_argument("port must be a positive integer");
  }

  if (flags < 0) {
    throw std::invalid_argument(
        "flags must be 0 or the bitwise OR of one or more socket option flags");
  }

  /* File descriptor initalization. */
  sock_info_.port = port;
  sock_info_.fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_info_.fd < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* See https://lwn.net/Articles/542629/ for reasoning behind use of
   * SO_REUSEPORT. */
  int on = 1;
  int status =
      ::setsockopt(sock_info_.fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
  if (status < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Set the socket to be non blocking. */
  int sock_flags = ::fcntl(sock_info_.fd, F_GETFL);
  status = ::fcntl(sock_info_.fd, F_SETFL, sock_flags | flags);
  if (status < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = ::htons(static_cast<uint16_t>(port));
  sock_info_.local.sin_addr.s_addr = ::htonl(INADDR_ANY);

  /* Bind us to listen for incoming datagrams. */
  status = bind(sock_info_.fd, reinterpret_cast<::sockaddr*>(&sock_info_.local),
                sizeof(sock_info_.local));
  if (status < 0) {
    throw std::runtime_error(std::strerror(errno));
  }
}

bool UdpSocket::IsValidIpv4Address(const std::string& addr) const {
  ::sockaddr_in sa;
  return (0 != ::inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr)));
}

}  // namespace keylogger
