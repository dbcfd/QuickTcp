#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(SERVER_INTERFACE_BUILD)
#define SERVER_INTERFACE_API __declspec(dllexport)
#else
#define SERVER_INTERFACE_API __declspec(dllimport)
#endif
