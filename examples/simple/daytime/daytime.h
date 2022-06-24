#ifndef MUDUO_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H
#define MUDUO_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;

// RFC 867
class DaytimeServer : public muduo::net::TcpServer {
 public:
  DaytimeServer(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& listenAddr)
      : TcpServer(loop, listenAddr, "DaytimeServer") {
    setConnectionCallback(std::bind(&DaytimeServer::onConnection, this, _1));
    setMessageCallback(std::bind(&DaytimeServer::onMessage, this, _1, _2, _3));
  }

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "DaytimeServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      conn->send(Timestamp::now().toFormattedString() + "\n");
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

#endif  // MUDUO_EXAMPLES_SIMPLE_DAYTIME_DAYTIME_H
