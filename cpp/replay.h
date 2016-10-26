#pragma once

#ifdef _MSC_VER
  #define MJ_EXPORT(X) extern "C" __declspec(dllexport) X __cdecl
#else
  #define MJ_EXPORT(X) X
#endif
