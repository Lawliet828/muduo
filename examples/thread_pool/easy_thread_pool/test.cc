#include <chrono>

#include "examples/thread_pool/easy_thread_pool/thread_pool.h"

void func(int i) {
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  LOG_INFO << "finish ----> " << i;
}

int main() {
  ThreadPool thread_pool;

  for (int i = 0; i < 50; i++) {
    thread_pool.AddTask(std::bind(&func, i));
  }

  getchar();
  return 0;
}