#pragma once

#include <atomic>
#include <chrono>

using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;

class TokenBucket {
 public:
  TokenBucket(const int64_t rate, const int64_t capacity) {
    if (rate > 1024 * 1024) {
      zoom_ratio_ = 1024;
    }
    int64_t new_rate = rate / zoom_ratio_;
    int64_t new_capacity = capacity / zoom_ratio_;
    timePerToken_ = nanoseconds::period::den / new_rate;
    timePerBurst_ = new_capacity * timePerToken_;
  }

  /**
   * 令牌桶实例可能会存进vector/list等容器中，因此需要提供拷贝构造函数
   */
  TokenBucket(const TokenBucket& other) {
    time_ = other.time_.load();
    timePerToken_ = other.timePerToken_;
    timePerBurst_ = other.timePerBurst_;
    zoom_ratio_ = other.zoom_ratio_;
  }

  TokenBucket& operator=(const TokenBucket& other) {
    time_ = other.time_.load();
    timePerToken_ = other.timePerToken_;
    timePerBurst_ = other.timePerBurst_;
    zoom_ratio_ = other.zoom_ratio_;
    return *this;
  }

  bool consume(const int64_t tokens) {
    const int64_t now = std::chrono::duration_cast<nanoseconds>(
                            steady_clock::now().time_since_epoch())
                            .count();
    // 产生这么多token需要的时间
    const int64_t timeNeeded = tokens / zoom_ratio_ * timePerToken_;
    const int64_t minTime = now - timePerBurst_;
    int64_t oldTime = time_.load(std::memory_order_relaxed);

    int64_t newTime = oldTime;
    if (minTime > oldTime) {
      newTime = minTime;
    }

    newTime += timeNeeded;
    if (newTime > now) {
      return false;
    }
    // 即使设置成功但返回false, 也不需要循环, 顶多浪费了一次消耗
    if (time_.compare_exchange_weak(oldTime, newTime, std::memory_order_relaxed,
                                    std::memory_order_relaxed)) {
      return true;
    }

    return false;
  }

 private:
  std::atomic<int64_t> time_ = {0};
  int64_t zoom_ratio_ = 1;  // 缩放比例
  int64_t timePerToken_;
  int64_t timePerBurst_;
};
