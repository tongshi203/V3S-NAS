///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			DVB PSI Tables 解析
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"

#ifdef _DEBUG
	#include <stdio.h>
#endif //_DEBUG

#include "dvbpsitables.h"
#include "dvb_crc.h"
#include "dvbdescriptors.h"

#ifdef _WIN32
	#include <MyCommonToolsLib.h>
	#include <zlib/zlib.h>
#else
	#include <zlib.h>
	#define _timezone timezone
#endif // _WIN32

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
	#undef __ENABLE_TRACE__
//	#define __ENABLE_TRACE__
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static PDVBPSI_DECODED_DESCRIPTOR_BASE DecodeDescriptors(PBYTE pBuf, int nLen, int & nCount, CMyHeap & MemAllocator );

CDVBPSITablesBase::CDVBPSITablesBase(int nMaxTableSize)
	: CMyHeap( nMaxTableSize )
{
	Reset();
}

CDVBPSITablesBase::~CDVBPSITablesBase()
{
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		添加一个TS分组
/// 输入参数:
///		pPacket			TS 分组
/// 返回参数:
///		无
void CDVBPSITablesBase::PushOneTSPacket(PDVB_TS_PACKET pPacket)
{
	ASSERT( IsValid() );

	bool bIsPayloadStart = pPacket->IsPayloadUnitStart();

	if( (!m_bHeaderReceived && !bIsPayloadStart) || pPacket->IsError() )
		return;					//	没有接收到数据头，且不是一个表的开始，

	int nDataLen;
	PBYTE pPayloadData = pPacket->GetPayloadData( nDataLen );
	if( NULL == pPayloadData )
		return;					//	没有数据

	BYTE byContinuityCounter = pPacket->GetContinuityCount();

	//	FIXME, 有必要判断TS分组是否重复
	if( m_byExpectTSContinuityCounter == ((byContinuityCounter+1)&0xF) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITablesBase::PushOneTSPacket, find one duplicated TS packeted.\n");
#endif // __ENABLE_TRACE__
		return;					//	重复的TS分组
	}

	if( bIsPayloadStart )
	{							//	接收到一个新表
		BYTE byPointerField = *pPayloadData;
		if( byPointerField )
			Write( pPayloadData+1, byPointerField );			//	结束原来的表

		if( m_bHeaderReceived && 0 == m_wErrorCounter && GetMemoryAllocated() > 0 )	// 没有发生错误，且成功接收到数据
			OnTableReceived();	//	通知接收到表
		Reset();				//	重新开始
		nDataLen -= (byPointerField+1);
//		ASSERT( nDataLen > 0 );
		if( nDataLen < 0 )
			return;
		pPayloadData += ( byPointerField+1 );
		m_bHeaderReceived = true;
	}
	if( false==bIsPayloadStart && m_byExpectTSContinuityCounter != byContinuityCounter )
	{
		m_wErrorCounter ++;
		m_bHeaderReceived = false;
		return;					//	发生错误，放弃此次接收
	}
	m_byExpectTSContinuityCounter = (byContinuityCounter+1) & 0xF;

	if( false == Write( pPayloadData, nDataLen ) )
	{
		m_wErrorCounter ++;
		m_bHeaderReceived = false;		//	写入数据失败，放弃接收
	}

	if( 0 == m_wErrorCounter && IsPSIPacketIntegral() )
	{							//	接收到一个完整的PSI表
		OnTableReceived();		//	通知接收到表
		Reset();				//	重新开始
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		复位表结构，相对于初始化
/// 输入参数:
///		bForce			是否强制复位，缺省为false
/// 返回参数:
///		无
void CDVBPSITablesBase::Reset(bool bForce)
{
	m_bHeaderReceived = false;
	m_wErrorCounter = 0;					// TS packet 发生错误的次数
	m_byExpectTSContinuityCounter = 0xF0;	// 上次 TS packet 的continuity counter

	CMyHeap::Reset();

	if( bForce )
		CDVBSectionReceivingLog::Reset();
}

CDVBSectionReceivingLog::CDVBSectionReceivingLog()
{
	Reset();
}

CDVBSectionReceivingLog::~CDVBSectionReceivingLog()
{
}

void CDVBSectionReceivingLog::Reset()
{
	memset( m_adwSectionRecLog, 0, sizeof(m_adwSectionRecLog) );		// 8*4*8 = 256，用于记录Section是否接收到
	m_nSectionCount = 0;				// section 个数，最大256
}

// 清除section记录状态
void CDVBSectionReceivingLog::ClearSectionLog()
{
	memset( m_adwSectionRecLog, 0, sizeof(m_adwSectionRecLog) );		// 8*4*8 = 256，用于记录Section是否接收到
}

static DWORD	s_adwBitShiftter[32]=
{
	  1,		  2,		  4,	   	  8,
	0x10,		0x20,		0x40,		0x80,
	0x100,		0x200,		0x400,		0x800,
	0x1000,		0x2000,		0x4000,		0x8000,
	0x10000,	0x20000,	0x40000,	0x80000,
	0x100000,	0x200000,	0x400000,	0x800000,
	0x1000000,	0x2000000,	0x4000000,	0x8000000,
	0x10000000,	0x20000000,	0x40000000,	0x80000000
};
extern unsigned int g_bitstream_bit_msk[33];

///-------------------------------------------------------
/// CYJ,2005-3-4
/// 函数功能:
///		设置接收状态
/// 输入参数:
///		nSectionNo			序号
///		bValue				数值，缺省为1
/// 返回参数:
///		无
void CDVBSectionReceivingLog::SetSectionNoStatus(int nSectionNo, bool bValue )
{
	nSectionNo &= 0xFF;
	int nIndex = nSectionNo / 32;
	DWORD dwMask = s_adwBitShiftter[ nSectionNo&0x1F ];
	if( bValue )
		m_adwSectionRecLog[nIndex] |= dwMask;
	else
		m_adwSectionRecLog[nIndex] &= (~dwMask);
}


///-------------------------------------------------------
/// CYJ,2005-3-4
/// 函数功能:
///		判断释放接收到
/// 输入参数:
///		nSectionNo				序号
/// 返回参数:
///		true					已经接收
///		false					没有接收
bool CDVBSectionReceivingLog::IsSectionReceived( int nSectionNo )
{
	nSectionNo &= 0xFF;
	int nIndex = nSectionNo / 32;
	DWORD dwMask = s_adwBitShiftter[ nSectionNo&0x1F ];
	return ( m_adwSectionRecLog[nIndex] & dwMask ) != 0;
}

///-------------------------------------------------------
/// CYJ,2005-3-4
/// 函数功能:
///		判断全部接收
/// 输入参数:
///		无
/// 返回参数:
///		true				全部接收
///		false				没有接收
bool CDVBSectionReceivingLog::IsAllSectionReceived()
{
	int nCount = m_nSectionCount;
	int nIndex = 0;
	while( nCount >= 32 )
	{
		if( m_adwSectionRecLog[nIndex] != 0xFFFFFFFF )
			return false;
		m_nSectionCount -= 32;
		nIndex ++;
	}
	return ( (m_adwSectionRecLog[nIndex]&g_bitstream_bit_msk[nCount]) == g_bitstream_bit_msk[nCount] );
}

///-------------------------------------------------------
/// CYJ,2005-3-4
/// 函数功能:
///		设置section个数
/// 输入参数:
///		nSectionCount			section 个数
/// 返回参数:
///		无
void CDVBSectionReceivingLog::SetSectionCount(int nSectionCount)
{
	m_nSectionCount = nSectionCount;
	memset( m_adwSectionRecLog, 0, sizeof(m_adwSectionRecLog) );		// 8*4*8 = 256，用于记录Section是否接收到
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		判断一个对象是否有效
/// 输入参数:
///		无
/// 返回参数:
///		true			有效
///		false			无效
bool CDVBPSITablesBase::IsValid()
{
	return CMyHeap::IsValid();
}

/////////////////////////////////////////////////////////////////////
/// PAT 表
/////////////////////////////////////////////////////////////////////

CDVBPSITable_PAT::CDVBPSITable_PAT()
   :CDVBPSITablesBase(PAT_TABLE_MAX_SIZE),
   m_PatTable( PAT_TABLE_DECODED_MAX_SIZE )
{

}

CDVBPSITable_PAT::~CDVBPSITable_PAT()
{
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		获取分组长度
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_PAT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		接收到PAT表，需要进行解释
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_PAT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
		return;

//	ASSERT( *pSrcBuf == DVBPSI_TBLID_PAT );
	if( *pSrcBuf != DVBPSI_TBLID_PAT )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PAT: Not a PAT table, tableId=%d != 0\n", *pSrcBuf );
#endif // __ENABLE_TRACE__
		return;						//	不是 PAT 表
	}
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PAT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0xC0 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PAT: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PAT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_PAT pPATTble = (PDVB_PSI_TABLE_PAT)GetTableLastReceived();
	if( pPATTble && pPATTble->m_byVersionNumber == (( pSrcBuf[5] >> 1) & 0x1F ) )
	{							//	相同的PAT，跳过
		if( IsAllSectionReceived() )
		{		// 所有
			#ifdef __ENABLE_TRACE__
			//	TRACE("CDVBPSITable_PAT: Same version, skip\n");
			#endif // __ENABLE_TRACE__

			return;
		}
	}
	else
	{
		pPATTble = NULL;					// 重新开始接收
		m_PatTable.Reset();
		SetSectionCount( pSrcBuf[7]+1 );	// 设置section个数
	}

	if( NULL == pPATTble )
	{
		// 2016.12.18 CYJ Modify, using size of instead of instance number
		const int nOneItemSize = sizeof( DVB_PSI_TABLE_PAT::tagOneProgramItem );
		pPATTble = (PDVB_PSI_TABLE_PAT) m_PatTable.Allocate( sizeof(DVB_PSI_TABLE_PAT) + PAT_TABLE_MAX_PROGRAM*nOneItemSize );
		if( NULL == pPATTble )
		{
			m_PatTable.Reset();						//	准备环境，开始解码
			return;
		}

		pPATTble->m_wCount = 0;
		pPATTble->m_byTableID = DVBPSI_TBLID_PAT;
		pPATTble->m_wTSID = ( pSrcBuf[3] << 8 ) | pSrcBuf[4];
		pPATTble->m_byVersionNumber = ( pSrcBuf[5] >> 1) & 0x1F;
		pPATTble->m_bCurrentNextIndicator = pSrcBuf[5] & 1;
		pPATTble->m_bySectionNumber = 0;			// 自己管理的，仅有一个
		pPATTble->m_byLastSectionNumber = 0;
	}

	SetSectionNoStatus( pSrcBuf[6] );		// 标记当前帧接收到

	int nCount = (wSectionLen-9)/4;

	PBYTE pStartBuf = pSrcBuf + 8;
	for(int i=0; i<nCount; i++)
	{
		WORD wSID = (pStartBuf[0] << 8) | pStartBuf[1];
		WORD wPMT_PID = ( (pStartBuf[2]&0x1F) << 8) | pStartBuf[3];
		int nIndex = pPATTble->FindPMT_PID( wSID );
		if( nIndex < 0 )
		{						//  不存在，新生成一个
			nIndex = pPATTble->m_wCount;
			if( nIndex >= PAT_TABLE_MAX_PROGRAM )
				break;			// 已经满
			pPATTble->m_wCount ++;
		}
		if( nIndex >= 0 )
		{
			pPATTble->m_aPrograms[nIndex].m_wSID = wSID;
			pPATTble->m_aPrograms[nIndex].m_wPMT_PID = wPMT_PID;
		}

		pStartBuf += 4;
	}

	// 已经接收到的节目表总长度
	pPATTble->m_dwTableSize = sizeof(DVB_PSI_TABLE_PAT) + pPATTble->m_wCount*sizeof(pPATTble->m_aPrograms[0]) - 4;

	OnPATReceived( pPATTble );
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		获取接收到的PAT表
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSITable_PAT::GetTableLastReceived()
{
	if( m_PatTable.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_PAT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_PatTable.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		复位并初始化所有数据
/// 输入参数:
///		bForce			是否强制复位，缺省为 false
/// 返回参数:
///		无
void CDVBPSITable_PAT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset(bForce);
	if( bForce )
		m_PatTable.Reset();
}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		判断对象是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_PAT::IsValid()
{
	if( false == CDVBPSITablesBase::IsValid() )
		return false;
	return m_PatTable.IsValid();
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	Function:
///		成功接受到 PAT 表
///	Input Parameter:
///		None
///	Output Parameter:
///		None
void CDVBPSITable_PAT::OnPATReceived( const PDVB_PSI_TABLE_PAT pTable )
{
#ifdef __ENABLE_TRACE__
	Dump();
#endif //__ENABLE_TRACE__
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		显示
/// 输入参数:
///		fOutput				输出文件句柄
/// 返回参数:
///		无
void CDVBPSITable_PAT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;

	fprintf(fOutput, "Dumping PAT:==========================================\n");

	PDVB_PSI_TABLE_PAT pTable = (PDVB_PSI_TABLE_PAT)GetTableLastReceived();
	ASSERT( pTable );

	fprintf(fOutput,  "Table id=%d, size decoded=%d\n", pTable->m_byTableID, pTable->m_dwTableSize );
	fprintf(fOutput, "Transport stream id=%d\n", pTable->m_wTSID);
	fprintf(fOutput, "VersionNumber=%d, CurrentNextIndicator=%d,SectionNumber=%d\n",
		pTable->m_byVersionNumber, pTable->m_bCurrentNextIndicator, pTable->m_bySectionNumber  );
	fprintf(fOutput, "LastSectionNumber=%d, nCount=%d\n" , pTable->m_byLastSectionNumber, pTable->m_wCount );

	for(int i=0; i<pTable->m_wCount; i++)
	{
		fprintf(fOutput, "[%d]  SID=%d, PID=%d\n", i, pTable->m_aPrograms[i].m_wSID, pTable->m_aPrograms[i].m_wPMT_PID );
	}
#else
	(void)fOutput;
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////////
// CAT
//////////////////////////////////////////////////////////////////////////

//------------------------------------------------
//	CAT 表
CDVBPSITable_CAT::CDVBPSITable_CAT()
	:CDVBPSITablesBase(CAT_TABLE_MAX_SIZE),
     m_CatTable( CAT_TABLE_DECODED_MAX_SIZE )
{

}

CDVBPSITable_CAT::~CDVBPSITable_CAT()
{

}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		TABLE 接收完成，需要进行解析
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSITable_CAT::GetTableLastReceived()
{
	if( m_CatTable.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_CAT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_CatTable.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		复位
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_CAT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset(bForce);
	if( bForce )
		m_CatTable.Reset();
}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_CAT::IsValid()
{
	if( false == CDVBPSITablesBase::IsValid() )
		return false;
	return m_CatTable.IsValid();
}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		CAT 表接收到
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_CAT::OnCATReceived( const PDVB_PSI_TABLE_CAT pTable )
{
#ifdef __ENABLE_TRACE__
	Dump();
#endif //__ENABLE_TRACE__
}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		接收到数据表
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_CAT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个CAT表至少需要12字节
		return;

//	ASSERT( *pSrcBuf == DVBPSI_TBLID_CA );
	if( *pSrcBuf != DVBPSI_TBLID_CA )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_CAT: Not a CA session table, tableId=%d != 0\n", *pSrcBuf );
#endif // __ENABLE_TRACE__
		return;						//	不是 CAT 表
	}
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_CAT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0xC0 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_CAT: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PAT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_CAT pCATTble = (PDVB_PSI_TABLE_CAT)GetTableLastReceived();
	if( pCATTble && pCATTble->m_byVersionNumber == (( pSrcBuf[5] >> 1) & 0x1F ) )
	{							//	相同的CAT，跳过
		if( IsAllSectionReceived() )
		{		// 所有
			#ifdef __ENABLE_TRACE__
			//	TRACE("CDVBPSITable_PAT: Same version, skip\n");
			#endif // __ENABLE_TRACE__

			return;
		}
	}
	else
	{
		pCATTble = NULL;					// 重新开始接收
		m_CatTable.Reset();
		SetSectionCount( pSrcBuf[7]+1 );	// 设置section个数
	}

	if( NULL == pCATTble )
	{
		pCATTble = (PDVB_PSI_TABLE_CAT) m_CatTable.Allocate( sizeof(DVB_PSI_TABLE_CAT) );
		if( NULL == pCATTble )
		{
			m_CatTable.Reset();						//	准备环境，开始解码
			return;
		}

		pCATTble->m_wCount = 0;
		pCATTble->m_byTableID = DVBPSI_TBLID_CA;
		pCATTble->m_byVersionNumber = ( pSrcBuf[5] >> 1) & 0x1F;
		pCATTble->m_bCurrentNextIndicator = pSrcBuf[5] & 1;
		pCATTble->m_bySectionNumber = 0;			// 自己管理的，仅有一个
		pCATTble->m_byLastSectionNumber = 0;
		pCATTble->m_pDescriptor = NULL;
	}

	SetSectionNoStatus( pSrcBuf[6] );		// 标记当前帧接收到

	PBYTE pStartBuf = pSrcBuf + 8;
	int nByteLen = wSectionLen - (5+4);			//  CYJ,2009-9-2; 5 bytes header + 4 byte crc
	int nTblCount = 0;
	pCATTble->m_pDescriptor = DecodeDescriptors( pStartBuf, nByteLen, nTblCount, m_CatTable );
	pCATTble->m_wCount = nTblCount;

	// 已经接收到的节目表总长度
	pCATTble->m_dwTableSize = m_CatTable.GetMemoryAllocated();

	OnCATReceived( pCATTble );
}

///-------------------------------------------------------
/// CYJ,2007-3-27
/// 函数功能:
///		是否完整
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_CAT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个CAT表至少需要12字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_CAT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;

	fprintf(fOutput, "Dumping CAT:==========================================\n");

	PDVB_PSI_TABLE_CAT pTable = (PDVB_PSI_TABLE_CAT)GetTableLastReceived();
	ASSERT( pTable );

	fprintf(fOutput,  "Table id=%d, size decoded=%d\n", pTable->m_byTableID, pTable->m_dwTableSize );
	fprintf(fOutput, "VersionNumber=%d, CurrentNextIndicator=%d,SectionNumber=%d\n",
		pTable->m_byVersionNumber, pTable->m_bCurrentNextIndicator, pTable->m_bySectionNumber  );
	fprintf(fOutput, "LastSectionNumber=%d, nCount=%d\n" , pTable->m_byLastSectionNumber, pTable->m_wCount );
#else
	(void)fOutput;
#endif //_DEBUG
}



//////////////////////////////////////////////////////
//	PMT 表
//////////////////////////////////////////////////////
CDVBPSITable_PMT::CDVBPSITable_PMT()
   :CDVBPSITablesBase(PMT_TABLE_MAX_SIZE),
   m_PmtTable( PMT_TABLE_DECODED_MAX_SIZE )
{
	m_wDstSID = INVALID_ID;
}

CDVBPSITable_PMT::~CDVBPSITable_PMT()
{
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		获取分组长度
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_PMT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		接收到数据表，需要进行解码
///	输入参数:
///		无
///	返回参数:
///		无
void CDVBPSITable_PMT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
		return;

//	ASSERT( *pSrcBuf == DVBPSI_TBLID_PMT );
	if( *pSrcBuf != DVBPSI_TBLID_PMT )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PMT: Not a PMT table, tableId=%d != %d\n", *pSrcBuf, DVBPSI_TBLID_PMT );
#endif // __ENABLE_TRACE__
		return;						//	不是 PMT 表
	}
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PMT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0xC0 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PMT: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PMT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	//	先进行 SID 过滤
	WORD wSID = ( pSrcBuf[3] << 8 ) | pSrcBuf[4];
	if( INVALID_ID != m_wDstSID && wSID != m_wDstSID )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_PMT: wSID(0x%X) != m_wDstSID(0x%X), abort.\n", wSID, m_wDstSID );
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_PMT pPMTable = (PDVB_PSI_TABLE_PMT)GetTableLastReceived();
	if( pPMTable && pPMTable->m_byVersionNumber == (( pSrcBuf[5] >> 1) & 0x1F )
		&& pPMTable->m_wSID == wSID )
	{							//	相同的PAT版本，且 SID 相同，跳过
#ifdef __ENABLE_TRACE__
//		TRACE("CDVBPSITable_PMT: Same version, skip\n");
#endif // __ENABLE_TRACE__
		return;
	}


	m_PmtTable.Reset();						//	准备环境，开始解码

	// 2016.12.18 CYJ Modify, using size of instead of instance number
	pPMTable = (PDVB_PSI_TABLE_PMT)m_PmtTable.Allocate( sizeof(DVB_PSI_TABLE_PMT) - sizeof(DVB_PSI_TABLE_PMT::tagSTREAMINFO) );
	if( NULL == pPMTable )						//	不分配第一个流结构
	{
		m_PmtTable.Reset();						//	准备环境，开始解码
		return;
	}

	pPMTable->m_byTableID = DVBPSI_TBLID_PMT;

	pPMTable->m_wSID = wSID;
	pPMTable->m_byVersionNumber = ( pSrcBuf[5] >> 1) & 0x1F;
	pPMTable->m_bCurrentNextIndicator = pSrcBuf[5] & 1;
	pPMTable->m_bySectionNumber = pSrcBuf[6];
	pPMTable->m_byLastSectionNumber = pSrcBuf[7];
	pPMTable->m_wPCR_PID = (( pSrcBuf[8]&0x1F)<<8)|pSrcBuf[9];

	WORD wProgramInfoLen = ((pSrcBuf[10]&3)<<8)|pSrcBuf[11];
	PBYTE pProgramInfoStart = pSrcBuf + 12;
	// 先跳过 ProgramInfo descriptors，一会再解释

	PBYTE pStreamStart = pProgramInfoStart + wProgramInfoLen;
	int nStreamInfoBytes = wSectionLen - 13 - wProgramInfoLen;	// 头占用9个字节，然后4个字节的crc32
	int nStreamCount = 0;
	if( nStreamInfoBytes > 0 )
	{
		nStreamCount = DecodeStreamInfo( pStreamStart, nStreamInfoBytes );
		if( nStreamCount < 0 )
		{
			TRACE( "!!! CDVBPSITable_PMT::OnTableReceived. DecodeStreamInfo() < 0\n" );
			m_PmtTable.Reset();						//	 分配内存失败，放弃
			return;
		}
	}
	pPMTable->m_wStreamCount = nStreamCount;

	pPMTable->m_pDescriptor = NULL;
	if( wProgramInfoLen > 0 )
	{
		int nCount = 0;
		if( wProgramInfoLen )
			pPMTable->m_pDescriptor = DecodeDescriptors( pProgramInfoStart, wProgramInfoLen, nCount, m_PmtTable );
		else
			pPMTable->m_pDescriptor = NULL;

		if( nCount < 0 )
		{
			TRACE( "!!! CDVBPSITable_PMT::OnTableReceived. DecodeDescriptors() < 0\n" );
			m_PmtTable.Reset();						//	 分配内存失败，放弃
			return;
		}
		pPMTable->m_wCount = nCount;
#ifdef _DEBUG
		if( nCount )
			ASSERT( pPMTable->m_pDescriptor );
		else
			ASSERT( NULL == pPMTable->m_pDescriptor );
#endif //_DEBUG
	}
	else
		pPMTable->m_wCount = 0;
	pPMTable->m_dwTableSize = m_PmtTable.GetMemoryAllocated();

#if 0
//#ifdef __ENABLE_TRACE__
	TRACE("dump decoded table, %ld bytes\n", pPMTable->m_dwTableSize );
	PBYTE pBufTmp = (PBYTE)pPMTable;
	for(int ii=0; ii<nByteReceived/8; ii ++)
	{
		printf("0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, \n",
		pBufTmp[ii*8],pBufTmp[ii*8+1],pBufTmp[ii*8+2],pBufTmp[ii*8+3],
		pBufTmp[ii*8+4],pBufTmp[ii*8+5],pBufTmp[ii*8+6],pBufTmp[ii*8+7] );
	}
#endif // __ENABLE_TRACE__

	OnPMTReceived( pPMTable );
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	Function:
///		获取上次接收到的节目
///	Input Parameter:
///		None
///	Output Parameter:
///		NULL				还没有接收到PMT表
///		其他				PMT表
PDVB_PSI_TABLE_BASE CDVBPSITable_PMT::GetTableLastReceived()
{
	if( m_PmtTable.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_PAT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_PmtTable.GetHeapBuf();
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		复位对象
///	输入参数:
///		bForce				强制复位，缺省为 false
///	返回参数:
///		无
void CDVBPSITable_PMT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset(bForce);
	if( bForce )
	{
		m_PmtTable.Reset();
		m_wDstSID = INVALID_ID;
	}
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		是否有效
///	输入参数:
///		无
///	返回参数:
///		true				有效
//		false				无效
bool CDVBPSITable_PMT::IsValid()
{
	if( false == CDVBPSITablesBase::IsValid() )
		return false;
	return m_PmtTable.IsValid();
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		接收到 PMT 表
///	输入参数:
///		无
///	返回参数:
///		无
void CDVBPSITable_PMT::OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable )
{
#ifdef __ENABLE_TRACE__
	Dump();
#endif //__ENABLE_TRACE__
}


void CDVBPSITable_PMT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;

	PDVB_PSI_TABLE_PMT pPMT = (PDVB_PSI_TABLE_PMT)GetTableLastReceived();
	fprintf(fOutput, "Dumping Program Map table---------------------------------------\n");
	fprintf(fOutput, "PMT: SID=%d, Version=%d, CurrentNextIndicator=%d\n",
		pPMT->m_wSID, pPMT->m_byVersionNumber, pPMT->m_bCurrentNextIndicator );
	fprintf(fOutput, "SectionNumber=%d, LastSectionNumber=%d, PCR_PID=0x%X\n",
		pPMT->m_bySectionNumber, pPMT->m_byLastSectionNumber, pPMT->m_wPCR_PID );
	fprintf(fOutput, "%d ProgramInfoDescriptors:\n", pPMT->m_wCount );
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDrItem = pPMT->m_pDescriptor;
	int i;
	for(i=0; i<pPMT->m_wCount; i++)
	{
		ASSERT( pDrItem );
		fprintf(fOutput, "   DrTag=%d\n", pDrItem->m_byDescriptorTag );
		pDrItem = pDrItem->m_pNext;
	}
	fprintf(fOutput, "%d streams:\n", pPMT->m_wStreamCount );
	for(i=0; i<pPMT->m_wStreamCount; i++)
	{
		fprintf(fOutput, "   StreamType=%d, PID=%d, %d Descriptors\n",
			pPMT->m_aStreams[i].m_byStreamType, pPMT->m_aStreams[i].m_wES_PID, pPMT->m_aStreams[i].m_byCount );
		pDrItem = pPMT->m_aStreams[i].m_pDescriptor;
		for(int j=0; j<pPMT->m_aStreams[i].m_byCount; j++)
		{
#if 0
//#ifdef __ENABLE_TRACE__
			TRACE("pStreamDescriptor=%p, pDrItem->m_byDescriptorTag=%p\n", pDrItem, &pDrItem->m_byDescriptorTag );
			PBYTE pBufTmp = (PBYTE)pDrItem;
			TRACE("0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n",
				pBufTmp[0], pBufTmp[1], pBufTmp[2], pBufTmp[3],
				pBufTmp[4], pBufTmp[5], pBufTmp[6], pBufTmp[7] );
#endif //#ifdef __ENABLE_TRACE__

			ASSERT( pDrItem );
			CDVBDescriptors::Dump( pDrItem );
			pDrItem = pDrItem->m_pNext;
		}
	}
#else
	(void)fOutput;
#endif //_DEBUG

}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		解释 Stream Info
///	输入参数:
///		pBuf				待解释的数据
///		nLen				缓冲区大小
///	返回参数:
///		>=0					流数目
///		<0					失败
int CDVBPSITable_PMT::DecodeStreamInfo(PBYTE pBuf, int nLen)
{
	int nRetVal = 0;
	int nStreamCount = GetStreamCount( pBuf, nLen );
	DVB_PSI_TABLE_PMT::tagSTREAMINFO * pStreamInfo =
		(DVB_PSI_TABLE_PMT::tagSTREAMINFO *)m_PmtTable.Allocate( nStreamCount*sizeof(DVB_PSI_TABLE_PMT::tagSTREAMINFO) );
	if( NULL == pStreamInfo )
	{
	#ifdef __ENABLE_TRACE__
		TRACE( "Allocate memory failed\n" );
	#endif //#ifdef __ENABLE_TRACE__
		return -1;				//	失败
	}

	while( nLen >= 5 )
	{
		pStreamInfo->m_wES_PID = ((pBuf[1]&0x1F)<<8)|pBuf[2];
		pStreamInfo->m_byStreamType = *pBuf;
		pStreamInfo->m_byCount = 0;
		pStreamInfo->m_pDescriptor = NULL;

#ifdef __ENABLE_TRACE__
		TRACE("StreamType=0x%02X, PID=0x%04X\n",
			pStreamInfo->m_byStreamType, pStreamInfo->m_wES_PID );
		TRACE("pStreamInfo=%p\n", pStreamInfo );
#endif // __ENABLE_TRACE__
		WORD wES_Info_Len = ( (pBuf[3]&3)<<8 ) | pBuf[4];
		if( wES_Info_Len )
		{
			int nDrCount;
			pStreamInfo->m_pDescriptor = DecodeDescriptors( pBuf+5, wES_Info_Len, nDrCount, m_PmtTable );
			if( nDrCount < 0 )
			{
#ifdef __ENABLE_TRACE__
				TRACE("DecodeStreamInfo, DecodeDescriptors failed.\n");
#endif // #ifdef __ENABLE_TRACE__
				break;
			}
			pStreamInfo->m_byCount = nDrCount;
		}
#ifdef _DEBUG
		if( pStreamInfo->m_byCount )
			ASSERT( pStreamInfo->m_pDescriptor );
		else
			ASSERT( pStreamInfo->m_pDescriptor == 0 );
	#if 0
		TRACE("%d Stream descriptor decoded\n", pStreamInfo->m_byCount );
		PBYTE pBufTmp = (PBYTE)pStreamInfo;
		TRACE("pStreamInfo->m_wES_PID=%p, should be=%p\n", &pStreamInfo->m_wES_PID, pBufTmp);
		TRACE("0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n",
			pBufTmp[0], pBufTmp[1], pBufTmp[2], pBufTmp[3],
			pBufTmp[4], pBufTmp[5], pBufTmp[6], pBufTmp[7] );
	#endif // 0
#endif //_DEBUG

		pBuf += ( 5 + wES_Info_Len);
		nLen -= ( 5 + wES_Info_Len);
		nRetVal ++;
		pStreamInfo ++;
	}
	ASSERT( 0 == nLen );
	ASSERT( nRetVal == nStreamCount );

#ifdef __ENABLE_TRACE__
	if( nLen )
		TRACE("CDVBPSITable_PMT::DecodeStreamInfo, error occur.\n");
	if( nRetVal != nStreamCount )
		TRACE("CDVBPSITable_PMT::DecodeStreamInfo, error occur, nRetVal!=nStreamCount.\n");
#endif //__ENABLE_TRACE__

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-16
/// 函数功能:
///		获取数据流个数
/// 输入参数:
///		pBuf			缓冲区地址
///		nLen			数据大小
/// 返回参数:
///		数据流个数
int CDVBPSITable_PMT::GetStreamCount(PBYTE pBuf, int nLen)
{
	int nRetVal = 0;

	while( nLen >= 5 )
	{
		WORD wES_Info_Len = ( ( (pBuf[3]&3)<<8 ) | pBuf[4] ) + 5;
		pBuf += wES_Info_Len;
		nLen -= wES_Info_Len;
		nRetVal ++;
	}
	ASSERT( 0 == nLen && nRetVal );
#ifdef __ENABLE_TRACE__
	if( nLen )
		TRACE("CDVBPSITable_PMT::DecodeStreamInfo, error occur.\n");
	TRACE("CDVBPSITable_PMT::DecodeStreamInfo, stream info count=%d.\n", nRetVal );
#endif //__ENABLE_TRACE__
	return nRetVal;
}

///--------------------------------------------------------------
///	CYJ, 2005-1-11
///	函数功能:
///		解码描述子
///	输入参数:
///		pBuf				缓冲区地址
///		nLen				大小
///		ppFirstList			输出第一个列表
///	返回参数:
///		>=0					描述子个数
///		<0					失败
static PDVBPSI_DECODED_DESCRIPTOR_BASE DecodeDescriptors(PBYTE pBuf, int nLen, int & nCount, CMyHeap & MemAllocator )
{
    // 2011.3.28 CYJ modify, to avoid pBuf is NULL
    nCount = 0;
	if( NULL == pBuf || 0 == nLen )
        return NULL;

	PDVBPSI_DECODED_DESCRIPTOR_BASE pRetVal = NULL;
	PDVBPSI_DECODED_DESCRIPTOR_BASE pLastItem = NULL;
	BYTE nDescriptorCount = 0;
	nCount = -1;
	while( nLen > 0 )
	{
		PDVB_PSI_DESCRIPTOR_BASE pSrcDr = (PDVB_PSI_DESCRIPTOR_BASE)pBuf;
#if defined( __ENABLE_TRACE__ )
		TRACE("Find one descriptor, tagID=0x%02X, nLenNeed=%d\n", pSrcDr->m_byDescriptorTag,
			CDVBDescriptors::GetDecodedDescriptorSize( pSrcDr ) );
		TRACE("0x%02X 0x%02X, tagID=0x%02X, Len=%d\n", pBuf[0], pBuf[1],
			pSrcDr->m_byDescriptorTag, pSrcDr->m_byDescriptorLength);
#endif //#ifdef __ENABLE_TRACE__
		pBuf += (pSrcDr->m_byDescriptorLength + 2);
		nLen -= (pSrcDr->m_byDescriptorLength + 2);
	// 2011.3.9 CYJ remove the assert, since some TS stream is bad
	//	ASSERT( nLen >= 0 );
		if( nLen < 0 )
		{
		#ifdef _DEBUG
			fprintf( stderr, "nLen(%d) < 0", nLen );
			TRACE("Find one descriptor, tagID=0x%02X, nLenNeed=%d\n", pSrcDr->m_byDescriptorTag,
				CDVBDescriptors::GetDecodedDescriptorSize( pSrcDr ) );
			TRACE("0x%02X 0x%02X, tagID=0x%02X, Len=%d\n", pBuf[0], pBuf[1],
				pSrcDr->m_byDescriptorTag, pSrcDr->m_byDescriptorLength);
		#endif //_DEBUG

			break;
		}

		int nByteNeed = CDVBDescriptors::GetDecodedDescriptorSize( pSrcDr );
		if( nByteNeed <= 0 )
		{
		#ifdef _DEBUG
			fprintf( stderr, "nByteNeed <= 0\n" );
		#endif //_DEBUG
			continue;					//	未知类型
		}

		PDVBPSI_DECODED_DESCRIPTOR_BASE pCurItem = (PDVBPSI_DECODED_DESCRIPTOR_BASE)MemAllocator.Allocate( nByteNeed );
		if( NULL == pCurItem )
		{
		#ifdef _DEBUG
			TRACE("Find one descriptor, tagID=0x%02X, nLenNeed=%d\n", pSrcDr->m_byDescriptorTag,
				CDVBDescriptors::GetDecodedDescriptorSize( pSrcDr ) );
			TRACE("0x%02X 0x%02X, tagID=0x%02X, Len=%d\n", pBuf[0], pBuf[1],
				pSrcDr->m_byDescriptorTag, pSrcDr->m_byDescriptorLength);
			fprintf( stderr, "MemAllocator.Allocate( %d ) failed.\n", nByteNeed );
		#endif //_DEBUG
			return NULL;
		}

		pCurItem->m_byDescriptorTag = pSrcDr->m_byDescriptorTag;
		pCurItem->m_pNext = NULL;

		if( false == CDVBDescriptors::DecodeVideoStreamDescriptor( pSrcDr, pCurItem ) )
		{
			MemAllocator.CancelLastAllocate();		//	解释失败，放弃这个解释
		#ifdef _DEBUG
			fprintf( stderr, "CDVBDescriptors::DecodeVideoStreamDescripto = false\n" );
		#endif //_DEBUG
			continue;
		}

		if( pLastItem )
			pLastItem->m_pNext = pCurItem;
		pLastItem = pCurItem;

		if( NULL == pRetVal )
			pRetVal = pCurItem;

		nDescriptorCount ++;					//	成功解码
	}

// 2011.3.9 CYJ remove the assert, since some TS stream is bad
//	ASSERT( 0 == nLen );
	nCount = nDescriptorCount;

	return pRetVal;
}

////////////////////////////////////////////////////////
// SDT, BAT, ST
////////////////////////////////////////////////////////
CDVBPSITable_SDT_BAT::CDVBPSITable_SDT_BAT()
	:CDVBPSITablesBase( TABLE_MAX_SIZE ),
	m_SDT(DECODED_SDT_MAX_SIZE),
	m_BAT( DECODED_BAT_MAX_SIZE )
{
}


CDVBPSITable_SDT_BAT::~CDVBPSITable_SDT_BAT()
{
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		一个表完整接收
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived >=3 );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 3 )
		return;
	switch( *pSrcBuf )
	{
	case DVBPSI_TBLID_SDT_ACTUAL:
	case DVBPSI_TBLID_SDT_OTHER:
		DecodeSDT();
		break;

	case DVBPSI_TBLID_BAT:
		DecodeBAT();
		break;
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		解释SDT表
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::DecodeSDT()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个SDT表至少需要12字节
		return;

	ASSERT( *pSrcBuf == DVBPSI_TBLID_SDT_ACTUAL || *pSrcBuf == DVBPSI_TBLID_SDT_OTHER );
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeSDT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( !(pSrcBuf[1] & 0x80 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeSDT: section_syntax_indicator should be 1,but actual=%d\n", (pSrcBuf[1] & 0x80 ) >> 7 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("DecodeSDT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

#if 0
	PDVB_PSI_TABLE_SDT pSDTTable = GetLastReceivedSDT();
	if( pSDTTable && pSDTTable->m_byVersionNumber == ((pSrcBuf[5]>>1)&0x1F) )
	{
		if( IsAllSectionReceived() )
		{		// 所有
			#ifdef __ENABLE_TRACE__
			//	TRACE("CDVBPSITable_PAT: Same version, skip\n");
			#endif // __ENABLE_TRACE__

			return;
		}
	}
#else
	PDVB_PSI_TABLE_SDT pSDTTable;
#endif // 0

	m_SDT.Reset();
	// 2016.12.18 CYJ Modify, using 7+sizeof() instead of 11, since sizeof( pointer ) = 4 in 32 bit, = 8 in 4 bits 
	ASSERT( sizeof(tagDVB_PSI_TABLE_SDT::tagSERVICELIST) == 7+sizeof(PDVBPSI_DECODED_DESCRIPTOR_BASE) );
	pSDTTable = (PDVB_PSI_TABLE_SDT)m_SDT.Allocate( sizeof(DVB_PSI_TABLE_SDT)-sizeof(tagDVB_PSI_TABLE_SDT::tagSERVICELIST) );
	if( NULL == pSDTTable )
	{
		ASSERT( FALSE );
		m_SDT.Reset();
		return;
	}
	PBYTE pBufTmp = pSrcBuf + 3;
	pSDTTable->m_wTSID = (WORD(pBufTmp[0]) << 8) || pBufTmp[1];
	pBufTmp += 2;
	pSDTTable->m_byVersionNumber = ( *pBufTmp >> 1 ) & 0x1F;
	pSDTTable->m_bCurrentNextIndicator = (*pBufTmp++) & 1;
	pSDTTable->m_bySectionNumber = *pBufTmp++;
	pSDTTable->m_byLastSectionNumber = *pBufTmp++;
	pSDTTable->m_wOriginalNetworkID = (WORD(pBufTmp[0])<<8) | pBufTmp[1];
	pBufTmp += 3;						// 还需要跳过一个字节
	pSDTTable->m_wCount = SDT_CalculateProgramCoumt( pBufTmp, wSectionLen-12 );	// 8 字节头部，加4字节CRC32
	if( 0 == pSDTTable->m_wCount )
		return;							//	完成解码

	DVB_PSI_TABLE_SDT::tagSERVICELIST * pProgramInfo =
		(DVB_PSI_TABLE_SDT::tagSERVICELIST *)m_SDT.Allocate( pSDTTable->m_wCount*sizeof(DVB_PSI_TABLE_SDT::tagSERVICELIST) );
	if( NULL == pProgramInfo )
	{
		m_SDT.Reset();
		ASSERT( FALSE );
		return;
	}
	for(int i=0; i<pSDTTable->m_wCount; i++ )
	{
		pProgramInfo->m_wSID = (WORD(pBufTmp[0])<<8) | pBufTmp[1];
		pBufTmp += 2;
		pProgramInfo->m_bEITScheduleFlag = (*pBufTmp>>1) & 1;
		pProgramInfo->m_bEITPresentFollowingFlag = (*pBufTmp++) & 1;	// pBufTmp is increase one byte
		pProgramInfo->m_byRunningStatus = (*pBufTmp) >> 5;
		pProgramInfo->m_bFreeCAMode = ( (*pBufTmp) >> 4 ) & 1;
		WORD wLoopLen = (WORD(pBufTmp[0]&0xF)<<8)|pBufTmp[1];
		pBufTmp += 2;
		if( wLoopLen )
		{
			int nDrCount;
			pProgramInfo->m_pDescriptor = DecodeDescriptors( pBufTmp, wLoopLen, nDrCount, m_SDT );
			pBufTmp += wLoopLen;
			if( nDrCount < 0 )
			{
				m_SDT.Reset();
				return;				//	失败
			}
			pProgramInfo->m_byCount = nDrCount;
		}
		else
		{
			pProgramInfo->m_byCount = 0;
			pProgramInfo->m_pDescriptor = NULL;
		}
		pProgramInfo ++;
	}

#ifdef _DEBUG
	ASSERT( (pBufTmp - pSrcBuf)+1 == wSectionLen );
#endif //_DEBUG

	pSDTTable->m_byTableID = *pSrcBuf;
	pSDTTable->m_dwTableSize = m_SDT.GetMemoryAllocated();

	OnSDTReceived( pSDTTable );		//	接收到 SDT
}

///-------------------------------------------------------
/// CYJ,2008-10-17
/// 函数功能:
///		解码 BAT
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::DecodeBAT()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个SDT表至少需要12字节
		return;

	ASSERT( *pSrcBuf == DVBPSI_TBLID_BAT );
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeBAT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( !(pSrcBuf[1] & 0x80 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeBAT: section_syntax_indicator should be 1,but actual=%d\n", (pSrcBuf[1] & 0x80 ) >> 7 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("DecodeBAT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_BAT pBATTable;

	m_BAT.Reset();
	// 2016.12.18 CYJ Modify, using 7+sizeof() instead of 11, since sizeof( pointer ) = 4 in 32 bit, = 8 in 4 bits
	ASSERT( sizeof(tagDVB_PSI_TABLE_BAT::tagONE_BAT_ITEM) == 6+sizeof(PDVBPSI_DECODED_DESCRIPTOR_BASE) );
	pBATTable = (PDVB_PSI_TABLE_BAT)m_BAT.Allocate( sizeof(DVB_PSI_TABLE_BAT)-sizeof(tagDVB_PSI_TABLE_BAT::tagONE_BAT_ITEM) );
	if( NULL == pBATTable )
	{
		ASSERT( FALSE );
		m_BAT.Reset();
		return;
	}
	PBYTE pBufTmp = pSrcBuf + 3;
	pBATTable->m_wBouquetID = (WORD(pBufTmp[0]) << 8) | pBufTmp[1];
	pBufTmp += 2;
	pBATTable->m_byVersionNumber = ( *pBufTmp >> 1 ) & 0x1F;
	pBATTable->m_bCurrentNextIndicator = (*pBufTmp++) & 1;
	pBATTable->m_bySectionNumber = *pBufTmp++;
	pBATTable->m_byLastSectionNumber = *pBufTmp++;

	WORD wBATLevelDescriptorLen = (WORD(pBufTmp[0])<<8) | pBufTmp[1];
	wBATLevelDescriptorLen &= 0xFFF;
	pBufTmp += 2;

	pBATTable->m_wCount = 0;
	pBATTable->m_pDescriptor = 0;

	PBYTE pBouquetDescStart = pSrcBuf + 10;
	PBYTE pBATItemStart = pBouquetDescStart + wBATLevelDescriptorLen;

	WORD wBATItemDescLen = ( pBATItemStart[0] << 8 ) + pBATItemStart[1];
	wBATItemDescLen &= 0xFFF;
	pBATTable->m_wItemCount = BAT_DecodeBATItems( pBATItemStart + 2, wBATItemDescLen );

	int nDrCount = 0;
	if( wBATLevelDescriptorLen )
		pBATTable->m_pDescriptor = DecodeDescriptors( pBouquetDescStart, wBATLevelDescriptorLen, nDrCount, m_BAT );
	else
		pBATTable->m_pDescriptor = NULL;
	pBATTable->m_wCount = nDrCount;

	pBATTable->m_byTableID = *pSrcBuf;
	ASSERT( pBATTable->m_byTableID == DVBPSI_TBLID_BAT );
	pBATTable->m_dwTableSize = m_BAT.GetMemoryAllocated();

	OnBATReceived( pBATTable );
}

///-------------------------------------------------------
/// CYJ,2008-10-18
/// 函数功能:
///		接收到 BAT
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::OnBATReceived( PDVB_PSI_TABLE_BAT pBAT )
{

}

///-------------------------------------------------------
/// CYJ,2008-10-18
/// 函数功能:
///		解码 BAT 项
/// 输入参数:
///		无
/// 返回参数:
///		无
int CDVBPSITable_SDT_BAT::BAT_DecodeBATItems( PBYTE pBuf, int nLen )
{
	int nItemCount = BAT_GetBatItemsCount( pBuf, nLen );
	if( 0 == nItemCount )
		return 0;

	tagDVB_PSI_TABLE_BAT::tagONE_BAT_ITEM * pItem = \
		(tagDVB_PSI_TABLE_BAT::tagONE_BAT_ITEM * )m_BAT.Allocate( nItemCount*sizeof(tagDVB_PSI_TABLE_BAT::tagONE_BAT_ITEM) );
	if( NULL == pItem )
		return -1;		// failed

	for(int i=0; i<nItemCount; i++ )
	{
		pItem->m_TransportStreamID = (pBuf[0]<<8) + pBuf[1];
		pItem->m_wOriginalNetworkID = (pBuf[2]<<8) + pBuf[3];
		WORD wInfoLen = ( (pBuf[4]<<8) + pBuf[5] ) & 0xFFF;
		pBuf += 6;
		nLen -= 6;

		int nDrCount = 0;
		if( wInfoLen )
			pItem->m_pDescriptor = DecodeDescriptors( pBuf, wInfoLen, nDrCount, m_BAT );
		else
			pItem->m_pDescriptor = NULL;
		pItem->m_wCount = nDrCount;

		pBuf += wInfoLen;
		nLen -= wInfoLen;
		pItem ++;
	}

	ASSERT( nLen == 0 );

	return nItemCount;
}

///-------------------------------------------------------
/// CYJ,2008-10-18
/// 函数功能:
///		计算 PAT 后续个数
/// 输入参数:
///		无
/// 返回参数:
///		无
int  CDVBPSITable_SDT_BAT::BAT_GetBatItemsCount( PBYTE pBuf, int nLen )
{
	int nItemCount = 0;

	while( nLen > 0 )
	{
		nLen -= 6;
		pBuf += 4;
		WORD wDescLen = ( ( pBuf[0] << 8 ) + pBuf[1] ) & 0xFFF;
		pBuf += wDescLen + 2;
		nLen -= wDescLen;

		nItemCount ++;
	}

	ASSERT( 0 == nLen );

	return nItemCount;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		计算后面可用的节目个数
/// 输入参数:
///		pBuf				节目开始缓冲区
///		nLen				剩余字节数
/// 返回参数:
///		节目个数
WORD CDVBPSITable_SDT_BAT::SDT_CalculateProgramCoumt(PBYTE pBuf, int nLen)
{
	//ASSERT( pBuf && nLen > 0 );
	if( NULL == pBuf || nLen <= 0 )
		return 0;

	WORD wCount = 0;
	while( nLen > 0 )
	{
		WORD wLoopLen = ( (WORD(pBuf[3]&0xF)<<8)|pBuf[4] ) + 5;
		nLen -= wLoopLen;
		pBuf += wLoopLen;
		wCount ++;
	}
	ASSERT( nLen == 0 );

	if( nLen )
	{
		wCount = 0;
		#ifdef __ENABLE_TRACE__
			TRACE("SDT_CalculateProgramCount, error len\n");
		#endif //__ENABLE_TRACE__
	}
	return wCount;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		获取上次接收到的SDT
/// 输入参数:
///		无
/// 返回参数:
///		NULL				失败
///		其他				上次接收到的SDT表
PDVB_PSI_TABLE_SDT	CDVBPSITable_SDT_BAT::GetLastReceivedSDT()
{
	if( m_SDT.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_SDT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_SDT)m_SDT.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2009-9-5
/// 函数功能:
///		获取上次接收到的 BAT
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BAT	CDVBPSITable_SDT_BAT::GetLastReceivedBAT()
{
	if( m_BAT.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_BAT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BAT)m_BAT.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		获取上次接收到的表
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSITable_SDT_BAT::GetTableLastReceived()
{
	ASSERT( FALSE );
	return NULL;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		复位解码器
/// 输入参数:
///		bForce			是否强制
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
	if( bForce )
	{
		m_BAT.Reset();
		m_SDT.Reset();
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		判断该对象释放有效
/// 输入参数:
///		无
/// 返回参数:
///		true			有效
///		false			无效
bool CDVBPSITable_SDT_BAT::IsValid()
{
	if( !CDVBPSITablesBase::IsValid() )
		return false;
	return m_SDT.IsValid() && m_BAT.IsValid();
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		判断该表是否完整接收
/// 输入参数:
///		无
/// 返回参数:
///		true			完整
///		false			还没有完整接收
bool CDVBPSITable_SDT_BAT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 3 )
		return false;						//	只要接收到3个字节，就可以获取 Section Length，然后再判断是否完整
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_SDT_BAT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;
	PDVB_PSI_TABLE_SDT pSDT = GetLastReceivedSDT();
	if( pSDT )
	{
		fprintf( fOutput, "====================Dumping SDT======================\n");
		fprintf( fOutput, "TSID=0x%04X, Version=%d, CurrentNextIndicator=%d\n",
			pSDT->m_wTSID, pSDT->m_byVersionNumber, pSDT->m_bCurrentNextIndicator  );
		fprintf( fOutput, "SectionNumber=%d, m_byLastSectionNumber=%d, OrginalNetwork=0x%04X\n",
			pSDT->m_bySectionNumber, pSDT->m_byLastSectionNumber, pSDT->m_wOriginalNetworkID );
		fprintf( fOutput, "There %d Programs:\n", pSDT->m_wCount );
		for(int i=0; i<pSDT->m_wCount; i++ )
		{
			fprintf(fOutput, "    Program %d, SID=%d, EITScheduleFlag=%d, PresentFollowFlag=%d\n",
				i+1, pSDT->m_aPrograms[i].m_wSID, pSDT->m_aPrograms[i].m_bEITScheduleFlag,
				pSDT->m_aPrograms[i].m_bEITPresentFollowingFlag	);
			fprintf(fOutput, "    RunningStatus=%d, FreeCAMode=%d\n",
				pSDT->m_aPrograms[i].m_byRunningStatus, pSDT->m_aPrograms[i].m_bFreeCAMode );
			fprintf( fOutput,"        There %d Descriptors:\n", pSDT->m_aPrograms[i].m_byCount );
			PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pSDT->m_aPrograms[i].m_pDescriptor;
			for(int j=0; j<pSDT->m_aPrograms[i].m_byCount; j++ )
			{
				ASSERT( pDr );
				CDVBDescriptors::Dump( pDr );
				pDr = pDr->m_pNext;
			}
		}
	}
#else
	(void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		接收到 SDT 表
/// 输入参数:
///		pSDT		接收到的SDT表
/// 返回参数:
///		无
void CDVBPSITable_SDT_BAT::OnSDTReceived( PDVB_PSI_TABLE_SDT pSDT )
{
#ifdef _DEBUG
	Dump();
#endif //_DEBUG
}

//////////////////////////////////////////////////////////
/// EIT
//////////////////////////////////////////////////////////
CDVBPSITable_EIT::CDVBPSITable_EIT()
	:CDVBPSITablesBase(TABLE_MAX_SIZE),
	m_EIT(DECODED_TABLE_MAX_SIZE)
{
	m_wDstSID = INVALID_ID;
}

CDVBPSITable_EIT::~CDVBPSITable_EIT()
{
}

inline int BCD_TO_DEC( int nBCD )
{
	return (nBCD & 0xF) + (nBCD>>4)*10;
}

inline int DEC_TO_BCD( int nDec )
{
	return (nDec % 10 ) + (nDec/10)*16;
}

///-------------------------------------------------------
/// CYJ,2006-1-18
/// 函数功能:
///		Convert Start time to UTC time_t
/// 输入参数:
///		无
/// 返回参数:
///		无
time_t ConvertTimeToUTC(MY_LONG64 llStartTime)
{
	struct tm tmTmp;
	memset( &tmTmp, 0, sizeof(tmTmp) );
	tmTmp.tm_hour = BCD_TO_DEC( DWORD( llStartTime >> 16 ) & 0xFF);
	tmTmp.tm_min = BCD_TO_DEC( DWORD( llStartTime >> 8 ) & 0xFF );
	tmTmp.tm_sec = BCD_TO_DEC( DWORD(llStartTime) & 0xFF );
	DWORD dwDate = (int)(llStartTime >> 24) & 0xFFFF;
	short nYear = (short)( (dwDate - 15078.2)/365.25 );
	short nMonth = (short)( ( dwDate-14956.1 - int(nYear*365.25f) ) / 30.6001f );
	short nDay = (short)( dwDate - 14956 - int(nYear*365.25f) - int(nMonth * 30.6001f) );
	int K = ( nMonth==14 || nMonth==15) ? 1 : 0;
	nYear += K;
	nMonth = nMonth -1 -K*12;

	tmTmp.tm_mday = nDay;

	// 2010.10.26 CYJ Add, 增加 nMonth 容错处理。
	if( nMonth < 1 )
	{
#ifdef _DEBUG
		fprintf( stderr, "ConvertTimeToUTC nMondth (%d) should >= 1\n", nMonth );
#endif //_DEBUG
		nMonth = 1;
	}
	else if( nMonth > 12 )
	{
#ifdef _DEBUG
		fprintf( stderr, "ConvertTimeToUTC nMondth (%d) should <= 1\n", nMonth );
#endif //_DEBUG
		nMonth = 12;
	}

	tmTmp.tm_mon = nMonth-1;
	tmTmp.tm_year = nYear;

	time_t tRetVal = mktime( &tmTmp );
	tRetVal -= _timezone;

	return tRetVal;
}

///-------------------------------------------------------
/// CYJ,2006-1-18
/// 函数功能:
///		转换时间到MJD
/// 输入参数:
///		tTime		本地时间
/// 返回参数:
///		无
MY_LONG64 ConvertTimeToMJD( time_t tTime )
{
	struct tm tmTmp;
	struct tm * pRetVal = localtime( &tTime );

	if( NULL == pRetVal )
		return 0;
	tmTmp = *pRetVal;

	tmTmp.tm_mon ++;
	int L = ( tmTmp.tm_mon <= 2 ) ? 1 : 0;
	WORD wMJD = 14956 + tmTmp.tm_mday + int( (tmTmp.tm_year-L)*365.25f ) + int( (tmTmp.tm_mon+1+L*12)*30.6001f );

	MY_LONG64 llMJDRetVal = wMJD;
	llMJDRetVal <<= 24;
	llMJDRetVal += ( DEC_TO_BCD(tmTmp.tm_hour) << 16 ) + (DEC_TO_BCD(tmTmp.tm_min)<<8) + DEC_TO_BCD(tmTmp.tm_sec);

#ifdef _DEBUG
	ASSERT( tTime == ConvertTimeToUTC(llMJDRetVal) );
#endif

	return llMJDRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		接收完整，需要进行解析
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_EIT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived < 18 )	// 一个EIT表至少需要18字节
		return;

	BYTE byTableID = *pSrcBuf;
	if( byTableID < DVBPSI_TBLID_EIT_ACTUAL || byTableID > DVBPSI_TBLID_EIT_OTHER_SCHEDULE_END  )
	{				//	只处理 EIT 表
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: Not a EIT table, tableId=%d != 0\n", byTableID );
#endif // __ENABLE_TRACE__
		return;						//	不是 PAT 表
	}
	WORD wSectionLen = ((pSrcBuf[1]&0xF) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0x80 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: section_syntax_indicator and next should be 1,but actual=%d\n", (pSrcBuf[1] & 0x80 ) >> 7 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	//	先进行 SID 过滤
	WORD wSID = ( pSrcBuf[3] << 8 ) | pSrcBuf[4];
	if( INVALID_ID != m_wDstSID && wSID != m_wDstSID )
	{
#ifdef __ENABLE_TRACE__
//		TRACE("CDVBPSITable_EIT: wSID(0x%X) != m_wDstSID(0x%X), abort.\n", wSID, m_wDstSID );
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_EIT pEIT = (PDVB_PSI_TABLE_EIT)GetTableLastReceived();
#if 0		//  CYJ,2006-4-26 暂时不作版本判断
	if( pEIT && pEIT->m_byVersionNumber == (( pSrcBuf[5] >> 1) & 0x1F )
		&& pEIT->m_wSID == wSID )
	{							//	相同的PAT，跳过
#ifdef __ENABLE_TRACE__
//		TRACE("CDVBPSITable_EIT: Same version, skip\n");
#endif // __ENABLE_TRACE__
		return;
	}
#endif // 0

	m_EIT.Reset();						//	准备环境，开始解码
	pEIT = (PDVB_PSI_TABLE_EIT)m_EIT.Allocate( sizeof(DVB_PSI_TABLE_EIT) - sizeof(DVB_PSI_TABLE_EIT::tagEVENTMSG) );
	if( NULL == pEIT )
	{
		ASSERT( FALSE );
		m_EIT.Reset();
		return;
	}
	PBYTE pBufTmp = pSrcBuf + 5;
	pEIT->m_wSID = wSID;
	pEIT->m_byVersionNumber = ( (*pBufTmp) >> 1 ) & 0x1F;
	pEIT->m_bCurrentNextIndicator = (*pBufTmp++) & 1;
	pEIT->m_bySectionNumber = *pBufTmp++;
	pEIT->m_byLastSectionNumber = *pBufTmp++;
	pEIT->m_wTSID = (WORD(pBufTmp[0])<<8) | pBufTmp[1];
	pBufTmp += 2;
	pEIT->m_wOriginalNetworkID = (WORD(pBufTmp[0])<<8) | pBufTmp[1];
	pBufTmp += 2;
	pEIT->m_bySegmentLastSectionNumber = *pBufTmp++;
	pEIT->m_byLastTableID = *pBufTmp++;;
	pEIT->m_wCount = GetEventsCount( pBufTmp, wSectionLen - 15 );	// 11 字节头，4字节 CRC32
	if( 0 == pEIT->m_wCount )
		return;							//	已经完成解码

	DVB_PSI_TABLE_EIT::tagEVENTMSG * pEvents =
		(DVB_PSI_TABLE_EIT::tagEVENTMSG *)m_EIT.Allocate(sizeof(DVB_PSI_TABLE_EIT::tagEVENTMSG)*pEIT->m_wCount);
	ASSERT( pEvents );
	if( NULL == pEvents )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT, Allocate Event buffer failed.\n");
#endif //__ENABLE_TRACE__
		m_EIT.Reset();
		return;
	}

	for( int i=0; i<pEIT->m_wCount; i++ )
	{
		pEvents->m_wEventID = (WORD(pBufTmp[0])<<8)|pBufTmp[1];
		pBufTmp += 2;
		MY_LONG64 llStartTime = (DWORD(pBufTmp[0])<<16)| (DWORD(pBufTmp[1])<<8) | DWORD(pBufTmp[2]);
		llStartTime <<= 16;
		llStartTime |= (DWORD(pBufTmp[3])<<8) | DWORD(pBufTmp[4]);

		if( (DWORD)llStartTime != 0xFFFFFFFF )	//  CYJ,2006-1-18 若全为1，则可能因为NVOD而没有定义
			pEvents->m_tStartTimeUTC = ConvertTimeToUTC(llStartTime);
		else
			pEvents->m_tStartTimeUTC = 0;

		pBufTmp += 5;
		pEvents->m_dwDuration = BCD_TO_DEC(pBufTmp[0])*3600 + BCD_TO_DEC(pBufTmp[1])*60 + BCD_TO_DEC(pBufTmp[2]);
		pBufTmp += 3;
		pEvents->m_byRunningStatus = (*pBufTmp>>5);
		pEvents->m_bFreeCAMode = (*pBufTmp>>4) & 1;

		WORD wLoopLen = ((pBufTmp[0] & 0xF)<<8) | pBufTmp[1];
		pBufTmp += 2;

		if( wLoopLen )
		{
			int nDrCount;
			pEvents->m_pDescriptor = DecodeDescriptors( pBufTmp, wLoopLen, nDrCount, m_EIT );
			pBufTmp += wLoopLen;
			if( nDrCount < 0 )
			{
				m_EIT.Reset();
				return;				//	失败
			}
			pEvents->m_byCount = nDrCount;
		}
		else
		{
			pEvents->m_byCount = 0;
			pEvents->m_pDescriptor = NULL;
		}
		pEvents ++;
	}
#ifdef _DEBUG
	ASSERT( (pBufTmp - pSrcBuf)+1 == wSectionLen );
#endif //_DEBUG

	pEIT->m_byTableID = *pSrcBuf;
	pEIT->m_dwTableSize = m_EIT.GetMemoryAllocated();

	OnEITReceived( pEIT );
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		获取上次接收到的表
/// 输入参数:
///		无
/// 返回参数:
///		NULL				没有接收到
///		其他				上次接收到的表
PDVB_PSI_TABLE_BASE CDVBPSITable_EIT::GetTableLastReceived()
{
	if( m_EIT.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_EIT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_EIT.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		复位解释器
/// 输入参数:
///		bForce			是否强制使用
/// 返回参数:
///		无
void CDVBPSITable_EIT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
	if( bForce )
	{
		m_EIT.Reset();
		m_wDstSID = INVALID_ID;		//  CYJ,2008-10-17 add
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		true			有效
///     false			无效，不能使用
bool CDVBPSITable_EIT::IsValid()
{
	return CDVBPSITablesBase::IsValid() && m_EIT.IsValid();
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		验证表是否接收完整
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_EIT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 15 )	// 一个EIT表至少需要15字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0xF) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_EIT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;
	fprintf( fOutput, "===================== Dump EIT ============================\n");

	PDVB_PSI_TABLE_EIT pEIT = (PDVB_PSI_TABLE_EIT)GetTableLastReceived();
	ASSERT( pEIT );

	fprintf( fOutput, "SID=0x%04X, Ver=%d, CurNextIndicator=%d\n",
		pEIT->m_wSID, pEIT->m_byVersionNumber, pEIT->m_bCurrentNextIndicator );
	fprintf( fOutput, "m_bySectionNumber=%d, m_byLastSectionNumber=%d, TSID=0x%04X\n",
		pEIT->m_bySectionNumber, pEIT->m_byLastSectionNumber, pEIT->m_wTSID );
	fprintf( fOutput, "OriginalNetID=0x%04X, SegmentLastSN=%d, LastTblID=%d\n",
		pEIT->m_wOriginalNetworkID, pEIT->m_bySegmentLastSectionNumber, pEIT->m_byLastTableID );
	fprintf( fOutput, "There are %d Events\n", pEIT->m_wCount );
	for(int i=0; i<pEIT->m_wCount; i++ )
	{
		fprintf( fOutput, "   Event[%d], EventID=0x%04X, StartTime=%s, Duration=%d\n",
			i+1, pEIT->m_aEvents[i].m_wEventID, ctime(&pEIT->m_aEvents[i].m_tStartTimeUTC),
			pEIT->m_aEvents[i].m_dwDuration );
		fprintf( fOutput, "         RunningStatus=%d, FreeCA=%d and there are %d descriptors\n",
			pEIT->m_aEvents[i].m_byRunningStatus, pEIT->m_aEvents[i].m_bFreeCAMode,
			pEIT->m_aEvents[i].m_byCount );
		PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pEIT->m_aEvents[i].m_pDescriptor;
		for(int j=0; j<pEIT->m_aEvents[i].m_byCount; j++ )
		{
			ASSERT(pDr);
			CDVBDescriptors::Dump( pDr );
			pDr = pDr->m_pNext;
		}
	}
#else
	(void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		获取事件个数
/// 输入参数:
///		pBuf				缓冲区地址
///		nLen				缓冲区大小
/// 返回参数:
///		实际个数
WORD CDVBPSITable_EIT::GetEventsCount(PBYTE pBuf, int nLen)
{
	WORD wCount = 0;

	while( nLen > 0 )
	{
		WORD wLoopLen = ( (WORD(pBuf[10]&0xF)<<8)|pBuf[11] ) + 12;
		pBuf += wLoopLen;
		nLen -= wLoopLen;
		wCount ++;
	}

	ASSERT( nLen == 0 );
	if( nLen != 0 )				//	发生错误
	{
		wCount = 0;
		#ifdef __ENABLE_TRACE__
			TRACE("CDVBPSITable_EIT, error len\n");
		#endif //__ENABLE_TRACE__
	}
	return wCount;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		接收到 EIT
/// 输入参数:
///		pEIT			EIT 表
/// 返回参数:
///		无
void CDVBPSITable_EIT::OnEITReceived( PDVB_PSI_TABLE_EIT pEIT )
{
#ifdef _DEBUG
	Dump();
#endif // _DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		设置目标SID
/// 输入参数:
///		wSID			新的SID
/// 返回参数:
///		原来的SID
WORD CDVBPSITable_EIT::SetDstSID( WORD wSID )
{
	WORD wRetVal = m_wDstSID;
	m_wDstSID = wSID;
	return wRetVal;
}

//////////////////////////////////////////////////////////
/// NIT
//////////////////////////////////////////////////////////
CDVBPSITable_NIT::CDVBPSITable_NIT()
	:CDVBPSITablesBase(TABLE_MAX_SIZE),
	m_NIT(DECODED_TABLE_MAX_SIZE)
{
	m_wDstNID = INVALID_ID;
}

CDVBPSITable_NIT::~CDVBPSITable_NIT()
{
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		接收到NIT表
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_NIT::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived < 13 )	// 一个NIT表至少需要18字节
		return;

	BYTE byTableID = *pSrcBuf;
	if( byTableID != DVBPSI_TBLID_NIT && byTableID != DVBPSI_TBLID_NIT_OTHER )	// 2010.4.7 Add, receive other Network
		return;						//	不是 NIT 表

	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_NIT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0x80 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: section_syntax_indicator and next should be 1,but actual=%d\n", (pSrcBuf[1] & 0x80 ) >> 7 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, wSectionLen+3 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋3。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	//	先进行 SID 过滤
	WORD wNID = ( pSrcBuf[3] << 8 ) | pSrcBuf[4];
	if( INVALID_ID != m_wDstNID && wNID != m_wDstNID )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_EIT: wNID(0x%X) != m_wDstNID(0x%X), abort.\n", wNID, m_wDstNID );
#endif // __ENABLE_TRACE__
		return;
	}

	PDVB_PSI_TABLE_NIT pNIT = (PDVB_PSI_TABLE_NIT)GetTableLastReceived();
	if( pNIT && pNIT->m_byVersionNumber == (( pSrcBuf[5] >> 1) & 0x1F )
		&& pNIT->m_wNetworkID == wNID )
	{							//	相同的PAT，跳过
#ifdef __ENABLE_TRACE__
//		TRACE("CDVBPSITable_EIT: Same version, skip\n");
#endif // __ENABLE_TRACE__
		return;
	}

	m_NIT.Reset();

	ASSERT( sizeof(DVB_PSI_TABLE_NIT::tagITEM) == 9 );
	pNIT = (PDVB_PSI_TABLE_NIT)m_NIT.Allocate( sizeof(DVB_PSI_TABLE_NIT) - sizeof(DVB_PSI_TABLE_NIT::tagITEM) );

	PBYTE pBufTmp = pSrcBuf + 5;
	pNIT->m_wNetworkID = wNID;
	pNIT->m_byVersionNumber = ( (*pBufTmp) >> 1 ) & 0x1F;
	pNIT->m_bCurrentNextIndicator = (*pBufTmp++) & 1;
	pNIT->m_bySectionNumber = *pBufTmp++;
	pNIT->m_byLastSectionNumber = *pBufTmp++;

	WORD wNetworkDrLen = (WORD(pBufTmp[0]&0xF)<<8)|pBufTmp[1];
	PBYTE pNetworkDrBuf = pBufTmp+2;
	pBufTmp += (2+wNetworkDrLen);

	WORD wTSItemLen = (WORD(pBufTmp[0]&0xF)<<8)|pBufTmp[1];
	pBufTmp += 2;
	int nCount = GetTSItemDescriptorCount( pBufTmp, wTSItemLen );
	if( nCount < 0 )
	{
	// 2011.3.10 Remove since GetTSItemDescriptorCount may <= 0
	//	ASSERT( FALSE );
		m_NIT.Reset();
		return;
	}
	pNIT->m_wItemCount = nCount;
	if( nCount > 0 )
	{
		DVB_PSI_TABLE_NIT::tagITEM * pItem =
			(DVB_PSI_TABLE_NIT::tagITEM*)m_NIT.Allocate( nCount*sizeof(DVB_PSI_TABLE_NIT::tagITEM) );
		for(int i=0; i<nCount; i++ )
		{
			pItem->m_wTSID = (WORD(pBufTmp[0])<<8)|pBufTmp[1];
			pItem->m_wOriginalNetworkID = (WORD(pBufTmp[2])<<8)|pBufTmp[3];
			WORD wLoopLen = (WORD(pBufTmp[4]&0xF)<<8)|pBufTmp[5];
			pBufTmp += 6;
			if( wLoopLen )
			{
				int nDrCount;
				pItem->m_pDescriptor = DecodeDescriptors( pBufTmp, wLoopLen, nDrCount, m_NIT );
				pBufTmp += wLoopLen;
				if( nDrCount < 0 )
				{
					m_NIT.Reset();
					return;				//	失败
				}
				pItem->m_byCount = nDrCount;
			}
			else
			{
				pItem->m_byCount = 0;
				pItem->m_pDescriptor = NULL;
			}
			pItem ++;
		}
	}

	if(wNetworkDrLen)
	{
		int nDrCount;
		pNIT->m_pDescriptor = DecodeDescriptors( pNetworkDrBuf, wNetworkDrLen, nDrCount, m_NIT );
		if( nDrCount < 0 )
		{
			m_NIT.Reset();
			return;				//	失败
		}
		pNIT->m_wNetworkDrCount = nDrCount;
	}
	else
	{
		pNIT->m_wNetworkDrCount = 0;
		pNIT->m_pDescriptor = NULL;
	}
	pNIT->m_byTableID = byTableID;		// 2010.4.7 modify, using the table ID received
	pNIT->m_dwTableSize = m_NIT.GetMemoryAllocated();

	OnNITReceived( pNIT );
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		计算传输流信息
/// 输入参数:
///		pBuf				缓冲区地址
///		nLen				缓冲区大小
/// 返回参数:
///		>=0					个数
///		<0					失败
int CDVBPSITable_NIT::GetTSItemDescriptorCount( PBYTE pBuf, int nLen )
{
	WORD wCount = 0;
	while( nLen > 0 )
	{
		WORD wItemLen = 6 + ((WORD(pBuf[4]&0xF)<<8)|WORD(pBuf[5]));
		pBuf += wItemLen;
		nLen -= wItemLen;
		wCount ++;
	}
//	ASSERT( 0 == nLen );
	if( nLen != 0 )
		return -1;			//	失败
	return wCount;
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		获取上次接收到的NIT表
/// 输入参数:
///		无
/// 返回参数:
///		NULL		没有接收到
PDVB_PSI_TABLE_BASE CDVBPSITable_NIT::GetTableLastReceived()
{
	if( m_NIT.GetMemoryAllocated() < (int)sizeof(DVB_PSI_TABLE_NIT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_NIT.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		复位
/// 输入参数:
///		bForce			是否强制复位
/// 返回参数:
///		无
void CDVBPSITable_NIT::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
	if( bForce )
		m_NIT.Reset();
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		判断对象是否有效
/// 输入参数:
///		无
/// 返回参数:
///		true			succ
///		false			无效
bool CDVBPSITable_NIT::IsValid()
{
	return m_NIT.IsValid() && CDVBPSITablesBase::IsValid();
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		判断PSI表是否完整
/// 输入参数:
///		无
/// 返回参数:
///		true			完整接收
///		false			没有完整接收
bool CDVBPSITable_NIT::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 12 )	// 一个PAT表至少需要12字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0x3) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_NIT::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;
	fprintf( fOutput, "===================== Dump NIT ============================\n");

	PDVB_PSI_TABLE_NIT pNIT = (PDVB_PSI_TABLE_NIT)GetTableLastReceived();
	ASSERT( pNIT );
	fprintf( fOutput, "  NetworkID=0x%04X, Version=%d, CurrentNext=%d\n",
		pNIT->m_wNetworkID, pNIT->m_byVersionNumber, pNIT->m_bCurrentNextIndicator );
	fprintf( fOutput, "  SectionNumber=%d, LastSectionNumber=%d\n",
		pNIT->m_bySectionNumber, pNIT->m_byLastSectionNumber );
	fprintf( fOutput, "  There are %d Network Descriptors:\n", pNIT->m_wNetworkDrCount );
	int i;
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pNIT->m_pDescriptor;
	for(i=0; i<pNIT->m_wNetworkDrCount; i++)
	{
		ASSERT( pDr );
		CDVBDescriptors::Dump( pDr, fOutput );
		pDr = pDr->m_pNext;
	}
	fprintf( fOutput, "   There are %d Transport stream descriptors:\n", pNIT->m_wItemCount );
	for( i=0; i<pNIT->m_wItemCount; i++)
	{
		fprintf(fOutput, "    Item[%d]:  TSID=0x%04X, OriginalNetID=0x%04X, %d descriptors\n",
			i+1, pNIT->m_aItems[i].m_wTSID, pNIT->m_aItems[i].m_wOriginalNetworkID,
			pNIT->m_aItems[i].m_byCount );
		pDr = pNIT->m_aItems[i].m_pDescriptor;
		for(int j=0; j<pNIT->m_aItems[i].m_byCount; j++ )
		{
			ASSERT( pDr );
			CDVBDescriptors::Dump( pDr, fOutput );
			pDr = pDr->m_pNext;
		}
	}
#else
	(void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		事件，接收到 NIT
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_NIT::OnNITReceived( PDVB_PSI_TABLE_NIT pNIT )
{
#ifdef _DEBUG
	Dump();
#endif // _DEBUG
}

///////////////////////////////////////////////////////////////////////
// Tongshi VOD UniversalTable
CDVBPSI_TSVOD_UniversalTable::CDVBPSI_TSVOD_UniversalTable()
 : CDVBPSITablesBase( TABLE_MAX_SIZE ),
   m_Tbl( DECODED_TABLE_MAX_SIZE )
{
	m_byBuildCounter = 0;
}

///-------------------------------------------------------
/// CYJ,2005-12-9
/// 函数功能:
///		由用户提供缓冲区
/// 输入参数:
///		pRawBuf				接收下来的原始数据缓冲区
///		nRawBufLen			原始数据缓冲区大小
///		pDstBuf				目标缓冲区大小
///		nDstBufLen			目标缓冲区大小
/// 返回参数:
///		无
///	说明：
///		用户提供的缓冲区，在对象删除前，不能被释放。
CDVBPSI_TSVOD_UniversalTable::CDVBPSI_TSVOD_UniversalTable(PBYTE pRawBuf,int nRawBufLen, PBYTE pDstBuf, int nDstBufLen)
	:CDVBPSITablesBase(0),
	m_Tbl(pDstBuf,nDstBufLen)
{
	CMyHeap::Attach( pRawBuf, nRawBufLen );
	m_byBuildCounter = 0;
}

CDVBPSI_TSVOD_UniversalTable::~CDVBPSI_TSVOD_UniversalTable()
{
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		接收到表，进行译码
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_TSVOD_UniversalTable::OnTableReceived()
{
//	TRACE("CDVBPSI_TSVOD_UniversalTable::OnTableReceived\n");

	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 10 )	// 一个PAT表至少需要10字节
		return;

//	ASSERT( *pSrcBuf >= DVBPSI_TBLID_PROGRAM_LIST_TABLE );
	if( *pSrcBuf < DVBPSI_TBLID_PROGRAM_LIST_TABLE)
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSI_TSVOD_UniversalTable: Not a PAT table, tableId=%d != 0\n", *pSrcBuf );
#endif // __ENABLE_TRACE__
		return;						//	不是 PAT 表
	}
	DWORD dwSectionLen = pSrcBuf[1] & 0xF;
	dwSectionLen <<= 8;
	dwSectionLen += pSrcBuf[2];
	dwSectionLen <<= 8;
	dwSectionLen += pSrcBuf[3];

	if( nByteReceived < (int)dwSectionLen+4 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSI_TSVOD_UniversalTable: Bytes received %d < %ld needed.\n", nByteReceived, dwSectionLen+4 );
#endif // __ENABLE_TRACE__
		return;
	}
	if( 0x80 != (pSrcBuf[1] & 0xC0 ) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSI_TSVOD_UniversalTable: section_syntax_indicator and next should be 2,but actual=%d\n", (pSrcBuf[1] & 0xC0 ) >> 6 );
#endif // __ENABLE_TRACE__
		return;
	}

	if( DVB_GetCRC32( pSrcBuf, dwSectionLen+4 ) )
	{							//	包括CRC32在内，整个结构大小＝wSectionLen＋4。
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSI_TSVOD_UniversalTable: Bad CRC32\n");
#endif // __ENABLE_TRACE__
		return;
	}

	// IsTableReceived()
	m_byBuildCounter = pSrcBuf[4];
	uLongf dwDataLen = dwSectionLen - 4 - 2;		// CRC32 and 2 bytes header
	PBYTE pDstBuf;
	if( pSrcBuf[5] & 0x80 )
	{							//	Is compress
		m_Tbl.Reset();			//  CYJ,2005-12-22 free old data in the heap

		DWORD dwOriginalDataLen = 0;
		dwOriginalDataLen = pSrcBuf[6];
		dwOriginalDataLen <<= 8;
		dwOriginalDataLen += pSrcBuf[7];
		dwOriginalDataLen <<= 8;
		dwOriginalDataLen += pSrcBuf[8];
		if( NULL == m_Tbl.Allocate( dwOriginalDataLen ) )		// 确保分配指定的字节数)
		{
#ifdef __ENABLE_TRACE__
			TRACE("Create heap memory failed.\n");
#endif
			m_Tbl.Reset();
			return;
		}
		pDstBuf = m_Tbl.GetHeapBuf();
		dwDataLen = dwOriginalDataLen;
		if( Z_OK != uncompress( pDstBuf, &dwDataLen, pSrcBuf+9, dwSectionLen - 4 - 2 - 3) )	//  CYJ,2005-12-16；4字节CRC32，2字节头部，3字节原始长度
		{
#ifdef __ENABLE_TRACE__
			TRACE("uncompress data failed.\n");
#endif
			return;
		}
#ifdef _DEBUG
		ASSERT( dwDataLen == dwOriginalDataLen );
#endif
	}
	else
		pDstBuf = pSrcBuf + 6;

	OnPrivateDataReceived( pDstBuf, dwDataLen, m_byBuildCounter, pSrcBuf[0] );	//  CYJ,2005-12-14 增加输出 TableID
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		获取上次接收到的表
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSI_TSVOD_UniversalTable::GetTableLastReceived()
{
	if( m_Tbl.GetMemoryAllocated() < 10 )		// 通视VOD表必须大于10字节
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_Tbl.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		复位
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_TSVOD_UniversalTable::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
	m_byBuildCounter = 0;

	if( bForce )
		m_Tbl.Reset();
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSI_TSVOD_UniversalTable::IsValid()
{
	if( false == CDVBPSITablesBase::IsValid() )
		return false;
	return m_Tbl.IsValid();
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		判断表是否完整接收
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSI_TSVOD_UniversalTable::IsPSIPacketIntegral()
{
	DWORD nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 10 )	// 一个PAT表至少需要10字节
		return false;
	DWORD dwSectionLen = pSrcBuf[1] & 0xF;
	dwSectionLen <<= 8;
	dwSectionLen += pSrcBuf[2];
	dwSectionLen <<= 8;
	dwSectionLen += pSrcBuf[3];
	return( nByteReceived >= dwSectionLen+4 );
}

void CDVBPSI_TSVOD_UniversalTable::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;
	fprintf(fOutput, "Dumping Tongshi VOD :=====================================\n");
#else
	(void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-4-20
/// 函数功能:
///		接收到私有数据
/// 输入参数:
///		pBuf				缓冲区
///		nLen				长度
///		byBuildCounter		编译次数计数器
///		byTableID			增加  byTableID
/// 返回参数:
///		无
///	修改记录
///		CYJ,2005-12-16 增加 byTableID
void CDVBPSI_TSVOD_UniversalTable::OnPrivateDataReceived( PBYTE pBuf, int nLen, BYTE byBuildCounter, BYTE byTableID )
{
#ifdef _DEBUG
	Dump();
#endif //_DEBUG
}

//////////////////////////////////////////////////////////////////////////
//	TDT		Time and Date Table
//	TOT		Time Offset Table
//	ST		Stuffing Table
CDVBPSITable_TDT_TOT_ST::CDVBPSITable_TDT_TOT_ST()
  :CDVBPSITablesBase( TABLE_MAX_SIZE )
{
}

CDVBPSITable_TDT_TOT_ST::~CDVBPSITable_TDT_TOT_ST()
{
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///		接收到数据表
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_TDT_TOT_ST::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived >=3 );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 3 )
		return;
	switch( *pSrcBuf )
	{
	case DVBPSI_TBLID_TDT:
		DecodeTDT();
		break;

	case DVBPSI_TBLID_TOT:
		DecodeTOT();
		break;
	}
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///		解码 Time Date Table
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_TDT_TOT_ST::DecodeTDT()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 8 )	// 一个TDT表至少需要 8 字节
		return;

	WORD wSectionLen = pSrcBuf[2];
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeTDT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}

	MY_LONG64 llDateTime = 0;
	for(int i=0; i<5; i++ )
	{
		llDateTime <<= 8;
		llDateTime += pSrcBuf[i+3];
	}

	time_t tCurrentDataTime = ConvertTimeToUTC( llDateTime );

	// 2013.3.28 CYJ Add
	PDVBPSI_TABLE_TOT pTDTTable = (PDVBPSI_TABLE_TOT) m_TOT.Allocate( sizeof(DVBPSI_TABLE_TOT) );
	if( NULL == pTDTTable )
	{
		m_TOT.Reset();
	#ifdef __ENABLE_TRACE__
		TRACE("DecodeTOT: Failed to allocate memory for pTOTTable.\n" );
	#endif // __ENABLE_TRACE__
		return;
	}

	pTDTTable->m_tUTCTime = tCurrentDataTime;
	pTDTTable->m_byTableID = DVBPSI_TBLID_TDT;
	pTDTTable->m_pDescriptor = NULL;
	pTDTTable->m_dwTableSize = m_TOT.GetMemoryAllocated();

	OnDateTimeReceived( tCurrentDataTime, pTDTTable );	// 2013.3.28 CYJ Add argument pTDTTable
}

///-------------------------------------------------------
/// CYJ,2009-9-9
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_TDT_TOT_ST::DecodeTOT()
{
	int i;

	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 14 )	// 一个TOT表至少需要 14 字节
		return;

	m_TOT.Reset();

	WORD wSectionLen = pSrcBuf[2] + (pSrcBuf[1]&3)*0x100;
	if( nByteReceived < wSectionLen+3 )
	{
#ifdef __ENABLE_TRACE__
		TRACE("DecodeTOT: Bytes received %d < %d needed.\n", nByteReceived, wSectionLen );
#endif // __ENABLE_TRACE__
		return;
	}

	pSrcBuf += 3;

	MY_LONG64 llDateTime = 0;
	for(i=0; i<5; i++ )
	{
		llDateTime <<= 8;
		llDateTime += pSrcBuf[i];
	}
	pSrcBuf += 5;

	time_t tCurrentDataTime = ConvertTimeToUTC( llDateTime );

	PDVBPSI_TABLE_TOT pTOTTable = (PDVBPSI_TABLE_TOT) m_TOT.Allocate( sizeof(DVBPSI_TABLE_TOT) );
	if( NULL == pTOTTable )
	{
		m_TOT.Reset();
	#ifdef __ENABLE_TRACE__
		TRACE("DecodeTOT: Failed to allocate memory for pTOTTable.\n" );
	#endif // __ENABLE_TRACE__
		return;
	}

	pTOTTable->m_tUTCTime = tCurrentDataTime;
	pTOTTable->m_byTableID = DVBPSI_TBLID_TOT;

	int nDrLoopLen = (pSrcBuf[0]&0xF) * 0x100 + pSrcBuf[1];
	pSrcBuf += 2;
	if( nDrLoopLen > 0 )
	{
		int nDrCount = 0;
		pTOTTable->m_pDescriptor = DecodeDescriptors( pSrcBuf, nDrLoopLen, nDrCount, m_TOT );
	}
	else
		pTOTTable->m_pDescriptor = NULL;

	pTOTTable->m_dwTableSize = m_TOT.GetMemoryAllocated();

	OnTimeOffsetTableReceived( pTOTTable );
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///
/// 输入参数:
///		无
/// 返回参数:
///		无
PDVB_PSI_TABLE_BASE CDVBPSITable_TDT_TOT_ST::GetTableLastReceived()
{
	// 2013.3.28 CYJ Modify, return TDT/TOT table
	if( m_TOT.GetMemoryAllocated() < (int)sizeof(DVBPSI_TABLE_TOT) )
		return NULL;			// 没有收到
	return (PDVB_PSI_TABLE_BASE)m_TOT.GetHeapBuf();
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///		复位
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSITable_TDT_TOT_ST::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_TDT_TOT_ST::IsValid()
{
	if( !CDVBPSITablesBase::IsValid() )
		return false;

	return true;
}

///-------------------------------------------------------
/// CYJ,2008-9-28
/// 函数功能:
///		是否接收完整
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSITable_TDT_TOT_ST::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 3 )
		return false;						//	只要接收到3个字节，就可以获取 Section Length，然后再判断是否完整
	WORD wSectionLen = pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_TDT_TOT_ST::Dump(FILE*fOutput)
{

}

///////////////////////////////////////////////////////////////////////
// ECM/EMM table
CDVBPSITable_ECM_EMM_Message::CDVBPSITable_ECM_EMM_Message()
  :CDVBPSITablesBase( TABLE_MAX_SIZE )
{
}

CDVBPSITable_ECM_EMM_Message::~CDVBPSITable_ECM_EMM_Message()
{
}

///--------------------------------------------------------
/// CYJ, 2010-9-29 上午10:29:40
/// Function:
///
/// Input:
///     None
/// Output:
///     None
void CDVBPSITable_ECM_EMM_Message::OnTableReceived()
{
	int nByteReceived = GetMemoryAllocated();
	ASSERT( nByteReceived );
	PBYTE pSrcBuf = GetHeapBuf();
	ASSERT( pSrcBuf );
	if( NULL == pSrcBuf || nByteReceived <= 3 )	// 一个ECM/EMM表至少需要3字节
		return;

	BYTE byTableID = *pSrcBuf;
	if( byTableID < 0x80 || byTableID > 0x8F )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITable_ECM_EMM_Message: Not a ECM/EMM table, tableId=%d (0x80~0x8F)\n", byTableID );
#endif // __ENABLE_TRACE__
		return;						//	不是 PMT 表
	}

	// 2011.4.1 CYJ Modify, since one ECM table may contain more than one ECM Section
	while( nByteReceived > 3 && (pSrcBuf[0]&0xF0) == 0x80 )
	{
		int nSectionLen = ( (pSrcBuf[1]&0xF) << 8 ) + pSrcBuf[2] + 3;
		if( nByteReceived < nSectionLen )
			break;

		OnECM_EMMMessageReceived( byTableID, pSrcBuf, nSectionLen );

		nByteReceived -= nSectionLen;
		pSrcBuf += nSectionLen;
	}
}

///--------------------------------------------------------
/// CYJ, 2010-9-29 上午10:29:40
/// Function:
///		获取上次接收的表
/// Input:
///     None
/// Output:
///     None
PDVB_PSI_TABLE_BASE CDVBPSITable_ECM_EMM_Message::GetTableLastReceived()
{
	return NULL;
}

///--------------------------------------------------------
/// CYJ, 2010-9-29 上午10:29:40
/// Function:
///		复位
/// Input:
///     None
/// Output:
///     None
void CDVBPSITable_ECM_EMM_Message::Reset(bool bForce)
{
	CDVBPSITablesBase::Reset( bForce );
}

///--------------------------------------------------------
/// CYJ, 2010-9-29 上午10:29:40
/// Function:
///
/// Input:
///     None
/// Output:
///     None
bool CDVBPSITable_ECM_EMM_Message::IsValid()
{
	return CDVBPSITablesBase::IsValid();
}

///--------------------------------------------------------
/// CYJ, 2010-9-29 上午10:29:40
/// Function:
///		判断是否已经完整接收
/// Input:
///     None
/// Output:
///     None
bool CDVBPSITable_ECM_EMM_Message::IsPSIPacketIntegral()
{
	int nByteReceived = GetMemoryAllocated();
	if( nByteReceived <= 0 )
        return false;
	PBYTE pSrcBuf = GetHeapBuf();
	if( NULL == pSrcBuf || nByteReceived <= 3 )	// 一个PAT表至少需要3字节
		return false;
	WORD wSectionLen = ((pSrcBuf[1]&0xF) << 8) | pSrcBuf[2];
	return( nByteReceived >= wSectionLen+3 );
}

void CDVBPSITable_ECM_EMM_Message::Dump(FILE*fOutput)
{
}
