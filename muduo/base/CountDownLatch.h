// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)
// 既可以用于所有子线程等待主线程发起 "起跑"
// 也可以用于主线程等待子线程初始化完毕才开始工作

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>

#include "muduo/base/noncopyable.h"

namespace muduo {

class CountDownLatch : noncopyable {
 public:
  explicit CountDownLatch(int count) : count_(count) {}

  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [&]() { return count_ == 0; });
  }

  void countDown() {
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    if (count_ == 0) {
      condition_.notify_all();
    }
  }

  int getCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
};

}  // namespace muduo
#endif  // MUDUO_BASE_COUNTDOWNLATCH_H
