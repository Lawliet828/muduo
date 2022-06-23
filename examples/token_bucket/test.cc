#include <iostream>
#include <thread>

#include "token_bucket.h"

int main() {
  int64_t rate = 300 * 1024 * 1024;
  int64_t burstSize = 1024 * 1024;
  TokenBucket bucket(rate, burstSize);

  uint64_t i{0};
  while (true) {
    if (bucket.consume(256 * 1024)) {
      std::cout << ++i << std::endl;
    } else {
      // std::cout << "not consumed " << std::endl;
      // std::this_thread::sleep_for(std::chrono::seconds(1));
      return 0;
    }
  }
  return 0;
}