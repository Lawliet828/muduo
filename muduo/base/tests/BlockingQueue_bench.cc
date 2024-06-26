#include <stdio.h>
#include <unistd.h>

#include <map>
#include <string>
#include <thread>
#include <vector>

#include "muduo/base/BlockingQueue.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"

bool g_verbose = false;

// Many threads, one queue.
class Bench {
 public:
  explicit Bench(int numThreads) : latch_(numThreads) {
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
      threads_.emplace_back(new std::thread(&Bench::threadFunc, this));
    }
  }

  void run(int times) {
    printf("waiting for count down latch\n");
    latch_.wait();
    LOG_INFO << threads_.size() << " threads started";
    int64_t total_delay = 0;
    for (int i = 0; i < times; ++i) {
      muduo::Timestamp now(muduo::Timestamp::now());
      queue_.put(now);
      total_delay += delay_queue_.take();
    }
    printf("Average delay: %.3fus\n", static_cast<double>(total_delay) / times);
  }

  void joinAll() {
    for (size_t i = 0; i < threads_.size(); ++i) {
      queue_.put(muduo::Timestamp::invalid());
    }

    for (auto& thr : threads_) {
      thr->join();
    }
    LOG_INFO << threads_.size() << " threads stopped";
  }

 private:
  void threadFunc() {
    if (g_verbose) {
      printf("tid=%d, %s started\n", muduo::CurrentThread::tid(),
             muduo::CurrentThread::name());
    }

    std::map<int, int> delays;
    latch_.countDown();
    bool running = true;
    while (running) {
      muduo::Timestamp t(queue_.take());
      muduo::Timestamp now(muduo::Timestamp::now());
      if (t.valid()) {
        int delay = static_cast<int>(timeDifference(now, t) * 1000000);
        // printf("tid=%d, latency = %d us\n",
        //        muduo::CurrentThread::tid(), delay);
        ++delays[delay];
        delay_queue_.put(delay);
      }
      running = t.valid();
    }

    if (g_verbose) {
      printf("tid=%d, %s stopped\n", muduo::CurrentThread::tid(),
             muduo::CurrentThread::name());
      for (const auto& delay : delays) {
        printf("tid = %d, delay = %d, count = %d\n",
               muduo::CurrentThread::tid(), delay.first, delay.second);
      }
    }
  }

  muduo::BlockingQueue<muduo::Timestamp> queue_;
  muduo::BlockingQueue<int> delay_queue_;
  muduo::CountDownLatch latch_;
  std::vector<std::unique_ptr<std::thread>> threads_;
};

int main(int argc, char* argv[]) {
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  Bench t(threads);
  t.run(100000);
  t.joinAll();
}
