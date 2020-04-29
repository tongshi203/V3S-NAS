///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-10
///
///		用途：
///			解释DVB系统的各个描述子
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#include "stdafx.h"
#include "dvbdescriptors.h"
#include "bitstream.h"

#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

#ifdef _DEBUG
	#define __ENABLE_TRACE__
	#include <stdio.h>
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static int GetExtenedEventDescriptorCount( PDVB_PSI_DESCRIPTOR_BASE pDescriptor );
extern time_t ConvertTimeToUTC(MY_LONG64 llStartTime);

///-------------------------------------------------------
/// CYJ,2010-9-16
/// 函数功能:
///		BCD 码转换为十进制码
/// 输入参数:
///		无
/// 返回参数:
///		无
inline int BCDToDec( BYTE byBCD )
{
	return (byBCD>>4)*10 + (byBCD&0xF);
}

CDVBDescriptors::CDVBDescriptors()
{
#ifdef __ENABLE_TRACE__
	TRACE("sizeof(DVB_PSI_DESCRIPTOR_BASE)=%ld, should be 3 bytes\n", (long)sizeof(DVB_PSI_DESCRIPTOR_BASE) );
#endif // __ENABLE_TRACE__
}

CDVBDescriptors::~CDVBDescriptors()
{

}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// 函数功能:
///		获取对应描述子解码后的大小
/// 输入参数:
///		pDescriptor		描述子缓冲区
/// 返回参数:
///		>0				解码后的大小
///		0				未知类型
int CDVBDescriptors::GetDecodedDescriptorSize( PDVB_PSI_DESCRIPTOR_BASE pDescriptor)
{
	ASSERT( pDescriptor );
	int nRetVal;
	switch( pDescriptor->m_byDescriptorTag )
	{
	case DVBPSI_DTID_VIDEO_STREAM:			// 2, video_stream_descriptor
		return sizeof(dvbpsi_vstream_dr_t);

	case DVBPSI_DTID_AUDIO_STREAM:			// mpeg audio
		return sizeof(dvbpsi_astream_dr_t);

	case DVBPSI_DTID_ISO_LANGUAGE:			// ISO language
		return sizeof(dvbpsi_iso639_dr_t) + pDescriptor->m_byDescriptorLength;	// 2011.4.8 Modify, to save memory

	case DVBPSI_DTID_CA:					// Condition Access
		return sizeof(dvbpsi_ca_dr_t) + pDescriptor->m_byDescriptorLength;		// 2011.4.7 Modify, to save memory

	case DVBPSI_DTID_TSVB_IDENTITY:			// Tongshi Video Broadcast Identity
		return sizeof(DVBPSI_TSVB_IDITENTITY_DESCRIPTOR);

	case DVBPSI_DTID_TSVB_MY_VIDEO:			// Tongshi Video descriptor
		return sizeof(DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR) + pDescriptor->m_abyData[16];

	case DVBPSI_DTID_TSVB_MY_AUDIO:			// Tongshi Audio descriptor
		return sizeof(DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR) + pDescriptor->m_abyData[23];

	case DVBPSI_DTID_SERVICE_DESCRIPTOR:
		{									// service descriptor
			BYTE byProviderNameLen = pDescriptor->m_abyData[1];
			nRetVal = sizeof(DVBPSI_SERVICE_DESCRIPTOR) + byProviderNameLen;
			nRetVal += pDescriptor->m_abyData[2+byProviderNameLen];
		}
		return nRetVal;

	case DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR:	// short event descriptor
		// 2012.11.08 CYJ Modify, not -5 to protection
		return sizeof(DVBPSI_SHORT_EVENT_DESCRIPTOR) + pDescriptor->m_byDescriptorLength;

	case DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR:			// extened event descriptor
		nRetVal = GetExtenedEventDescriptorCount(pDescriptor);
		if( nRetVal < 0 )
			return 0;
		nRetVal *= sizeof(DVBPSI_EXTENDED_EVENT_DESCRIPTOR::tagITEMDESCRIPTOR);
		ASSERT( 8 == sizeof(DVBPSI_EXTENDED_EVENT_DESCRIPTOR::tagITEMDESCRIPTOR) );
		// 2012.11.08 CYJ Modify, not -5 to protection
		nRetVal += ( sizeof(DVBPSI_EXTENDED_EVENT_DESCRIPTOR) + pDescriptor->m_byDescriptorLength \
				    -sizeof(DVBPSI_EXTENDED_EVENT_DESCRIPTOR::tagITEMDESCRIPTOR) );
		return nRetVal;

	case DVBPSI_DTID_CONTENT_DESCRIPTOR:			// content descriptor
		ASSERT( sizeof(DVBPSI_CONTENT_DESCRIPTOR::tagITEM) == 4 );
		return sizeof(DVBPSI_CONTENT_DESCRIPTOR) + pDescriptor->m_byDescriptorLength * 2\
			- sizeof(DVBPSI_CONTENT_DESCRIPTOR::tagITEM);

	case DVBPSI_DTID_PARENTAL_RATING:				// parental rating
		ASSERT( sizeof(DVBPSI_PARENTAL_RATING_DESCRIPTOR::tagITEM) == 5);
		// 2012.11.08 CYJ Modify, sizeof(DVBPSI_PARENTAL_RATING_DESCRIPTOR) <= sizeof(DVBPSI_DTID_PARENTAL_RATING)
		return sizeof(DVBPSI_PARENTAL_RATING_DESCRIPTOR) + (pDescriptor->m_byDescriptorLength/4)*sizeof(DVBPSI_PARENTAL_RATING_DESCRIPTOR::tagITEM);

	case DVBPSI_DTID_NETWORK_NAME:
	case DVBPSI_DTID_BOUQUET_NAME:
		return sizeof(DVBPSI_NETWORK_NAME_DESCRIPTOR)+pDescriptor->m_byDescriptorLength;

	case DVBPSI_DTID_SERVICE_LIST_DESCRIPTOR:	// CYJ, 2008-10-18 add
		return sizeof(DVBPSI_SERVICE_LIST_DESCRIPTOR)+pDescriptor->m_byDescriptorLength-3;

	case DVBPSI_DTID_CA_IDENTIFIER:
		return sizeof(DVBPSI_CA_IDENTIFIER_DESCRIPTOR)+pDescriptor->m_byDescriptorLength-2;

	case DVBPSI_DTID_TELETEXT:
		nRetVal = pDescriptor->m_byDescriptorLength / 5;
		if( nRetVal )
			nRetVal --;			// 结构中，已经有一个
		nRetVal *= sizeof(tagDVBPSI_TELETEXT_DESCRIPTOR::tagPAGEINFO);
		return sizeof(DVBPSI_TELETEXT_DESCRIPTOR) + nRetVal;

	case DVBPSI_DTID_SUBTITLE:		// CYJ 2010.3.5 Add
		nRetVal = pDescriptor->m_byDescriptorLength / 8;
		if( nRetVal )
			nRetVal --;			// 结构中，已经有一个
		nRetVal *= sizeof(tagDVBPSI_SUBTITLE_DESCRIPTOR::tagITEM);
		return sizeof(DVBPSI_SUBTITLE_DESCRIPTOR) + nRetVal;

	case DVBPSI_DTID_AC3:			// CYJ, 2010.3.6 Add
		return sizeof(DVBPSI_AC3_DESCRIPTOR);

	case DVBPSI_DTID_LOCAL_TIME_OFFSET:
		{							//  CYJ,2010-9-16 Local Time offset descriptor
			int nCount = pDescriptor->m_byDescriptorLength / 13;
			return sizeof(DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR) + ( sizeof(DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR)-sizeof(int) ) * (nCount-1);
		}


	default:	//  CYJ,2009-9-5 支持DVBPSI_UNDECODED_DESCRIPTOR，输出未解码的原始数据
		return sizeof(DVBPSI_UNDECODED_DESCRIPTOR) + pDescriptor->m_byDescriptorLength;	//	缺省，只记录 Descriptor ID

	}
	return 0;				// 未知类型
}

