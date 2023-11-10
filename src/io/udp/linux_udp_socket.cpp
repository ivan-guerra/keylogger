#include <unistd.h>

#include <cstring>
#include <stdexcept>
#include <string>

#include "io/udp/udp_socket.h"

namespace keylogger {

void UdpSocket::SetupSender(const std::string& addr, int port) {
  if (!IsValidIpv4Address(addr)) {
    throw std::invalid_argument("invalid IP addr -> " + addr);
  }

  if (port <= 0) {
    throw std::invalid_argument("port must be a positive integer");
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

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = ::htons(0);
  sock_info_.local.sin_addr.s_addr = ::htonl(INADDR_ANY);
}

void UdpSocket::SetupReceiver(int port) {
  if (port <= 0) {
    throw std::invalid_argument("port must be a positive integer");
  }

  /* File descriptor initalization. */
  sock_info_.port = port;
  sock_info_.fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_info_.fd < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = ::htons(static_cast<uint16_t>(port));
  sock_info_.local.sin_addr.s_addr = ::htonl(INADDR_ANY);

  /* Bind us to listen for incoming datagrams. */
  int status =
      ::bind(sock_info_.fd, reinterpret_cast<::sockaddr*>(&sock_info_.local),
             sizeof(sock_info_.local));
  if (status < 0) {
    throw std::runtime_error(std::strerror(errno));
  }
}

bool UdpSocket::IsValidIpv4Address(const std::string& addr) const {
  ::sockaddr_in sa;
  return (0 != ::inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr)));
}

UdpSocket::UdpSocket(const std::string& addr, int port) {
  SetupSender(addr, port);
}

UdpSocket::UdpSocket(int port) { SetupReceiver(port); }

UdpSocket::~UdpSocket() {
  if (sock_info_.fd) {
    ::close(sock_info_.fd);
  }
}

int UdpSocket::Send(void* buf, size_t len) {
  int bytes_sent = ::sendto(sock_info_.fd, buf, len, 0,
                            reinterpret_cast<::sockaddr*>(&sock_info_.remote),
                            sizeof(sock_info_.remote));
  if (bytes_sent < 0) {
    throw std::runtime_error(std::strerror(errno));
  }
  return bytes_sent;
}

int UdpSocket::Recv(void* buf, size_t len) {
  int bytes_recved = ::recv(sock_info_.fd, buf, len, 0);
  if (bytes_recved < 0) {
    throw std::runtime_error(std::strerror(errno));
  }
  return bytes_recved;
}

}  // namespace keylogger
