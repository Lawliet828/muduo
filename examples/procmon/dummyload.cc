#include "muduo/base/CurrentThread.h"
#include "muduo/base/noncopyable.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/EventLoop.h"

#include <atomic>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include <condition_variable>
#include <mutex>

using namespace muduo;
using namespace muduo::net;

int g_cycles = 0;
int g_percent = 82;
std::atomic<int32_t> g_done{0};
bool g_busy = false;
std::mutex g_mutex;
std::condition_variable g_cond;

double busy(int cycles)
{
  double result = 0;
  for (int i = 0; i < cycles; ++i)
  {
    result += sqrt(i) * sqrt(i+1);
  }
  return result;
}

double getSeconds(int cycles)
{
  Timestamp start = Timestamp::now();
  busy(cycles);
  return timeDifference(Timestamp::now(), start);
}

void findCycles()
{
  g_cycles = 1000;
  while (getSeconds(g_cycles) < 0.001)
    g_cycles = g_cycles + g_cycles / 4;  // * 1.25
  printf("cycles %d\n", g_cycles);
}

void threadFunc()
{
  while (g_done.load() == 0)
  {
    {
    std::unique_lock<std::mutex> guard(g_mutex);
    g_cond.wait(guard, [&]{ return g_busy; });
    }
    busy(g_cycles);
  }
  printf("thread exit\n");
}

// this is open-loop control
void load(int percent)
{
  percent = std::max(0, percent);
  percent = std::min(100, percent);

  // Bresenham's line algorithm
  int err = 2*percent - 100;
  int count = 0;

  for (int i = 0; i < 100; ++i)
  {
    bool busy = false;
    if (err > 0)
    {
      busy = true;
      err += 2*(percent - 100);
      ++count;
      // printf("%2d, ", i);
    }
    else
    {
      err += 2*percent;
    }

    {
    std::unique_lock<std::mutex> guard(g_mutex);
    g_busy = busy;
    g_cond.notify_all();
    }

    CurrentThread::sleepUsec(10*1000); // 10 ms
  }
  assert(count == percent);
}

void fixed()
{
  while (true)
  {
    load(g_percent);
  }
}

void cosine()
{
  while (true)
    for (int i = 0; i < 200; ++i)
    {
      int percent = static_cast<int>((1.0 + cos(i * 3.14159 / 100)) / 2 * g_percent + 0.5);
      load(percent);
    }
}

void sawtooth()
{
  while (true)
    for (int i = 0; i <= 100; ++i)
    {
      int percent = static_cast<int>(i / 100.0 * g_percent);
      load(percent);
    }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s [fctsz] [percent] [num_threads]\n", argv[0]);
    return 0;
  }

  printf("pid %d\n", getpid());
  findCycles();

  g_percent = argc > 2 ? atoi(argv[2]) : 43;
  int numThreads = argc > 3 ? atoi(argv[3]) : 1;
  std::vector<std::unique_ptr<Thread>> threads;
  for (int i = 0; i < numThreads; ++i)
  {
    threads.emplace_back(new Thread(threadFunc));
    threads.back()->start();
  }

  switch (argv[1][0])
  {
    case 'f':
    {
      fixed();
    }
    break;

    case 'c':
    {
      cosine();
    }
    break;

    case 'z':
    {
      sawtooth();
    }
    break;

    // TODO: square and triangle waves

    default:
    break;
  }

  g_done.exchange(1);
  {
  std::unique_lock<std::mutex> guard(g_mutex);
  g_busy = true;
  g_cond.notify_all();
  }
  for (int i = 0; i < numThreads; ++i)
  {
    threads[i]->join();
  }
}
