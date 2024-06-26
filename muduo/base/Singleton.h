// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_SINGLETON_H
#define MUDUO_BASE_SINGLETON_H

#include <assert.h>
#include <stdlib.h>  // atexit

#include <mutex>  // for once_flag call_once

#include "muduo/base/noncopyable.h"

namespace muduo {

namespace detail {
// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template <typename T>
struct has_no_destroy {
  template <typename C>
  static char test(decltype(&C::no_destroy));
  template <typename C>
  static int32_t test(...);
  const static bool value = sizeof(test<T>(0)) == 1;
};
}  // namespace detail

template <typename T>
class Singleton : noncopyable {
 public:
  Singleton() = delete;
  ~Singleton() = delete;

  static T& instance() {
    std::call_once(once_, &Singleton::init);
    assert(value_ != NULL);
    return *value_;
  }

 private:
  static void init() {
    value_ = new T();
    if (!detail::has_no_destroy<T>::value) {
      ::atexit(destroy);
    }
  }

  static void destroy() {
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;

    delete value_;
    value_ = NULL;
  }

 private:
  static std::once_flag once_;
  static T* value_;
};

template <typename T>
std::once_flag Singleton<T>::once_;

template <typename T>
T* Singleton<T>::value_ = NULL;

}  // namespace muduo

#endif  // MUDUO_BASE_SINGLETON_H
