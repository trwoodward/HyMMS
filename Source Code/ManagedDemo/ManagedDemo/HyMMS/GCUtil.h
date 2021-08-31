#pragma once
#ifndef  GCUTIL_H
#define GCUTIL_H

#include <climits>

#ifndef WIN32
#define __alignof alignof
#endif // !WIN32

//Object flags, used in reference table
#define GC_WHITE_FLAG 1
#define GC_GREY_FLAG 2
#define GC_BLACK_FLAG 4
#define GC_GREEN_FLAG 8
#define GC_ARRAY_FLAG 16
#define GC_HANDLE_ARRAY_FLAG 32
#define GC_UNMANAGED_ARRAY_FLAG 64

//Creation macros
#define gcnew(x) new(__alignof(x), false)x
#define gcnew_Pinned(x) new(__alignof(x), true)x

//Sentinel value for index
const unsigned int INDEX_SENTINEL = UINT_MAX;

//Helper function for calculating alignment offsets
inline char CalculateAdjustment(void* targetAddress, char alignment)
{
	char misalignment = ((static_cast<unsigned int>(alignment - 1)) & (unsigned int)targetAddress);
	return (misalignment != 0)? alignment - misalignment : 0;
}

#endif // ! GCUTIL_H