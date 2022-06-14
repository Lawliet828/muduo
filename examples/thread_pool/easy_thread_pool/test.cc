#include <chrono>
#include <iostream>

#include "examples/thread_pool/easy_thread_pool/thread_pool.h"

void testFunc() {
  // loop to print character after a random period of time
  for (int i = 1; i < 4; ++i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_INFO << "testFunc() [" << i << "] output";
  }
}

int main() {
  ThreadPool thread_pool;

  for (int i = 0; i < 5; i++) thread_pool.add_task(testFunc);

  getchar();
  return 0;
}