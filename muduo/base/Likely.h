#ifndef LIKELY_H_
#define LIKELY_H_

#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

#endif