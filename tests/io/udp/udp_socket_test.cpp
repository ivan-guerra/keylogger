#include "io/udp/udp_socket.h"

#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>
#include <string>

static const char* kTestAddr = "127.0.0.1";
static const int kTestPort = 5555;

TEST(UdpSocketTest, UdpSocketConstructorSuccessfullyCreatesSender) {
  ASSERT_NO_THROW(auto socket = keylogger::UdpSocket(kTestAddr, kTestPort));
}

TEST(UdpSocketTest, UdpSocketConstructorSuccessfullyCreatesReceiver) {
  ASSERT_NO_THROW(auto socket = keylogger::UdpSocket(kTestPort));
}

TEST(UdpSocketTest, UdpSocketConstructorThrowsInvalidArgumentOnInvalidIp) {
  ASSERT_THROW(auto socket = keylogger::UdpSocket("foo", kTestPort),
               std::invalid_argument);
}

TEST(UdpSocketTest,
     SenderUdpSocketConstructorThrowsInvalidArgumentOnInvalidPort) {
  ASSERT_THROW(auto socket = keylogger::UdpSocket(kTestAddr, -1),
               std::invalid_argument);
  ASSERT_THROW(auto socket = keylogger::UdpSocket(kTestAddr, 0),
               std::invalid_argument);
}

TEST(UdpSocketTest,
     SenderUdpSocketConstructorThrowsInvalidArgumentOnInvalidOptionFlag) {
  ASSERT_THROW(auto socket = keylogger::UdpSocket(kTestAddr, kTestPort, -1),
               std::invalid_argument);
}

TEST(UdpSocketTest,
     ReceiverUdpSocketConstructorThrowsInvalidArgumentOnInvalidPort) {
  ASSERT_THROW(auto socket = keylogger::UdpSocket(-1), std::invalid_argument);
  ASSERT_THROW(auto socket = keylogger::UdpSocket(0), std::invalid_argument);
}

TEST(UdpSocketTest,
     ReceiverUdpSocketConstructorThrowsInvalidArgumentOnInvalidOptionFlag) {
  ASSERT_THROW(auto socket = keylogger::UdpSocket(kTestPort, -1),
               std::invalid_argument);
}

TEST(UdpSocketTest, GetPortReturnsSenderSocketPort) {
  auto socket = keylogger::UdpSocket(kTestAddr, kTestPort);
  ASSERT_EQ(kTestPort, socket.GetPort());
}

TEST(UdpSocketTest, GetPortReturnsReceiverSocketPort) {
  auto socket = keylogger::UdpSocket(kTestPort);
  ASSERT_EQ(kTestPort, socket.GetPort());
}

TEST(UdpSocketTest, GetAddressReturnsSenderIpV4Address) {
  auto socket = keylogger::UdpSocket(kTestAddr, kTestPort);
  ASSERT_EQ(kTestAddr, socket.GetAddress());
}
