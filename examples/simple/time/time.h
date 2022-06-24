#ifndef MUDUO_EXAMPLES_SIMPLE_TIME_TIME_H
#define MUDUO_EXAMPLES_SIMPLE_TIME_TIME_H

#include "muduo/base/Logging.h"
#include "muduo/net/Endian.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;

// RFC 868
class TimeServer : public muduo::net::TcpServer {
 public:
  TimeServer(muduo::net::EventLoop* loop,
             const muduo::net::InetAddress& listenAddr)
      : TcpServer(loop, listenAddr, "TimeServer") {
    setConnectionCallback(
        std::bind(&TimeServer::onConnection, this, _1));
    setMessageCallback(
        std::bind(&TimeServer::onMessage, this, _1, _2, _3));
  }

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "TimeServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      time_t now = ::time(NULL);
      int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
      conn->send(&be32, sizeof be32);
      conn->shutdown();
    }
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes received at " << time.toString();
  }
};

#endif  // MUDUO_EXAMPLES_SIMPLE_TIME_TIME_H
