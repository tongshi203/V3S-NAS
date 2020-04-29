#ifndef __MYTEMPLATEBASE_H_INCLUDE_20010702__
#define __MYTEMPLATEBASE_H_INCLUDE_20010702__

#include "stdafx.h"

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
    #ifdef _WIN32	// Windows
	    inline void *__cdecl operator new(size_t, void *_P) {return (_P); }
    	#if     _MSC_VER >= 1200
	    	inline void __cdecl operator delete(void *, void *){ return; }
    	#endif
    #else			// lLinux
        #include <new>
    #endif // _WIN32
#endif

#ifndef MY_INLINE 
	#ifdef _DEBUG
		#define MY_INLINE 
	#else
		#define MY_INLINE inline
	#endif //_DEBUG
#endif // MY_INLINE

//----------------------------------
//	define POSITION for Linux
#ifndef _WIN32		
   	struct __POSITION { };
	typedef __POSITION* POSITION;
    #define BEFORE_START_POSITION ((POSITION)-1L)
#endif // _WIN32

template<class TYPE> MY_INLINE void MyConstructElements(TYPE* pElements, int nCount)
{
	// first do bit-wise zero initialization
#ifdef _WIN32
	RtlZeroMemory((void*)pElements, nCount * sizeof(TYPE));
#else		// Linux
	bzero( (void*)pElements, nCount * sizeof(TYPE) );
#endif //_WIN32

	// then call the constructor(s)
	for (; nCount--; pElements++)
		::new((void*)pElements) TYPE;
}

template<class TYPE> MY_INLINE void MyConstructElementsNotClearMemory(TYPE* pElements, int nCount)
{
	// then call the constructor(s)
	for (; nCount--; pElements++)
		::new((void*)pElements) TYPE;
}

template<class TYPE> MY_INLINE void MyDestructElements(TYPE* pElements, int nCount)
{
	// call the destructor(s)
	for (; nCount--; pElements++)
		pElements->~TYPE();
}

template<class TYPE, class ARG_TYPE>
BOOL MyCompareElements(TYPE* pElement1, ARG_TYPE* pElement2)
{
	return *pElement1 == *pElement2;
}

template<class ARG_KEY> MY_INLINE UINT MyHashKey(ARG_KEY key)
{
	// default identity hash - works for most primitive values
	// FIXME for X86_64
#ifdef __x86_64__
	return (DWORD)(((ULONGLONG)key)>>4);
#else
	return ((UINT)(DWORD)key) >> 4;
#endif // __x86_64__
}

#ifndef _WIN32
#ifdef __BORLANDC__
template<> UINT MyHashKey<LPCSTR> (LPCSTR key)
{
	UINT nHash = 0;
    while( *key )
    {
    	nHash = (nHash<<5) + nHash + *key ++;
    }
    return nHash;
}
#endif // __BORLANDC__
#endif // _WIN32

template<class TYPE> MY_INLINE void MyCopyElements( TYPE * pDest, const TYPE * pSrc, int nCount )
{
	while( nCount -- )
    {
    	*pDest ++ = *pSrc ++;
    }
}

#endif // __MYTEMPLATEBASE_H_INCLUDE_20010702__
