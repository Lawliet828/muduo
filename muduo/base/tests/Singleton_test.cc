#include "muduo/base/Singleton.h"

#include <stdio.h>

#include <thread>

#include "muduo/base/CurrentThread.h"

class Test : muduo::noncopyable {
 public:
  Test() {
    printf("tid=%d, constructing %p\n", muduo::CurrentThread::tid(), this);
  }

  ~Test() {
    printf("tid=%d, destructing %p %s\n", muduo::CurrentThread::tid(), this,
           name_.c_str());
  }

  const std::string& name() const { return name_; }
  void setName(const std::string& n) { name_ = n; }

 private:
  std::string name_;
};

class TestNoDestroy : muduo::noncopyable {
 public:
  // Tag member for Singleton<T>
  void no_destroy();

  TestNoDestroy() {
    printf("tid=%d, constructing TestNoDestroy %p\n",
           muduo::CurrentThread::tid(), this);
  }

  ~TestNoDestroy() {
    printf("tid=%d, destructing TestNoDestroy %p\n",
           muduo::CurrentThread::tid(), this);
  }
};

void threadFunc() {
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(),
         &muduo::Singleton<Test>::instance(),
         muduo::Singleton<Test>::instance().name().c_str());
  muduo::Singleton<Test>::instance().setName("only one, changed");
}

int main() {
  muduo::Singleton<Test>::instance().setName("only one");
  std::thread t1(threadFunc);
  t1.join();
  printf("tid=%d, %p name=%s\n", muduo::CurrentThread::tid(),
         &muduo::Singleton<Test>::instance(),
         muduo::Singleton<Test>::instance().name().c_str());
  muduo::Singleton<TestNoDestroy>::instance();
  printf("with valgrind, you should see %zd-byte memory leak.\n",
         sizeof(TestNoDestroy));
}
