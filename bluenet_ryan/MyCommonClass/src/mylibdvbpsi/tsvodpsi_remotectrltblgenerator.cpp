///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2007-1-17
///
///		用途：
///			通视 VOD 点播远程控制表
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// TSVODPSI_RemoteCtrlTblGenerator.cpp: implementation of the CTSVODPSI_RemoteCtrlTblGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tsvodpsi_remotectrltblgenerator.h"
#include "bitstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSVODPSI_RemoteCtrlTblGenerator::CTSVODPSI_RemoteCtrlTblGenerator(BYTE byTableID, bool bDoCompress)
  :CTSVODPSI_TableGeneratorBase( byTableID, bDoCompress )
{
	m_wSID = 0;						// 如果 != 0 ，则表示该表是针对一个SID的，反之，针对一个STB ID
	m_llSTBID = 0;
	m_llEndSTBID = 0;
	
	m_pDataBuf = NULL;
	m_nBufSize = 0;
	m_nDataLen = 0;

	m_nInstructionCount = 0;
	m_dwEncryptParameter = 0;
}

CTSVODPSI_RemoteCtrlTblGenerator::~CTSVODPSI_RemoteCtrlTblGenerator()
{
	if( m_pDataBuf )
		free( m_pDataBuf );
	m_pDataBuf = NULL;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		获取私有数据
/// 输入参数:
///		无
/// 返回参数:
///		无
PBYTE CTSVODPSI_RemoteCtrlTblGenerator::GetPrivateData( int & nOutLen )
{
	int nHeaderLen = 2;						// 固定的比特
	if( m_wSID )
		nHeaderLen += 2;					// SID 长度
	else
	{
		nHeaderLen += 8;					// 特定的STB ID
		if( m_llEndSTBID )
			nHeaderLen += 8;				// 范围 STB ID
	}
	if( m_dwEncryptParameter )
		nHeaderLen += 2;					// 进行加密
	nHeaderLen ++;							// 指令长度

#ifdef _DEBUG
	DWORD dwValueBak = *(PDWORD)(m_pDataBuf + BUFSIZE_FOR_TBL_HEADER);
#endif //_DEBUG

	PBYTE pInstructionCompiledBufPtr = m_pDataBuf + BUFSIZE_FOR_TBL_HEADER - nHeaderLen;
	int nInstructionCompiledLen = m_nDataLen - BUFSIZE_FOR_TBL_HEADER + nHeaderLen;

	CMyBitStream bs( pInstructionCompiledBufPtr, nHeaderLen );
	bs.PutBit( m_wSID ? 1 : 0 );
	bs.PutBit( m_llEndSTBID ? 1 : 0 );
	bs.PutBit( m_dwEncryptParameter ? 1 : 0 );
	bs.PutBits( 0, 13 );

	if( m_wSID )
		bs.PutBits16( m_wSID );
	else 
	{
		bs.PutBits32( (DWORD)( m_llSTBID >> 32 ) );
		bs.PutBits32( (DWORD)m_llSTBID );
		if( m_llEndSTBID )
		{
			bs.PutBits32( (DWORD)( m_llEndSTBID >> 32 ) );
			bs.PutBits32( (DWORD)m_llEndSTBID );
		}
	}

	if( m_dwEncryptParameter )
		bs.PutBits16( (WORD)m_dwEncryptParameter );
	bs.PutBits8( m_nInstructionCount );
	bs.FinishWrite();
	ASSERT( bs.GetTotalWriteBits()/8 == nHeaderLen );

#ifdef _DEBUG
	// 验证是否被破坏
	ASSERT( dwValueBak == *(PDWORD)(m_pDataBuf + BUFSIZE_FOR_TBL_HEADER) );
#endif //_DEBUG

	nOutLen = nInstructionCompiledLen;
	return pInstructionCompiledBufPtr;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		初始化
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSVODPSI_RemoteCtrlTblGenerator::Initialize()
{
	m_nDataLen = 0;
	m_wSID = 0;										// 如果 != 0 ，则表示该表是针对一个SID的，反之，针对一个STB ID
	m_llSTBID = m_llEndSTBID = 0;
	m_nInstructionCount = 0; 

	SetModifyFlag( true );

	m_dwEncryptParameter = 0;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		设置该命令是针对一个节目的
/// 输入参数:
///		wSID				节目号
/// 返回参数:
///		无
void CTSVODPSI_RemoteCtrlTblGenerator::SetSID( WORD wSID )
{
	m_llSTBID = 0;
	m_wSID = wSID;
	
	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		设置该命令针对一个接收终端
/// 输入参数:
///		llSTBID					机顶盒ID
/// 返回参数:
///		true					成功
///		false					失败，已经设置过SID，要先调用 Initialize
void CTSVODPSI_RemoteCtrlTblGenerator::SetSTBID( MY_LONG64 llSTBID, MY_LONG64 llEndSTBID )
{
	m_wSID = 0;

	m_llSTBID = llSTBID;
	m_llEndSTBID = llEndSTBID;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-18
/// 函数功能:
///		设置 STB ID
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSVODPSI_RemoteCtrlTblGenerator::SetSTBID( BYTE abySTBID[] )
{
	m_wSID = 0;

	m_llSTBID = 0;
	for(int i=0; i<8; i++ )
	{
		m_llSTBID <<= 8;
		m_llSTBID += abySTBID[i];
	}

	m_llEndSTBID = 0;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		设置加密参数
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSVODPSI_RemoteCtrlTblGenerator::SetEncryptParameter( WORD wParameter )
{
	// FIXME
	m_dwEncryptParameter = (1<<16) + wParameter;		// 高 16 比特表示是否进行加密
	SetModifyFlag( true );

	ASSERT( FALSE );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		清除所有指令
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSVODPSI_RemoteCtrlTblGenerator::CleanInstructions()
{
	m_nDataLen = BUFSIZE_FOR_TBL_HEADER;	// 保留空间
	m_nInstructionCount = 0;

	SetModifyFlag( true );
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		添加切换频道的指令
/// 输入参数:
///		byChNo				即将切换的频道号
/// 返回参数:
///		指令表总长度
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_SwitchChannel( BYTE byChNo )
{
	m_nInstructionCount ++;
	
	if( false == AcquireMem(3) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+3 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_SWITCH_CHANNEL );
	bs.PutBits8( byChNo );
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == 3 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		添加接收指定节目的命令
/// 输入参数:
///		byPhysNo			物理频道号
///		wPMT_PID			PMT_PID
/// 返回参数:
///		指令表总长度
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_ReceiveProgram( BYTE byPhysNo, WORD wPMT_PID )
{
	m_nInstructionCount ++;
	
	if( false == AcquireMem(5) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+5 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_RECEIVE_PROGRAM );
	bs.PutBits8( byPhysNo );
	bs.PutBits16( wPMT_PID );
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == 5 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		添加：点播命令返回值
/// 输入参数:
///		pBuf				返回值
///		byDataLen			数据长度
/// 返回参数:
///		指令表总长度
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_VODOperatorResponse( PBYTE pBuf, BYTE byDataLen )
{
	ASSERT( pBuf && byDataLen );
	if( NULL == pBuf || 0 == byDataLen )
		return -1;

	m_nInstructionCount ++;
	
	if( false == AcquireMem(byDataLen+3) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+byDataLen+3 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( RCMDID_VOD_OPERATION_RESPONSE );
	bs.PutBits8( byDataLen );
	for(int i=0; i<byDataLen; i++ )
	{
		bs.PutBits8( pBuf[i] );
	}
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == byDataLen+3 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		添加通用命名
/// 输入参数:
///		wCommand			命令
///		pBuf				缓冲区
///		nLen				长度
/// 返回参数:
///		指令表总长度
int CTSVODPSI_RemoteCtrlTblGenerator::AddIns_PrecompiledInstructions( WORD wCommand, PBYTE pBuf, int nLen )
{
	ASSERT( pBuf && nLen );
	if( NULL == pBuf || 0 == nLen )
		return -1;

	m_nInstructionCount ++;
	
	if( false == AcquireMem(nLen+2) )
		return -1;

	ASSERT( m_nBufSize >= m_nDataLen+nLen+2 );

	CMyBitStream bs( m_pDataBuf+m_nDataLen, m_nBufSize-m_nDataLen);
	bs.PutBits16( wCommand );
	for(int i=0; i<nLen; i++ )
	{
		bs.PutBits8( pBuf[i] );
	}
	bs.FinishWrite();

	ASSERT( bs.GetTotalWriteBits()/8 == nLen+2 );

	m_nDataLen += (bs.GetTotalWriteBits()/8);

	SetModifyFlag( true );

	return m_nDataLen;
}

///-------------------------------------------------------
/// CYJ,2007-1-17
/// 函数功能:
///		增加内存
/// 输入参数:
///		nIncBytes			要求新增加的内存
/// 返回参数:
///		true				succ
///		false				no memory
bool CTSVODPSI_RemoteCtrlTblGenerator::AcquireMem( int nIncBytes )
{
	ASSERT( nIncBytes > 0 );
	if( m_nDataLen + nIncBytes < m_nBufSize )
		return true;

	int nNewBufSize;
	if( NULL == m_pDataBuf )
	{
		nNewBufSize = ( BUFSIZE_FOR_TBL_HEADER + nIncBytes + 4095) & (~4095);	// 按4K对齐
		m_pDataBuf = (PBYTE)malloc( nNewBufSize );
		if( m_pDataBuf )
			m_nDataLen = BUFSIZE_FOR_TBL_HEADER;			// 第一次分配，需要保留空间给表头
	}
	else
	{
		nNewBufSize = ( m_nDataLen + nIncBytes + 4095) & (~4095);	// 按4K对齐
		m_pDataBuf = (PBYTE)realloc( m_pDataBuf, nNewBufSize );
	}

	if( m_pDataBuf )
		m_nBufSize = nNewBufSize;
	else
		m_nBufSize = 0;

	return ( NULL != m_pDataBuf );
}

