#ifndef MUDUO_EXAMPLES_SIMPLE_DISCARD_DISCARD_H
#define MUDUO_EXAMPLES_SIMPLE_DISCARD_DISCARD_H

#include "muduo/base/Logging.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;

// RFC 863
class DiscardServer : public muduo::net::TcpServer {
 public:
  DiscardServer(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& listenAddr)
      : muduo::net::TcpServer(loop, listenAddr, "DiscardServer") {
    setConnectionCallback(std::bind(&DiscardServer::onConnection, this, _1));
    setMessageCallback(std::bind(&DiscardServer::onMessage, this, _1, _2, _3));
  }

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf, muduo::Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes received at " << time.toString();
  }
};

#endif  // MUDUO_EXAMPLES_SIMPLE_DISCARD_DISCARD_H
