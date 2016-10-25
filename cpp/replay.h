#pragma once

#define TICK_RATE (5)
#define TICK_TIME (1.0f / TICK_RATE)

#ifdef _MSC_VER
  #define MJ_EXPORT(X) extern "C" __declspec(dllexport) X __cdecl
#else
  #define MJ_EXPORT(X) X
#endif
