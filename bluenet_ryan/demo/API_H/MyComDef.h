///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-7-22
///
///		用途：
///			我的 COM 组件接口
///=======================================================

#ifndef __MY_COM_DEFINE_20030722__
#define __MY_COM_DEFINE_20030722__

#include <string.h>

#pragma pack(push,8)

#ifndef _WIN32

  #if defined(__BORLANDC__) && defined(SYSMAC_H)
    typedef BOOL *      PBOOL;
    typedef WORD *      PWORD;
    typedef LONG *      PLONG;
    typedef BYTE *      PBYTE;
  #else
    typedef int BOOL, * PBOOL;
    typedef int INT, * PINT;
    typedef long LONG, * PLONG;
    typedef unsigned char BYTE, *PBYTE;
    typedef unsigned short WORD, *PWORD;
  #endif // defined(__BORLANDC__) && defined(SYSMAC_H)

  	typedef unsigned int DWORD,*PDWORD;		// 2015.11.22 CYJ Modify, using unsigned int instead of unsigned long, since sizeof(long)=8 under linux-64
  	typedef unsigned int UINT,*PUINT;
  	typedef unsigned long long ULONGLONG;
  	typedef long long LONGLONG;
  	typedef const char * LPCSTR;
	typedef unsigned long HRESULT;

#ifndef TRUE
	#define TRUE    1  
#endif //TRUE

#ifndef FALSE
	#define FALSE   0
#endif //FALSE

  #ifndef offsetof
      #define offsetof(s,m)  (size_t)&(((s*)0)->m)
  #endif //offsetof

#endif // _WIN32

#undef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#undef min
#define min(a,b) ((a) < (b) ? (a) : (b))

#if !( defined(_WIN32) || defined(SYSMAC_H) )
  #ifndef GUID_DEFINED
      #define GUID_DEFINED
      typedef struct _GUID
      {
          unsigned int   Data1;
          unsigned short Data2;
          unsigned short Data3;
          unsigned char  Data4[8];
      } GUID;
  #endif /* GUID_DEFINED */

  #if defined(__cplusplus)
      #ifndef _REFIID_DEFINED
          #define _REFIID_DEFINED
          #define REFIID              const GUID &
      #endif // !_REFIID_DEFINED
  #else
      #ifndef _REFIID_DEFINED
          #define _REFIID_DEFINED
          typedef const GUID *REFIID;
      #endif // !_REFIID_DEFINED
  #endif // !__cplusplus

   typedef GUID IID;

  #ifdef __cplusplus
      // because GUID is defined elsewhere in WIN32 land, the operator == and !=
      // are moved outside the class to global scope.
      #ifdef _OLE32_
          __inline BOOL operator==(const GUID& guidOne, const GUID& guidOther)
          {
              return IsEqualGUID(guidOne,guidOther);
          }
      #else   // !_OLE32_
          __inline BOOL operator==(const GUID& guidOne, const GUID& guidOther)
          {
          #ifdef _WIN32
              return !memcmp(&guidOne,&guidOther,sizeof(GUID));
          #else
              return !memcmp(&guidOne,&guidOther,sizeof(GUID));
          #endif
          }
      #endif // _OLE32_

      __inline BOOL operator!=(const GUID& guidOne, const GUID& guidOther)
      {
          return !(guidOne == guidOther);
      }
  #endif // __cpluscplus_

  #ifndef DEFINE_GUID
      #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
          static const GUID name \
                  = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
  #endif // DEFINE_GUID

  #define EXTERN_GUID	DEFINE_GUID
#endif // !( _WIN32||SYSMAC_H )

#ifndef __MY_UNKNOWN_IMPLEMENT__
#define __MY_UNKNOWN_IMPLEMENT__
   //--------------------------------------------------
   //	{00000000-0000-0000-C000-000000000046}
   static const GUID IID_IMyUnknown = { 0, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} };

   //--------------------------------------------------
   class IMyUnknown
   {
   public:
	   IMyUnknown(){}
	   virtual ~ IMyUnknown(){}
       virtual long AddRef(void) = 0;
       virtual long Release(void) = 0;
       virtual DWORD QueryInterface( REFIID iid, void **ppvObject) = 0;
   };

#endif // __MY_UNKNOWN_IMPLEMENT__


#pragma pack(pop)

#endif // __MY_COM_DEFINE_20030722__

