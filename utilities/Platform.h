#pragma once

#include <stdexcept>

#pragma warning(disable:4251)

//windows defines
#if defined(UTILITIES_BUILD)
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif
