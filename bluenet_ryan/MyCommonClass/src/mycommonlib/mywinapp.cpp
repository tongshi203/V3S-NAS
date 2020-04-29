///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005.3.9
///
///	    用途：
///         My Windows application base
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"
#include "mywinapp.h"
#include <MyRingPool.h>
#include <sys/time.h>
#include <MyList.h>
#include <stdio.h>

typedef struct tagMYMSG
{
    CMyCommandResponser *   m_pResponser;   // responser, just like hWnd
    int     m_nMessage;     // Message ID, such as WM_KEYDOWN
    int     m_wParam;       // parameter 1
    int     m_lParam;       // parameter 2
}MYMSG,*PMYMSG;

//////////////////////////////////////////////////////////
// 最多可以缓冲1024个消息
// 只用于处理 PostMessage 调用，对于SendMessage，则直接调用相关的 DefWindowProc
#define MAX_MSG_BUF_COUNT   1024
static CMyRingPool<MYMSG>   s_MsgRingBuf;
static CMyCommandResponser * s_ActiveResponser = NULL;

//--------------------------------------------------------------
// 
CMyCommandResponser	* GetActiveResponser()
{
	return s_ActiveResponser;
}

//////////////////////////////////////////////////////////
// CTimerMgr
class CTimerMgr
{
public:
    CTimerMgr();
    ~CTimerMgr();

    int SetTimer( CMyCommandResponser * pResponser, int nIDEvent, int nElapse );
    void KillTimer( CMyCommandResponser * pResponser, int nIDEvent );
    void DoSchedule();

    struct tagTIMER_STRUCT
    {
        CMyCommandResponser * m_pResponser;
        int m_nEventID;
        int m_nElapse;
#ifdef _WIN32
        LONGLONG m_llExpireTick;
#else
        long long m_llExpireTick;
#endif //_WIN32
    };

private:
    CMyList<tagTIMER_STRUCT> m_listTimers;
private:
    int SetTimer( tagTIMER_STRUCT & ItemData );
#ifdef _DEBUG
    void CheckExpireTickOrder();
#endif //_DEBUG
};

#ifndef _WIN32
static DWORD s_dwStartTicks = 0;

extern "C" DWORD WINAPI GetTickCount()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((t.tv_sec * 1000) + (t.tv_usec / 25000) * 25) - s_dwStartTicks;
}
#endif //_WIN32	
	
CTimerMgr::CTimerMgr()
{    
	s_dwStartTicks = 0;
	s_dwStartTicks = GetTickCount();
}

CTimerMgr::~CTimerMgr()
{
}

int CTimerMgr::SetTimer( CMyCommandResponser * pResponser, int nIDEvent, int nElapse )
{
    tagTIMER_STRUCT OneItem;
    OneItem.m_pResponser = pResponser;
    OneItem.m_nEventID = nIDEvent;
    OneItem.m_nElapse = nElapse;

    return SetTimer( OneItem );
}

int CTimerMgr::SetTimer( tagTIMER_STRUCT & ItemData )
{
    if( ItemData.m_pResponser == NULL || ItemData.m_nElapse <= 0 || ItemData.m_nEventID <= 0 )
    {
        ASSERT( 0 );
        return 0;
    }

    KillTimer( ItemData.m_pResponser, ItemData.m_nEventID );

    ItemData.m_llExpireTick = GetTickCount();
    ItemData.m_llExpireTick += ItemData.m_nElapse;

    if( m_listTimers.IsEmpty() )
    {			// empty list, insert directly
        m_listTimers.AddTail( ItemData );
        return ItemData.m_nEventID;
    }

    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        POSITION posOld = pos;
        tagTIMER_STRUCT & ExistItem = m_listTimers.GetNext( pos );
        if( ExistItem.m_llExpireTick <= ItemData.m_llExpireTick )
            continue;			// early expire
        m_listTimers.InsertBefore( posOld, ItemData );

#ifdef _DEBUG
        CheckExpireTickOrder();
#endif //_DEBUG

        return ItemData.m_nEventID;
    }

    // all item's expire tick is less the mine, so add to the tail
    m_listTimers.AddTail( ItemData );

#ifdef _DEBUG
    CheckExpireTickOrder();
#endif //_DEBUG

    return ItemData.m_nEventID;
}

///--------------------------------------------------------------------------
/// CYJ, 2005.3.28
/// Function:
///	Kill Timer
/// Input Parameter:
///	pResponser			responser
///	nIDEvent				event, -1 kill all timer associatived to the pResponser
/// Output:
///	None
void CTimerMgr::KillTimer( CMyCommandResponser * pResponser, int nIDEvent )
{
    if( m_listTimers.IsEmpty() )
        return;
    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        POSITION posOld = pos;
        tagTIMER_STRUCT & OneItem = m_listTimers.GetNext( pos );
        if( OneItem.m_pResponser == pResponser 
			&& ( OneItem.m_nEventID == nIDEvent || -1 == nIDEvent ) )
        {
            m_listTimers.RemoveAt( posOld );
            return;
        }
    }
}

void CTimerMgr::DoSchedule()
{
#ifdef _WIN32
    LONGLONG llCurTick = GetTickCount();
#else
    long long llCurTick = GetTickCount();
#endif //_Win32

    while( false == m_listTimers.IsEmpty() )
    {
        tagTIMER_STRUCT & OneItem = m_listTimers.GetHead();
        if( llCurTick < OneItem.m_llExpireTick )
            return;			// no more event
        // call the event handle
#ifdef _DEBUG
        printf("%p time out, id=%d, %08lu, next expire=%08lu.\n",
            OneItem.m_pResponser, OneItem.m_nEventID, DWORD(OneItem.m_llExpireTick),
            DWORD(llCurTick+OneItem.m_nElapse) );
#endif //_DEBUG

		tagTIMER_STRUCT ItemData = m_listTimers.RemoveHead();
        SetTimer( ItemData );		// 再次启动

		//	在调用 OnTimer 时，该Timer可能被删除，所以需要在调用之前设置Timer
        ASSERT( OneItem.m_pResponser );
        OneItem.m_pResponser->DefWindowProc( WM_TIMER, OneItem.m_nEventID, 0 );        
    }
}

#ifdef _DEBUG
void CTimerMgr::CheckExpireTickOrder()
{
#ifdef _WIN32
    LONGLONG llCurTick = 0;
#else
    long long llCurTick = 0;
#endif //_Win32

    if( m_listTimers.IsEmpty() )
        return;
    POSITION pos = m_listTimers.GetHeadPosition();
    while( pos )
    {
        tagTIMER_STRUCT & OneItem = m_listTimers.GetNext( pos );
        ASSERT( llCurTick <= OneItem.m_llExpireTick );
        llCurTick = OneItem.m_llExpireTick;
    }
}

#endif //_DEBUG

///-------------------------------------------------------
static CTimerMgr   s_TimerMgr;

