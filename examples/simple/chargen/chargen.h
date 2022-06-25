#ifndef MUDUO_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H
#define MUDUO_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H

#include <stdio.h>

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;

// RFC 864
class ChargenServer : public muduo::net::TcpServer {
 public:
  ChargenServer(muduo::net::EventLoop* loop,
                const muduo::net::InetAddress& listenAddr, bool print = false)
      : TcpServer(loop, listenAddr, "ChargenServer"),
        transferred_(0),
        startTime_(Timestamp::now()) {
    setConnectionCallback(std::bind(&ChargenServer::onConnection, this, _1));
    setMessageCallback(std::bind(&ChargenServer::onMessage, this, _1, _2, _3));
    setWriteCompleteCallback(
        std::bind(&ChargenServer::onWriteComplete, this, _1));
    if (print) {
      loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
    }

    std::string line;
    for (int i = 33; i < 127; ++i) {
      line.push_back(char(i));
    }
    line += line;

    for (size_t i = 0; i < 127 - 33; ++i) {
      message_ += line.substr(i, 72) + '\n';
    }
  }

 private:
  void onConnection(const muduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << "ChargenServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      conn->setTcpNoDelay(true);
      conn->send(message_);
    }
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf, muduo::Timestamp time) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes received at " << time.toString();
  }

  void onWriteComplete(const muduo::net::TcpConnectionPtr& conn) {
    transferred_ += message_.size();
    conn->send(message_);
  }

  // 打印吞吐量
  void printThroughput() {
    Timestamp endTime = Timestamp::now();
    double time = timeDifference(endTime, startTime_);
    printf("%4.3f MiB/s\n",
           static_cast<double>(transferred_) / time / 1024 / 1024);
    transferred_ = 0;
    startTime_ = endTime;
  }

  std::string message_;
  int64_t transferred_;
  muduo::Timestamp startTime_;
};

#endif  // MUDUO_EXAMPLES_SIMPLE_CHARGEN_CHARGEN_H
