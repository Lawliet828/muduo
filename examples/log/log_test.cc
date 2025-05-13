#include <stdio.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <utility>

#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/base/CurrentThread.h"

using namespace muduo;

int kRollSize = 500 * 1024; // 500KB

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

  // 循环打印日志
  for (int i = 0; i < 30000; ++i) {
      // LOG_INFO << "Hello, this is a test log message number: " << i;
      std::cout << "Hello, this is a test log message number: " << i << std::endl;
      usleep(1000); // 睡眠1毫秒
  }
}
