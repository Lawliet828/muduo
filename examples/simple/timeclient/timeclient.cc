#include <stdio.h>
#include <unistd.h>

#include <utility>

#include "muduo/base/Logging.h"
#include "muduo/net/Endian.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpClient.h"

using namespace muduo;
using namespace muduo::net;

class TimeClient : public muduo::net::TcpClient {
 public:
  TimeClient(EventLoop* loop, const InetAddress& serverAddr)
      : TcpClient(loop, serverAddr, "TimeClient") {
    setConnectionCallback(
        std::bind(&TimeClient::onConnection, this, _1));
    setMessageCallback(
        std::bind(&TimeClient::onMessage, this, _1, _2, _3));
    // enableRetry();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (!conn->connected()) {
      getLoop()->quit();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime) {
    if (buf->readableBytes() >= sizeof(int32_t)) {
      const void* data = buf->peek();
      int32_t be32 = *static_cast<const int32_t*>(data);
      buf->retrieve(sizeof(int32_t));
      time_t time = sockets::networkToHost32(be32);
      Timestamp ts(static_cast<int64_t>(time) *
                   Timestamp::kMicroSecondsPerSecond);
      LOG_INFO << "Server time = " << time << ", " << ts.toFormattedString();
    } else {
      LOG_INFO << conn->name() << " no enough data " << buf->readableBytes()
               << " at " << receiveTime.toFormattedString();
    }
  }
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 1) {
    EventLoop loop;
    InetAddress serverAddr(argv[1], 2037);

    TimeClient timeClient(&loop, serverAddr);
    timeClient.connect();
    loop.loop();
  } else {
    printf("Usage: %s host_ip\n", argv[0]);
  }
}
