#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace keylogger {

/*!
 * \class UdpSocket
 * \brief The UdpSocket class is a wrapper around a Linux UDP socket.
 */
class UdpSocket {
 public:
  /*!
   * \brief The default constructor creates an uninitialized UDP socket.
   * \details The Caller must call Open() on the socket to get a valid
   * send/receive UDP socket.
   */
  UdpSocket();

  /*!
   * \brief Cleanup socket resources.
   */
  ~UdpSocket();

  UdpSocket(const UdpSocket&) = default;
  UdpSocket& operator=(const UdpSocket&) = default;
  UdpSocket(UdpSocket&&) = default;
  UdpSocket& operator=(UdpSocket&&) = default;

  /*!
   * \brief Return \c true if the socket is initialized.
   */
  operator bool() const { return is_open_; }

  /*!
   * \brief Open a sender socket on addr:port.
   * \details Open() will open a UDP socket for sending on address \a addr and
   *          port \a port. If Open() fails, errno will be set to indicate
   *          the error.
   * \param addr  IPv4 address of remote machine.
   * \param port  The port at which the remote machine is listening.
   * \param flags Bitwise OR of zero or more option flags.
   * \return \c true on success.
   */
  bool Open(const std::string& addr, int port, int flags = 0) {
    return (is_open_ = !SetupSender(addr, port, flags));
  }

  /*!
   * \brief Open a receiver socket on \a port.
   * \details Open() will open a UDP socket on the host for receiving incoming
   *          datagrams on port \a port. If Open() fails, errno will be set
   *          to indicate the error.
   * \param port  Port number at which to listen for incoming datagrams.
   * \param flags Bitwise OR of zero or more option flags.
   * \return \c true on success.
   */
  bool Open(int port, int flags = 0) {
    return (is_open_ = !SetupReceiver(port, flags));
  }

  /*!
   * \brief Return this socket's port number.
   */
  int GetPort() const { return sock_info_.port; }

  /*!
   * \brief Return this socket's IP address.
   */
  std::string GetAddress() const {
    return inet_ntoa(sock_info_.local.sin_addr);
  }

  /*!
   * \brief Send \a len bytes out on the socket.
   *
   * \return The number of bytes sent, -1 on error.
   */
  int Send(void* buf, size_t len) {
    return sendto(sock_info_.fd, buf, len, 0,
                  (struct sockaddr*)&sock_info_.remote,
                  sizeof(sock_info_.remote));
  }

  /*!
   * \brief Receive \a len bytes and store them in \a buf.
   * \return The number of bytes received, -1 on error.
   */
  int Recv(void* buf, size_t len) { return recv(sock_info_.fd, buf, len, 0); }

 private:
  struct SocketInfo {
    int fd;                    /*!< Socket file descriptor. */
    int port;                  /*!< Target port number. */
    struct sockaddr_in local;  /*!< Local IPv4 address. */
    struct sockaddr_in remote; /*!< Remote IPv4 address. */
  };                           // end SocketInfo

  /*!
   * \brief Initialize a sender socket.
   * \return 0 on success, -1 on failure.
   */
  int SetupSender(const std::string& addr, int port, int flags);

  /*!
   * \brief Initialize a receiver socket.
   * \return 0 on succes, -1 on failure.
   */
  int SetupReceiver(int port, int flags);

  bool is_open_;         /*!< \c true if the socket is initialized. */
  SocketInfo sock_info_; /*!< Socket data. */
};                       // end UdpSocket

}  // namespace keylogger

#endif
