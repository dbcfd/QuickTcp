#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#ifdef BUILD_SHARED_LIBS
#if defined(WindowsServer_EXPORTS)
#define WINDOWSSERVER_API __declspec(dllexport)
#else
#define WINDOWSSERVER_API __declspec(dllimport)
#endif
#else
#define WINDOWSSERVER_API
#endif