//////////////////////////////////////////////////////////
/// CYJ,2005.3.9
/// 函数功能:
///		提交延迟处理的消息
/// 输入参数:
///		pWnd            处理的对象
///     nMessage        消息
///     wParam          参数 1
///     lParam          参数 2
/// 返回参数:
///		true            提交成功
///     false           失败
bool PostMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam )
{
#ifdef _DEBUG
    ASSERT( pWnd );
#endif //_DEBUG

    if( NULL == pWnd )
        return false;
    MYMSG * pMsg = s_MsgRingBuf.GetNextWriteUnit();
    if( NULL == pMsg )
        return false;
    pMsg->m_pResponser = pWnd;
    pMsg->m_nMessage = nMessage;
    pMsg->m_wParam = wParam;
    pMsg->m_lParam = lParam;

    s_MsgRingBuf.IncreaseWritePtr();

    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		提交延迟处理的消息
/// 输入参数:
///		pWnd            处理的对象
///     nMessage        消息
///     wParam          参数 1
///     lParam          参数 2
/// 返回参数:
///     消息处理函数返回值
long SendMessage( CMyCommandResponser * pWnd, int nMessage, int wParam, int lParam )
{
    ASSERT( pWnd );
    if( NULL == pWnd )
        return -1;
    return pWnd->DefWindowProc( nMessage, wParam, lParam );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		构造函数
/// 输入参数:
///		x,y         起始坐标，缺省为0
///     cx,cy       宽度、高度，缺省为0，自动获取
/// 返回参数:
///		无
CMyCommandResponser::CMyCommandResponser(int x, int y, int cx, int cy)
{
	m_pParent = s_ActiveResponser;		// 保存当前有效的
	s_ActiveResponser = this;
}

CMyCommandResponser::~CMyCommandResponser()
{
	s_ActiveResponser = m_pParent;
	KillTimer( -1 );		// kill all timer associated with this responser
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		一个键按下
/// 输入参数:
///		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// 返回参数:
///		无
/// 说明：
///     对于ASCII字符，还会紧接着调用 OnChar 函数
void CMyCommandResponser::OnKeyDown( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnKeyDown( vtChar, nFlags );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		输入一个字符
/// 输入参数:
//////		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// 返回参数:
///		无
/// 说明：
///     在此之前，OnKeyDown一定会被调用
void CMyCommandResponser::OnChar( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnChar( vtChar, nFlags );
}

//-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		一个键弹起
/// 输入参数:
///		vtChar                 Virtual Key value
///   		nFlags              	Bit0-Bit14			1
///					Bit15-Bit23			Scan Code
///					Bit24=>SHIFT,		1 => Pressed
///					Bit25=>ALT, 		1 => Pressed
///					Bit26=>CONTROL status	1 ==> Pressed
///				  	Bit27 => Caps lock, 	1 => Locked
///					Bit28=> Num Lock	1 => Loced
///					Bit29-31			reserved
/// 返回参数:
///		无
void CMyCommandResponser::OnKeyUp( WORD vtChar, int nFlags )
{
	if( m_pParent )
		m_pParent->OnKeyUp( vtChar, nFlags );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		左键鼠标被按下后松开
/// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnLButtonUp( int x,int y, int nFlags )
{	
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		左键鼠标被按下后松开
/// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnLButtonDown( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:///		左键鼠标双
// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnLButtonDbClk( int x, int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		左键鼠标被按下后松开
/// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnRButtonUp( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		左键鼠标被按下后松开
/// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnRButtonDown( int x,int y, int nFlags )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:///		左键鼠标双
// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnRButtonDbClk( int x, int y, int nFlags )
{
}

/// CYJ,2005.3.9
/// 函数功能:///		左键鼠标双
// 输入参数:
///		x, y        相对坐标
///     nFlags      shift, control, alt 的状态
/// 返回参数:
///		无
void CMyCommandResponser::OnMouseMove( int x, int y, int nFlags )
{
}

///---------------------------------------------------------------------------
/// CYJ,	2005.3.28
/// Function:
///		On Front key pressed
/// Input Parameter:
///		byKeyMask		key bit mask, bit0~bit6 	for combination of the front key, and bit7 = 1 mean this key is repeated
///		byOldKeyMask		old key bit mask
/// Output Paramere:
///		None
void CMyCommandResponser::OnFrontKeyPressed( BYTE byKeyMask, BYTE byOldKeyMask )
{
	if( m_pParent )
		m_pParent->OnFrontKeyPressed( byKeyMask, byOldKeyMask );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		定时器
/// 输入参数:
///		nEventID        时间类型，同SetTimer 的返回值
/// 返回参数:
///		无
void CMyCommandResponser::OnTimer( int nEventID )
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		设置定时器
/// 输入参数:
///		nEventID       定时器事件
///     nElapse        定时周期，单位:毫秒
/// 返回参数:
///		>=0            成功
///     <0             失败
int CMyCommandResponser::SetTimer( int nEventID, int nElapse )
{
    ASSERT( nElapse > 0 );
    return s_TimerMgr.SetTimer( this, nEventID, nElapse );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		删除一个定时器
/// 输入参数:
///		nEventID        定时器时间，由SetTimer返回
/// 返回参数:
///		无
void CMyCommandResponser::KillTimer( int nEventID )
{
    s_TimerMgr.KillTimer( this, nEventID );
}

///-------------------------------------------------------
/// CYJ,2005.3.10
/// 函数功能:
///	缺省的窗口消息处理函数
/// 输入参数:
///	无
/// 返回参数:
///	无
long CMyCommandResponser::DefWindowProc( int nMessage, int wParam, int lParam )
{
    switch( nMessage )
    {
    case WM_TIMER:
        OnTimer( wParam );
        break;
    case WM_KEYDOWN:
        OnKeyDown( (WORD)wParam, lParam );
        break;
    case WM_CHAR:
        OnChar( char(wParam), lParam );
        break;
    }
    return 0;
}

///-------------------------------------------------------
/// CYJ,2005.3.10
/// 函数功能:
///	递交消息
/// 输入参数:
///	无
/// 返回参数:
///	无
bool CMyCommandResponser::PostMessage( int nMessage, int wParam, int lParam )
{
    return ::PostMessage( this, nMessage, wParam, lParam );
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///	    发送消息
/// 输入参数:
///	    nMessage            消息
///     wParam              参数 1
///     lParam              参数 2
/// 返回参数:
///	    返回值
long CMyCommandResponser::SendMessage( int nMessage, int wParam, int lParam )
{
    return DefWindowProc( nMessage, wParam, lParam );
}


///////////////////////////////////////////////////////////////
/// CMyWinApp
CMyWinApp::CMyWinApp()
{
}

CMyWinApp::~CMyWinApp()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		初始化实例
/// 输入参数:
///		无
/// 返回参数:
///		true            继续执行
///     false           退出执行程序
bool CMyWinApp::InitInstance()
{	
    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		退出前的通知，用于释放资源
/// 输入参数:
///		无
/// 返回参数:
///		程序的返回值
int CMyWinApp::ExitInstance()
{
    return 0;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		程序的运行主体，主要进行事件调度。一般情况下不需要接管该函数
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyWinApp::Run()
{
    while( DispatchMessage() )
    {
        s_TimerMgr.DoSchedule();        // 调度定时器
        CheckKeyboardMouseInput();      // 检查键盘、鼠标输入

        OnIdle();
    }
}

void CMyWinApp::PumpOneMessage()
{
   	if( false == DispatchMessage() )
		::PostMessage( NULL, WM_QUIT, 0, 0 );	// 读取到退出请求，退回该消息
		
	s_TimerMgr.DoSchedule();        // 调度定时器
	CheckKeyboardMouseInput();      // 检查键盘、鼠标输入
}

///-------------------------------------------------------
/// CYJ,2005.3.11
/// 函数功能:
///	派发消息
/// 输入参数:
///	无
/// 返回参数:
///	true            继续运行
///     false           接收到 WM_QUIT 消息，应该退出程序
bool CMyWinApp::DispatchMessage()
{
    PMYMSG pMsg;
    while( (pMsg = s_MsgRingBuf.GetCurrentReadUnit() ) )
    {
        if( pMsg->m_nMessage == WM_QUIT )
            return false;           // 请求退出
        if( pMsg->m_pResponser )
            pMsg->m_pResponser->DefWindowProc( pMsg->m_nMessage, pMsg->m_wParam, pMsg->m_lParam );

        s_MsgRingBuf.Skip( 1 );     // 该消息已经处理。
    }

    return true;
}

///-------------------------------------------------------
/// CYJ,
/// 函数功能:
///		空闲时调用
/// 输入参数:
///		无
/// 返回参数:
///		无
void CMyWinApp::OnIdle()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		进入循环，一般在 main 函数中运行
/// 输入参数:
///		无
/// 返回参数:
///		无
int CMyWinApp::DoRun()
{
    if( false == PrepareEnv() )
        return -1;
    if( InitInstance() )
    	Run();
    return ExitInstance();
}


///-------------------------------------------------------
/// CYJ,2005.3.11
/// 函数功能:
///	准备环境
/// 输入参数:
///	无
/// 返回参数:
///	无
bool CMyWinApp::PrepareEnv()
{
	s_ActiveResponser = NULL;
    if( false == s_MsgRingBuf.Initialize(MAX_MSG_BUF_COUNT) )
        return false;

    return true;
}

///-------------------------------------------------------
/// CYJ,2005.3.9
/// 函数功能:
///		查询键盘、鼠标、定时器事件
/// 输入参数:
///		无
/// 返回参数:
///		true               exist the application
///     false
void CMyWinApp::CheckKeyboardMouseInput()
{
}

///-------------------------------------------------------
/// CYJ,2005.3.28
/// 函数功能:
///		处理鼠标消息
/// 输入参数:
// 		dwFlags			Bit0 Left button down, 
//						Bit1 Right button
//						Bit2 Shift is Pressed, 
//						Bit3 Contrl is pressed
//						Bit4 Mid button down
//						高16比特为上次的状态
//		dwPositon			高16比特为x坐标，低16比特为y坐标
/// 返回参数:
///		None
void CMyWinApp::HandleMouseEvent( DWORD dwFlags, DWORD dwPositon )
{
	if( NULL == s_ActiveResponser )
		return;	
	DWORD dwNow = GetTickCount();
	
	DWORD dwLastStatus = dwFlags >> 16;
	dwLastStatus ^= dwFlags;
	
#define DOUBLE_CLICK_MS		500

	if( (dwLastStatus & 3) == 0 )
		s_ActiveResponser->OnMouseMove( dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
	else
	{
		if( dwLastStatus & 1 )
		{				// Left button action
			static DWORD dwLastLeftButtonAction = 0;
			if( dwFlags & 1 )
			{			// Left Button down
				if( DWORD(dwNow - dwLastLeftButtonAction) < DOUBLE_CLICK_MS )
					s_ActiveResponser->OnLButtonDbClk(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
				else
					s_ActiveResponser->OnLButtonDown(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}
			else
			{			// Left button Up
				s_ActiveResponser->OnLButtonUp(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}	
			dwLastLeftButtonAction = dwNow;
		}
		if( dwLastStatus & 2 )
		{				// right button action
			static DWORD dwLastRightButtonAction = 0;
			if( dwFlags & 2 )
			{			// Right Button down
				if( DWORD(dwNow - dwLastRightButtonAction) < DOUBLE_CLICK_MS )
					s_ActiveResponser->OnRButtonDbClk(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
				else
					s_ActiveResponser->OnRButtonDown(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}
			else
			{			// Right button Up
				s_ActiveResponser->OnRButtonUp(dwPositon>>16,dwPositon&0xFFFF,dwFlags&0xFFFF );
			}	
			dwLastRightButtonAction = dwNow;
		}		
	}
}
