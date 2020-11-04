#ifndef __BASE_COMPILER_HH__
#define __BASE_COMPILER_HH__

#include <memory>

#if defined(__GNUC__) // clang or gcc
#  define M5_ATTR_NORETURN  __attribute__((noreturn))
#  define M5_DUMMY_RETURN
#  define M5_VAR_USED __attribute__((unused))
#  define M5_ATTR_PACKED __attribute__ ((__packed__))
#  define M5_NO_INLINE __attribute__ ((__noinline__))
#  define M5_DEPRECATED __attribute__((deprecated))
#  define M5_DEPRECATED_MSG(MSG) __attribute__((deprecated(MSG)))
#  define M5_UNREACHABLE __builtin_unreachable()
#  define M5_PUBLIC __attribute__ ((visibility ("default")))
#  define M5_LOCAL __attribute__ ((visibility ("hidden")))
#endif

#endif // __BASE_COMPILER_HH__
