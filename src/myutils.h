#pragma once

#ifdef _DEBUG
# define DEBUG_PRINT(format, ...) printf(format "\n", ##__VA_ARGS__);
#else
# define DEBUG_PRINT(format, ...) do {} while (0);
#endif



typedef signed char           S8;
typedef char                  U8;
typedef short                 S16;
typedef unsigned short        U16;
typedef int                   S32;
typedef unsigned int          U32;
typedef long long             S64;
typedef unsigned long long    U64;