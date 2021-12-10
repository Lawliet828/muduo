// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Types.h"

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace muduo
{

class Thread : noncopyable
{
 public:
  typedef std::function<void ()> ThreadFunc;

  explicit Thread(ThreadFunc, const string& name = string());
  // FIXME: make it movable in C++11
  ~Thread();

  void start();
  void join();

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.load(); }

 private:
  void setDefaultName();

  bool       started_;
  bool       joined_;
  std::thread thread_;
  pid_t      tid_;
  ThreadFunc func_;
  string     name_;
  CountDownLatch latch_;

  static std::atomic<int32_t> numCreated_;
};

}  // namespace muduo
#endif  // MUDUO_BASE_THREAD_H
