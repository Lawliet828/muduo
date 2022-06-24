// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include <assert.h>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Types.h"

namespace muduo {

class Thread : noncopyable {
 public:
  typedef std::function<void()> ThreadFunc;

  explicit Thread(ThreadFunc func, const string& name = string())
      : started_(false),
        joined_(false),
        tid_(0),
        func_(std::move(func)),
        name_(name),
        latch_(1) {
    setDefaultName();
  }

  // FIXME: make it movable in C++11
  ~Thread() {
    if (started_ && !joined_) {
      thread_.detach();
    }
  }

  void start();
  void join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    thread_.join();
  }

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.load(); }

 private:
  void setDefaultName() {
    int num = (numCreated_ += 1);
    if (name_.empty()) {
      char buf[32];
      snprintf(buf, sizeof buf, "Thread%d", num);
      name_ = buf;
    }
  }

  bool started_;
  bool joined_;
  std::thread thread_;
  pid_t tid_;
  ThreadFunc func_;
  string name_;
  CountDownLatch latch_;

  static std::atomic<int32_t> numCreated_;
};

}  // namespace muduo
#endif  // MUDUO_BASE_THREAD_H
