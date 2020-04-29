// MyWnd.h: interface for the CWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYWND_H__D13A87AB_4EA4_4C1A_9078_26BA27FE94C8__INCLUDED_)
#define AFX_MYWND_H__D13A87AB_4EA4_4C1A_9078_26BA27FE94C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDC;

class CWnd  
{
public:
	CWnd();
	virtual ~CWnd();

public:
	HWND	m_hWnd;	

public:
	static BOOL RegisterWndClass( LPCSTR lpszClassName=NULL, DWORD dwStyle=0 );
	virtual BOOL CreateEx( DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, 
		HINSTANCE hInstance ); 
	virtual BOOL Create( LPCSTR lpClassName, LPCSTR lpWindowName,
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, 
		HINSTANCE hInstance ); 
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL DestroyWindow();	
	BOOL Attach( HWND hWnd );
	HWND Detach();

	BOOL ShowWindow( int nCmdShow );
	BOOL InvalidateRect( const RECT * pRect, BOOL bErease=TRUE);
	BOOL UpdateWindow();
	HWND GetParent();
	BOOL GetClientRect( RECT * lpRect );
	BOOL GetWindowRect( RECT * lpRect );
	BOOL ClientToScreen( RECT * lpRect );
	BOOL ClientToScreen( LPPOINT lpPoint );
	BOOL ScreenToClient( LPPOINT lpPoint );
	BOOL ScreenToClient( RECT * lpRect );	
	BOOL MoveWindow( int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
	BOOL MoveWindow( LPRECT lpRect, BOOL bRepaint = TRUE );
	UINT SetTimer( UINT nIDEvent, UINT nElapse );
	BOOL KillTimer( UINT idTimer );
	HWND GetDlgItem( int nIDDlgItem );
private:	
};

#endif // !defined(AFX_MYWND_H__D13A87AB_4EA4_4C1A_9078_26BA27FE94C8__INCLUDED_)
