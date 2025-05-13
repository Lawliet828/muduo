#include <stdio.h>
#include <unistd.h>

#include <functional>
#include <utility>

#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"

using namespace muduo;
using namespace muduo::net;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

int numThreads = 0;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
      : loop_(loop), server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(numThreads);
  }

  void start() { server_.start(); }

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
      loop_->quit();
    }
    conn->send(msg);
  }

  EventLoop* loop_;
  TcpServer server_;
};

int kRollSize = 500 * 1000 * 1000;

std::unique_ptr<muduo::AsyncLogging> g_asyncLog;

void asyncOutput(const char* msg, int len) { g_asyncLog->append(msg, len); }

void setLogging(const char* argv0) {
  muduo::Logger::setOutput(asyncOutput);
  char name[256];
  strncpy(name, argv0, sizeof(name) - 1);
  name[sizeof(name) - 1] = '\0';
  g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->start();
}

int main(int argc, char* argv[]) {
  if (argc >= 2 && strcmp(argv[1], "-f") == 0) {
    // 前台运行
  } else {
    setLogging(argv[0]);
  }

  muduo::Logger::setLogLevel(Logger::LogLevel::TRACE);

  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
  if (argc > 2) {
    numThreads = atoi(argv[2]);
  }
  EventLoop loop;
  InetAddress listenAddr(2007);
  EchoServer server(&loop, listenAddr);

  server.start();
  loop.loop();
}
