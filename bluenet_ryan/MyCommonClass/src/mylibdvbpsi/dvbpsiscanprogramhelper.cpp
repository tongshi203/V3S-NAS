///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-3-5
///
///		用途：
///			扫描节目
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"
#ifndef _WIN32
	#include <stdlib.h>
	#include <stdio.h>
#endif //_WIN32
#include <time.h>
#include "dvbpsiscanprogramhelper.h"
#include <MyMap.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBPSIScanProgramHelper::CDVBPSIScanProgramHelper(int nTimeOutSecond)
{
	m_dwPAT_TSID = 0xFFFFFFFF;
	m_nStartTime = -1;
	m_nTimeOutSecond = nTimeOutSecond;
}

CDVBPSIScanProgramHelper::~CDVBPSIScanProgramHelper()
{
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		是否有效
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSIScanProgramHelper::IsValid()
{
	if( false == CDVBPSITable_PAT::IsValid() )
		return false;
	if( false == CDVBPSITable_SDT_BAT::IsValid() )
		return false;
	return true;
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		接收到PAT表
/// 输入参数:
///		pTable			PAT表
/// 返回参数:
///		无
void CDVBPSIScanProgramHelper::OnPATReceived( const PDVB_PSI_TABLE_PAT pTable )
{
	int i;
	int nCount = pTable->m_wCount;
	WORD wNetworkID = 0;

	OnDisablePID( 0 );		// 2010.11.24 ClosePID PID 0 ( PAT )

	for(i=0; i<nCount; i++ )
	{
		WORD wSID = pTable->m_aPrograms[i].m_wSID;
		if( wSID )
			continue;			// Network ID
		wNetworkID = pTable->m_aPrograms[i].m_wPMT_PID;
		break;
	}

	for(i=0; i<nCount; i++ )
	{
		WORD wSID = pTable->m_aPrograms[i].m_wSID;
		if( 0 == wSID )
			continue;			// Network ID
		PONE_DVB_PROGRAM_PARAM pProgram = FindProgram( wSID, true );
		if( NULL == pProgram )
			continue;			// 没有空间
		pProgram->m_wPMT_PID = pTable->m_aPrograms[i].m_wPMT_PID;
		pProgram->m_wTransportStreamID = pTable->m_wTSID;
		pProgram->m_wNetworkID = wNetworkID;

		if( m_pTSDemux )
		{
			OnEnablePID( pProgram->m_wPMT_PID );
			CDVBPSI_MultiPMT_Receiver::AddSID( wSID, pProgram->m_wPMT_PID );
			m_pTSDemux->RegisterResponser( pProgram->m_wPMT_PID, static_cast<CDVBPSI_MultiPMT_Receiver*>(this) );
		}
	}
	m_dwPAT_TSID = pTable->m_wTSID;
	if( m_nStartTime < 0 )
		m_nStartTime = time(NULL);

	// CYJ, 2010.2.24 Modify, only PAT received, then register SDT
	m_pTSDemux->RegisterResponser( 0x11, static_cast<CDVBPSITable_SDT_BAT*>(this) );
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		接收到 SDT 表
/// 输入参数:
///		pSDT			SDT 表
/// 返回参数:
///		无
void CDVBPSIScanProgramHelper::OnSDTReceived( PDVB_PSI_TABLE_SDT pSDT )
{
	if( m_aProgram.GetSize() == 0 )
		return;				//	还没有接收PAT

	if( pSDT->m_byTableID != DVBPSI_TBLID_SDT_ACTUAL )
		return;

	// CYJ, 2010.2.24 Modify
	if(m_SDT_SectionRecStatus.GetSectionCount() != (pSDT->m_byLastSectionNumber + 1 ))
		m_SDT_SectionRecStatus.SetSectionCount( pSDT->m_byLastSectionNumber + 1 );
	m_SDT_SectionRecStatus.SetSectionNoStatus( pSDT->m_bySectionNumber );

	int nCount = pSDT->m_wCount;
	for(int i=0; i<nCount; i++ )
	{
		WORD wSID = pSDT->m_aPrograms[i].m_wSID;
		if( 0 == wSID )
			continue;			// Network ID
		PONE_DVB_PROGRAM_PARAM pProgram = FindProgram( wSID );
		if( NULL == pProgram )
			continue;			// not fount
		if( 0 == pSDT->m_aPrograms[i].m_byCount || NULL == pSDT->m_aPrograms[i].m_pDescriptor )
			continue;

		pProgram->m_byEIT_Schedule = pSDT->m_aPrograms[i].m_bEITScheduleFlag;
		pProgram->m_byEIT_Present_Following = pSDT->m_aPrograms[i].m_bEITPresentFollowingFlag;
		pProgram->m_byRuningStatus = pSDT->m_aPrograms[i].m_byRunningStatus;
		pProgram->m_byFreeCAMode = pSDT->m_aPrograms[i].m_bFreeCAMode;

		DVBPSI_SERVICE_DESCRIPTOR * pService =
			(DVBPSI_SERVICE_DESCRIPTOR*)pSDT->m_aPrograms[i].m_pDescriptor->Find(DVBPSI_DTID_SERVICE_DESCRIPTOR);

		if( NULL == pService )
			continue;

		pProgram->m_byServiceType = pService->m_byServiceType;		// 由SDT提供的服务类型
		pProgram->m_strProviderName = pService->m_pszProviderName;	// 提供商名称
		pProgram->m_strServiceName = pService->m_pszServiceName;	// 服务名称（台名）
	}
	if( IsSDT_PMTFullReceived() || time(NULL) - m_nStartTime > m_nTimeOutSecond	)
	{
		OnProgramListReceived( m_aProgram.GetData(), m_aProgram.GetSize() );
	}
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		查找一个节目
/// 输入参数:
///		wSID			待查找的节目
///		bInsertNewOne	添加一个新的节目.,缺省为 false
/// 返回参数:
///		!= NULL			成功
///		NULL			失败，没有空间
PONE_DVB_PROGRAM_PARAM CDVBPSIScanProgramHelper::FindProgram( WORD wSID, bool bInsertNewOne )
{
	PONE_DVB_PROGRAM_PARAM pRetVal = NULL;

	int nCount = m_aProgram.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		if( m_aProgram[i].m_wSID == wSID )
			return &m_aProgram[i];
	}

	if( FALSE == bInsertNewOne )
		return NULL;

	//	没有发现，再添加一个新的
	ONE_DVB_PROGRAM_PARAM Item;
	Item.m_wSID = wSID;				// 节目号
	Item.m_wPMT_PID = INVALID_PID;			// PMT 对应的 PID

	Item.m_wTransportStreamID = 0;
	Item.m_wNetworkID = 0;

	// PMT
	Item.m_wPCR_PID = INVALID_PID;
	Item.m_wESCount = 0;			// ES 个数
	memset( Item.m_aES_PID, 0, sizeof(Item.m_aES_PID) );
	Item.m_byCAIDCount = 0;			// CA 描述符号
	memset( Item.m_awCAIDs, 0, sizeof(Item.m_awCAIDs) );

	// SDT
	Item.m_byServiceType = 0;	// 由SDT提供的服务类型
	Item.m_byEIT_Schedule = false;
	Item.m_byEIT_Present_Following = false;
	Item.m_byRuningStatus = 0;
	Item.m_byFreeCAMode = false;

	int nNo = m_aProgram.Add( Item );
	pRetVal = &m_aProgram[nNo];

	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		注册回调函数
/// 输入参数:
///		TSDemux				解复用器
///		bDeRegister			注销
/// 返回参数:
///		无
void CDVBPSIScanProgramHelper::Initialize( CTSPacketDemux * pTSDemux )
{
	ASSERT( pTSDemux );
	if( NULL == pTSDemux )
		return;

	m_aProgram.RemoveAll();
	CDVBPSI_MultiPMT_Receiver::Reset();

	m_pTSDemux = pTSDemux;

	m_pTSDemux->RegisterResponser( 0, static_cast<CDVBPSITable_PAT*>(this) );

	OnEnablePID( 0 );
	OnEnablePID( 0x11 );
}

///-------------------------------------------------------
/// CYJ,2005-3-5
/// 函数功能:
///		复位
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSIScanProgramHelper::Reset()
{
	m_aProgram.RemoveAll();

	CDVBPSITable_PAT::Reset( true );
	CDVBPSITable_SDT_BAT::Reset( true );
	CDVBPSI_MultiPMT_Receiver::Reset();

	m_dwPAT_TSID = 0xFFFFFFFF;
	m_nStartTime = -1;
}

///-------------------------------------------------------
/// CYJ,2005-3-7
/// 函数功能:
///		判断所有SDT接收到
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSIScanProgramHelper::IsSDT_PMTFullReceived()
{
	int nCount = m_aProgram.GetSize();
	for(int i=0; i<nCount; i++)
	{
		if( m_aProgram[i].m_strServiceName.IsEmpty() )	// SDT Not recieved
			return false;
		if( 0 == m_aProgram[i].m_wESCount )				// PMT Not Received
			return false;
	}
	return true;
}

///--------------------------------------------------------
/// CYJ, 2010-11-25 下午04:56:44
/// Function:
///    是否与此PMT相关的 Program 都接收到了。
///		如果都接收到了，则可以DisablePID
/// Input:
///     None
/// Output:
///     None
bool CDVBPSIScanProgramHelper::IsThisPMTPIDCanBeDisabled( WORD wPMT )
{
	int nCount = m_aProgram.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		ONE_DVB_PROGRAM_PARAM & Item = m_aProgram[i];
		if( Item.m_wPMT_PID != wPMT )
			continue;
		if( 0 == Item.m_wESCount )
			return false;	//	还有节目没有接收到
	}

	return true;			// 可以删除
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		接收到 PMT
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSIScanProgramHelper::OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable )
{
	if( m_aProgram.GetSize() == 0 )
	{
		ASSERT( FALSE );
		return;				//	还没有接收PAT
	}

	WORD wSID = pTable->m_wSID;

	PONE_DVB_PROGRAM_PARAM pProgram = FindProgram( wSID );
	if( NULL == pProgram )
		return;

	pProgram->m_wPMTVersion = pTable->m_byVersionNumber;		// 2010.8.26 CYJ Add
	pProgram->m_wPCR_PID = pTable->m_wPCR_PID;
	pProgram->m_wESCount = pTable->m_wStreamCount;			// ES 个数
	if( pProgram->m_wESCount > sizeof(pProgram->m_aES_PID)/sizeof(pProgram->m_aES_PID[0]) )
		pProgram->m_wESCount = sizeof(pProgram->m_aES_PID)/sizeof(pProgram->m_aES_PID[0]);

	if( IsThisPMTPIDCanBeDisabled(pProgram->m_wPMT_PID) )
		OnDisablePID( pProgram->m_wPMT_PID );		// 2010.11.24 CYJ Add

	int i;
	for( i=0; i<pProgram->m_wESCount; i++ )
	{
		pProgram->m_aES_PID[i].m_wES_PID = pTable->m_aStreams[i].m_wES_PID;
		pProgram->m_aES_PID[i].m_byESType = pTable->m_aStreams[i].m_byStreamType;
		pProgram->m_aES_PID[i].m_byLanguageDescriptorCount = 0;
		// CYJ 2010.3.5 Add
		if( 0 == pTable->m_aStreams[i].m_byCount || NULL == pTable->m_aStreams[i].m_pDescriptor )
			continue;
		if( 6 == pProgram->m_aES_PID[i].m_byESType )	// try to find Teletext, Subtitle
			TryToFind_ES6_Language( pProgram, i, pTable );
		else	// try to find audio language
			TryToFind_Audio_Language( pProgram, i, pTable );
	}

	// 判断是否加密
	// 在 GetCAIDs 函数中，会修改 pProgram->m_byCAIDCount
	pProgram->m_byCAIDCount = 0;
	if( pTable->m_pDescriptor )
		GetCAIDs( pProgram, pTable->m_pDescriptor );
	for(i=0; i<pTable->m_wStreamCount; i++ )
	{
		if( pTable->m_aStreams[i].m_pDescriptor )
			GetCAIDs( pProgram, pTable->m_aStreams[i].m_pDescriptor );
	}

	if( IsSDT_PMTFullReceived() || time(NULL) - m_nStartTime > m_nTimeOutSecond	)
	{
		OnProgramListReceived( m_aProgram.GetData(), m_aProgram.GetSize() );
	}
}

//------------------------------------------------------------------------
/* 2011.2.12
 * Get CA IDs from the descriptors
 *
 * @param[in]	pProgram 		Program item
 * @param[in]	pDescriptor		DVB Descriptor
 */
void CDVBPSIScanProgramHelper::GetCAIDs( PONE_DVB_PROGRAM_PARAM pProgram, PDVBPSI_DECODED_DESCRIPTOR_BASE pDescriptor )
{
	while( pDescriptor )
	{
		if( DVBPSI_DTID_CA == pDescriptor->m_byDescriptorTag )
		{
			dvbpsi_ca_dr_t * pCADr = (dvbpsi_ca_dr_t*)pDescriptor;

			if( pProgram->m_byCAIDCount >= ONE_DVB_PROGRAM_MAX_CAID_NUMBER )
                return;		// too many CA IDs

            // 2011.2.17 try to find if the CAIDs is exist or not.
            bool bAddNew = true;
            for(int i=0; i<pProgram->m_byCAIDCount; i++ )
            {
                if( pProgram->m_awCAIDs[ i ] == pCADr->i_ca_system_id )
                {
                    bAddNew = false;
                    break;
                }
            }

            if( bAddNew )
            {
                pProgram->m_awCAIDs[ pProgram->m_byCAIDCount ] = pCADr->i_ca_system_id;
                pProgram->m_byCAIDCount ++;
            }
		}

		pDescriptor = pDescriptor->m_pNext;
	}
}


//------------------------------------------------------------------------
// Try to find ES [Type=6] language
void CDVBPSIScanProgramHelper::TryToFind_ES6_Language( PONE_DVB_PROGRAM_PARAM pProgram, int nIndex, const PDVB_PSI_TABLE_PMT pTable )
{
	struct tagDVBPSI_DECODED_DESCRIPTOR_BASE * pDr = pTable->m_aStreams[nIndex].m_pDescriptor->Find( DVBPSI_DTID_TELETEXT );
	if( pDr )
	{	// Teletext
		DVBPSI_TELETEXT_DESCRIPTOR * pTeletextDr = (DVBPSI_TELETEXT_DESCRIPTOR*)pDr;
		for(int i=0; i<pTeletextDr->m_byCount; i++ )
		{
			ONE_DVB_PROGRAM_ES_PARAM & Item = pProgram->m_aES_PID[nIndex].m_aESDescriptors[ pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ];
			memset( &Item, 0, sizeof(Item) );
			Item.m_byTagID = DVBPSI_DTID_TELETEXT;

			memcpy( Item.m_szISOLanguageCode, pTeletextDr->m_aPages[i].ISO_639_LangCode, 3 );
			Item.m_Teletext.m_byTeleTextType = pTeletextDr->m_aPages[i].m_byTeletextType;
			Item.m_Teletext.m_byMagNo = pTeletextDr->m_aPages[i].m_byMagNum;
			Item.m_Teletext.m_byPageNoBCD = pTeletextDr->m_aPages[i].m_byPageNumBCD;

			pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ++;
			if( pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount >= ONE_DVB_PROGRAM_ES_MAX_LANGUAGE_COUNT )
				break;
		}
		return;
	}

	// try to find Subtitle
	pDr = pTable->m_aStreams[nIndex].m_pDescriptor->Find( DVBPSI_DTID_SUBTITLE );
	if( pDr )
	{	// subtitle
		DVBPSI_SUBTITLE_DESCRIPTOR * pSubtitleDr = (DVBPSI_SUBTITLE_DESCRIPTOR*)pDr;
		for(int i=0; i<pSubtitleDr->m_byCount; i++ )
		{
			ONE_DVB_PROGRAM_ES_PARAM & Item = pProgram->m_aES_PID[nIndex].m_aESDescriptors[ pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ];
			memset( &Item, 0, sizeof(Item) );
			Item.m_byTagID = DVBPSI_DTID_SUBTITLE;

			memcpy( Item.m_szISOLanguageCode, pSubtitleDr->m_aItems[i].ISO_639_LangCode, 3 );
			Item.m_SubTitle.m_bySubtitleType = pSubtitleDr->m_aItems[i].m_byType;
			Item.m_SubTitle.m_wCompositionPageID = pSubtitleDr->m_aItems[i].m_wCompositionPageID;
			Item.m_SubTitle.m_wAncillaryPageID = pSubtitleDr->m_aItems[i].m_wAncillaryPageID;

			pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ++;
			if( pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount >= ONE_DVB_PROGRAM_ES_MAX_LANGUAGE_COUNT )
				break;
		}
		return;
	}

	// try to find AC-3
	pDr = pTable->m_aStreams[nIndex].m_pDescriptor->Find( DVBPSI_DTID_AC3 );
	if( pDr )
	{	// AC-3
		DVBPSI_AC3_DESCRIPTOR * pAC3Dr = (DVBPSI_AC3_DESCRIPTOR *)pDr;

		ONE_DVB_PROGRAM_ES_PARAM & Item = pProgram->m_aES_PID[nIndex].m_aESDescriptors[ pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ];
		memset( &Item, 0, sizeof(Item) );
		Item.m_byTagID = DVBPSI_DTID_AC3;
		Item.m_AC3.m_byFlags = pAC3Dr->m_byFlags;
		Item.m_AC3.m_byAC3Type = pAC3Dr->m_byAC3Type;
		Item.m_AC3.m_byBSID = pAC3Dr->m_byBSID;
		Item.m_AC3.m_byMAINID = pAC3Dr->m_byMAINID;
		Item.m_AC3.m_byASVC = pAC3Dr->m_byASVC;

		pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ++;
	}

	// 2010.8.26 CYJ Add
	// try to find EAC-3
	pDr = pTable->m_aStreams[nIndex].m_pDescriptor->Find( DVBPSI_DTID_EAC3 );
	if( pDr )
	{	// EAC-3
		ONE_DVB_PROGRAM_ES_PARAM & Item = pProgram->m_aES_PID[nIndex].m_aESDescriptors[ pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ];
		memset( &Item, 0, sizeof(Item) );
		Item.m_byTagID = DVBPSI_DTID_EAC3;
		// FIXME, more flags ?
		pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ++;
	}
}

//---------------------------------------------------------------
// Try to find Audio language
void CDVBPSIScanProgramHelper::TryToFind_Audio_Language( PONE_DVB_PROGRAM_PARAM pProgram, int nIndex, const PDVB_PSI_TABLE_PMT pTable )
{
	struct tagDVBPSI_DECODED_DESCRIPTOR_BASE * pDr = pTable->m_aStreams[nIndex].m_pDescriptor->Find( DVBPSI_DTID_ISO_LANGUAGE );
	if( NULL == pDr )
		return;

	dvbpsi_iso639_dr_t * pAudioDr = (dvbpsi_iso639_dr_t *)pDr;
	for(int i=0; i<pAudioDr->i_code_count; i++)
	{
		ONE_DVB_PROGRAM_ES_PARAM & Item = pProgram->m_aES_PID[nIndex].m_aESDescriptors[ pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ];
		memset( &Item, 0, sizeof(Item) );
		Item.m_byTagID = DVBPSI_DTID_ISO_LANGUAGE;

		memcpy( Item.m_szISOLanguageCode, pAudioDr->i_iso_639_code + i*3, 3 );
		Item.m_byAudioType = pAudioDr->i_audio_type;

		pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount ++;
		if( pProgram->m_aES_PID[nIndex].m_byLanguageDescriptorCount >= ONE_DVB_PROGRAM_ES_MAX_LANGUAGE_COUNT )
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
CDVBPSI_MultiPMT_Receiver::CDVBPSI_MultiPMT_Receiver()
{
}

CDVBPSI_MultiPMT_Receiver::~CDVBPSI_MultiPMT_Receiver()
{
	Reset();
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		复位
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_MultiPMT_Receiver::Reset()
{
	int nCout = m_apPMTs.GetSize();
	for( int i=0; i<nCout; i++ )
	{
		delete m_apPMTs[i];
	}
	m_apPMTs.RemoveAll();
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		删除
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_MultiPMT_Receiver::Remove( WORD wSID )
{
	int nCount = m_apPMTs.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		if( m_apPMTs[i]->GetSID() != wSID )
			continue;

		delete m_apPMTs[i];
		m_apPMTs.RemoveAt( i );
		break;
	}
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		添加一个 PMT
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBPSI_MultiPMT_Receiver::AddSID( WORD wSID, WORD wPMTPID )
{
	int nCount = m_apPMTs.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		if( m_apPMTs[i]->GetSID() == wSID )
		{
			m_apPMTs[i]->Reset();

			m_apPMTs[i]->SetSID( wSID );
			m_apPMTs[i]->m_wPMT_PID = wPMTPID;

			return true;
		}
	}

	// 没有发现
	CMyPMT * pPmt = new CMyPMT( this );
	if( NULL == pPmt )
		return false;

	pPmt->SetSID( wSID );
	pPmt->m_wPMT_PID = wPMTPID;

	m_apPMTs.Add( pPmt );
	return true;
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		接收到一个PMT TS
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_MultiPMT_Receiver::PushOneTSPacket( PDVB_TS_PACKET pPacket )
{
	WORD wPID = pPacket->GetPID();
	int nCount = m_apPMTs.GetSize();
	for(int i=0; i<nCount; i++ )
	{
		if( m_apPMTs[i]->m_wPMT_PID != wPID )
			continue;
		m_apPMTs[i]->PushOneTSPacket( pPacket );
	}
}


//////////////////////////////////////////////////////////////////////////
CDVBPSI_MultiPMT_Receiver::CMyPMT::CMyPMT( CDVBPSI_MultiPMT_Receiver * pResponser )
{
	m_pResponser = pResponser;
}

CDVBPSI_MultiPMT_Receiver::CMyPMT::~CMyPMT()
{
}

///-------------------------------------------------------
/// CYJ,2008-10-23
/// 函数功能:
///		接收到 PMT
/// 输入参数:
///		无
/// 返回参数:
///		无
void CDVBPSI_MultiPMT_Receiver::CMyPMT::OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable )
{
	if( m_pResponser )
		m_pResponser->OnPMTReceived( pTable );
}
