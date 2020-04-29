///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005.3.9
///
/// 用途：
///       Win Application base
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///! 该文件仅限在“通视”内部使用!           !
///!              !
///! 我不保证该文件的百分之百正确!      !
///! 若要使用该文件，您需要承担这方面的风险!    !
///=======================================================

#ifndef __MY_WINAPP_H_20050309__
#define __MY_WINAPP_H_20050309__

#pragma pack(push,4)

#if defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)
	#include <windows.h>
#endif // defined(_WIN32) || defined(__FOR_MICROWIN_EMBED__)

////////////////////////////////////////////////////////
// define WM_TIMER ect.
#if !defined(WM_TIMER) && !defined(_WIN32)
	#include "mywinuser.h"
#endif // !defined(WM_TIMER) && !defined(_WIN32)


#define KEYBF_SHIFT_DOWN  (1<<24)
#define KEYBF_ALT_DOWN   (1<<25)
#define KEYBF_CONTRL_DOWN  (1<<26)
#define KEYBF_CAPLOCK_DOWN  (1<<27)
#define KEYBF_NUMLOCK_DOWN  (1<<28)
#define GetScanCode( nFlags ) BYTE((nFlags>>16)&0xFF)


class CMyCommandResponser
{
public:
 CMyCommandResponser(int x=0, int y=0, int cx=0, int cy=0);
 virtual ~CMyCommandResponser();

    virtual long DefWindowProc( int nMessage, int wParam, int lParam );
    bool PostMessage( int nMessage, int wParam=0, int lParam=0 );
    long SendMessage( int nMessage, int wParam=0, int lParam=0 );

 ///  vtChar                 Virtual Key value
 ///     nFlags               Bit0-Bit14   1
 ///     Bit15-Bit23   Scan Code
 ///     Bit24=>SHIFT,  1 => Pressed
 ///     Bit25=>ALT,   1 => Pressed
 ///     Bit26=>CONTROL status 1 ==> Pressed
 ///       Bit27 => Caps lock,  1 => Locked
 ///     Bit28=> Num Lock 1 => Loced
 ///     Bit29-31   reserved
 virtual void OnKeyDown( WORD vtChar, int nFlags );
 virtual void OnChar( WORD vtChar, int nFlags );
 virtual void OnKeyUp( WORD vtChar, int nFlags );
 
 virtual void OnFrontKeyPressed( BYTE byKeyMask, BYTE byOldKeyMask );

 virtual void OnLButtonUp( int x,int y, int nFlags );
 virtual void OnLButtonDown( int x,int y, int nFlags );
 virtual void OnLButtonDbClk( int x, int y, int nFlags ); 
 virtual void OnRButtonUp( int x,int y, int nFlags );
 virtual void OnRButtonDown( int x,int y, int nFlags );
 virtual void OnRButtonDbClk( int x, int y, int nFlags );
 virtual void OnMouseMove( int x, int y, int nFlags );

 virtual void OnTimer( int nEventID );

public:
 int SetTimer( int nEventID, int nElapse );
 void KillTimer( int nEventID );
 
private:
 CMyCommandResponser * m_pParent; 
};

bool PostMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam );
long SendMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam );
CMyCommandResponser * GetActiveResponser();

#ifndef _WIN32
extern "C" DWORD WINAPI GetTickCount();    
extern "C" void WINAPI my_usleep( DWORD dwUS );
#endif //_WIN32


class CMyWinApp
{
public:
    CMyWinApp();
    virtual ~CMyWinApp();

    virtual bool InitInstance();
    virtual int ExitInstance();
 virtual void Run();
 virtual void OnIdle();
 void PumpOneMessage();

 int DoRun();
 
protected:
 virtual void CheckKeyboardMouseInput();
 virtual bool DispatchMessage();
    virtual bool PrepareEnv();
 
 void HandleMouseEvent( DWORD dwFlags, DWORD dwPositon );
};

#pragma pack(pop)

#endif // __MY_WINAPP_H_20050309__
