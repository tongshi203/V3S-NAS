#ifndef __MY_DATATYPE_H_20080623__
#define __MY_DATATYPE_H_20080623__

typedef unsigned char 	BYTE;
typedef BYTE * PBYTE;

typedef unsigned short WORD;
typedef WORD * PWORD;

typedef unsigned int DWORD;	// 2015.11.22 CYJ Modify, using unsigned int instead of unsigned long, since sizeof(long)=8 under linux-64
typedef DWORD * PDWORD;

typedef int BOOL;
typedef BOOL * PBOOL;

typedef unsigned long long ULONGLONG;
typedef void * PVOID;
typedef void * LPVOID;

typedef short * PSHORT;
typedef short SHORT;

typedef unsigned long * HWND;

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif //TRUE

#ifndef NULL
	#define NULL	((void*)0)
#endif //NULL

// ST STB
#ifdef __COMPILE_FOR_STSTB__
	#define WINAPI
#else
	#define WINAPI	__attribute__((regparm(3)))
#endif //__COMPILE_FOR_STSTB__


#ifndef MAKEWORD
	#define MAKEWORD( low, high ) ( ((BYTE)low) + (((BYTE)high)<<8) )
#endif //MAKEWORD


#endif // __MY_DATATYPE_H_20080623__

