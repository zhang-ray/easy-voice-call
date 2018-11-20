#pragma once

#ifdef _WIN32

#ifdef EVC_EXPORT
#define EVC_API __declspec(dllexport)
#else
#define EVC_API __declspec(dllimport)
#endif

#endif// _WIN32


/// macOS and Linux
#ifndef EVC_API
#define EVC_API
#endif // EVC_API
