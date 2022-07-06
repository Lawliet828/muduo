// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_ASYNCLOGGING_H
#define MUDUO_BASE_ASYNCLOGGING_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "muduo/base/CountDownLatch.h"
#include "muduo/base/LogStream.h"
#include "muduo/base/Thread.h"

namespace muduo {

class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const string& basename, off_t rollSize, int flushInterval = 3);

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

  // 供前端生产者线程调用（日志数据写到缓冲区）
  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.start(); // 日志线程启动
    latch_.wait();
  }

  void stop() {
    running_ = false;
    cond_.notify_one();
    thread_.join();
  }

 private:
  // 供后端消费者线程调用（将数据写到日志文件）
  void threadFunc();

  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef std::unique_ptr<Buffer> BufferPtr;

  // 超时时间，在flushInterval_秒内，缓冲区没写满，仍将缓冲区中的数据写到文件中
  const int flushInterval_;
  std::atomic<bool> running_;
  const string basename_;
  const off_t rollSize_;
  muduo::Thread thread_;
  muduo::CountDownLatch latch_;  // 用于等待线程启动
  std::mutex mutex_;
  std::condition_variable cond_;
  BufferPtr currentBuffer_;  // 当前缓冲区
  BufferPtr nextBuffer_;     // 预备缓冲区
  BufferVector buffers_;     // 待写入文件的已填满的缓冲区
};

}  // namespace muduo

#endif  // MUDUO_BASE_ASYNCLOGGING_H
