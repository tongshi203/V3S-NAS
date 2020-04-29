///=======================================================
///    
///     作者: 王国雄
///    
///     niniryuhappy@gmail.com
///    
///     日期: 2006-4-17
///     文件: OneProgram_EITGenerator.cpp
///     版本: 1.0.0.0
///     说明: 
///    
///========================================================
// OneProgram_EITGenerator.cpp: implementation of the COneProgram_EITGenerator class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "dvbpsitablesdefine.h"
#include "dvbpsieitgenerator.h"
#include <time.h>
#include "dvb_crc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ONE_EXTEND_DSCPTOR_TEXT_MAXLEN 240
#define EXTEND_DSCPTOR_HEADER_LEN 8
#define SHOTR_DECPTOR_HEADER_LEN 7
#define EVENT_HEANDER_LEN 12
#define EIT_TABLE_HEADER_LEN 14
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COneProgram_EITGenerator::COneProgram_EITGenerator()
{
	m_pFollowingEvent = NULL;
	m_pPresentEvent = NULL;
	Preset();
	m_nRefCount = 0;
	memset( m_aPFEventBuffer, 0, sizeof(m_aPFEventBuffer) );	//  CYJ,2006-12-31 用于当前/下一个事件缓冲区，这样不用老申请/释放内存	
	
}

