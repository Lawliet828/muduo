// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include <assert.h>

#include <condition_variable>
#include <deque>
#include <mutex>

#include "muduo/base/noncopyable.h"

namespace muduo
{

template<typename T>
class BoundedBlockingQueue : noncopyable
{
 public:
  explicit BoundedBlockingQueue(unsigned maxSize)
    : maxSize_(maxSize)
  {
  }

  void put(const T& x)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [&]() { return queue_.size() < maxSize_; });
    assert(queue_.size() < maxSize_);
    queue_.push_back(x);
    notEmpty_.notify_one();
  }

  void put(T&& x)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    notFull_.wait(lock, [&]() { return queue_.size() < maxSize_; });
    assert(queue_.size() < maxSize_);
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
    notFull_.notify_one();
    return front;
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool full() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() == maxSize_;
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.capacity();
  }

 private:
  const unsigned maxSize_;
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_;
  std::condition_variable notFull_;
  std::deque<T> queue_;
};

}  // namespace muduo

#endif  // MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
