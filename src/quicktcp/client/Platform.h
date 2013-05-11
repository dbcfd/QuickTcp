#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(BUILD_SHARED_LIBS)
#if defined(Client_EXPORTS)
#define CLIENT_API __declspec(dllexport)
#else
#define CLIENT_API __declspec(dllimport)
#endif
#else
#define CLIENT_API
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
