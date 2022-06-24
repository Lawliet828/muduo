#ifndef MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
#define MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H

#include <stdio.h>
#include <unistd.h>

#include <utility>

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

int numThreads = 0;

// RFC 862
class EchoServer : public muduo::net::TcpServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
      : TcpServer(loop, listenAddr, "EchoServer") {
    setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
    setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    setThreadNum(numThreads);
  }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    LOG_INFO << conn->getTcpInfoString();

    conn->send("hello\n");
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at "
              << time.toString();
    if (msg == "exit\n") {
      conn->send("bye\n");
      conn->shutdown();
    }
    if (msg == "quit\n") {
      getLoop()->quit();
    }
    conn->send(msg);
  }
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
