#pragma once

#include <stdexcept>

#pragma warning(disable:4251)

//windows defines
#ifdef BUILD_SHARED_LIBS
#ifdef Utilities_EXPORTS
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif
#else
#define UTILITIES_API
#endif

namespace quicktcp {
typedef char stream_data_t;
typedef unsigned long stream_size_t;
}
