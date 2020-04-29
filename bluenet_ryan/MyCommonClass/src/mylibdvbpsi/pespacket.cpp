///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-12
///
///		用途：
///			PES 分组重新构造
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"

#include <stdio.h>

#include "pespacket.h"
#include "bitstream.h"
#include <MyArray.h>
#include "dvb_crc.h"

#ifdef _DEBUG
	#define __ENABLE_TRACE__
#endif // _DEBUG

class CPESResponserArray : public CMyArray<CPESPacket*>
{
public:
	CPESResponserArray(){}
	virtual ~CPESResponserArray(){}
};

CPCR_TSPacketResponser::CPCR_TSPacketResponser()
{
	m_paPESResponser = new CPESResponserArray;
}

CPCR_TSPacketResponser::~CPCR_TSPacketResponser()
{
	if( m_paPESResponser )
		delete m_paPESResponser;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		接收 PCR TS 分组
/// 输入参数:
///		pPacket			TS 分组
/// 返回参数:
///		无
void CPCR_TSPacketResponser::PushOneTSPacket( PDVB_TS_PACKET pPacket )
{
	ASSERT( m_paPESResponser );

	if( NULL == m_paPESResponser )
		return;
	int nCount = m_paPESResponser->GetSize();
	if( 0 == nCount )
		return;
	CPESPacket ** ppResponser = m_paPESResponser->GetData();
	if( !(*ppResponser)->HandlePCR( pPacket ) )
		return;				//	没有 PCR

	MY_LONG64 llPCR = 0;
	WORD wExtension = 0;
	(*ppResponser)->GetPCR( llPCR, wExtension );

	for(int i=1; i<nCount; i++)
	{						//	优化操作，不用再解析PCR了。
		ppResponser ++;
		(*ppResponser)->SetPCR( llPCR, wExtension );
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CPCR_TSPacketResponser::IsValid()
{
	return ( m_paPESResponser != NULL );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		删除所有 PES 处理器
/// 输入参数:
///		无
/// 返回参数:
///		无
void CPCR_TSPacketResponser::RemoveAll()
{
	if( m_paPESResponser )
		m_paPESResponser->RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		删除一个 PES 处理器
/// 输入参数:
///		pResponser		待删除的 PES 处理器
/// 返回参数:
///		无
void CPCR_TSPacketResponser::Remove( CPESPacket * pResponser )
{
	int nNo = Find( pResponser );
	if( nNo >= 0 )
		m_paPESResponser->RemoveAt( nNo );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		查找一个 PES 处理器
/// 输入参数:
///		pResponser			查找对象
/// 返回参数:
///		>=0					序号
///		<0					失败
int	CPCR_TSPacketResponser::Find( CPESPacket * pResponser )
{
	ASSERT( m_paPESResponser && pResponser );
	if( NULL == m_paPESResponser )
		return -1;
	int nCount = m_paPESResponser->GetSize();
	for( int i=0; i<nCount; i++)
	{
		if( m_paPESResponser->ElementAt( i ) == pResponser )
			return i;
	}
	return -1;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		添加一个 PES 处理器
/// 输入参数:
///		pResponser		待添加的 PES 处理器
/// 返回参数:
///		无
void CPCR_TSPacketResponser::Add( CPESPacket * pResponser )
{
	ASSERT( m_paPESResponser && pResponser );
	if( NULL == m_paPESResponser || NULL == pResponser )
		return;
	if( Find( pResponser ) >= 0 )
		return;				//	已经存在
	m_paPESResponser->Add( pResponser );
}

//---------------------------------------------------------
//	PES Packet data struct
SELFDEFINE_PES_PROCESS_MODE tagDVB_PES_PACKET_HEADER::GetProcessMode()
{
	ASSERT( IsPESPacket() );
	if( DVBPESSI_PADING_STREAM == m_byStreamID )
		return PES_PROCESS_MODE_PADING;
	else if( DVBPESSI_PROGRAM_STREAM_MAP == m_byStreamID
			|| DVBPESSI_PRIVATE_STREAM_2 == m_byStreamID
			|| DVBPESSI_ECM_STREAM == m_byStreamID
			|| DVBPESSI_EMM_STREAM == m_byStreamID
			|| DVBPESSI_PROGRAM_STREAM_DIRECTORY == m_byStreamID )
	{
		return PES_PROCESS_MODE_EMM_EMC_COMPATIBLE;
	}
	return PES_PROCESS_MODE_AV_STREAM_COMPATIBLE;
}

///-------------------------------------------------------
/// CYJ,2005-1-13
/// 函数功能:
///		获取PES头大小
/// 输入参数:
///		无
/// 返回参数:
///		PES 头大小
///		<0			失败
int tagDVB_PES_PACKET_HEADER::GetHeaderLen()
{
	SELFDEFINE_PES_PROCESS_MODE eProcessMode = GetProcessMode();
	if( PES_PROCESS_MODE_PADING == eProcessMode )
		return -1;

	else if( PES_PROCESS_MODE_EMM_EMC_COMPATIBLE == eProcessMode )
		return 6;

	WORD wOfs;
	if( 0x80 == ( m_abyData[0] & 0xC0 ) )
	{									// 以 '10' 打头
		wOfs = 3 + m_abyData[2];		// 两个字节的标志位，1个字节的头长度
	}
	else
	{
		// FIXME
		wOfs = 0;
		while( wOfs < 23 && m_abyData[wOfs] == 0xFF )
		{								// 跳过 MPEG－1 的填充字节
			wOfs ++;
		}
		if( wOfs >= 23 )
			return -1;
	}
	return wOfs + 6;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		获取有效负载地址与长度
/// 输入参数:
///		nPacketLen			输出负载大小
/// 返回参数:
///		有效负载地址
///		NULL				没有数据
PBYTE tagDVB_PES_PACKET_HEADER::GetPayloadData( int & nPayloadDataLen )
{
	nPayloadDataLen = 0;
	int nHeaderSize = GetHeaderLen();
	if( nHeaderSize <= 6 )			//	必须超过 6 字节
		return NULL;

	nHeaderSize -= 6;				//	固定包头，因为 HeaderLen 包含固定头,而数据是从第六个字节开始算的.

	WORD wPackLen = GetPacketLength();
	if( wPackLen )
		nPayloadDataLen = wPackLen - nHeaderSize;
	else
		nPayloadDataLen = 0;			//	未知大小

	ASSERT( nPayloadDataLen >= 0 );
	return m_abyData + nHeaderSize;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPESPacket::CPESPacket(int nCacheBufSize) : CMyHeap( nCacheBufSize )
{
	m_wPS_SCR_Base_0_14 = 0;			//	system clock reference
	m_wPS_SCR_Base_15_29 = 0;
	m_wPS_SCR_Base_30_32 = 0;
	m_wPS_SCR_Extension = 0;			// system clock reference extension
	m_bRequireCompletePacket = true;	//	要求输出完整的PES分组，对于视频，允许输出不完整的分组，后面处理器再拼接
	m_nOutputMethod = OUTPUT_METHOD_AS_ELEMENTARY_STREAM;	// 缺省输出为元素流
	// FIXME,不知该填写什么值
	m_dwProgramMuxRate = 0x71CC;
	m_pPCRResponser = NULL;
	m_nPayloadLen = 0;
	m_bDoCRCCheck = true;
	m_byLastHasCRCFlags = 0;

	Reset( true );
}

CPESPacket::~CPESPacket()
{

}

///-------------------------------------------------------
/// CYJ,2005-1-11
/// 函数功能:
///		复位表结构，相对于初始化
/// 输入参数:
///		bForce			是否强制复位，缺省为false
/// 返回参数:
///		无
void CPESPacket::Reset(bool bForce)
{
	m_bHeaderReceived = false;
	m_wErrorCounter = 0;					// TS packet 发生错误的次数
	m_byExpectTSContinuityCounter = 0xF0;	// 上次 TS packet 的continuity counter
	m_dwPESBytesReceived = 0;

	CMyHeap::Reset();

	m_bHasPTS = false;
	m_bHasDTS = false;
	m_bHasESMuxRate = false;

	m_byStreamID = 0;
	m_wPacketLength = 0;
	m_bHasSCR = false;
	m_nPayloadLen = 0;
	m_bRandomAccessPoint = false;
	m_bTSPacketDiscontinuity = false;

	if( bForce )
	{
		m_bRequireCompletePacket = true;		//	要求输出完整的PES分组，对于视频，允许输出不完整的分组，后面处理器再拼接
		m_nOutputMethod = OUTPUT_METHOD_AS_ELEMENTARY_STREAM;	// 缺省输出为元素流
		m_pPCRResponser = NULL;
		m_bDoCRCCheck = true;			//  CYJ,2006-3-2 增加是否进行CRC检测
	}
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
bool CPESPacket::IsValid()
{
	return CMyHeap::IsValid();
}

///--------------------------------------------------------------
///	CYJ, 2005-1-13
///	函数功能:
///		处理一个TS分组
///	输入参数:
///		pPacket			接收到的TS分组
///	返回参数:
///		无
void CPESPacket::PushOneTSPacket( PDVB_TS_PACKET pPacket )
{
	ASSERT( IsValid() );

	bool bIsPayloadStart = pPacket->IsPayloadUnitStart();

	if( !m_bHeaderReceived && !bIsPayloadStart )
		return;					//	没有接收到数据头，且不是一个表的开始，

	int nDataLen;
	PBYTE pPayloadData = pPacket->GetPayloadData( nDataLen );
	if( NULL == pPayloadData )
	{
#ifdef __ENABLE_TRACE__
	//	TRACE("No TS payload data.\n");
#endif //__ENABLE_TRACE__
		HandlePCR( pPacket );
		return;					//	没有数据
	}

	BYTE byContinuityCounter = pPacket->GetContinuityCount();
	bool m_bTSPacketDiscontinuity = (pPacket->m_abyData[3]&0x20) && (pPacket->m_abyData[5] & 0x80);

	//	FIXME, 有必要判断TS分组是否重复
#if 0
	if( !m_bTSPacketDiscontinuity && (m_byExpectTSContinuityCounter == ((byContinuityCounter+1)&0xF)) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBPSITablesBase::PushOneTSPacket, find one duplicated TS packeted.\n");
#endif // __ENABLE_TRACE__
		return;					//	重复的TS分组
	}
#endif // 0

	int nByteHasAllocated = GetMemoryAllocated();
	if( bIsPayloadStart )
	{									// 接收到一个新表
		if( m_bHeaderReceived && nByteHasAllocated  > 0 )	// 提交原来的数据
		{
			if( m_nPayloadLen > 0 )
			{							//	有效的负载大小，判断是否越界
				int nByteLeft = m_nPayloadLen - m_dwPESBytesReceived;	// m_dwPESBytesReceived 表示已经发送的字节数据
				if( nByteHasAllocated > nByteLeft )
					nByteHasAllocated = nByteLeft;		//	去掉多余的部分
			}
			if( nByteHasAllocated > 0 )				//  CYJ,2006-3-1 防止错误的 nByteHasAllocated
			{
				bool bDoSendEvent = true;
				if( m_nPayloadLen && m_bDoCRCCheck && m_bRequireCompletePacket && 0 == m_dwPESBytesReceived )
				{	// 只有明确定义 m_nPayloadLen，且要求作 CRC 检测，同时要求完整输出，且还没有强制输出时，才会做CRC检查
					// 一般情况下，只有在ES输出时，才需要做CRC检测
					PBYTE pCRC = GetCRC( pPayloadData, nDataLen );
					m_byLastHasCRCFlags <<= 1;
					if( pCRC )
					{								// 计算CRC
						m_byLastHasCRCFlags |= 1;	// 标记上一次有CRC
						int nPESHeaderLen = nByteHasAllocated - m_nPayloadLen;
						if( nPESHeaderLen >= 0 && nByteHasAllocated-nPESHeaderLen > 0 )
						{
							WORD wCRC = DVB_GetCRC16( GetHeapBuf()+nPESHeaderLen, nByteHasAllocated-nPESHeaderLen );
							bDoSendEvent = ( wCRC == ( (pCRC[0]<<8) | pCRC[1] ) );
						}
						else
							bDoSendEvent = false;

					#if 1 // _DEBUG
						if( !bDoSendEvent )
							fprintf( stderr, "One PES\'s CRC error, abort\n" );
					#endif //_DEBUG
					}
					else
					{
						 if( m_byLastHasCRCFlags & 0xF )	// 若连续4次都没有CRC，才认为没有；否则先认为是PES本身的错误，若下次还没有，才认为没有
						{
							bDoSendEvent = false;
						#ifdef _DEBUG
							fprintf(stderr,"PESPacket, m_byLastHasCRCFlags=%x, same as CRC error\n", m_byLastHasCRCFlags );
						#endif //_DEBUG
						}
					}
				}
				if( bDoSendEvent )
				{
					int nByteLeftInHeap = GetHeapSize() - nByteHasAllocated;
					if( nByteLeftInHeap > 16 )
						nByteLeftInHeap = 16;
					if( nByteLeftInHeap > 0 )
						memset( GetHeapBuf() + nByteHasAllocated, 0xFF, nByteLeftInHeap );		// 末端填充 0XFF，防止出错
					OnPESReceived( GetHeapBuf(), nByteHasAllocated, m_dwPESBytesReceived, m_wErrorCounter );	//	通知接收到表
				}
			}
			Reset();				//	重新开始
		}
		PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pPayloadData;
		if( !pPESHeader->IsPESPacket() )
		{							//	错误的PES头
			m_bHeaderReceived = false;
#ifdef __ENABLE_TRACE__
			TRACE("No a PES packet.\n");
#endif //__ENABLE_TRACE__
			return;
		}
		m_bHeaderReceived = true;

		DecodeTSAdaptionField( pPacket );
		DecodePESHeader( pPayloadData, nDataLen );
		HandlePCR( pPacket );		// handle PCR

		PBYTE pPESPayloadData = pPESHeader->GetPayloadData( m_nPayloadLen );
		ASSERT( m_nPayloadLen >= 0 && pPESPayloadData );
		if( m_nPayloadLen < 0 || NULL == pPESPayloadData )
		{			//  CYJ,2005-7-1 防止错误数据
#ifdef __ENABLE_TRACE__
			TRACE("No a PES packet. Payload len < 0\n");
#endif //__ENABLE_TRACE__
			return;
		}


		if( OUTPUT_METHOD_AS_ELEMENTARY_STREAM == m_nOutputMethod )
		{		//	输出为元素流，还要去掉 PES 头
			pPayloadData = pPESPayloadData;
			ASSERT( pPayloadData );

			nDataLen = PBYTE(pPacket) + DVB_TS_PACKET_SIZE - pPayloadData;	//	需要自己计算

			if( NULL == pPayloadData || nDataLen <= 0 )
			{
				m_nPayloadLen = 0;
#ifdef __ENABLE_TRACE__
			//	TRACE("No payload data.\n");
#endif //__ENABLE_TRACE__
				return;								//	没有元素流
			}
		}
		else
		{
			m_nPayloadLen = 0;						//	其他方式，不做 Payload data len 限制
			if( OUTPUT_METHOD_AS_PROGRAM_STREAM == m_nOutputMethod )
				ConstructPSPackHeader();				//	输出PS头
		}

		nByteHasAllocated = GetMemoryAllocated();	//	重新获取已经写入的字节数
	}
	else
	{
		ASSERT( m_bHeaderReceived );
		HandlePCR( pPacket );
	}

	if( m_pPCRResponser )				//	触发其他PES处理器
		m_pPCRResponser->PushOneTSPacket( pPacket );

	if( m_bTSPacketDiscontinuity )		// 已经指定TS分组不连续了
		m_byExpectTSContinuityCounter = byContinuityCounter;

	if( pPacket->IsError() || (false==bIsPayloadStart && m_byExpectTSContinuityCounter != byContinuityCounter) )
	{
	#ifdef __ENABLE_TRACE__
		TRACE("Expect=%d != %d\n", m_byExpectTSContinuityCounter, byContinuityCounter );
	#endif
		m_wErrorCounter ++;		//	丢包，只是标记，但不放弃
	}

	//  CYJ,2009-6-23 修改，GetHeapSize() ==> GetHeapMaxSize()
	if( (!m_bRequireCompletePacket) ||\
		( (0==m_wPacketLength) && ( (nByteHasAllocated+nDataLen) >= GetHeapMaxSize() ) ) )
	{		//	不要求输出完整的PES分组或包长度为0，且缓冲区即将溢出
		if( m_nPayloadLen > 0 )
		{							//	有效的负载大小，判断是否越界
			int nByteLeft = m_nPayloadLen - m_dwPESBytesReceived;
			if( nByteHasAllocated > nByteLeft )
				nByteHasAllocated = nByteLeft;		//	去掉多余的部分
		}
		if( nByteHasAllocated > 0 )					//  CYJ,2006-3-1 防止错误数据
		{
			int nByteLeftInHeap = GetHeapSize() - nByteHasAllocated;
			if( nByteLeftInHeap > 16 )
				nByteLeftInHeap = 16;
			if( nByteLeftInHeap > 0 )
				memset( GetHeapBuf() + nByteHasAllocated, 0xFF, nByteLeftInHeap );		// 末端填充 0XFF，防止出错
			OnPESReceived( GetHeapBuf(), nByteHasAllocated, m_dwPESBytesReceived, m_wErrorCounter );	//	通知接收到表
			m_dwPESBytesReceived += nByteHasAllocated;
		}
		else
			m_dwPESBytesReceived = 0;				// 出错

		CMyHeap::Reset();		//	清除缓冲
	}

	m_byExpectTSContinuityCounter = (byContinuityCounter+1) & 0xF;
	if( false == Write( pPayloadData, nDataLen ) )
	{
		Reset();						//	放弃，重新开始
		m_bHeaderReceived = false;		//	写入数据失败，放弃接收
	}
}

///-------------------------------------------------------
/// CYJ,2005-2-25
/// 函数功能:
///		解析TS分组的适配域字段
/// 输入参数:
///		pTSPacket			TS 分组
/// 返回参数:
///		无
void CPESPacket::DecodeTSAdaptionField( PDVB_TS_PACKET pTSPacket )
{
	if( !(pTSPacket->m_abyData[3] & 0x20) )
		return;			//	没有 adaption field
	m_bRandomAccessPoint = (pTSPacket->m_abyData[5] & 0x40) ? true : false;
}

///--------------------------------------------------------------
///	CYJ, 2005-1-13
///	函数功能:
///		打印
///	输入参数:
///		无
///	返回参数:
///		无
void CPESPacket::Dump(FILE*fOutput)
{
#ifdef _DEBUG
	PDVB_PES_PACKET_HEADER pPES = (PDVB_PES_PACKET_HEADER)GetHeapBuf();
	if( NULL == fOutput )
		fOutput = stderr;
	fprintf( fOutput, "Dumping PES. -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n" );
	fprintf( fOutput, "StartPreFix=%02X,%02X,%02X, StreamID=%2X, PacketLen=%d.\n",
		pPES->m_abyStartCodePrefix[0], pPES->m_abyStartCodePrefix[1],
		pPES->m_abyStartCodePrefix[2], pPES->m_byStreamID,
		pPES->GetPacketLength() );
	int nLen;
	PBYTE pPayloadPtr = pPES->GetPayloadData( nLen );
	fprintf( fOutput, "Payload Buffer=%p, nLen=%d\n", pPayloadPtr, nLen );
	if( pPayloadPtr )
	{
		fprintf( fOutput, "   Payload Data: %02X %02X %02X %02X %02X %02X\n",
			pPayloadPtr[0], pPayloadPtr[1], pPayloadPtr[2], pPayloadPtr[3],
			pPayloadPtr[4], pPayloadPtr[5] );
	}
#else
	(void)fOutput;
#endif // _DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		设置是否以Program Stream方式输出
/// 输入参数:
///		nMethod				输出方式
/// 返回参数:
///		原来的设置
///	说明：
///		1、以Program Stream方式输出，表明，在最开始的时候，需要输出一个PS头（包括system_header)＋PES分组
///		   以后，当PES分组包含 ESCR 字段时，输出一PS头（无system_header)＋PES分组
///		2、只输出PES分组，即元素流前面还有PES头
///		3、元素流，不包括PS头，也不包括PES头
int CPESPacket::SetOutputMethod( int nMethod )
{
	int nRetVal = m_nOutputMethod;
	m_nOutputMethod = nMethod;

	Reset();

	if( OUTPUT_METHOD_AS_PROGRAM_STREAM == m_nOutputMethod )
		ConstructPSPackHeader( true );

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		设置是否要求输出完整的PES分组
/// 输入参数:
///		bComplete			缺省为 true
/// 返回参数:
///		原来的设置
///	说明：
///		若要求输出完整的PES分组，表示必须接收到完整的PES分组后，才调用事件，提交接收到的PES分组
///		反之，只要接收到TS分组，就调用事件，输出分组部分数据。
bool CPESPacket::SetRequireCompletePESPacket(bool bComplete)
{
	bool bRetVal = m_bRequireCompletePacket;

	m_bRequireCompletePacket = bComplete;
	Reset();

	return bRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-1-14
/// 函数功能:
///		构造 PS Pack header，并输出到缓冲区中
/// 输入参数:
///		无
/// 返回参数:
///		无
void CPESPacket::ConstructPSPackHeader(bool bOutputSystemHeader)
{
	PBYTE pBuf = Allocate(14);			// 固定大小
	CMyBitStream OutputBs( pBuf, 14 );

	OutputBs.PutBits32( 0x1BA );		// pack_start_code, 0x1BA
	OutputBs.PutBits( 1, 2 );			// 01
	OutputBs.PutBits( m_wPS_SCR_Base_30_32, 3 );
	OutputBs.PutBit( 1 );				// market

	OutputBs.PutBits( m_wPS_SCR_Base_15_29, 15 );
	OutputBs.PutBit( 1 );				// market bit

	OutputBs.PutBits( m_wPS_SCR_Base_0_14, 15 );
	OutputBs.PutBit( 1 );

	OutputBs.PutBits( m_wPS_SCR_Extension, 9 );
	OutputBs.PutBit( 1 );

	OutputBs.PutBits( m_dwProgramMuxRate, 22 );
	OutputBs.PutBits( 3, 2 );			// 2 market bits
	OutputBs.PutBits( 0, 5 );			// reserved 5
	OutputBs.PutBits( 0, 3 );			// packet stuffing_length

	ASSERT( OutputBs.GetTotalWriteBits() == 14*8 );

	OutputBs.FinishWrite();

	ASSERT( OutputBs.GetTotalWriteBits() == 14*8 );

	if( false == bOutputSystemHeader )
		return;

	static BYTE abySystemHeader[]=
	{
		0x0,	0x0,	0x01,	0xbb,	0x00,	0x0c,	0x80,	0x7F,
		0x5d,	0x04,	0x21,	0x7f,	0xE0,	0xE0,	0xE0,	0xC0,
		0xC0,	0x20,	0x00,	0x00,	0x01,	0xbe,	0x07,	0xDA
	};
	Write( abySystemHeader, sizeof(abySystemHeader) );
}

///-------------------------------------------------------
/// CYJ,2005-1-19
/// 函数功能:
///		解析PES头
/// 输入参数:
///		pBuf				缓冲区地址
///		nLen				数据长度
/// 返回参数:
///		无
void CPESPacket::DecodePESHeader(PBYTE pBuf, int nLen)
{
	ASSERT( !m_bHasPTS && !m_bHasDTS && pBuf && nLen > 6 );

	PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pBuf;
	if( !pPESHeader->IsPESPacket() )
		return;
	if( pPESHeader->GetProcessMode() != PES_PROCESS_MODE_AV_STREAM_COMPATIBLE || nLen <= 6 )
		return;

	m_byStreamID = pPESHeader->m_byStreamID;
	m_wPacketLength = pPESHeader->GetPacketLength();

	pBuf += 6;				//	跳过 PES 头，6个字节
	CMyBitStream InBs( pBuf, nLen-6 );
	if( InBs.getbits(2) != 2 )
		return;
	InBs.getbits( 6 );		//	跳过 scrambling_control,priority, data alignment, copyright, original_or_copy
	if( InBs.getbits(1) )
	{						//	首先有PTS
		m_bHasPTS = true;
		m_bHasDTS = (InBs.getbits(1) == 1);
	}
	else
		InBs.getbits(1);	// 跳过后面的一个无用比特

	bool bIsSCR = ( InBs.getbits(1) == 1 );
	m_bHasESMuxRate = ( InBs.getbits(1) == 1 );

	InBs.getbits(12);		//	跳过后续的12比特

	if( m_bHasPTS )
	{
		InBs.getbits(4);
		m_PTS = InBs.getbits(3);
		m_PTS <<= 30;
		InBs.getbits(1);	// marker bit
		m_PTS |= (InBs.getbits(15)<<15);
		InBs.getbits(1);	// marker bit
		m_PTS |= InBs.getbits(15);
		InBs.getbits(1);	// marker bit
	}
	else
		m_PTS = 0;

	if( m_bHasDTS )
	{
		ASSERT( m_bHasPTS );
		InBs.getbits(4);
		m_DTS = InBs.getbits(3);
		m_DTS <<= 30;
		InBs.getbits(1);	// marker bit
		m_DTS |= (InBs.getbits(15)<<15);
		InBs.getbits(1);	// marker bit
		m_DTS |= InBs.getbits(15);
		InBs.getbits(1);	// marker bit
	}
	else
		m_DTS = 0;

	if( bIsSCR )
	{
		m_bHasSCR = true;
		InBs.getbits(2);
		m_wPS_SCR_Base_30_32 = InBs.getbits(3);
		m_PCR = m_wPS_SCR_Base_30_32;
		m_PCR <<= 18;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Base_15_29 = InBs.getbits(15);
		m_PCR |= m_wPS_SCR_Base_15_29;
		m_PCR <<= 15;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Base_0_14 = InBs.getbits(15);
		m_PCR |= m_wPS_SCR_Base_0_14;
		InBs.getbits(1);	// marker bit
		m_wPS_SCR_Extension = InBs.getbits(9);
		InBs.getbits(1);	// marker bit
	}

	if( m_bHasESMuxRate )
	{
		InBs.getbits(1);	// marker bit
		m_dwProgramMuxRate = InBs.getbits(22);	// marker bit
		InBs.getbits(1);	// marker bit
	}
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		处理 TS 包中的PCR数据
/// 输入参数:
///		pTSPacket		待处理的TS分组
/// 返回参数:
///		true			有 PCR
///		false			无 PCR
bool CPESPacket::HandlePCR(PDVB_TS_PACKET pTSPacket)
{
	if( !( (pTSPacket->m_abyData[3] & 0x20) && (pTSPacket->m_abyData[5] & 0x10 )
		&& ( pTSPacket->m_abyData[4] >= 7 ) ) )
	{
		return false;			//	没有 adaption field，或没有PCR字段
	}


	m_PCR = ( (DWORD)(pTSPacket->m_abyData[6]) << 24) |
			( (DWORD)(pTSPacket->m_abyData[7]) << 16) |
			( (DWORD)(pTSPacket->m_abyData[8]) <<  8) |
			( (DWORD)(pTSPacket->m_abyData[9]) );
	m_PCR <<= 1;
	m_PCR |= ( (pTSPacket->m_abyData[10]>>7)&1 );

	m_wPS_SCR_Base_0_14 = WORD(m_PCR >> 30);
	m_wPS_SCR_Base_15_29 = WORD( ( m_PCR >> 15) & 0x7FFF );
	m_wPS_SCR_Base_30_32 = WORD( m_PCR & 0x7FFF );

	m_wPS_SCR_Extension = pTSPacket->m_abyData[11] | ( (pTSPacket->m_abyData[10]&1) << 8 );

	m_bHasSCR = true;

	return true;			//	没有 adaption field
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		读取 PCR
/// 输入参数:
///		scr				输出
///		wExtension		扩展输出
/// 返回参数:
///		true			成功
///		false			没有 scr
bool CPESPacket::GetPCR(MY_LONG64 & scr, WORD & wExtension)
{
	if( m_bHasSCR )
	{
		scr = m_PCR;
		wExtension = m_wPS_SCR_Extension;
	}
	return m_bHasSCR;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		设置 PCR
/// 输入参数:
///		scr					SCR
///		wExtension			扩展时间
/// 返回参数:
///		无
void CPESPacket::SetPCR(MY_LONG64 & scr, WORD wExtension )
{
	m_bHasSCR = true;
	m_PCR = scr;
	m_wPS_SCR_Extension = wExtension;

	m_wPS_SCR_Base_0_14 = WORD(m_PCR >> 30);
	m_wPS_SCR_Base_15_29 = WORD( ( m_PCR >> 15) & 0x7FFF );
	m_wPS_SCR_Base_30_32 = WORD( m_PCR & 0x7FFF );
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		获取相关联的PCR处理器
/// 输入参数:
///		无
/// 返回参数:
///		PCR处理器
/// 说明：
///		当 PCR_PID = 本PES's PID 时，需要用该对象去驱动其他的PES处理器
CPCR_TSPacketResponser * CPESPacket::GetAssociatedPCRResponser()
{
	return m_pPCRResponser;
}

///-------------------------------------------------------
/// CYJ,2005-1-20
/// 函数功能:
///		设置 PCR 处理器对象
/// 输入参数:
///		pResponser		PCR 处理器
/// 返回参数:
///		原来的处理器对象指针
/// 说明：
///		当 PCR_PID = 本PES's PID 时，需要用该对象去驱动其他的PES处理器
CPCR_TSPacketResponser * CPESPacket::SetAssociatedPCRResponser( CPCR_TSPacketResponser * pResponser)
{
	CPCR_TSPacketResponser * pRetVal = m_pPCRResponser;
	m_pPCRResponser = pResponser;

	if( m_pPCRResponser )
		m_pPCRResponser->Remove( this );		//	删除自己

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2006-3-2
/// 函数功能:
///		获取PES头部中的CRC字段
/// 输入参数:
///		pBuf			PES 头部
///		nLen			长度
/// 返回参数:
///		NULL				失败
///		其他				低16位为CRC16
PBYTE CPESPacket::GetCRC(PBYTE pBuf, int nLen) const
{
	PDVB_PES_PACKET_HEADER pPESHeader = (PDVB_PES_PACKET_HEADER)pBuf;
	if( !pPESHeader->IsPESPacket() )
		return NULL;
	if( pPESHeader->GetProcessMode() != PES_PROCESS_MODE_AV_STREAM_COMPATIBLE || nLen <= 6 )
		return NULL;
	//	跳过 PES 头，6个字节
	pBuf += 6;
	nLen -= 6;
	if( (pBuf[0] & 0xC0) != 0x80 )
		return NULL;			// not '10'
	if( (pBuf[1] & 2) == 0 )
		return NULL;			// not CRC

	CMyBitStream InBs( pBuf, nLen );
	InBs.getbits(8);

	pBuf += 3;					// 头部3个字节

	switch( InBs.getbits(2) )
	{							// PTS_DTS
	case 2:						// '10'
		pBuf += 5;
		break;
	case 3:						// '11'
		pBuf += 10;
		break;
	}

	if( InBs.getbits(1) )		// ESCR
		pBuf += 6;
	if( InBs.getbits(1) )		// ES Rate
		pBuf += 3;
	if( InBs.getbits(1) )		// DSM
		pBuf ++;
	if( InBs.getbits(1) )		// additional copy info
		pBuf ++;
	ASSERT( InBs.getbits(1) );
	return pBuf;
}

///-------------------------------------------------------
/// CYJ,2006-3-2
/// 函数功能:
///		进行CRC检测
/// 输入参数:
///		bDoCheck			是否进行检测
/// 返回参数:
///		true				允许
///		false				不能进行crc检测，因为没有要求完整输出
bool CPESPacket::DoCRCCheck(bool bDoCheck)
{
	if( false == m_bRequireCompletePacket )
		return false;
	m_bDoCRCCheck = bDoCheck;
	return true;
}
