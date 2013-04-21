#pragma once

#include <stdexcept>

#pragma warning(disable:4251)
#pragma warning(disable:4275)
//windows defines
#if defined(QuickTcp_EXPORTS)
#define QUICKTCP_API __declspec(dllexport)
#else
#define QUICKTCP_API __declspec(dllimport)
#endif
