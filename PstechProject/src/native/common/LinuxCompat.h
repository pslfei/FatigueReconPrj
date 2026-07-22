#pragma once
#if defined(__linux__) || defined(__unix__)
    // 基础类型兼容
    #ifndef LONG
        typedef int LONG;
    #endif
    #ifndef DWORD
        typedef unsigned int DWORD;
    #endif
    #ifndef BYTE
        typedef unsigned char BYTE;
    #endif
#endif