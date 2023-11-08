#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace keylogger {

/*!
 * \class UdpSocket
 * \brief The UdpSocket class is a cross platform wrapper around a UDP socket.
 */
class UdpSocket {
 public:
  /*!
   * \brief Open a sender socket on addr:port.
   * \details Open() will open a UDP socket for sending on address \a addr and
   *          port \a port. If Open() fails, errno will be set to indicate
   *          the error.
   * \param addr  IPv4 address of remote machine.
   * \param port  The port at which the remote machine is listening.
   * \param flags Bitwise OR of zero or more option flags.
   * \throws std::invalid_argument When \p addr, \p port, or \p flags have
   *                               an invalid format.
   * \throws std::runtime_error When the sender socket cannot be initialized
   *                            successfully.
   */
  [[nodiscard]] UdpSocket(const std::string& addr, int port, int flags = 0) {
    SetupSender(addr, port, flags);
  }

  /*!
   * \brief Open a receiver socket on \a port.
   * \details Open() will open a UDP socket on the host for receiving incoming
   *          datagrams on port \a port. If Open() fails, errno will be set
   *          to indicate the error.
   * \param port  Port number at which to listen for incoming datagrams.
   * \param flags Bitwise OR of zero or more option flags.
   * \throws std::invalid_argument When \p port or \p flags have an invalid
   *                               format.
   * \throws std::runtime_error When the receiver socket cannot be initialized
   *                            successfully.
   */
  [[nodiscard]] explicit UdpSocket(int port, int flags = 0) {
    SetupReceiver(port, flags);
  }

  /*!
   * \brief Cleanup socket resources.
   */
  ~UdpSocket();

  UdpSocket() = delete;
  UdpSocket(const UdpSocket&) = default;
  UdpSocket& operator=(const UdpSocket&) = default;
  UdpSocket(UdpSocket&&) = default;
  UdpSocket& operator=(UdpSocket&&) = default;

  /*!
   * \brief Return this socket's port number.
   */
  [[nodiscard]] int GetPort() const { return sock_info_.port; }

  /*!
   * \brief Return this socket's IP address.
   */
  [[nodiscard]] std::string GetAddress() const {
    return ::inet_ntoa(sock_info_.remote.sin_addr);
  }

  /*!
   * \brief Send \a len bytes out on the socket.
   *
   * \return The number of bytes sent, -1 on error.
   */
  [[nodiscard]] int Send(void* buf, size_t len) {
    return ::sendto(sock_info_.fd, buf, len, 0, (::sockaddr*)&sock_info_.remote,
                    sizeof(sock_info_.remote));
  }

  /*!
   * \brief Receive \a len bytes and store them in \a buf.
   * \return The number of bytes received, -1 on error.
   */
  [[nodiscard]] int Recv(void* buf, size_t len) {
    return ::recv(sock_info_.fd, buf, len, 0);
  }

 private:
  struct SocketInfo {
    int fd;                      /*!< Socket file descriptor. */
    int port;                    /*!< Target port number. */
    struct ::sockaddr_in local;  /*!< Local IPv4 address. */
    struct ::sockaddr_in remote; /*!< Remote IPv4 address. */
  };                             // end SocketInfo

  /*!
   * \brief Initialize a sender socket.
   * \throws std::runtime_error When the sender socket cannot be initialized
   *                            successfully.
   */
  void SetupSender(const std::string& addr, int port, int flags);

  /*!
   * \brief Initialize a receiver socket.
   * \throws std::runtime_error When the receiver socket cannot be initialized
   *                            successfully.
   */
  void SetupReceiver(int port, int flags);

  /*!
   * \brief Return \c true is \p addr is a valid IPv4 address.
   */
  bool IsValidIpv4Address(const std::string& addr) const;

  SocketInfo sock_info_; /*!< Socket data. */
};                       // end UdpSocket

}  // namespace keylogger

#endif
