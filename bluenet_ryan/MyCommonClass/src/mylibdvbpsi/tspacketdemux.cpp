///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-12
///
///		用途：
///			TS 流解复用
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"
#include "tspacketdemux.h"
#include <MyMap.h>

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CPID_ResponserMapper : public CMyMap<WORD,WORD,CTSPacketResponser*,CTSPacketResponser*>
{
public:
	CPID_ResponserMapper(){}
	virtual ~CPID_ResponserMapper(){}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSPacketDemux::CTSPacketDemux()
{
	m_pMapper = new CPID_ResponserMapper;
}

CTSPacketDemux::~CTSPacketDemux()
{
	if( m_pMapper )
		delete m_pMapper;
	m_pMapper = NULL;
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		接收到一个TS分组，找出其中的处理对象，并调用之
/// 输入参数:
///		pPacket			TS 分组
/// 返回参数:
///		无
void CTSPacketDemux::PushOneTSPacket(PDVB_TS_PACKET pPacket)
{
	ASSERT( pPacket && m_pMapper );
	if( NULL == pPacket )
		return;
	WORD wPID = pPacket->GetPID();
	if( INVALID_PID == wPID )
		return;					//	无效分组

	CTSPacketResponser * pResponser = NULL;
	if( FALSE == m_pMapper->Lookup( wPID, pResponser ) || NULL == pResponser )
		return;					//	没有处理器
	pResponser->PushOneTSPacket( pPacket );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		注册PID处理器
/// 输入参数:
///		wPID				待处理的PID
///		pResponser			处理对象
/// 返回参数:
///		无
void CTSPacketDemux::RegisterResponser(WORD wPID, CTSPacketResponser *pResponser)
{
	ASSERT( wPID != INVALID_PID && pResponser && m_pMapper );
	if( wPID == INVALID_PID || NULL == pResponser || NULL == m_pMapper )
		return;
	m_pMapper->SetAt( wPID, pResponser );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		注销TS分组响应器
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSPacketDemux::DeregisterResponser(WORD wPID)
{
	ASSERT( m_pMapper );
	if( m_pMapper )
		m_pMapper->RemoveKey( wPID );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		删除所有对象（响应器）
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSPacketDemux::Reset()
{
	ASSERT( m_pMapper );
	if( m_pMapper )
		m_pMapper->RemoveAll();
}
