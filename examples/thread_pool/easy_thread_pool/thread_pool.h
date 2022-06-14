#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <assert.h>

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "muduo/base/Logging.h"

class ThreadPool {
 public:
  typedef std::function<void()> Task;

  ThreadPool() : stop_(false) {
    thread_num_ = std::thread::hardware_concurrency();
    if (thread_num_ < 2) {
      thread_num_ = 3;
    }
    LOG_INFO << "thread_num = " << thread_num_;
    assert(threads_.empty());
    threads_.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
      threads_.push_back(std::thread(&ThreadPool::thread_loop, this));
    }
  }

  ~ThreadPool() {
    LOG_INFO << "ThreadPool::~ThreadPool()";
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stop_ = true;
    }
    cond_.notify_all();
    for (std::thread& worker : threads_) {
      worker.join();
    }
  }

  ThreadPool(const ThreadPool&) = delete;
  const ThreadPool& operator=(const ThreadPool&) = delete;

  void add_task(const Task& task) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [&] {
      return stop_ || static_cast<int>(tasks_.size()) < thread_num_;
    });

    tasks_.push_back(task);
    LOG_INFO << "Add task success";
    cond_.notify_one();
  }

 private:
  void thread_loop() {
    LOG_INFO << "ThreadPool::ThreadLoop() start.";
    while (true) {
      std::unique_lock<std::mutex> lock(mutex_);
      cond_.wait(lock, [&] { return stop_ || !tasks_.empty(); });
      if (stop_ && tasks_.empty()) {
        return;
      }

      Task task = std::move(tasks_.front());
      tasks_.pop_front();

      lock.unlock();
      // 如果task需要长时间执行, 会造成阻塞
      task();
      lock.lock();

      cond_.notify_one();
    }
    LOG_INFO << "ThreadPool::ThreadLoop() exit.";
  }

  bool stop_;
  int thread_num_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::vector<std::thread> threads_;
  std::deque<Task> tasks_;
};

#endif