///--------------------------------------------------------------
///	CYJ, 2005-1-12
///	函数功能:
///		解码
///	输入参数:
///		pDescriptor			输入描述子
///		pOutBuf				输出解码后的描述子
///	返回参数:
///		true				成功
///		false				失败
bool CDVBDescriptors::DecodeVideoStreamDescriptor( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, PDVBPSI_DECODED_DESCRIPTOR_BASE pOutBuf )
{
	ASSERT( pDescriptor );
	switch( pDescriptor->m_byDescriptorTag )
	{
	case DVBPSI_DTID_VIDEO_STREAM:			// 2, video_stream_descriptor
		return DecodeVideoStreamDr_02(pDescriptor, (dvbpsi_vstream_dr_t*)pOutBuf);

	case DVBPSI_DTID_AUDIO_STREAM:
		return DecodeAudioStreamDr_03(pDescriptor, (dvbpsi_astream_dr_t*)pOutBuf);

	case DVBPSI_DTID_ISO_LANGUAGE:
		return DecodeISOLanguageStreamDr_0A( pDescriptor, (dvbpsi_iso639_dr_t*)pOutBuf );

	case DVBPSI_DTID_CA:
		return DecodeCAStreamDr_09( pDescriptor, (dvbpsi_ca_dr_t*)pOutBuf );

	case DVBPSI_DTID_TSVB_IDENTITY:			// Tongshi Video Broadcast Identity
		return DecodeTSCVIDDr_200( pDescriptor, (DVBPSI_TSVB_IDITENTITY_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_TSVB_MY_VIDEO:			// Tongshi Video descriptor
		return DecodeTSMyVideoDr_201( pDescriptor, (DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_TSVB_MY_AUDIO:			// Tongshi Audio descriptor
		return DecodeTSMyAudioDr_202( pDescriptor, (DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_SERVICE_DESCRIPTOR:
		return DecodeServiceDescriptor_0x48( pDescriptor, (DVBPSI_SERVICE_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR:	// short event descriptor
		return DecodeShortEventDescriptor_0x4D( pDescriptor, (DVBPSI_SHORT_EVENT_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR:
		return DecodeExtenedEventDescriptor_0x4E( pDescriptor, (DVBPSI_EXTENDED_EVENT_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_CONTENT_DESCRIPTOR:
		return DecodeContentDescriptor_0x54( pDescriptor, (DVBPSI_CONTENT_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_PARENTAL_RATING:
		return DecodeParentalRatingDescriptor_0x55( pDescriptor, (DVBPSI_PARENTAL_RATING_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_NETWORK_NAME:
	case DVBPSI_DTID_BOUQUET_NAME:
		{
			pOutBuf->m_byDescriptorTag = pDescriptor->m_byDescriptorTag;
			DVBPSI_NETWORK_NAME_DESCRIPTOR*pDr = (DVBPSI_NETWORK_NAME_DESCRIPTOR*)pOutBuf;
			memcpy(pDr->m_szName, pDescriptor->m_abyData, pDescriptor->m_byDescriptorLength);
			pDr->m_szName[pDescriptor->m_byDescriptorLength] = 0;
		}
		return true;
	case DVBPSI_DTID_CA_IDENTIFIER:
		return DecodeCAIdentifierDescriptor_0x53( pDescriptor, (DVBPSI_CA_IDENTIFIER_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_TELETEXT:		//  CYJ,2006-6-20 支持 teletext
		return DecodeTeletextDescriptor_0x56( pDescriptor, (DVBPSI_TELETEXT_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_SERVICE_LIST_DESCRIPTOR:	// CYJ, 2008-10-18 add
		return DecodeServiceListDescriptor( pDescriptor, (DVBPSI_SERVICE_LIST_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_SUBTITLE:		// CYJ 2010.3.5 Add
		return DecodeSubtitleDescriptor_0x59( pDescriptor, (DVBPSI_SUBTITLE_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_AC3:			// CYJ, 2010.3.6 Add
		return DecodeAC3Descriptor_0x6A( pDescriptor, (DVBPSI_AC3_DESCRIPTOR*)pOutBuf );

	case DVBPSI_DTID_LOCAL_TIME_OFFSET:	//  CYJ,2010-9-16 Local time offset table
		return DecodeLocalTimeOffsetDescriptor_0x58( pDescriptor, (DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR*)pOutBuf );


	default:
		{		//  CYJ,2009-9-5 支持DVBPSI_UNDECODED_DESCRIPTOR，输出未解码的原始数据
			pOutBuf->m_byDescriptorTag = pDescriptor->m_byDescriptorTag;
			DVBPSI_UNDECODED_DESCRIPTOR * pDr = (DVBPSI_UNDECODED_DESCRIPTOR*)pOutBuf;
			pDr->m_byDataLen = pDescriptor->m_byDescriptorLength;
			memcpy( pDr->m_abyData, pDescriptor->m_abyData, pDescriptor->m_byDescriptorLength );
		}
		return true;					//	缺省，只记录 Descriptor ID
	}

	return false;				// 未知类型
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// 函数功能:
///		解释视频流描述子, tag = 02
/// 输入参数:
///		pDescriptor			原始描述子
///		pOutBuf				输出缓冲区
/// 返回参数:
///		true				成功解码
///		false				解码失败
bool CDVBDescriptors::DecodeVideoStreamDr_02( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_vstream_dr_t * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_VIDEO_STREAM == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_VIDEO_STREAM != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_VIDEO_STREAM;

	/* Decode data and check the length */
	pOutBuf->b_mpeg2 = (pDescriptor->m_abyData[0] & 0x04) ? 1 : 0;

	if( (!pOutBuf->b_mpeg2 && (pDescriptor->m_byDescriptorLength != 1))
	  || (pOutBuf->b_mpeg2 && (pDescriptor->m_byDescriptorLength != 3)) )
	{								//	输入长度判断
#ifdef __ENABLE_TRACE__
		TRACE("CDVBDescriptors::DecodeVideoStreamDr_02: bad length (%d)\n", pDescriptor->m_byDescriptorLength );
#endif //_DEBUG
		return false;
	}

	pOutBuf->b_multiple_frame_rate = (pDescriptor->m_abyData[0] & 0x80) ? 1 : 0;
	pOutBuf->i_frame_rate_code = (pDescriptor->m_abyData[0] & 0x78) >> 3;
	pOutBuf->b_constrained_parameter = (pDescriptor->m_abyData[0] & 0x02) ? 1 : 0;
	pOutBuf->b_still_picture = (pDescriptor->m_abyData[0] & 0x01) ? 1 : 0;

	if(pOutBuf->b_mpeg2)
	{
		pOutBuf->i_profile_level_indication = pDescriptor->m_abyData[1];
		pOutBuf->i_chroma_format = (pDescriptor->m_abyData[2] & 0xc0) >> 6;
		pOutBuf->b_frame_rate_extension = (pDescriptor->m_abyData[2] & 0x20) ? 1 : 0;
	}

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// 函数功能:
///		解释音频流描述子，tag = 03
/// 输入参数:
///		pDescriptor			原始描述子
///		pOutBuf				输出缓冲区
/// 返回参数:
///		true				成功解码
///		false				解码失败
bool CDVBDescriptors::DecodeAudioStreamDr_03( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_astream_dr_t * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_AUDIO_STREAM == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_AUDIO_STREAM != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_AUDIO_STREAM;

	if( 1 != pDescriptor->m_byDescriptorLength )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBDescriptors::DecodeAudioStreamDr_03: bad length (%d) != 1\n", pDescriptor->m_byDescriptorLength );
#endif //_DEBUG
		return false;
	}

	pOutBuf->b_free_format = (pDescriptor->m_abyData[0] & 0x80) ? 1 : 0;
	pOutBuf->i_id = (pDescriptor->m_abyData[0] & 0x40) >> 6;
	pOutBuf->i_layer = (pDescriptor->m_abyData[0] & 0x30) >> 4;

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-10
/// 函数功能:
///		解释 ISO 639 语言编码
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeISOLanguageStreamDr_0A( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_iso639_dr_t * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_ISO_LANGUAGE == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_ISO_LANGUAGE != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_ISO_LANGUAGE;

  /* Decode data and check the length */
	if( (pDescriptor->m_byDescriptorLength < 1) || ((pDescriptor->m_byDescriptorLength - 1) % 3 != 0) )
	{
#ifdef __ENABLE_TRACE__
		TRACE("CDVBDescriptors::DecodeISOLanguageStreamDr_0A bad length (%d)\n", pDescriptor->m_byDescriptorLength );
#endif // _DEBUG
		return false;
	}

	pOutBuf->i_audio_type = pDescriptor->m_abyData[ pDescriptor->m_byDescriptorLength - 1 ];
	pOutBuf->i_code_count = (pDescriptor->m_byDescriptorLength - 1) / 3;
	if( pOutBuf->i_code_count )
		memcpy( pOutBuf->i_iso_639_code, pDescriptor->m_abyData, pDescriptor->m_byDescriptorLength - 1 );
	return ( pOutBuf->i_code_count > 0 );
}

///-------------------------------------------------------
/// CYJ,2005-1-12
/// 函数功能:
///		解释 CA 描述子
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeCAStreamDr_09( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_ca_dr_t * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_CA == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_CA != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_CA;

    /* Decode data and check the length */
	if(pDescriptor->m_byDescriptorLength < 4)
	{
#ifdef _DEBUG
		TRACE("dr_09 decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	pOutBuf->i_ca_system_id =   ( pDescriptor->m_abyData[0] << 8) | pDescriptor->m_abyData[1];
	pOutBuf->i_ca_pid =   ((pDescriptor->m_abyData[2] & 0x1f) << 8) | pDescriptor->m_abyData[3];
	pOutBuf->i_private_length = pDescriptor->m_byDescriptorLength - 4;
	if( pOutBuf->i_private_length )
		memcpy(pOutBuf->i_private_data, pDescriptor->m_abyData + 4, pOutBuf->i_private_length );
	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-22
/// 函数功能:
///		通视特征播出描述子
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeTSCVIDDr_200( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_IDITENTITY_DESCRIPTOR * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_TSVB_IDENTITY == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_TSVB_IDENTITY != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_TSVB_IDENTITY;
	if( pDescriptor->m_byDescriptorLength < 6 )
	{
#ifdef _DEBUG
		TRACE("DecodeTSCVIDDr_200 decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}
	pOutBuf->m_dwFourCC =  ( DWORD( pDescriptor->m_abyData[0] ) << 24 )
						  |( DWORD( pDescriptor->m_abyData[1] ) << 16 )
						  |( DWORD( pDescriptor->m_abyData[2] ) << 8 )
						  |  DWORD( pDescriptor->m_abyData[3] );
	pOutBuf->m_wFormatTag = ( WORD( pDescriptor->m_abyData[4] ) << 8 ) | ( WORD(pDescriptor->m_abyData[5]) );

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-22
/// 函数功能:
///		通视特征播出描述子
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeTSMyVideoDr_201( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_TSVB_MY_VIDEO == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_TSVB_MY_VIDEO != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_TSVB_MY_VIDEO;
	if( pDescriptor->m_byDescriptorLength < 17 )
	{
#ifdef _DEBUG
		TRACE("DecodeTSMyVideoDr_201 decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	CMyBitStream InBs( pDescriptor->m_abyData, pDescriptor->m_byDescriptorLength );

	pOutBuf->m_dwHandlerFourCC = InBs.getbits32();
	pOutBuf->m_wWidth = InBs.getbits( 16 );
	pOutBuf->m_wHeight = InBs.getbits( 16 );
	pOutBuf->m_dwTimeScale = InBs.getbits32();
	pOutBuf->m_dwRate = InBs.getbits32();
	pOutBuf->m_byExtensionLength = InBs.getbits( 8 );

	PBYTE pbyExtData = pDescriptor->m_abyData + 17;
	for(int i=0; i<pOutBuf->m_byExtensionLength; i++)
	{
		pOutBuf->m_abyExtensionData[i] = pbyExtData[i];
	}

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-22
/// 函数功能:
///		通视特征播出描述子
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeTSMyAudioDr_202( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_TSVB_MY_AUDIO == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_TSVB_MY_AUDIO != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_TSVB_MY_AUDIO;
	if( pDescriptor->m_byDescriptorLength < 24 )
	{
#ifdef _DEBUG
		TRACE("DecodeTSMyAudioDr_202 decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	CMyBitStream InBs( pDescriptor->m_abyData, pDescriptor->m_byDescriptorLength );

	pOutBuf->m_byStreamID = InBs.getbits( 8 );
	pOutBuf->m_wAudioFormat = InBs.getbits( 16 );
	pOutBuf->m_byChannels = InBs.getbits( 8 );
	pOutBuf->m_dwSamplesPerSecond = InBs.getbits32();
	pOutBuf->m_dwAverageBytesPerSecond = InBs.getbits32();
	pOutBuf->m_dwTimeScale = InBs.getbits32();
	pOutBuf->m_dwRate = InBs.getbits32();
	pOutBuf->m_dwISOLanguageID = InBs.getbits( 24 );
	pOutBuf->m_byExtensionLength = InBs.getbits( 8 );

	PBYTE pbyExtData = pDescriptor->m_abyData + 24;

	if( MY_AUDIO_FORMATID_PCM == pOutBuf->m_wAudioFormat )
	{
		pOutBuf->m_wPCM_BitsPerSample = InBs.getbits( 16);		//	仅类型为PCM时，才会出现该字段
		pbyExtData += 2;
	}
	else
		pOutBuf->m_wPCM_BitsPerSample = 0;

	for(int i=0; i<pOutBuf->m_byExtensionLength; i++)
	{
		pOutBuf->m_abyExtensionData[i] = pbyExtData[i];
	}

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-26
/// 函数功能:
///		解释 Service Descriptor
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeServiceDescriptor_0x48( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SERVICE_DESCRIPTOR * pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_SERVICE_DESCRIPTOR == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_SERVICE_DESCRIPTOR != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_SERVICE_DESCRIPTOR;
	if( pDescriptor->m_byDescriptorLength < 3 )
	{
#ifdef _DEBUG
		TRACE("DecodeServiceDescriptor_0x48 decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	PBYTE pBuf = pDescriptor->m_abyData;
	pOutBuf->m_byServiceType = *pBuf ++;	// service type
	BYTE byLen = *pBuf ++;
	pOutBuf->m_pszProviderName = pOutBuf->m_szPSNames;
	if( byLen )
		memcpy( pOutBuf->m_pszProviderName, pBuf, byLen );
	pOutBuf->m_pszProviderName[ byLen ] = 0;
	pBuf += byLen;

	pOutBuf->m_pszServiceName = pOutBuf->m_szPSNames + byLen + 1;
	byLen = *pBuf ++;
	if( byLen )
		memcpy( pOutBuf->m_pszServiceName, pBuf, byLen );
	pOutBuf->m_pszServiceName[byLen] = 0;

#ifdef _DEBUG
	pBuf += byLen;
	ASSERT( (pBuf-pDescriptor->m_abyData) == pDescriptor->m_byDescriptorLength );
#endif //_DEBUG
	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-27
/// 函数功能:
///		解释短形式事件描述
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeShortEventDescriptor_0x4D( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SHORT_EVENT_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR;
	if( pDescriptor->m_byDescriptorLength < 5 )
	{
#ifdef _DEBUG
		TRACE("DecodeShortEventDescriptor_0x4D decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	PBYTE pBuf = pDescriptor->m_abyData;
	pOutBuf->m_dwISOLanguageID = (DWORD(pBuf[0])<<16) | (DWORD(pBuf[1])<<8) | DWORD(pBuf[2]);
	pBuf += 3;

	if( pBuf[0] + 3 >= pDescriptor->m_byDescriptorLength )
	{
		fprintf( stderr, "Error event name length(%d) > Descriptor Len(%d)\n",
			pBuf[0]+3, pDescriptor->m_byDescriptorLength ) ;
		return false;
	}

	BYTE byLen = *pBuf ++;
	pOutBuf->m_pszEventName = pOutBuf->m_szDataBuf;
	if( byLen )
		memcpy( pOutBuf->m_pszEventName, pBuf, byLen );
	pOutBuf->m_pszEventName[ byLen ] = 0;
	pBuf += byLen;

	if( pBuf[0] + 3 + byLen >= pDescriptor->m_byDescriptorLength )
	{
		fprintf( stderr, "Error event name length(%d) > Descriptor Len(%d)\n",
			pBuf[0]+3+byLen, pDescriptor->m_byDescriptorLength ) ;
		return false;
	}

	pOutBuf->m_pszText = pOutBuf->m_szDataBuf + byLen + 1;
	byLen = *pBuf ++;
	if( byLen )
		memcpy( pOutBuf->m_pszText, pBuf, byLen );
	pOutBuf->m_pszText[byLen] = 0;

#ifdef _DEBUG
	pBuf += byLen;
	if( (pBuf-pDescriptor->m_abyData) != pDescriptor->m_byDescriptorLength )
	{	// 2011.3.6 CYJ Add, to show why
		fprintf( stderr, "ShowEventDescriptor Assert failed.\n" );
		fprintf( stderr, "pBuf(%p)-pDescriptor->m_abyData(%p)[=%d] != pDescriptor->m_byDescriptorLength(%d) )\n",
			pBuf, pDescriptor->m_abyData, (int)( pBuf-pDescriptor->m_abyData ),
			pDescriptor->m_byDescriptorLength );
		for(int i=0; i<pDescriptor->m_byDescriptorLength; i++)
		{
			fprintf( stderr, "%02X ", pDescriptor->m_abyData[i] );
			if( ( i & 15 ) == 15 )
				fprintf( stderr, "\n");
			else if( (i&7) == 7 )
				fprintf( stderr, " -  " );
		}
		fprintf( stderr, "\n");
	}
	ASSERT( (pBuf-pDescriptor->m_abyData) == pDescriptor->m_byDescriptorLength );
#endif //_DEBUG
	return true;
}

void CDVBDescriptors::Dump(PDVBPSI_DECODED_DESCRIPTOR_BASE pDescriptor, FILE * fOutput)
{
#ifdef _DEBUG
	if( NULL == fOutput )
		fOutput = stderr;

	const char * pszIndent = "       ";
	fprintf( fOutput, "%sTagID=0x%02X\n", pszIndent, pDescriptor->m_byDescriptorTag );
	ASSERT( pDescriptor );
	switch( pDescriptor->m_byDescriptorTag )
	{
	case DVBPSI_DTID_VIDEO_STREAM:			// 2, video_stream_descriptor
		{
			dvbpsi_vstream_dr_t* pVideoDr = (dvbpsi_vstream_dr_t*)pDescriptor;
			fprintf( fOutput, "%s  bMultiFrameRate=%d,FrameRateCode=%d, IsMpeg2=%d\n",
				pszIndent, pVideoDr->b_multiple_frame_rate,pVideoDr->i_frame_rate_code,
				pVideoDr->b_mpeg2 );
			fprintf( fOutput, "%s  b_constrained_parameter=%d,b_still_picture=%d, \n",
				pszIndent, pVideoDr->b_constrained_parameter, pVideoDr->b_still_picture );
			if( pVideoDr->b_mpeg2  )
			{
				fprintf( fOutput, "%s  profile=%d,chroma=%d,FrameRateExtension=%d\n",
					pszIndent, pVideoDr->i_profile_level_indication,pVideoDr->i_chroma_format,
					pVideoDr->b_frame_rate_extension );
			}
		}
		break;

	case DVBPSI_DTID_AUDIO_STREAM:
		{
			dvbpsi_astream_dr_t * pAudioDr = (dvbpsi_astream_dr_t*)pDescriptor;
			fprintf( fOutput, "%s  FreeFormat=%d, id=%d, Layer=%d\n",
				pszIndent, pAudioDr->b_free_format, pAudioDr->i_id, pAudioDr->i_layer );
		}
		break;

	case DVBPSI_DTID_ISO_LANGUAGE:
		{
			dvbpsi_iso639_dr_t * pDr = (dvbpsi_iso639_dr_t*)pDescriptor;
			fprintf( fOutput, "%s  There are %d languages:\n", pszIndent, pDr->i_code_count );
			for(int i=0; i<pDr->i_code_count; i++ )
			{
				fprintf( fOutput, "%s  %c%c%c\n", pszIndent, pDr->i_iso_639_code[i*3],
					pDr->i_iso_639_code[i*3+1],pDr->i_iso_639_code[i*3+2] );
			}
			fprintf( fOutput, "%s  audio=%d\n", pszIndent, pDr->i_audio_type );
		}
		break;

	case DVBPSI_DTID_CA:
		break;

	case DVBPSI_DTID_TSVB_IDENTITY:			// Tongshi Video Broadcast Identity
		{
			DVBPSI_TSVB_IDITENTITY_DESCRIPTOR * pDr = (DVBPSI_TSVB_IDITENTITY_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s  FourCC=%c%c%c%c, Format=%d\n", pszIndent,
				char(pDr->m_dwFourCC>>24),char(pDr->m_dwFourCC>>16),char(pDr->m_dwFourCC>>8),
				char(pDr->m_dwFourCC), pDr->m_wFormatTag );
		}
		break;

	case DVBPSI_DTID_TSVB_MY_VIDEO:			// Tongshi Video descriptor
		{
			DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * pDr = (DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s  FourCC=%c%c%c%c, Width=%d, Height=%d, TimeScale=%d, Rate=%d\n",
				pszIndent, char(pDr->m_dwHandlerFourCC>>24),char(pDr->m_dwHandlerFourCC>>16),
				char(pDr->m_dwHandlerFourCC>>8),char(pDr->m_dwHandlerFourCC),
				pDr->m_wWidth, pDr->m_wHeight, pDr->m_dwTimeScale, pDr->m_dwRate );
		}
		break;

	case DVBPSI_DTID_TSVB_MY_AUDIO:			// Tongshi Audio descriptor
		{
			DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pDr = (DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s  StreamID=0x%02X, Format=0x%04X, Channels=%d\n",
				pszIndent, pDr->m_byStreamID, pDr->m_wAudioFormat, pDr->m_byChannels );
			fprintf( fOutput, "%s  SameplePerSec=%d, AvgByte=%d, TimeScale=%d\n",
				pszIndent, pDr->m_dwSamplesPerSecond, pDr->m_dwAverageBytesPerSecond,
				pDr->m_dwTimeScale );
			fprintf( fOutput, "%s  Rate=%d, ISOLanguage=%c%c%c, BitPerSample=%d\n",
				pszIndent, pDr->m_dwRate, char(pDr->m_dwISOLanguageID>>16),
				char(pDr->m_dwISOLanguageID>>8),char(pDr->m_dwISOLanguageID), pDr->m_wPCM_BitsPerSample );
		}
		break;

	case DVBPSI_DTID_SERVICE_DESCRIPTOR:
		{
			DVBPSI_SERVICE_DESCRIPTOR * pDr = (DVBPSI_SERVICE_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s  ServiceType=0x%02X, Provider=%s, \n%s  ServiceName=%s\n",
				pszIndent, pDr->m_byServiceType, pDr->m_pszProviderName, pszIndent, pDr->m_pszServiceName );
		}
		break;

	case DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR:	// short event descriptor
		{
			DVBPSI_SHORT_EVENT_DESCRIPTOR * pDr = (DVBPSI_SHORT_EVENT_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s  Language=%c%c%c, Event=%s, Text=%s\n",
				pszIndent, char(pDr->m_dwISOLanguageID>>16),char(pDr->m_dwISOLanguageID>>8),
				char(pDr->m_dwISOLanguageID), pDr->m_pszEventName, pDr->m_pszText );
		}
		break;
	case DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR:
		{
			DVBPSI_EXTENDED_EVENT_DESCRIPTOR * pDr = (DVBPSI_EXTENDED_EVENT_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s   DrNumber=%d, LastDrNumber=%d, Language=%c%c%c\n",
				pszIndent, pDr->m_byDescriptorNumber, pDr->m_byLastDescriptorNumber,
				char(pDr->m_dwISOLanguageID>>16),char(pDr->m_dwISOLanguageID>>8),char(pDr->m_dwISOLanguageID) );
			fprintf( fOutput, "%s   There are %d Item Descriptors:\n", pszIndent, pDr->m_byCount );
			for(int i=0; i<pDr->m_byCount; i++ )
			{
				fprintf( fOutput, "%s      Descriptor=%s, Text=%s\n",
					pszIndent, pDr->m_aItems[i].m_pszDescriptor, pDr->m_aItems[i].m_pszText );
			}
			fprintf( fOutput, "%s    Detail=%s\n", pszIndent, pDr->m_pszDetail );
		}
		break;
	case DVBPSI_DTID_CONTENT_DESCRIPTOR:
		{
			DVBPSI_CONTENT_DESCRIPTOR * pDr = (DVBPSI_CONTENT_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s     There are %d items\n", pszIndent, pDr->m_byCount );
			for(int i=0; i<pDr->m_byCount; i++)
			{
				fprintf( fOutput, "%s       NibbleLevel1=%d, Level2=%d, UserNibble1=%d, User2=%d\n",
					pszIndent, pDr->m_aItems[i].m_byNibble_Level1, pDr->m_aItems[i].m_byNibble_Level2,
					pDr->m_aItems[i].m_byUserNibble1, pDr->m_aItems[i].m_byUserNibble2 );
			}
		}
		break;
	case DVBPSI_DTID_PARENTAL_RATING:
		{
			DVBPSI_PARENTAL_RATING_DESCRIPTOR * pDr = (DVBPSI_PARENTAL_RATING_DESCRIPTOR*)pDescriptor;
			for(int i=0; i<pDr->m_byCount; i++ )
			{
				fprintf( fOutput, "%s      CountryCode=%c%c%c, Rating=%d\n",
					pszIndent, char(pDr->m_aItems[i].m_dwCountryCode>>16),
					char(pDr->m_aItems[i].m_dwCountryCode>>8),char(pDr->m_aItems[i].m_dwCountryCode),
					pDr->m_aItems[i].m_byRating );
			}
		}
		break;
	case DVBPSI_DTID_NETWORK_NAME:
	case DVBPSI_DTID_BOUQUET_NAME:
		{
			DVBPSI_NETWORK_NAME_DESCRIPTOR * pDr = (DVBPSI_NETWORK_NAME_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s      %s\n", pszIndent, pDr->m_szName );
		}
		break;

	case DVBPSI_DTID_CA_IDENTIFIER:
		{
			DVBPSI_CA_IDENTIFIER_DESCRIPTOR * pDr = (DVBPSI_CA_IDENTIFIER_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s      There are %d items\n", pszIndent, pDr->m_byCount );
			for(int i=0; i<pDr->m_byCount; i++ )
			{
				fprintf( fOutput, "%s         CA SystemID=0x%04X\n", pszIndent, pDr->m_awCASystemID[i] );
			}
		}
		break;
	case DVBPSI_DTID_TELETEXT:
		{
			DVBPSI_TELETEXT_DESCRIPTOR * pDr = (DVBPSI_TELETEXT_DESCRIPTOR*)pDescriptor;
			fprintf( fOutput, "%s      There are %d items\n", pszIndent, pDr->m_byCount );
			for(int i=0; i<pDr->m_byCount; i++ )
			{
				fprintf( fOutput, "%s         Teletext[%d]=%c%c%c, T=%d,M=%d,P=%02X\n", pszIndent, i,
					pDr->m_aPages[i].ISO_639_LangCode[0],pDr->m_aPages[i].ISO_639_LangCode[1],
					pDr->m_aPages[i].ISO_639_LangCode[2],pDr->m_aPages[i].m_byTeletextType,
					pDr->m_aPages[i].m_byMagNum, pDr->m_aPages[i].m_byPageNumBCD );
			}
		}
		break;
	}
#else
	(void)pDescriptor;
	(void)fOutput;
#endif //_DEBUG
}

///-------------------------------------------------------
/// CYJ,2005-1-28
/// 函数功能:
///		计算扩展事件描述子个数
/// 输入参数:
///		pDescriptor			描述子
/// 返回参数:
///		个数
///		<0					失败
static int GetExtenedEventDescriptorCount( PDVB_PSI_DESCRIPTOR_BASE pDescriptor )
{
	BYTE byCount = 0;
	int nLen = pDescriptor->m_abyData[4];
	PBYTE pBufTmp = pDescriptor->m_abyData + 5;
	while( nLen > 0 )
	{
		BYTE byLen = (*pBufTmp) + 1;
		pBufTmp += byLen;
		nLen -= byLen;

		byLen = (*pBufTmp) + 1;
		pBufTmp += byLen;
		nLen -= byLen;

		byCount ++;
	}
	ASSERT( 0==nLen );
	if( nLen != 0 )
		return -1;				//	错误
	return byCount;
}

///-------------------------------------------------------
/// CYJ,2005-1-28
/// 函数功能:
///		扩展的事件描述子
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeExtenedEventDescriptor_0x4E( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_EXTENDED_EVENT_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR;
	if( pDescriptor->m_byDescriptorLength < 6 )
	{
#ifdef _DEBUG
		TRACE("DecodeShortEventDescriptor_0x4E decoder: bad length (%d)\n", pDescriptor->m_byDescriptorLength);
#endif // _DEBUG
		return false;
	}

	PBYTE pBuf = pDescriptor->m_abyData;
	pOutBuf->m_byDescriptorNumber = (*pBuf) >> 4;
	pOutBuf->m_byLastDescriptorNumber = (*pBuf ++) & 0xF;
	pOutBuf->m_dwISOLanguageID = (DWORD(pBuf[0])<<16) | (DWORD(pBuf[1])<<8) | DWORD(pBuf[2]);
	pBuf += 4;		//	并跳过后面的长度字节

	BYTE byCount = GetExtenedEventDescriptorCount( pDescriptor );
	pOutBuf->m_byCount = byCount;

	BYTE byLen;
	char * pszDst = ((char*)&pOutBuf->m_byCount) + 1;
	pszDst += byCount * sizeof(DVBPSI_EXTENDED_EVENT_DESCRIPTOR::tagITEMDESCRIPTOR);
	for( int i=0; i<byCount; i++ )
	{
		pOutBuf->m_aItems[i].m_pszDescriptor = pszDst;
		byLen = *pBuf ++;
		if( byLen )
			memcpy( pszDst, pBuf, byLen );
		pBuf += byLen;
		pszDst[byLen] = 0;
		pszDst += byLen + 1;

		pOutBuf->m_aItems[i].m_pszText = pszDst;
		byLen = *pBuf ++;
		if( byLen )
			memcpy( pszDst, pBuf, byLen );
		pBuf += byLen;
		pszDst[byLen] = 0;
		pszDst += byLen + 1;
	}

	pOutBuf->m_pszDetail = pszDst;
	byLen = *pBuf ++;
	if( byLen )
		memcpy( pszDst, pBuf, byLen );
	pszDst[byLen] = 0;

#ifdef _DEBUG
	pBuf += byLen;
	ASSERT( (pBuf-pDescriptor->m_abyData) == pDescriptor->m_byDescriptorLength );
#endif //_DEBUG

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-28
/// 函数功能:
///		解释 Content descriptor
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeContentDescriptor_0x54( PDVB_PSI_DESCRIPTOR_BASE pDescriptor,  DVBPSI_CONTENT_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_CONTENT_DESCRIPTOR == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_CONTENT_DESCRIPTOR != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_CONTENT_DESCRIPTOR;
	BYTE byCount = pDescriptor->m_byDescriptorLength / 2;
	pOutBuf->m_byCount = byCount;
	DVBPSI_CONTENT_DESCRIPTOR::tagITEM * pItem = pOutBuf->m_aItems;
	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<byCount; i++ )
	{
		pItem->m_byNibble_Level1 = (*pBuf>>4);
		pItem->m_byNibble_Level2 = (*pBuf ++) & 0xF;
		pItem->m_byUserNibble1 = (*pBuf>>4);
		pItem->m_byUserNibble2 = (*pBuf ++) & 0xF;

		pItem ++;
	}

#ifdef _DEBUG
	ASSERT( (pBuf-pDescriptor->m_abyData) == pDescriptor->m_byDescriptorLength );
#endif //_DEBUG

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		解释 Parental rating descriptor
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeParentalRatingDescriptor_0x55( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_PARENTAL_RATING_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_PARENTAL_RATING == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_PARENTAL_RATING != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_PARENTAL_RATING;
	BYTE byCount = pDescriptor->m_byDescriptorLength / 4;
	pOutBuf->m_byCount = byCount;
	DVBPSI_PARENTAL_RATING_DESCRIPTOR::tagITEM * pItem = pOutBuf->m_aItems;
	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<byCount; i++ )
	{
		pItem->m_dwCountryCode = (DWORD(pBuf[0])<<16) | (DWORD(pBuf[1])<<8) | DWORD(pBuf[2]);
		pBuf += 3;
		pItem->m_byRating = *pBuf ++;

		pItem ++;
	}

#ifdef _DEBUG
	ASSERT( (pBuf-pDescriptor->m_abyData) == pDescriptor->m_byDescriptorLength );
#endif //_DEBUG

	return true;
}

///-------------------------------------------------------
/// CYJ,2005-1-29
/// 函数功能:
///		解释 Parental rating descriptor
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeCAIdentifierDescriptor_0x53( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_CA_IDENTIFIER_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( DVBPSI_DTID_CA_IDENTIFIER == pDescriptor->m_byDescriptorTag );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_CA_IDENTIFIER != pDescriptor->m_byDescriptorTag )
		return false;

	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_CA_IDENTIFIER;
//	ASSERT( pDescriptor->m_byDescriptorLength & 1 == 0 );
	BYTE byCount = pDescriptor->m_byDescriptorLength / 2;
	pOutBuf->m_byCount = byCount;
	PWORD paItem = pOutBuf->m_awCASystemID;
	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<byCount; i++	)
	{
		*paItem ++ = ( WORD(pBuf[0])<<8 ) | WORD(pBuf[1]);
		pBuf += 2;
	}
	return true;
}

///-------------------------------------------------------
/// CYJ,2006-6-20
/// 函数功能:
///		解码 Teletext 信息
/// 输入参数:
///		pDescriptor		输入原始数据
///		pOutBuf			输出缓冲区
/// 返回参数:
///		true			succ
///		false			failed
bool CDVBDescriptors::DecodeTeletextDescriptor_0x56( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TELETEXT_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor&& pOutBuf );
	ASSERT( pDescriptor->m_byDescriptorTag == DVBPSI_DTID_TELETEXT );
	if( NULL == pDescriptor || NULL == pOutBuf || DVBPSI_DTID_TELETEXT != pDescriptor->m_byDescriptorTag )
		return false;
	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_TELETEXT;

	ASSERT( (pDescriptor->m_byDescriptorLength % 5) == 0 );

	int nCount = pDescriptor->m_byDescriptorLength / 5;
	PBYTE pBuf = pDescriptor->m_abyData;
	pOutBuf->m_byCount = nCount;
	tagDVBPSI_TELETEXT_DESCRIPTOR::tagPAGEINFO * pPageInfo = pOutBuf->m_aPages;
	for( int i=0; i<nCount; i++ )
	{
		pPageInfo->ISO_639_LangCode[0] = *pBuf ++;
		pPageInfo->ISO_639_LangCode[1] = *pBuf ++;
		pPageInfo->ISO_639_LangCode[2] = *pBuf ++;
		pPageInfo->m_byTeletextType = (*pBuf) >> 3;
		pPageInfo->m_byMagNum = (*pBuf) & 7;
		pBuf ++;
		pPageInfo->m_byPageNumBCD = *pBuf ++;

		pPageInfo ++;
	}

	return true;
}

///-------------------------------------------------------
/// CYJ,2008-10-18
/// 函数功能:
///		解释 Service List
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBDescriptors::DecodeServiceListDescriptor( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SERVICE_LIST_DESCRIPTOR*pOutBuf )
{
	ASSERT( pDescriptor->m_byDescriptorTag == DVBPSI_DTID_SERVICE_LIST_DESCRIPTOR );
	pOutBuf->m_byCount = pDescriptor->m_byDescriptorLength / 3;
	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<pOutBuf->m_byCount; i++ )
	{
		pOutBuf->m_aItems[i].m_wSID = (pBuf[0] << 8) + pBuf[1];
		pOutBuf->m_aItems[i].m_byServiceType = pBuf[2];
		pBuf += 3;
	}

	return true;
}

///--------------------------------------------------
/// CYJ, 2010.3.5 add
/// Function:
///		Decode Subtitle
///	Input:
///		None
/// Output:
///
bool CDVBDescriptors::DecodeSubtitleDescriptor_0x59( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SUBTITLE_DESCRIPTOR* pOutBuf )
{
	ASSERT( pDescriptor->m_byDescriptorTag == DVBPSI_DTID_SUBTITLE );
	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_SUBTITLE;		// 2010.6.8 Add

	pOutBuf->m_byCount = pDescriptor->m_byDescriptorLength / 8;
	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<pOutBuf->m_byCount; i++ )
	{
		memcpy( pOutBuf->m_aItems[i].ISO_639_LangCode, pBuf, 3 );
		pOutBuf->m_aItems[i].m_byType = pBuf[3];
		pOutBuf->m_aItems[i].m_wCompositionPageID = pBuf[4]*0x100 + pBuf[5];
		pOutBuf->m_aItems[i].m_wAncillaryPageID = pBuf[6]*0x100 + pBuf[7];
		pBuf += 8;
	}
	return true;
}

///--------------------------------------------------
/// CYJ, 2010.3.6 add
/// Function:
///		Decode AC-3
///	Input:
///		None
/// Output:
///
bool CDVBDescriptors::DecodeAC3Descriptor_0x6A( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_AC3_DESCRIPTOR* pOutBuf )
{
	ASSERT( DVBPSI_DTID_AC3 == pDescriptor->m_byDescriptorTag );
	PBYTE pBuf = pDescriptor->m_abyData;
	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_AC3;	// 2010.6.8 Add

	BYTE byFlags = *pBuf ++;
	pOutBuf->m_byFlags = byFlags;			// see also #define DVBPSI_AC3_FLAGS_xxx

#define DVBPSI_AC3_FLAGS_COMPONENT_TYPE_VALID	0x80		// m_byAC3Type valid
#define DVBPSI_AC3_FLAGS_BSID_VALID				0x40		// m_byBSID valid
#define DVBPSI_AC3_FLAGS_MAINID_VALID			0x20		// m_byMAINID valid
#define DVBPSI_AC3_FLAGS_ASVC_VALID				0x10		// m_byASVC valid
	if( byFlags & DVBPSI_AC3_FLAGS_COMPONENT_TYPE_VALID )
		pOutBuf->m_byAC3Type = *pBuf ++;
	else
		pOutBuf->m_byAC3Type = 0;

	if( byFlags & DVBPSI_AC3_FLAGS_BSID_VALID )
		pOutBuf->m_byBSID = *pBuf ++;
	else
		pOutBuf->m_byBSID = 0;

	if( byFlags & DVBPSI_AC3_FLAGS_MAINID_VALID )
		pOutBuf->m_byMAINID = *pBuf ++;
	else
		pOutBuf->m_byMAINID = 0;

	if( byFlags & DVBPSI_AC3_FLAGS_ASVC_VALID )
		pOutBuf->m_byASVC = *pBuf ++;
	else
		pOutBuf->m_byASVC = 0;

	return true;		// 2010.6.8 CYJ Add, mean succ
}

///-------------------------------------------------------
/// CYJ,2010-9-16
/// 函数功能:
///		解析本地时间偏移
/// 输入参数:
///		无
/// 返回参数:
///		无
bool CDVBDescriptors::DecodeLocalTimeOffsetDescriptor_0x58( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR* pOutBuf )
{
	ASSERT( DVBPSI_DTID_LOCAL_TIME_OFFSET == pDescriptor->m_byDescriptorTag );
	pOutBuf->m_byDescriptorTag = DVBPSI_DTID_LOCAL_TIME_OFFSET;

	int nCount = pDescriptor->m_byDescriptorLength / 13;
	pOutBuf->m_nCount = nCount;

	PBYTE pBuf = pDescriptor->m_abyData;
	for(int i=0; i<nCount; i++ )
	{
		pOutBuf->m_aLocalTimes[i].m_abyCountryCode[0] = *pBuf ++;
		pOutBuf->m_aLocalTimes[i].m_abyCountryCode[1] = *pBuf ++;
		pOutBuf->m_aLocalTimes[i].m_abyCountryCode[2] = *pBuf ++;
		pOutBuf->m_aLocalTimes[i].m_byRegionID = (*pBuf) >> 2;
		pOutBuf->m_aLocalTimes[i].m_bPolarity = (*pBuf++)&1;
		pOutBuf->m_aLocalTimes[i].m_nLocalTimeOffsetMin = BCDToDec( pBuf[0] )*60 + BCDToDec( pBuf[1] );	// 本地时间偏移，单位：分钟
		pBuf += 2;

		// 下次即将开始/结束（夏令时）的时间（包含日期）
		MY_LONG64 llDateTime = 0;
		for(int j=0; j<5; j++ )
		{
			llDateTime <<= 8;
			llDateTime += *pBuf ++;
		}
		pOutBuf->m_aLocalTimes[i].m_tTimeOfChange = ConvertTimeToUTC( llDateTime );

		pOutBuf->m_aLocalTimes[i].m_nNextTimeOffsetMin = BCDToDec( pBuf[0] )*60 + BCDToDec( pBuf[1] );	// 本地时间偏移，单位：分钟;				// 下次变化后的时间偏移，单位：分钟
		pBuf += 2;
	}

	return true;		// 2010.6.8 CYJ Add, mean succ
}
