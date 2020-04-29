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

#if !defined(AFX_DVBDESCRIPTORS_H__C71D86C6_0740_4CC6_81F4_A2C6F9EAAE89__INCLUDED_)
#define AFX_DVBDESCRIPTORS_H__C71D86C6_0740_4CC6_81F4_A2C6F9EAAE89__INCLUDED_

#include "dvbdescriptorsdefine.h"
#ifndef _WIN32
	#include <stdio.h>
#endif //_WIN32

#pragma pack(push,4)

class CDVBDescriptors
{
public:
	static bool DecodeVideoStreamDr_02( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_vstream_dr_t * pOutBuf );
	static bool DecodeAudioStreamDr_03( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_astream_dr_t * pOutBuf );
	static bool DecodeISOLanguageStreamDr_0A( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_iso639_dr_t * pOutBuf );
	static bool DecodeCAStreamDr_09( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, dvbpsi_ca_dr_t * pOutBuf );
	static bool DecodeTSCVIDDr_200( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_IDITENTITY_DESCRIPTOR * pOutBuf );
	static bool DecodeTSMyVideoDr_201( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR * pOutBuf );
	static bool DecodeTSMyAudioDr_202( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR * pOutBuf );
	static bool DecodeServiceDescriptor_0x48( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SERVICE_DESCRIPTOR * pOutBuf );
	static bool DecodeShortEventDescriptor_0x4D( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SHORT_EVENT_DESCRIPTOR* pOutBuf );
	static bool DecodeExtenedEventDescriptor_0x4E( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_EXTENDED_EVENT_DESCRIPTOR* pOutBuf );
	static bool DecodeCAIdentifierDescriptor_0x53( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_CA_IDENTIFIER_DESCRIPTOR* pOutBuf );
	static bool DecodeContentDescriptor_0x54( PDVB_PSI_DESCRIPTOR_BASE pDescriptor,  DVBPSI_CONTENT_DESCRIPTOR* pOutBuf );
	static bool DecodeParentalRatingDescriptor_0x55( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_PARENTAL_RATING_DESCRIPTOR* pOutBuf );
	static bool DecodeTeletextDescriptor_0x56( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_TELETEXT_DESCRIPTOR* pOutBuf );
	static bool DecodeServiceListDescriptor( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SERVICE_LIST_DESCRIPTOR*pOutBuf );
	static bool DecodeLocalTimeOffsetDescriptor_0x58( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR* pOutBuf );
	static bool DecodeSubtitleDescriptor_0x59( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_SUBTITLE_DESCRIPTOR* pOutBuf );
	static bool DecodeAC3Descriptor_0x6A( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, DVBPSI_AC3_DESCRIPTOR* pOutBuf );

	static int GetDecodedDescriptorSize( PDVB_PSI_DESCRIPTOR_BASE pDescriptor );
	static bool DecodeVideoStreamDescriptor( PDVB_PSI_DESCRIPTOR_BASE pDescriptor, PDVBPSI_DECODED_DESCRIPTOR_BASE pOutBuf );

	static void Dump(PDVBPSI_DECODED_DESCRIPTOR_BASE pDescriptor, FILE * fOutput = NULL);

	CDVBDescriptors();
	virtual ~CDVBDescriptors();

};

#pragma pack(pop)

#endif // !defined(AFX_DVBDESCRIPTORS_H__C71D86C6_0740_4CC6_81F4_A2C6F9EAAE89__INCLUDED_)
