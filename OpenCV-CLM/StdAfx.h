// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CCCD0ACF_C279_47FF_9E3C_FFB3610124C1__INCLUDED_)
#define AFX_STDAFX_H__CCCD0ACF_C279_47FF_9E3C_FFB3610124C1__INCLUDED_

#include <stdio.h>
#include <stdint.h>

// TODO: reference additional headers your program requires here

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef long LONG;
typedef uint64_t LONGLONG;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

typedef struct _SYSTEMTIME *LPSYSTEMTIME;

void QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
bool QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
void GetSystemTime(LPSYSTEMTIME lpSystemTime);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CCCD0ACF_C279_47FF_9E3C_FFB3610124C1__INCLUDED_)
