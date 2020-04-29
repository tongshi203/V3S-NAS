///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-2-17
///
///		用途：
///			通视MPEG4播出辅助类，用于判断是否通视播出
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"
#include "tsmpeg4bropmthelper.h"
#include "tspacket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///-------------------------------------------------------
/// CYJ,2005-2-17
/// 函数功能:
///		判断是否通视播出
/// 输入参数:
///		pTable				PMT 表
/// 返回参数:
///		>=0					成功 
///		<0					失败
int CTSMPEG4BroPMTHelper::GetTongshiMPEG4BroFormat( const PDVB_PSI_TABLE_PMT pTable )
{
	if( 0 == pTable->m_wCount || NULL == pTable->m_pDescriptor )
		return -1;
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_pDescriptor->Find( DVBPSI_DTID_TSVB_IDENTITY );
	if( NULL == pDr )
		return -1;
	DVBPSI_TSVB_IDITENTITY_DESCRIPTOR * pTSIDDr = (DVBPSI_TSVB_IDITENTITY_DESCRIPTOR*)pDr;
	if( pTSIDDr->m_dwFourCC != TONGSHI_VIDEO_BRO_FOURCC )
		return -1;
	return pTSIDDr->m_wFormatTag;
}


///-------------------------------------------------------
/// CYJ,2005-2-17
/// 函数功能:
///		获取视频节目描述子
/// 输入参数:
///		pTable			PMT 表
///		wPID			输出PID
/// 返回参数:
///		NULL			失败
///		其他			视频描述子
DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * CTSMPEG4BroPMTHelper::GetVideoDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID )
{
	ASSERT( GetTongshiMPEG4BroFormat(pTable) >= 0 );
	wPID = INVALID_PID;
	
	if( 0 == pTable->m_wStreamCount )
		return NULL;
	int nNo = pTable->FindStream( 0xA0 );			//	自定义播出
	if( nNo < 0 )
		return NULL;

	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_aStreams[nNo].m_pDescriptor->Find( DVBPSI_DTID_TSVB_MY_VIDEO );
	if( NULL == pDr )
		return NULL;
	wPID = pTable->m_aStreams[nNo].m_wES_PID;
	return (DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR*)pDr;
}

///-------------------------------------------------------
/// CYJ,2005-2-17
/// 函数功能:
///		获取音频节目描述子
/// 输入参数:
///		pTable			PMT 表
///		wPID			输出PID
///		dwLanguage		匹配的语言，缺省为0，表示不进行判断
/// 返回参数:
///		NULL			失败
///		其他			音频描述子
/// 说明：
///		PID 可能与视频相同，若相同则不能再用一个PES处理器
DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * CTSMPEG4BroPMTHelper::GetAudioDr( const PDVB_PSI_TABLE_PMT pTable, WORD & wPID, DWORD dwLanguage )
{
	ASSERT( GetTongshiMPEG4BroFormat(pTable) >= 0 );
	wPID = INVALID_PID;

	if( 0 == pTable->m_wStreamCount )
		return NULL;

	DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pRetVal = NULL;

	int nNo = pTable->FindStream( 0xA0 );				// 先判断是否在同一个TS分组中
	if( nNo >= 0 )										// 2 代表视频类型
	{
		pRetVal = MatchAudioDr( pTable, nNo, dwLanguage );
		if( pRetVal )
		{
			wPID = pTable->m_aStreams[nNo].m_wES_PID;
			return pRetVal;
		}
	}
	
	for(int i=0; i<pTable->m_wStreamCount; i++ )
	{
		if( pTable->m_aStreams[i].m_byStreamType != 0xA1 )		// 0xA1 代表音频类型
			continue;
		pRetVal = MatchAudioDr( pTable, i, dwLanguage );
		if( NULL == pRetVal )
			continue;
		wPID = pTable->m_aStreams[i].m_wES_PID;
		return pRetVal;
	}

	return NULL;
}

///-------------------------------------------------------
/// CYJ,2005-2-17
/// 函数功能:
///		匹配音频
/// 输入参数:
///		pTable			PMT 表
///		nProgramNo		m_aProgram 数组下标
///		dwLanguage		语言，若为 0，则表示不做语言的限制
/// 返回参数:
///		NULL			失败
///		其他			描述子
DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * CTSMPEG4BroPMTHelper::MatchAudioDr(const PDVB_PSI_TABLE_PMT pTable, int nProgramNo, DWORD dwLanguage )
{
	ASSERT( pTable && nProgramNo >= 0 );
	PDVBPSI_DECODED_DESCRIPTOR_BASE pDr = pTable->m_aStreams[nProgramNo].m_pDescriptor;
	while( pDr )
	{
		if( pDr->m_byDescriptorTag == DVBPSI_DTID_TSVB_MY_AUDIO )
		{
			DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pAudioDr = (DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR*)pDr;
			if( 0 == dwLanguage || dwLanguage == pAudioDr->m_dwISOLanguageID )
				return pAudioDr;
		}
		pDr = pDr->m_pNext;
	}
	return NULL;
}
