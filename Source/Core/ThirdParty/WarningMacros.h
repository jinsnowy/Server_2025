#pragma once

#ifdef _MSC_VER


#define PROTOBUF_IGNORE_WARNINGS_PUSH \
        __pragma(warning(push)) \
        __pragma(warning(disable: 4251 4267 4100 4127 4189 4702))
#define PROTOBUF_IGNORE_WARNINGS_POP \
        __pragma(warning(pop))

#define BOOST_IGNORE_WARNINGS_PUSH \
        __pragma(warning(push)) \
        __pragma(warning(disable: 4459))
#define BOOST_IGNORE_WARNINGS_POP \
        __pragma(warning(pop))

#elif defined(__GNUC__) || defined(__clang__)
#define PROTOBUF_IGNORE_WARNINGS_PUSH \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
        _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"")
#define PROTOBUF_IGNORE_WARNINGS_POP \
        _Pragma("GCC diagnostic pop")
#else
#define PROTOBUF_IGNORE_WARNINGS_PUSH
#define PROTOBUF_IGNORE_WARNINGS_POP
#endif
