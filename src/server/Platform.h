#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(Server_EXPORTS)
#define SERVER_API __declspec(dllexport)
#else
#define SERVER_API __declspec(dllimport)
#endif