COneProgram_EITGenerator::~COneProgram_EITGenerator()
{
	m_aPFEITBuf.RemoveAll();
	m_aScheduleSegment.RemoveAll();
	m_aScheduleEvent.RemoveAll();
	m_aScheduleEITBuf.RemoveAll();
	m_aScheduleSection.RemoveAll();	
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		增加引用计数
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
long COneProgram_EITGenerator::AddRef()
{
	return InterlockedIncrement(&m_nRefCount);
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		减少引用计数
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
long COneProgram_EITGenerator::Release()
{
	long nRetval = InterlockedDecrement(&m_nRefCount);
	if (0 == nRetval) 
		delete this;
	return nRetval;
}
///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		删除所有Schedule事件
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::RemoveAllSchedule()
{
	m_aScheduleEvent.RemoveAll();
	m_bEventScheduleModified = true;
}
///-------------------------------------------------------
/// 王国雄, 2006-4-5
/// 函数功能:
///		编码生成EIT Section
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///     编码后，Schedule EIT Section 存入m_aScheduleEITBuf
///     P/F EIT Section存入m_aPFEITBuf
///-------------------------------------------------------
void COneProgram_EITGenerator::Build()
{
	int nCountPEBuf = m_aPFEITBuf.GetSize();
	if(m_bEventPFModified || 0 == nCountPEBuf)	// 0 表示未曾生成过
		BuildPFEITSection();

	int nCountScheduleBuf = m_aScheduleEITBuf.GetSize();
	if( m_bEventScheduleModified || 0 == nCountScheduleBuf)
	{											// 0 表示未曾生成过
		if( SplitSegmentEdge() <= 0 )
			return;

		BuildScheduleTable();
	}
}
///-------------------------------------------------------
/// 王国雄, 2006-4-4
/// 函数功能:
///		编码形成EIT P/F section
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		P/F EIT TableID:0x4e
///     present Event: section 0x0
///     following Event : section 0x1 
///-------------------------------------------------------
void COneProgram_EITGenerator::BuildPFEITSection()
{
	if( false == m_bEventPFModified )
		return;

	m_aPFEITBuf.RemoveAll();
	if( NULL == m_pPresentEvent )
		return;

	m_bEventPFModified = false;
	
	m_aPFEITBuf.SetSize(2);
	m_BaseCfg.m_byEITPFVersion++;
	m_BaseCfg.m_byEITPFVersion &= 0x1F;

	/////////////    Present Event section   ///////////////

	EIT_BUFFER& CacheItemPresent = m_aPFEITBuf[0];
	CMyBitStream OutPBSP(CacheItemPresent.m_abyEITBuf, sizeof(CacheItemPresent.m_abyEITBuf));

	OutPBSP.PutBits8(DVBPSI_TBLID_EIT_ACTUAL);
	OutPBSP.PutBit(1);//syntax, always 1
	OutPBSP.PutBits(0, 3);  //reserved
	OutPBSP.PutBits(0, 12); //section length, keep space
	OutPBSP.PutBits16(m_BaseCfg.m_wSID); //SID
	OutPBSP.PutBits(0, 2); //reserved
	OutPBSP.PutBits(m_BaseCfg.m_byEITPFVersion, 5); //Version_number
	OutPBSP.PutBit(1); //current_next_indicator
	OutPBSP.PutBits8(0x00); //section_number
	OutPBSP.PutBits8(0x01); //last_section_number
	OutPBSP.PutBits16(m_BaseCfg.m_wTSID); //transport_stream_id
	OutPBSP.PutBits16(m_BaseCfg.m_wOriginalNetworkID); //original_network_id
	OutPBSP.PutBits8(0x01); //segment_last_section_number
	OutPBSP.PutBits8(DVBPSI_TBLID_EIT_ACTUAL); //last_table_id

	OutPBSP.FinishWrite();
	ASSERT( (OutPBSP.GetTotalWriteBits()/8) == EIT_TABLE_HEADER_LEN);
	ASSERT( 0 == (OutPBSP.GetTotalWriteBits()&7));
	WORD dwEventlen = BuildOneEvent(&OutPBSP, *m_pPresentEvent);
	
	OutPBSP.FinishWrite();
	ASSERT( 0 == (OutPBSP.GetTotalWriteBits()&7));
	CacheItemPresent.m_wBufSize = (WORD)OutPBSP.GetTotalWriteBits()/8;

	ASSERT( OutPBSP.GetTotalWriteBits()/8 == dwEventlen + EIT_TABLE_HEADER_LEN );
	int nPressentSectionlen = CacheItemPresent.m_wBufSize +4 -3;// CRC 4Bytes, discount 3 bytes at the beginning
	CacheItemPresent.m_abyEITBuf[2] = (BYTE)nPressentSectionlen;
	CacheItemPresent.m_abyEITBuf[1] |= (nPressentSectionlen>>8)&0xF ; // 12 bit for length
	
	DWORD dwCRC32 = DVB_GetCRC32( CacheItemPresent.m_abyEITBuf, CacheItemPresent.m_wBufSize);
	OutPBSP.PutBits32(dwCRC32);
	CacheItemPresent.m_wBufSize += 4;

	OutPBSP.FinishWrite();
	ASSERT( 0 == (OutPBSP.GetTotalWriteBits() & 7));
	ASSERT( CacheItemPresent.m_wBufSize == OutPBSP.GetTotalWriteBits()/8);

	//////////// Following Event Section  ////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	EIT_BUFFER& CacheItemFollowing = m_aPFEITBuf[1];
	CMyBitStream OutPBSF( CacheItemFollowing.m_abyEITBuf, sizeof(CacheItemFollowing.m_abyEITBuf));
	
	OutPBSF.PutBits8(DVBPSI_TBLID_EIT_ACTUAL);
	OutPBSF.PutBit(1);//syntax, always 1
	OutPBSF.PutBits(0, 3);  //reserved
	OutPBSF.PutBits(0, 12); //section length, keep space
	OutPBSF.PutBits16(m_BaseCfg.m_wSID); //SID
	OutPBSF.PutBits(0, 2); //reserved
	OutPBSF.PutBits(m_BaseCfg.m_byEITPFVersion, 5); //Version_number
	OutPBSF.PutBit(1); //current_next_indicator
	OutPBSF.PutBits8(0x01); //section_number
	OutPBSF.PutBits8(0x01); //last_section_number
	OutPBSF.PutBits16(m_BaseCfg.m_wTSID); //transport_stream_id
	OutPBSF.PutBits16(m_BaseCfg.m_wOriginalNetworkID); //original_network_id
	OutPBSF.PutBits8(0x01); //segment_last_section_number
	OutPBSF.PutBits8(DVBPSI_TBLID_EIT_ACTUAL); //last_table_id
	
	OutPBSF.FinishWrite();
	ASSERT( (OutPBSF.GetTotalWriteBits()/8) == EIT_TABLE_HEADER_LEN);
	ASSERT( 0 == (OutPBSF.GetTotalWriteBits()&7));
	if( m_pFollowingEvent)
		BuildOneEvent(&OutPBSF,*m_pFollowingEvent);

	OutPBSF.FinishWrite();
	CacheItemFollowing.m_wBufSize = (WORD)OutPBSF.GetTotalWriteBits()/8;
	
	int nFollowingSectionlen = CacheItemFollowing.m_wBufSize +4 -3;// CRC 4Bytes, discount 3 bytes at the beginning
	CacheItemFollowing.m_abyEITBuf[2] = (BYTE)nFollowingSectionlen;
	CacheItemFollowing.m_abyEITBuf[1] |= (nFollowingSectionlen>>8)&0xF ; // 12 bit for length
	
	dwCRC32 = DVB_GetCRC32( CacheItemFollowing.m_abyEITBuf, CacheItemFollowing.m_wBufSize);
	OutPBSF.PutBits32(dwCRC32);
	CacheItemFollowing.m_wBufSize += 4;

	OutPBSF.FinishWrite();
	ASSERT( 0 == (OutPBSF.GetTotalWriteBits() & 7));
	ASSERT( CacheItemFollowing.m_wBufSize == OutPBSF.GetTotalWriteBits()/8);
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		编码生成Schedule EIT Table
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		Schedule EIT TableID : 0x50 ~ 0x5F;
///     每个Table包含256个Section,每8个Section组成一个Segment.
///     每个Segment包含三个小时的事件 
///-------------------------------------------------------
void COneProgram_EITGenerator::BuildScheduleTable()
{
	int nCountSegment = m_aScheduleSegment.GetSize();
	int nCountSection = m_aScheduleSection.GetSize();
	int nCountEvent = m_aScheduleEvent.GetSize();
	if (nCountSegment <= 0 || nCountSection <= 0 || nCountEvent <= 0) 
		return;

	//清空
	m_bEventScheduleModified = false;
	m_aScheduleEITBuf.RemoveAll();
	m_aScheduleEITBuf.SetSize(nCountSection);

	m_BaseCfg.m_byEITScheduleVersion++;
	m_BaseCfg.m_byEITScheduleVersion &= 0xFF;

	BYTE byTableID ;
	BYTE bySection_Num = 0x00;
	BYTE byTable_Last_Section_Num = 0x00;
	BYTE bySegment_Last_Section_Num;

	//Last Table ID
	BYTE byLastTableID  = DVBPSI_TBLID_EIT_ACTUAL_SCHEDULE_START + nCountSegment/32;//32 segment in one table
	if( nCountSegment%32 == 0 )
		byLastTableID--;
	ASSERT(byLastTableID <= DVBPSI_TBLID_EIT_ACTUAL_SCHEDULE_END);

	
	for( int i = 0; i < nCountSegment; i++)
	{
		ONE_SEGMENT_ITEM& ItemSegment = m_aScheduleSegment[i];
		ASSERT(ItemSegment.m_wSectionCount <= 8);
		
		if( i%32 == 0 )
		{
			//新Table开始， 每一个Table包含32个segment
			byTableID = DVBPSI_TBLID_EIT_ACTUAL_SCHEDULE_START + i/32;
			ASSERT(byTableID <= byLastTableID);
			if( i+31 >nCountSegment-1)
			{
				//最后一个Table
				byTable_Last_Section_Num = (nCountSegment-1-i)*0x8 + m_aScheduleSegment[nCountSegment-1].m_wSectionCount -1;
			}
			else
			{
				//0xF8 : start section_num of last segment in one table
				byTable_Last_Section_Num = 0xF8+ m_aScheduleSegment[i+31].m_wSectionCount-1;
			}
			bySection_Num = 0x00;
		}
		else
			bySection_Num += 8 ;
		
		//当前Segment最后一个Section序号
		bySegment_Last_Section_Num = bySection_Num + ItemSegment.m_wSectionCount -1;		
		for ( int j = 0; j < ItemSegment.m_wSectionCount; j++)
		{
			ONE_SECTION_CFG OneSectionCfg;
			OneSectionCfg.m_byTableID = byTableID;
			OneSectionCfg.m_bySection_Num = bySection_Num +j;
			OneSectionCfg.m_byLastSection_Num = byTable_Last_Section_Num;
			OneSectionCfg.m_byLastTableID = byLastTableID;
			OneSectionCfg.m_bySegment_LastSection_Num = (BYTE)( bySection_Num+ ItemSegment.m_wSectionCount -1 );
			BuildScheduleOneSection(OneSectionCfg, ItemSegment.m_wFirstSectionIndex+j);
		}
	}

}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		编码输出一个Schedule section
/// 输入参数:
///		SecCfg		Section头部的基本配置
///     nNo			m_aScheduleSection中的索引号
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::BuildScheduleOneSection(ONE_SECTION_CFG& SecCfg, int nNo)
{
	ASSERT(nNo < m_aScheduleSection.GetSize());

	EIT_BUFFER& CacheItem = m_aScheduleEITBuf[nNo];
	
	CMyBitStream OutPBS(CacheItem.m_abyEITBuf, sizeof(CacheItem.m_abyEITBuf));
	
	//section header
	OutPBS.PutBits8(SecCfg.m_byTableID);
	OutPBS.PutBit(1);//syntax, always 1
	OutPBS.PutBits(0, 3);  //reserved
	OutPBS.PutBits(0, 12); //section length, keep space
	OutPBS.PutBits16(m_BaseCfg.m_wSID); //SID
	OutPBS.PutBits(0, 2); //reserved
	OutPBS.PutBits(m_BaseCfg.m_byEITScheduleVersion, 5); //Version_number
	OutPBS.PutBit(1); //current_next_indicator
	OutPBS.PutBits8(SecCfg.m_bySection_Num); //section_number
	OutPBS.PutBits8(SecCfg.m_byLastSection_Num); //last_section_number
	OutPBS.PutBits16(m_BaseCfg.m_wTSID); //transport_stream_id
	OutPBS.PutBits16(m_BaseCfg.m_wOriginalNetworkID); //original_network_id
	OutPBS.PutBits8(SecCfg.m_bySegment_LastSection_Num); //segment_last_section_number
	OutPBS.PutBits8(SecCfg.m_byLastTableID); //last_table_id

	OutPBS.FinishWrite();
	ASSERT( (OutPBS.GetTotalWriteBits()/8) == EIT_TABLE_HEADER_LEN);
	ASSERT( 0 == (OutPBS.GetTotalWriteBits()&7));

	// events
	WORD wIndexEventStart  =(WORD)( m_aScheduleSection[nNo]&0xFFFF );
	WORD wEventCount = (WORD)( m_aScheduleSection[nNo]>>16 )&0xFFFF;
	DWORD dwEventlen = 0;
	for ( int i = 0 ; i < wEventCount; i++)
	{
		OutPBS.FinishWrite();
		ASSERT( 0 == (OutPBS.GetTotalWriteBits()&7) );
		ASSERT( wIndexEventStart+i < m_aScheduleEvent.GetSize());
		dwEventlen += BuildOneEvent(&OutPBS,  m_aScheduleEvent[wIndexEventStart+i]);
	}
	
	OutPBS.FinishWrite();
	ASSERT( 0 == (OutPBS.GetTotalWriteBits()&7));
	CacheItem.m_wBufSize = (WORD)OutPBS.GetTotalWriteBits()/8;

	//section length
	ASSERT( (WORD)( OutPBS.GetTotalWriteBits()/8 ) == dwEventlen + EIT_TABLE_HEADER_LEN );
	int nSectionlen = CacheItem.m_wBufSize +4 -3;// CRC 4Bytes, discount 3 bytes at the beginning
	CacheItem.m_abyEITBuf[2] = (BYTE)nSectionlen;
	CacheItem.m_abyEITBuf[1] |= (nSectionlen>>8)&0xF ; // 12 bit for length
	
	//CRC
	DWORD dwCRC32 = DVB_GetCRC32( CacheItem.m_abyEITBuf, CacheItem.m_wBufSize);
	OutPBS.PutBits32(dwCRC32);
	CacheItem.m_wBufSize += 4;

	OutPBS.FinishWrite();
	ASSERT( 0 == (OutPBS.GetTotalWriteBits() & 7));
	ASSERT( CacheItem.m_wBufSize == OutPBS.GetTotalWriteBits()/8);

}

///-------------------------------------------------------
/// 王国雄, 2006-4-4
/// 函数功能:
///		编码事件
/// 输入参数:
///		pStream		码流
///     OneEvent	事件
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
WORD COneProgram_EITGenerator::BuildOneEvent(CMyBitStream* pStream, ONE_EVENT_ITEM& OneEvent)
{
	ASSERT(pStream);
	
	BYTE tempBuf[5];
	UTCTimetoMJD(OneEvent.m_tStartUTCTime, tempBuf);
	
	//event_id
	pStream->PutBits16(OneEvent.m_wEventID); 
	
	//start_time
	int i;
	for(  i = 0; i < sizeof(tempBuf); i++)
	{
		pStream->PutBits8(tempBuf[i]);
	}
	
	//Duration
	BYTE bysecond = (BYTE)(OneEvent.m_dwDuration%60);
	BYTE byhour = (BYTE)(OneEvent.m_dwDuration/3600);
	BYTE byminute = (BYTE)((OneEvent.m_dwDuration - byhour*3600)/60);
	byhour = byhour/10 *16 +byhour%10;
	byminute = byminute/10*16 +byminute%10;
	bysecond = bysecond/10*16 + bysecond%10;
	pStream->PutBits8(byhour);
	pStream->PutBits8(byminute);
	pStream->PutBits8(bysecond);
	
	//running_status
	pStream->PutBits(OneEvent.m_byRunningStatus, 3);
	//Free_CA_mode
	pStream->PutBit(OneEvent.m_bFreeCAMode);
	
	//descriptor_loop_length
	int nDetailDescriptionLen = OneEvent.m_strDetailDescription.GetLength();
	int nCountofExtendDescriptor = nDetailDescriptionLen/ONE_EXTEND_DSCPTOR_TEXT_MAXLEN;
	if (OneEvent.m_strDetailDescription.GetLength()%ONE_EXTEND_DSCPTOR_TEXT_MAXLEN != 0)
		nCountofExtendDescriptor++;

	WORD wdescroptor_loop_length = SHOTR_DECPTOR_HEADER_LEN+ OneEvent.m_strEventName.GetLength() +
		OneEvent.m_strShortDescription.GetLength()+ (EXTEND_DSCPTOR_HEADER_LEN*nCountofExtendDescriptor) + nDetailDescriptionLen;
	
	//event_name+ short_event_description <= 250 bytes
	ASSERT( OneEvent.m_strEventName.GetLength() + OneEvent.m_strShortDescription.GetLength() <= 250);
	//detail event description <= 2000bytes
	ASSERT( OneEvent.m_strDetailDescription.GetLength() <= 2000 );

	wdescroptor_loop_length &=0x0FFF;
	pStream->PutBits(wdescroptor_loop_length, 12);
	
	pStream->FinishWrite();
	ASSERT( 0 == (pStream->GetTotalWriteBits()&7));
	
	//ISO language code
	DWORD dwLanguageCode;
	dwLanguageCode = (OneEvent.m_aISOLanguage[0])<<16;
	dwLanguageCode |= (OneEvent.m_aISOLanguage[1])<<8;
	dwLanguageCode |= OneEvent.m_aISOLanguage[2];

	//short event descriptor 
	WORD wShortDes = BuildShortDescriptor(pStream, OneEvent.m_strEventName,
		OneEvent.m_strShortDescription, dwLanguageCode);

	//Extend Event Descriptor
	WORD wExtendDes = 0;
	
	if ( OneEvent.m_strDetailDescription.GetLength() != 0 ) 
	{
		wExtendDes = BuildExtendDescriptor(pStream, OneEvent.m_strDetailDescription, dwLanguageCode);
	}	
	
	ASSERT(wExtendDes+wShortDes == wdescroptor_loop_length);
	pStream->FinishWrite();
	int nlen = pStream->GetTotalWriteBits()/8;
	return wdescroptor_loop_length+EVENT_HEANDER_LEN;
}

///-------------------------------------------------------
/// 王国雄, 2006-4-5
/// 函数功能:
///		编码short event descriptor
/// 输入参数:
///		pStream		    码流
///     strEventName    事件名称
///     strDescriptor   事件简单描述
///     dwISOlanguage   低三字节有效
/// 输出参数:
///		无
/// 返回值:
///		short Event Descriptor在码流中所占的字节数
/// 函数说明:
///		无
///-------------------------------------------------------
WORD COneProgram_EITGenerator::BuildShortDescriptor(CMyBitStream *pStream, CMyString strEventName,CMyString strDescriptor, DWORD dwISOlanguage)
{
	int i;
	ASSERT(pStream);
	
	WORD wShortDescriptionLen = strDescriptor.GetLength();
	WORD wEventNameLen = strEventName.GetLength();
	
	// short event descriptor tag
	pStream->PutBits8(DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR);
	
	// descriptor length
	pStream->PutBits8( (BYTE)(wEventNameLen+wShortDescriptionLen+5) ); 

	BYTE atempISOlanguage[3];
	atempISOlanguage[2] = (BYTE)dwISOlanguage;
	atempISOlanguage[1] = (BYTE)(dwISOlanguage>>8);
	atempISOlanguage[0] = (BYTE)(dwISOlanguage>>16);
	//ISO_language_code
	for( i = 0; i < sizeof(atempISOlanguage); i++ )
	{		
		pStream->PutBits8(atempISOlanguage[i]);
	}

	// evnet_name_length
	pStream->PutBits8( (BYTE)(wEventNameLen) ); 
	for (i = 0; i < (wEventNameLen&0xFF); i++)
	{
		//event_name_char
		pStream->PutBits8( strEventName[i] );
	}

	//text_length
	pStream->PutBits8((BYTE)(wShortDescriptionLen));
	for( i = 0; i < (wShortDescriptionLen&0xFF); i++)
	{
		//text_char
		pStream->PutBits8( (BYTE)(strDescriptor[i]) ); 
	}

	pStream->FinishWrite();
	ASSERT( 0 == (pStream->GetTotalWriteBits() & 7) );

	return SHOTR_DECPTOR_HEADER_LEN+wEventNameLen+wShortDescriptionLen; 
}

///-------------------------------------------------------
/// 王国雄, 2006-4-5
/// 函数功能:
///		编码Extend Event Descriptor
/// 输入参数:
///     pStream			码流
///		strDescripotr   事件详细介绍
///		dwISOlanguage   低三字节有效
/// 输出参数:
///		无
/// 返回值:
///		Extend Event Descriptor在码流中所占的字节数
/// 函数说明:
///		
///-------------------------------------------------------
WORD COneProgram_EITGenerator::BuildExtendDescriptor(CMyBitStream *pStream, CMyString& strDescriptor, DWORD dwISOlanguage)
{
	ASSERT(pStream);

	WORD wTatolDescriptorLen =0;
	BYTE byDescriptorNum ;
	BYTE byLastDescriptorNum =(BYTE) (strDescriptor.GetLength()/ONE_EXTEND_DSCPTOR_TEXT_MAXLEN);
	byLastDescriptorNum &= 0x0F;//低四位有效

	if ( strDescriptor.GetLength()%ONE_EXTEND_DSCPTOR_TEXT_MAXLEN ==0)
		byLastDescriptorNum--;
	
	BYTE atempISOlanguage[3];
	atempISOlanguage[2] = (BYTE)dwISOlanguage;
	atempISOlanguage[1] = (BYTE)(dwISOlanguage>>8);
	atempISOlanguage[0] = (BYTE)(dwISOlanguage>>16);
	
	for ( byDescriptorNum = 0 ; byDescriptorNum <= byLastDescriptorNum; byDescriptorNum++)
	{
		BYTE byDescriptorLen;
		
		//Extend Event descriptor tag
		pStream->PutBits8(DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR);
		
		if( byDescriptorNum != byLastDescriptorNum)
			byDescriptorLen = ONE_EXTEND_DSCPTOR_TEXT_MAXLEN;
		else
		{
			byDescriptorLen = ((strDescriptor.GetLength())%ONE_EXTEND_DSCPTOR_TEXT_MAXLEN)&0xFF;
			if( byDescriptorLen == 0 )
				byDescriptorLen = ONE_EXTEND_DSCPTOR_TEXT_MAXLEN;
		}
		wTatolDescriptorLen += byDescriptorLen+8;

		//descriptor_length
 		pStream->PutBits8(byDescriptorLen+6);
		//descriptor_number
		pStream->PutBits(byDescriptorNum, 4);
		//last_descriptor_number
		pStream->PutBits(byLastDescriptorNum, 4);
		//ISO_language_code
		for ( int j = 0 ; j < sizeof(atempISOlanguage); j++)
		{			
			pStream->PutBits8(atempISOlanguage[j]);
		}
		//length of items
		pStream->PutBits8(0);
		//text length
		pStream->PutBits8(byDescriptorLen);
		
		for ( int k =0; k < byDescriptorLen; k++)
		{
			ASSERT(k+byDescriptorNum*ONE_EXTEND_DSCPTOR_TEXT_MAXLEN <= strDescriptor.GetLength());
			pStream->PutBits8(strDescriptor[k+byDescriptorNum*ONE_EXTEND_DSCPTOR_TEXT_MAXLEN]);//text char
		}		
	}	
	return wTatolDescriptorLen;
}
///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		确定某一个section边界
/// 输入参数:
///		segmentItem   
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::SplitScheduleSectionEdge(ONE_SEGMENT_ITEM& segmentItem)
{	
	int nEventCountofSeg= segmentItem.m_wEventCount;// count of event in one segment
	int nSectionCount = 0;// count of sections in one segment
	
	WORD wStartEventIndex = segmentItem.m_wFirstEventIndex;
	int nEventCountofSec = 0; // count of event in one section
	int nBytesUsed = EIT_TABLE_HEADER_LEN;//section header

	
	for (int i = 0; i< nEventCountofSeg; i++)
	{
		ONE_EVENT_ITEM& Item = m_aScheduleEvent[segmentItem.m_wFirstEventIndex +i];

		int nCurrentEventBytes = EVENT_HEANDER_LEN;//event header
		nCurrentEventBytes += SHOTR_DECPTOR_HEADER_LEN;// short descriptor header
		nCurrentEventBytes += Item.m_strEventName.GetLength();
		nCurrentEventBytes += Item.m_strShortDescription.GetLength();
		
		//extent descriptor header
		if (false == Item.m_strDetailDescription.IsEmpty()) 
		{
			int nDetailDescriptionLen = Item.m_strDetailDescription.GetLength();
			int nCountofExtendDescriptor = nDetailDescriptionLen/ONE_EXTEND_DSCPTOR_TEXT_MAXLEN;
			if (Item.m_strDetailDescription.GetLength()%ONE_EXTEND_DSCPTOR_TEXT_MAXLEN != 0)
				nCountofExtendDescriptor++;

			nCurrentEventBytes += nCurrentEventBytes*EXTEND_DSCPTOR_HEADER_LEN + nDetailDescriptionLen;
		}
		nBytesUsed += nCurrentEventBytes;

		// EIT section size < 4000, describe one event in only one section.
		if(nBytesUsed > 4000)
		{
			DWORD dwSectionItem = nEventCountofSec*0x10000 + wStartEventIndex;
			int nSectionIndex = m_aScheduleSection.Add(dwSectionItem);
			nSectionCount++;
			if( wStartEventIndex == segmentItem.m_wFirstEventIndex)
			{
				segmentItem.m_wFirstSectionIndex = nSectionIndex;
			}			
			nEventCountofSec = 0;
			wStartEventIndex= segmentItem.m_wFirstEventIndex+i-1;
			nBytesUsed = EIT_TABLE_HEADER_LEN + nCurrentEventBytes;
		}
		nEventCountofSec++;
	}
	
	if( nEventCountofSec)
	{
		DWORD dwSectionItem = nEventCountofSec*0x10000 + wStartEventIndex;
		int nSectionIndex = m_aScheduleSection.Add(dwSectionItem);
		nSectionCount++;
		if( wStartEventIndex == segmentItem.m_wFirstEventIndex)
		{
			segmentItem.m_wFirstSectionIndex = nSectionIndex;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// segment为空,也就是在segment三个小时内没有任何事件发生
	if( 0 == nEventCountofSeg)
	{
		DWORD dwSectionItem = 0*0x10000 + wStartEventIndex;
		int nSectionIndex = m_aScheduleSection.Add(dwSectionItem);
		segmentItem.m_wFirstSectionIndex = nSectionIndex;
		nSectionCount = 1;
	}
	segmentItem.m_wSectionCount =  nSectionCount ;	
	ASSERT( segmentItem.m_wSectionCount <= 8);// 8 sections in one segment at the most
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		根据事件的时间顺序确定segment的边界
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		每三个小时事件作为一个Segment
///     Table0x50的第1段：午夜（UTC）--"今天"02:59:59(UTC)
///-------------------------------------------------------
WORD COneProgram_EITGenerator::SplitSegmentEdge()
{
	int nCountEvent = m_aScheduleEvent.GetSize();
	
	int nStartNo = 0;
	int nEventNum = 0;
	
	time_t tSegmentStart = m_tScheduleStartTime;     //UTC 午夜;
	time_t tSegmentEnd = tSegmentStart + 3*60*60;//每个segment占用三小时的事件;
	ONE_SEGMENT_ITEM segmentItem;

	m_aScheduleSection.RemoveAll();
	m_aScheduleSegment.RemoveAll();
	
	for( int i = 0; i < nCountEvent ; i++)
	{
		ONE_EVENT_ITEM& Item = m_aScheduleEvent[i];
	
		if( (time_t)(Item.m_tStartUTCTime + Item.m_dwDuration )<= tSegmentStart)
		{
			//事件在此段开始之前, 已经结束
			nStartNo = i+1;
			nEventNum = 0;
			continue;
		}	
	
		if( Item.m_tStartUTCTime >= tSegmentStart &&
			(time_t) (Item.m_tStartUTCTime + Item.m_dwDuration ) <= tSegmentEnd)
		{
			//事件在此段开始,在此段结束
			nEventNum++;
			continue;
		}
		
		if( Item.m_tStartUTCTime < tSegmentEnd && Item.m_tStartUTCTime >= tSegmentStart
			&& (time_t)( Item.m_tStartUTCTime+ Item.m_dwDuration ) > tSegmentEnd)
		{
			//事件在此段开始,此段之后结束
			nEventNum++;
			segmentItem.m_wFirstEventIndex = nStartNo;
			segmentItem.m_wEventCount = nEventNum;
			SplitScheduleSectionEdge(segmentItem);
			m_aScheduleSegment.Add(segmentItem);
			nStartNo = i+1;
			nEventNum = 0;
			tSegmentStart = tSegmentEnd ;
			tSegmentEnd  = tSegmentStart + 3*60*60 ;
			continue;
		}
		
		if( Item.m_tStartUTCTime< tSegmentStart && (time_t)(Item.m_tStartUTCTime + Item.m_dwDuration) > tSegmentEnd)
		{
			//事件在此段之前开始，在此段之后结束
			segmentItem.m_wFirstEventIndex = nStartNo;
			segmentItem.m_wEventCount = nEventNum;
			SplitScheduleSectionEdge(segmentItem);
			m_aScheduleSegment.Add(segmentItem);
			nStartNo = i+1;
			nEventNum = 0;
			tSegmentStart = tSegmentEnd ;
			tSegmentEnd  = tSegmentStart + 3*60*60 ;
			continue;
		}
		if( Item.m_tStartUTCTime >= tSegmentEnd )
		{
			//事件在此段之后开始， 
			//下一段开始,不能确认该事件是否在下一段开始,需要对此事件进行再次判断(i--)
			segmentItem.m_wFirstEventIndex = nStartNo;
			segmentItem.m_wEventCount = nEventNum;
			SplitScheduleSectionEdge(segmentItem);
			m_aScheduleSegment.Add(segmentItem);
			nStartNo = i;
			nEventNum = 0;
			tSegmentStart = tSegmentEnd ;
			tSegmentEnd  = tSegmentStart + 3*60*60 ;
			i--;
			continue;
		}
	}
	
	if( nEventNum )
	{
		ONE_SEGMENT_ITEM segmentItem;
		segmentItem.m_wFirstEventIndex = nStartNo;
		segmentItem.m_wEventCount = nEventNum;
		SplitScheduleSectionEdge(segmentItem);
		m_aScheduleSegment.Add(segmentItem);
	}
	ASSERT( m_aScheduleSegment.GetSize() <= 32*16 ); // 32 segment in one table ,table: 0x50~0x5F
	return m_aScheduleSegment.GetSize();
}


///-------------------------------------------------------
/// 王国雄, 2006-4-4
/// 函数功能:
///		设置Present Event
/// 输入参数:
///		OneEvent	 Present事件
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::SetPresentEvent(ONE_EVENT_ITEM &OneEvent)
{
	m_bEventPFModified = true;

	m_pPresentEvent = m_aPFEventBuffer;
	
	m_pPresentEvent->m_wEventID = OneEvent.m_wEventID;
	m_pPresentEvent->m_tStartUTCTime = OneEvent.m_tStartUTCTime;
	m_pPresentEvent->m_dwDuration = OneEvent.m_dwDuration;
	m_pPresentEvent->m_byRunningStatus = OneEvent.m_byRunningStatus;
	m_pPresentEvent->m_bFreeCAMode = OneEvent.m_bFreeCAMode;
	m_pPresentEvent->m_strEventName = OneEvent.m_strEventName;
	m_pPresentEvent->m_strShortDescription = OneEvent.m_strShortDescription;
	m_pPresentEvent->m_strDetailDescription = OneEvent.m_strDetailDescription;
	memcpy(m_pPresentEvent->m_aISOLanguage, OneEvent.m_aISOLanguage, sizeof(OneEvent.m_aISOLanguage));

}

///-------------------------------------------------------
/// 王国雄, 2006-4-4
/// 函数功能:
///		设置Following Event
/// 输入参数:
///		OneEvent	 following 事件
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::SetFollowingEvent(ONE_EVENT_ITEM &OneEvent)
{
	m_bEventPFModified = true;
	m_pFollowingEvent = m_aPFEventBuffer + 1;
	m_pFollowingEvent->m_wEventID = OneEvent.m_wEventID;
	m_pFollowingEvent->m_tStartUTCTime = OneEvent.m_tStartUTCTime;
	m_pFollowingEvent->m_dwDuration = OneEvent.m_dwDuration;
	m_pFollowingEvent->m_byRunningStatus = OneEvent.m_byRunningStatus;
	m_pFollowingEvent->m_bFreeCAMode = OneEvent.m_bFreeCAMode;
	m_pFollowingEvent->m_strEventName = OneEvent.m_strEventName;
	m_pFollowingEvent->m_strShortDescription = OneEvent.m_strShortDescription;
	m_pFollowingEvent->m_strDetailDescription = OneEvent.m_strDetailDescription;
	memcpy(m_pFollowingEvent->m_aISOLanguage, OneEvent.m_aISOLanguage, sizeof(OneEvent.m_aISOLanguage));
}

///-------------------------------------------------------
/// 王国雄, 2006-4-4
/// 函数功能:
///		预制所有参数
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::Preset()
{
	m_aPFEITBuf.RemoveAll();
	m_aScheduleEITBuf.RemoveAll();
	m_aScheduleEvent.RemoveAll();
	m_aScheduleSection.RemoveAll();
	m_aScheduleSegment.RemoveAll();

	m_bEventScheduleModified = false;
	m_bEventPFModified = false;
	
	m_pPresentEvent = NULL;
	m_pFollowingEvent = NULL;

	memset( &m_BaseCfg, 0, sizeof(m_BaseCfg) );

	time_t timenow;
	time(&timenow);
	m_tScheduleStartTime = timenow - timenow%(24*3600);//当天UTC午夜时间
}

///-------------------------------------------------------
/// 王国雄, 2006-4-5
/// 函数功能:
///		将UTC时间转换成MJD+BCD(H:M:S)
/// 输入参数:
///		UTCtime		UTC时间
///     pBuf		存放转换后的时间
/// 输出参数:
///		pBuf		MJD+BCD时间
/// 返回值:
///		无
/// 函数说明:
///		pBuf的大小固定为5
///-------------------------------------------------------
void COneProgram_EITGenerator::UTCTimetoMJD(time_t UTCtime, PBYTE pBuf)
{
	struct tm* when;
	when = gmtime(&UTCtime);
	
	//MJD time
	int MJD;
	int year = when->tm_year + 1900;
	int month = when->tm_mon +1;
	int day = when->tm_mday;
	int hour = when->tm_hour;
	int minute = when->tm_min;
	int second = when->tm_sec;
	
	if (month <= 2) 
	{
		month += 12;
		year -= 1;
	}		
	
	MJD = (int) ( int(365.25f*year)+int(30.6001f*(month+1)) + day + hour/(24.0f) +1720981.5f - 2400000.5f );
	MJD &= 0xFFFF;

	BYTE byBuf[5];
	byBuf[1] = (BYTE)MJD;
	byBuf[0] = (BYTE)(MJD>>8);
	byBuf[2] = hour/10 *16 +hour%10;
	byBuf[3] = minute/10 *16 + minute%10;
	byBuf[4] = second/10 *16 + second%10;
	memcpy(pBuf, byBuf, sizeof(byBuf));
}


///-------------------------------------------------------
/// 王国雄, 2006-4-5
/// 函数功能:
///		设置逻辑频道基本信息
/// 输入参数:
///		BaseCf   逻辑频道EIT基本设置
///     dwFlags  
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::SetBaseCfg(ONE_PROGRAMEIT_BASE &Basecf, DWORD dwFlags)
{
	if( 0 == dwFlags)
		return;
	
	//SID
	if ( dwFlags & ENABLE_SID)
		m_BaseCfg.m_wSID = Basecf.m_wSID;
	
	//TSID
	if ( dwFlags & ENABLE_TSID)
		m_BaseCfg.m_wTSID = Basecf.m_wTSID;

	//Original_Network_ID
	if ( dwFlags & ENABLE_ORIGINAL_NETWORK_ID)
		m_BaseCfg.m_wOriginalNetworkID = Basecf.m_wOriginalNetworkID;

	//Current_Next_Indicator
	if ( dwFlags & ENABLE_CURRENT_NEXT_INDICATOR) 
		m_BaseCfg.m_bCurrentNextIndicator = Basecf.m_bCurrentNextIndicator;

	//Schedule_version
	if ( dwFlags & ENABLE_SCHEDULE_VERSION)
		m_BaseCfg.m_byEITScheduleVersion = Basecf.m_byEITScheduleVersion;

	//PF_Version
	if (dwFlags & ENABLE_PF_VERSION)
		m_BaseCfg.m_byEITPFVersion = Basecf.m_byEITPFVersion;

	m_bEventPFModified = true;
	m_bEventScheduleModified = true;		
}

///-------------------------------------------------------
/// 王国雄, 2006-4-18
/// 函数功能:
///		设置ScheduleEIT起始时间
/// 输入参数:
///		tUTCmidnight  一般为UTC午夜时间
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::SetCurrentDate(time_t tUTCmidnight)
{
	m_tScheduleStartTime = tUTCmidnight;
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		添加schedule事件
/// 输入参数:
///		OneEvnet	待添加的事件
/// 输出参数:
///		无
/// 返回值:
///		int		 索引
/// 函数说明:
///		无
///-------------------------------------------------------
int COneProgram_EITGenerator::AddScheduleEvent(ONE_EVENT_ITEM &OneEvent)
{
	m_bEventScheduleModified = true;
	
	int nNo = FindOneEvent(OneEvent.m_wEventID);
	
	if ( nNo < 0 )
	{
		return m_aScheduleEvent.Add(OneEvent);
	}
	else
	{
		m_aScheduleEvent[nNo] = OneEvent;
		return nNo;
	}
}
///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		查找某一事件
/// 输入参数:
///		wEventID	事件ID
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
int COneProgram_EITGenerator::FindOneEvent(WORD wEventID)
{
	int nCount = m_aScheduleEvent.GetSize();
	for( int i = 0 ; i < nCount; i++)
	{
		if (m_aScheduleEvent[i].m_wEventID == wEventID)
			return i ;
	}
	return -1;
}
///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		删除事件
/// 输入参数:
///		事件ID
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::RemoveEvent(WORD wEventID)
{
	int nNo = FindOneEvent(wEventID);
	if ( nNo >= 0 )
	{
		m_aScheduleEvent.RemoveAt(nNo);
		m_bEventScheduleModified = true;
	}
}

///-------------------------------------------------------
/// 王国雄, 2006-4-19
/// 函数功能:
///		删除P/F事件
/// 输入参数:
///		无
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void COneProgram_EITGenerator::RemovePFEvent()
{
	m_pPresentEvent = NULL;
	m_pFollowingEvent = NULL;

	m_aPFEITBuf.RemoveAll();
	m_bEventPFModified = true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  CEITGenerator

CDVBPSI_EITGenerator::CDVBPSI_EITGenerator()
{
	SetPID(0x12);
}

CDVBPSI_EITGenerator::~CDVBPSI_EITGenerator()
{
	m_MapSID_ProgarmEIT.RemoveAll();
}

///-------------------------------------------------------
/// 王国雄, 2006-4-7
/// 函数功能:
///		注册某一逻辑频道EIT生成器
/// 输入参数:
///		SID				逻辑频道
///		pProgramEIT		逻辑频道EIT生成器
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void CDVBPSI_EITGenerator::RegisterOneProgramEIT(WORD wSID, COneProgram_EITGenerator *pProgramEIT)
{
	ASSERT(pProgramEIT);
	
#if 0	// CYJ, 这小段程序，是冗余的
	COneProgram_EITGenerator* pExistProgram;
	if( m_MapSID_ProgarmEIT.Lookup(wSID, pExistProgram))
	{
		m_MapSID_ProgarmEIT.RemoveKey(wSID);
	}
#endif //0

	m_MapSID_ProgarmEIT.SetAt(wSID, pProgramEIT);
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		注销某一逻辑频道EIT
/// 输入参数:
///		SID		逻辑频道
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		无
///-------------------------------------------------------
void CDVBPSI_EITGenerator::DeregisterProgramEIT(WORD wSID)
{
	m_MapSID_ProgarmEIT.RemoveKey(wSID);
}

///-------------------------------------------------------
/// CYJ,2006-12-30
/// 函数功能:
///		 删除所有逻辑频道
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_EITGenerator::RemoveAll()
{
	m_MapSID_ProgarmEIT.RemoveAll();
}

///-------------------------------------------------------
/// 王国雄, 2006-4-6
/// 函数功能:
///		将所有SID的EIT表整合到TS包
/// 输入参数:
///		bPFSectionOnly			只输出PF
/// 输出参数:
///		无
/// 返回值:
///		无
/// 函数说明:
///		调用该函数之前,必须对每一逻辑频道EIT生成器进行编码(Build)
///-------------------------------------------------------
void CDVBPSI_EITGenerator::Build( DWORD dwBuildMask )
{
	int nCountProgramEIT = m_MapSID_ProgarmEIT.GetCount();
	COneProgram_EITGenerator* pProgramEIT = NULL;
		
	int nMaxEITBufLen = 0;
	WORD wKeySID;
		
	//获取所有逻辑频道中含有EIT schedule section个数的最大值
	POSITION pos = m_MapSID_ProgarmEIT.GetStartPosition();	
	while ( pos)
	{
		pProgramEIT = NULL;
		m_MapSID_ProgarmEIT.GetNextAssoc(pos, wKeySID, pProgramEIT);
		if ( NULL == pProgramEIT)
			continue;
		// 统计最大的节数
		if ( pProgramEIT->m_aScheduleEITBuf.GetSize() >= nMaxEITBufLen)
			nMaxEITBufLen = pProgramEIT->m_aScheduleEITBuf.GetSize();
		
		if( dwBuildMask & BUILD_FLAG_PF_SECTION )
		{	//生成P/F EIT
			for( int k = 0; k < pProgramEIT->m_aPFEITBuf.GetSize(); k++)
			{
				COneProgram_EITGenerator::EIT_BUFFER& BufEit = pProgramEIT->m_aPFEITBuf[k];			
				EncapsulateOneSection(BufEit.m_abyEITBuf, BufEit.m_wBufSize );
			}
		}
	}
	
	if( dwBuildMask & BUILD_FLAG_SCHEDULE )
	{
		//逻辑频道间隔生成Schedule EIT
		for( int i = 0;  i < nMaxEITBufLen ; i++)
		{		
			pos = m_MapSID_ProgarmEIT.GetStartPosition();
			while( pos )
			{
				pProgramEIT = NULL;
				m_MapSID_ProgarmEIT.GetNextAssoc(pos, wKeySID, pProgramEIT);
				if ( pProgramEIT == NULL)
					continue;
				if( i < pProgramEIT->m_aScheduleEITBuf.GetSize())
				{
					COneProgram_EITGenerator::EIT_BUFFER& BufEittemp =  pProgramEIT->m_aScheduleEITBuf[i];
					EncapsulateOneSection(BufEittemp.m_abyEITBuf, BufEittemp.m_wBufSize);
				}
			}
		}	
	}
}

///-------------------------------------------------------
/// CYJ,2007-1-4
/// 函数功能:
///		封装一个数据项
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_EITGenerator::EncapsulateOneSection(PBYTE pBuf, int nLen )
{
	ASSERT( pBuf && nLen );
	if( pBuf && nLen > 0 )
		Encapsulate( pBuf, nLen );
}





