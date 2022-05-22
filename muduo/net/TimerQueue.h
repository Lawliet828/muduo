// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TimerId.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Timer;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  TimerId addTimer(TimerCallback cb,
                   Timestamp when,
                   double interval);

  // 取消某个定时器
  void cancel(TimerId timerId);

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer*> Entry; // 到期的时间和指向其的定时器
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer*, int64_t> ActiveTimer; // 定时器和其定时器的序列号
  typedef std::set<ActiveTimer> ActiveTimerSet;

  // 以下两个成员函数只可能在其所属的I/O线程中调用，因而不必加锁。
  // 服务器性能杀手之一是锁竞争，所以要尽可能少用锁
  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  // 返回超时的定时器列表
  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(Timer* timer); // 在两个序列中插入定时器

  EventLoop* loop_;
  const int timerfd_; // 只有一个定时器,防止同时开启多个定时器,占用多余的文件描述符
  Channel timerfdChannel_; // 定时器关心的channel对象
  // Timer list sorted by expiration
  TimerList timers_; // 定时器列表, 按到期时间排序

  // for cancel()
  // activeTimers_和timers_保存的是相同的数据
  // timers_是按照到期的时间排序的, activeTimers_是按照对象地址排序
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_; /* atomic */ // 是否正在处理超时事件
  ActiveTimerSet cancelingTimers_; // 保存的是取消的定时器
};

}  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_TIMERQUEUE_H
