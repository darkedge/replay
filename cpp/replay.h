#pragma once

// Specify tick rate in ms
// 15 ms = 66.66
// 30 ms = 33.33
// 60 ms = 16.66
// 120 ms = 8.33
#define TICK_TIME 0.120f

// Specify tick rate in ticks
//#define TICK_RATE 20
//#define TICK_TIME (1.0f / TICK_RATE)

#ifdef _MSC_VER
  #define MJ_EXPORT(X) extern "C" __declspec(dllexport) X __cdecl
#else
  #define MJ_EXPORT(X) X
#endif
