// MyWnd.cpp: implementation of the CWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mywnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK MyWindowProc( HWND hWnd, UINT uMsg,WPARAM wParam, LPARAM lParam );
#define 	MYWND_CLASS_NAME	"MY_WIN_CLASS"

BOOL CWnd::RegisterWndClass( LPCSTR lpszClassName, DWORD dwStyle )
{
	if( NULL == lpszClassName || 0 == *lpszClassName )
		lpszClassName = MYWND_CLASS_NAME;
	if( 0 == dwStyle )
		dwStyle = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		
	WNDCLASS wndclass;
	wndclass.style          = dwStyle;
	wndclass.lpfnWndProc    = MyWindowProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = 0;
	wndclass.hIcon          = 0;
	wndclass.hCursor        = 0;
	wndclass.hbrBackground  =(HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszMenuName   =(const char*)NULL;
	wndclass.lpszClassName  = lpszClassName;
	
	return RegisterClass(&wndclass);			
}

CWnd::CWnd()
{
	m_hWnd = (HWND)NULL;
}

CWnd::~CWnd()
{
	if( m_hWnd && IsWindow(m_hWnd) )
		::DestroyWindow( m_hWnd );
	m_hWnd = NULL;
}

BOOL CWnd::DestroyWindow()
{
	if( m_hWnd )
	{		
		BOOL bRetVal = ::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
		return bRetVal;
	}
	return TRUE;
}

BOOL CWnd::ShowWindow( int nCmdShow )
{
	return ::ShowWindow( m_hWnd, nCmdShow );
}

BOOL CWnd::InvalidateRect( const RECT * pRect, BOOL bErease)
{
	return ::InvalidateRect( m_hWnd, pRect, bErease );
}

BOOL CWnd::UpdateWindow()
{
	return ::UpdateWindow( m_hWnd );
}

HWND CWnd::GetParent()
{
	return ::GetParent( m_hWnd );
}

BOOL CWnd::GetClientRect( RECT * lpRect )
{
	return ::GetClientRect( m_hWnd, lpRect );
}

BOOL CWnd::GetWindowRect( RECT * lpRect )
{
	return ::GetWindowRect( m_hWnd, lpRect );
}

BOOL CWnd::ClientToScreen( RECT * lpRect )
{
	POINT ptTmp;
	ptTmp.x = lpRect->left;
	ptTmp.y = lpRect->top;
	if( FALSE == ::ClientToScreen( m_hWnd, &ptTmp ) )
		return FALSE;
	lpRect->left = ptTmp.x;
	lpRect->top = ptTmp.y;

	ptTmp.x = lpRect->right;
	ptTmp.y = lpRect->bottom;
	if( FALSE == ::ClientToScreen( m_hWnd, &ptTmp ) )
		return FALSE;
	lpRect->right = ptTmp.x;
	lpRect->bottom = ptTmp.y;

	return TRUE;
}

BOOL CWnd::ClientToScreen( LPPOINT lpPoint )
{
	return ::ClientToScreen( m_hWnd, lpPoint );
}

BOOL CWnd::ScreenToClient( LPPOINT lpPoint )
{
	return ::ScreenToClient( m_hWnd, lpPoint );
}

BOOL CWnd::ScreenToClient( RECT * lpRect )
{
	POINT ptTmp;
	ptTmp.x = lpRect->left;
	ptTmp.y = lpRect->top;
	if( FALSE == ::ScreenToClient( m_hWnd, &ptTmp ) )
		return FALSE;
	lpRect->left = ptTmp.x;
	lpRect->top = ptTmp.y;

	ptTmp.x = lpRect->right;
	ptTmp.y = lpRect->bottom;
	if( FALSE == ::ScreenToClient( m_hWnd, &ptTmp ) )
		return FALSE;
	lpRect->right = ptTmp.x;
	lpRect->bottom = ptTmp.y;

	return TRUE;
}

BOOL CWnd::MoveWindow( int x, int y, int nWidth, int nHeight, BOOL bRepaint )
{
	return ::MoveWindow( m_hWnd, x, y, nWidth, nHeight, bRepaint );
}

BOOL CWnd::MoveWindow( LPRECT lpRect, BOOL bRepaint )
{
	return ::MoveWindow( m_hWnd, lpRect->left, lpRect->top, \
		lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bRepaint );
}

UINT CWnd::SetTimer( UINT nIDEvent, UINT nElapse )
{
	return ::SetTimer( m_hWnd, nIDEvent, nElapse, (TIMERPROC)NULL );
}

BOOL CWnd::KillTimer( UINT idTimer )
{
	return ::KillTimer( m_hWnd, idTimer );
}

HWND CWnd::GetDlgItem( int nIDDlgItem )
{
	return ::GetDlgItem( m_hWnd, nIDDlgItem );
}

static LRESULT CALLBACK MyWindowProc( HWND hWnd, UINT uMsg,WPARAM wParam, LPARAM lParam )
{
	CWnd * pThis = NULL;
	if( WM_CREATE == uMsg )
	{
		CREATESTRUCT * pcs = (CREATESTRUCT *)lParam;
		if( pcs )
		{
			pThis = (CWnd*)pcs->lpCreateParams;
			pThis->m_hWnd = hWnd;
		}
	}
	else
		pThis = (CWnd*)::GetWindowLong( hWnd, GWL_USERDATA );
		
	if( NULL == pThis )
		return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
		
	return pThis->WindowProc( uMsg, wParam, lParam );
}

LRESULT CWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc( message, wParam, lParam );
}

LRESULT CWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::DefWindowProc( m_hWnd, message, wParam, lParam );
}

BOOL CWnd::CreateEx( DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, \
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, \
		HINSTANCE hInstance )
{
	if( NULL == lpClassName || 0 == *lpClassName )
		lpClassName = MYWND_CLASS_NAME;
	m_hWnd = ::CreateWindowEx( dwExStyle, lpClassName, lpWindowName, dwStyle,\
		x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, (LPVOID)this );
	if( NULL == m_hWnd )
		return FALSE;
	SetWindowLong( m_hWnd, GWL_USERDATA, (DWORD)this );
	return TRUE;
}

BOOL CWnd::Create( LPCSTR lpClassName, LPCSTR lpWindowName, \
		DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, \
		HINSTANCE hInstance )
{
	return CreateEx( 0L, lpClassName, lpWindowName, dwStyle,\
		x, y, nWidth, nHeight, hWndParent, hMenu, hInstance );
}
