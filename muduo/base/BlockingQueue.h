// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include <assert.h>

#include <condition_variable>
#include <deque>
#include <mutex>

#include "muduo/base/noncopyable.h"

namespace muduo
{

template<typename T>
class BlockingQueue : noncopyable
{
 public:
  using queue_type = std::deque<T>;

  BlockingQueue()
    : queue_()
  {
  }

  void put(const T& x)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(x);
    notEmpty_.notify_one();
  }

  void put(T&& x)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(std::move(x));
    notEmpty_.notify_one();
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [&]() { return !queue_.empty(); });
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    return front;
  }

  queue_type drain()
  {
    std::deque<T> queue;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue = std::move(queue_);
      assert(queue_.empty());
    }
    return queue;
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_;
  queue_type queue_;
};  // __attribute__ ((aligned (64)));

}  // namespace muduo

#endif  // MUDUO_BASE_BLOCKINGQUEUE_H
