#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <types.h>
#else
#define LPCTSTR const char *
#define _TCHAR char

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int

#define _tmain main
#define _tprintf printf
#define _tfprintf fprintf
#define _tfopen fopen

#define _T(X) X

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/types.h>
#endif

#include <string>
