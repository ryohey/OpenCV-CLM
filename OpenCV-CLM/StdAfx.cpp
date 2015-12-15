// stdafx.cpp : source file that includes just the standard includes
//	OpenCV_CLM.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
    // not supported
}

bool QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
    // not supported
    return false;
}

void GetSystemTime(LPSYSTEMTIME lpSystemTime) {
    // not supported
}