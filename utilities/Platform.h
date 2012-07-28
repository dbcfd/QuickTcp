#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(OBJECTS_BUILD)
#define OBJECTS_API __declspec(dllexport)
#else
#define OBJECTS_API __declspec(dllimport)
#endif
