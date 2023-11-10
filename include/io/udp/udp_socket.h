#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#ifdef __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#endif

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
   *          port \a port.
   * \param addr  IPv4 address of remote machine.
   * \param port  The port at which the remote machine is listening.
   * \throws std::invalid_argument When \p addr or \p port have an invalid
   *                               format.
   * \throws std::runtime_error When the sender socket cannot be initialized
   *                            successfully.
   */
  [[nodiscard]] UdpSocket(const std::string& addr, int port);

  /*!
   * \brief Open a receiver socket on \a port.
   * \details Open() will open a UDP socket on the host for receiving incoming
   *          datagrams on port \a port.
   * \param port  Port number at which to listen for incoming datagrams.
   * \throws std::invalid_argument When \p port has an invalid format.
   * \throws std::runtime_error When the receiver socket cannot be initialized
   *                            successfully.
   */
  [[nodiscard]] explicit UdpSocket(int port);

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
   * \throws std::runtime_error When the OS encounters and error sending the
   *                            bytes.
   * \return The number of bytes sent.
   */
  [[nodiscard]] int Send(void* buf, size_t len);

  /*!
   * \brief Receive \a len bytes and store them in \a buf.
   * \throws std::runtime_error When the OS encounters and error receiving the
   *                            bytes.
   * \return The number of bytes received.
   */
  [[nodiscard]] int Recv(void* buf, size_t len);

 private:
  struct SocketInfo {
#ifdef __linux__
    int fd;
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    SOCKET fd;
#endif
    int port;
    struct ::sockaddr_in local;
    struct ::sockaddr_in remote;
  };  // end SocketInfo

  void SetupSender(const std::string& addr, int port);
  void SetupReceiver(int port);
  [[nodiscard]] bool IsValidIpv4Address(const std::string& addr) const;

  SocketInfo sock_info_;
};  // end UdpSocket

}  // namespace keylogger

#endif
