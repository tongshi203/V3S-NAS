///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			DVB 描述子定义
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __DVB_DESCRIPTORS_H_20050111__
#define __DVB_DESCRIPTORS_H_20050111__

#include <time.h>

typedef enum
{
	DVBPSI_DTID_VIDEO_STREAM = 2,
	DVBPSI_DTID_AUDIO_STREAM,
	DVBPSI_DTID_HIERARCHY,
	DVBPSI_DTID_REGISTRATION,

	DVBPSI_DTID_CA = 9,
	DVBPSI_DTID_ISO_LANGUAGE,

	DVBPSI_DTID_NETWORK_NAME = 0x40,
	DVBPSI_DTID_SERVICE_LIST_DESCRIPTOR,
	DVBPSI_DTID_BOUQUET_NAME = 0x47,
	DVBPSI_DTID_SERVICE_DESCRIPTOR = 0x48,
	DVBPSI_DTID_SHORT_EVENT_DESCRIPTOR = 0x4D,
	DVBPSI_DTID_EXTENDED_EVENT_DESCRIPTOR,

	DVBPSI_DTID_CA_IDENTIFIER = 0x53,
	DVBPSI_DTID_CONTENT_DESCRIPTOR = 0x54,
	DVBPSI_DTID_PARENTAL_RATING,		// 0x55
	DVBPSI_DTID_TELETEXT,				// 0x56，注：不同于VBI_TeleText, VBI_Teletext 的tagid=0x45,0x46
	DVBPSI_DTID_LOCAL_TIME_OFFSET=0x58,	// 0x58, Local time offset descriptor
	DVBPSI_DTID_SUBTITLE = 0x59,		// Subtitle

	DVBPSI_DTID_AC3 = 0x6A,				// AC-3
	DVBPSI_DTID_EAC3 = 0x7A,			// EAC-3

	DVBPSI_DTID_TSVB_IDENTITY = 200,
	DVBPSI_DTID_TSVB_MY_VIDEO,
	DVBPSI_DTID_TSVB_MY_AUDIO,
}DVBPSI_DESCRIPTOR_TAG;

#pragma pack(push,1)

#ifndef __MY_PACKTED__
	#ifdef _WIN32
		#define __MY_PACKTED__
	#else
		#define	__MY_PACKTED__ __attribute__ ((packed))
	#endif //_WIN32
#endif // #ifndef __MY_PACKTED__

#ifndef MY_LONG64
	#ifdef _WIN32
		typedef __int64	MY_LONG64;
	#else
		typedef long long MY_LONG64;
	#endif //_WIN32
#endif // MY_LONG64


typedef struct tagDVB_PSI_DESCRIPTOR_BASE
{
	BYTE	m_byDescriptorTag;
	BYTE	m_byDescriptorLength;
	BYTE	m_abyData[1];
}__MY_PACKTED__ DVB_PSI_DESCRIPTOR_BASE,*PDVB_PSI_DESCRIPTOR_BASE;

//-----------------------------------------------------
// decoded dvbpsi_descriptor_base
typedef struct tagDVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byDescriptorTag;					// descriptor ID
	struct tagDVBPSI_DECODED_DESCRIPTOR_BASE * m_pNext;	// next descriptor
public:
	// Find descriptor by descriptor tag id, NULL = not found, else the descriptor
	struct tagDVBPSI_DECODED_DESCRIPTOR_BASE * Find( BYTE byDescriptorTag )
	{
		struct tagDVBPSI_DECODED_DESCRIPTOR_BASE * pRetVal = this;
		while( pRetVal )
		{
			if( pRetVal->m_byDescriptorTag == byDescriptorTag )
				break;
			pRetVal = pRetVal->m_pNext;
		}

		return pRetVal;
	}
}__MY_PACKTED__ DVBPSI_DECODED_DESCRIPTOR_BASE,*PDVBPSI_DECODED_DESCRIPTOR_BASE;


/*****************************************************************************
 * dvbpsi_vstream_dr_t 02
 *****************************************************************************/
/*!
 * \struct dvbpsi_vstream_dr_s
 * \brief "video stream" descriptor structure.
 *
 * This structure is used to store a decoded "video stream" descriptor.
 * (ISO/IEC 13818-1 section 2.6.2).
 */
/*!
 * \typedef struct dvbpsi_vstream_dr_s dvbpsi_vstream_dr_t
 * \brief dvbpsi_vstream_dr_t type definition.
 */
typedef struct dvbpsi_vstream_dr_t : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
  BYTE      b_multiple_frame_rate;      /*!< multiple_frame_rate_flag */
  BYTE	    i_frame_rate_code;          /*!< frame_rate_code */
  BYTE      b_mpeg2;                    /*!< MPEG_2_flag */
  BYTE      b_constrained_parameter;    /*!< constrained_parameter_flag */
  BYTE      b_still_picture;            /*!< still_picture_flag */

  /* used if b_mpeg2 is true */
  BYTE	    i_profile_level_indication; /*!< profile_and_level_indication */
  BYTE	    i_chroma_format;            /*!< chroma_format */
  BYTE      b_frame_rate_extension;     /*!< frame_rate_extension_flag */
}__MY_PACKTED__ dvbpsi_vstream_dr_t;

/*****************************************************************************
 * dvbpsi_astream_dr_t 03
 *****************************************************************************/
/*!
 * \struct dvbpsi_astream_dr_s
 * \brief "audio stream" descriptor structure.
 *
 * This structure is used to store a decoded "audio stream" descriptor.
 * (ISO/IEC 13818-1 section 2.6.4).
 */
/*!
 * \typedef struct dvbpsi_astream_dr_s dvbpsi_astream_dr_t
 * \brief dvbpsi_astream_dr_t type definition.
 */
typedef struct dvbpsi_astream_dr_s : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE           b_free_format;              /*!< free_format_flag */
	BYTE		   i_id;                       /*!< ID */
	BYTE		   i_layer;                    /*!< layer */
}__MY_PACKTED__ dvbpsi_astream_dr_t;

/*****************************************************************************
 * dvbpsi_iso639_dr_t 0xA
 *****************************************************************************/
/*!
 * \struct dvbpsi_iso639_dr_s
 * \brief "ISO 639 language" descriptor structure.
 *
 * This structure is used to store a decoded "ISO 639 language"
 * descriptor. (ISO/IEC 13818-1 section 2.6.18).
 */
/*!
 * \typedef struct dvbpsi_iso639_dr_s dvbpsi_iso639_dr_t
 * \brief dvbpsi_iso639_dr_t type definition.
 */
typedef struct dvbpsi_iso639_dr_s : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
  BYTE       i_code_count;           /*!< length of the i_iso_639_code array */
  BYTE       i_audio_type;           /*!< audio_type */
  BYTE       i_iso_639_code[1];    /*!< ISO_639_language_code */
}__MY_PACKTED__ dvbpsi_iso639_dr_t;

///////////////////////////////////////////////////////////
// dvbpsi_CA_r_t 9
typedef struct dvbpsi_ca_dr_s : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
  WORD      i_ca_system_id;         /*!< CA_system_ID */
  WORD      i_ca_pid;               /*!< DVBPSI_DTID_CA */
  BYTE      i_private_length;       /*!< length of the i_private_data
                                             array */
  BYTE      i_private_data[1];    /*!< private_data_byte */
}__MY_PACKTED__ dvbpsi_ca_dr_t;

////////////////////////////////////////////////////////
#define TONGSHI_VIDEO_BRO_FOURCC	0x54535642

#define TSVB_FORMAT_PES		0
#define TSVB_FORMAT_RTP		1

//	DVBPSI_TSVB_IDITENTITY_DESCRIPTOR, 200
typedef struct tagDVBPSI_TSVB_IDITENTITY_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	DWORD	m_dwFourCC;				//	特征字符串，必须等于 TONGSHI_VIDEO_BRO_FOURCC
	WORD	m_wFormatTag;			//	传送的格式，参见TSVB_FORMAT_XXXX
}__MY_PACKTED__ DVBPSI_TSVB_IDITENTITY_DESCRIPTOR;

////////////////////////////////////////////////////////
// DVBPSI_DTID_TSVB_MY_VIDEO, 201
// 必须确定是通视MPEG4播出后，才能使用该描述子
typedef struct tagDVBPSI_TSVB_MY_VIDEO_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	DWORD	m_dwHandlerFourCC;		//	特征字符串
	WORD	m_wWidth;
	WORD	m_wHeight;
	DWORD	m_dwTimeScale;
	DWORD	m_dwRate;
	BYTE	m_byExtensionLength;	//  扩展数据
	BYTE	m_abyExtensionData[1];	//	扩展数据区，可变长数据
}__MY_PACKTED__ DVBPSI_TSVB_MY_VIDEO_DESCRIPTOR;

#define MY_MK_FOURCC( a, b, c, d )	(DWORD)( (((DWORD)a)<<24)|(((DWORD)b)<<16)|(((DWORD)c)<<8)|((DWORD)d) )

#define MY_VIDIO_FOURCC_DIVX	MY_MK_FOURCC( 'D', 'I', 'V', 'X' )
#define MY_VIDIO_FOURCC_divx	MY_MK_FOURCC( 'd', 'i', 'v', 'x' )
#define MY_VIDIO_FOURCC_DX50	MY_MK_FOURCC( 'D', 'X', '5', '0' )
#define MY_VIDIO_FOURCC_XVID	MY_MK_FOURCC( 'X', 'V', 'I', 'D' )
#define MY_VIDIO_FOURCC_xvid	MY_MK_FOURCC( 'x', 'v', 'i', 'd' )
#define MY_VIDIO_FOURCC_XCRD	MY_MK_FOURCC( 'X', 'C', 'R', 'D' )
#define MY_VIDIO_FOURCC_RMP4	MY_MK_FOURCC( 'R', 'M', 'P', '4' )
#define MY_VIDIO_FOURCC_DIV3	MY_MK_FOURCC( 'D', 'I', 'V', '3' )
#define MY_VIDIO_FOURCC_div3	MY_MK_FOURCC( 'd', 'i', 'v', '3' )
#define MY_VIDIO_FOURCC_MP43	MY_MK_FOURCC( 'M', 'P', '4', '3' )
#define MY_VIDIO_FOURCC_mp43	MY_MK_FOURCC( 'm', 'p', '4', '3' )
#define MY_VIDIO_FOURCC_MP42	MY_MK_FOURCC( 'M', 'P', '4', '3' )
#define MY_VIDIO_FOURCC_mp42	MY_MK_FOURCC( 'm', 'p', '4', '3' )

////////////////////////////////////////////////////////
// DVBPSI_DTID_TSVB_MY_AUDIO, 202
// 必须确定是通视MPEG4播出后，才能使用该描述子
typedef struct tagDVBPSI_TSVB_MY_AUDIO_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byStreamID;				//	对应PES中的音频ID，因为会出现多语种
	WORD	m_wAudioFormat;
	BYTE	m_byChannels;
	DWORD	m_dwSamplesPerSecond;
	DWORD	m_dwAverageBytesPerSecond;
	DWORD	m_dwTimeScale;
	DWORD	m_dwRate;
	DWORD	m_dwISOLanguageID;
	WORD	m_wPCM_BitsPerSample;		//	仅类型为PCM时，才会出现该字段
	BYTE	m_byExtensionLength;
	BYTE	m_abyExtensionData[1];
}__MY_PACKTED__ DVBPSI_TSVB_MY_AUDIO_DESCRIPTOR;

#define MY_AUDIO_FORMATID_PCM			1
#define MY_AUDIO_FORMATID_MPEGLAYER3	0x55
#define MY_AUDIO_FORMATID_AC3			0x2000
#define MY_AUDIO_FORMATID_DTS			0x2001

/*****************************************************************************
 * dvbpsi_service_dr_t 0x48
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_dr_s
 * \brief "service" descriptor structure.
 *
 * This structure is used to store a decoded "service"
 * descriptor. (ETSI EN 300 468 section 6.2.30).
 */
/*!
 * \typedef struct dvbpsi_service_dr_s dvbpsi_service_dr_t
 * \brief dvbpsi_service_dr_t type definition.
 */
typedef struct tagDVBPSI_SERVICE_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byServiceType;
	char *  m_pszProviderName;		//	提供商名称，以字符'\0'结尾
	char *  m_pszServiceName;		//	服务名称，以字符'\0'结尾
	char	m_szPSNames[2];			//	变长区域，根据实际的字符长度来计算
}__MY_PACKTED__ DVBPSI_SERVICE_DESCRIPTOR;

/////////////////////////////////////////////////////////////
//	Short Event Descriptor, 0x4D
typedef struct tagDVBPSI_SHORT_EVENT_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	DWORD	m_dwISOLanguageID;		//	使用的语言
	char *	m_pszEventName;			//	事件名称
	char *	m_pszText;				//	正文
	char	m_szDataBuf[2];
}__MY_PACKTED__ DVBPSI_SHORT_EVENT_DESCRIPTOR;

//////////////////////////////////////////////////////////////
/// Extened event descriptor, 0x4E
typedef struct tagDVBPSI_EXTENDED_EVENT_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byDescriptorNumber;
	BYTE	m_byLastDescriptorNumber;
	DWORD	m_dwISOLanguageID;
	char *	m_pszDetail;
	BYTE	m_byCount;
	struct tagITEMDESCRIPTOR
	{
		char * m_pszDescriptor;
		char * m_pszText;
	}__MY_PACKTED__ m_aItems[1];
}__MY_PACKTED__ DVBPSI_EXTENDED_EVENT_DESCRIPTOR;

//////////////////////////////////////////////////////////////
/// Content Descriptor 0x54
typedef struct tagDVBPSI_CONTENT_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byCount;
	struct tagITEM
	{
		BYTE	m_byNibble_Level1;
		BYTE	m_byNibble_Level2;
		BYTE	m_byUserNibble1;
		BYTE	m_byUserNibble2;
	}__MY_PACKTED__ m_aItems[1];
}__MY_PACKTED__ DVBPSI_CONTENT_DESCRIPTOR;


//////////////////////////////////////////////////////////////
/// PARENTAL_RATING, 0x55
typedef struct tagDVBPSI_PARENTAL_RATING_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byCount;
	struct tagITEM
	{
		DWORD	m_dwCountryCode;
		BYTE	m_byRating;
	}
	#ifndef _WIN32
	__attribute__ ((packed))
	#endif //_WIN32
	m_aItems[1];
}__MY_PACKTED__ DVBPSI_PARENTAL_RATING_DESCRIPTOR;

//////////////////////////////////////////////////////////////////////////
/// Teletext, DVBPSI_DTID_TELETEXT
#define DVB_TELETEXT_TYPE_RFU						0
#define DVB_TELETEXT_TYPE_INITIAAL_TELETEXT_PAGE	1
#define DVB_TELETEXT_TYPE_SUBTITLE_PAGE				2
#define DVB_TELETEXT_TYPE_ADDITIONAL_INFO_PAGE		3
#define DVB_TELETEXT_TYPE_PROGRAM_SCHEDULE_PAGE		4
#define DVB_TELETEXT_TYPE_SUBTITLE_FOR_HEARING		5
typedef struct tagDVBPSI_TELETEXT_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byCount;
	struct tagPAGEINFO
	{
		char ISO_639_LangCode[3];
		BYTE m_byTeletextType;
		BYTE m_byMagNum;		// 0 ~ 7;  8 = 0 ；杂志号
		BYTE m_byPageNumBCD;	// 页码
	}__MY_PACKTED__ m_aPages[1];
}__MY_PACKTED__ DVBPSI_TELETEXT_DESCRIPTOR;

//////////////////////////////////////////////////////////////
/// NETWORK NAME Descriptor, 0x40, 0x47
typedef struct tagDVBPSI_NETWORK_NAME_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	char m_szName[1];		//	变长结构
}__MY_PACKTED__ DVBPSI_NETWORK_NAME_DESCRIPTOR, DVBPSI_BOUQUET_NAME_DESCRIPTOR;

//////////////////////////////////////////////////////////////////////////
// DVBPSI_DTID_SERVICE_LIST_DESCRIPTOR 0x41
typedef struct tagDVBPSI_SERVICE_LIST_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE m_byCount;
	struct
	{
		WORD m_wSID;
		BYTE m_byServiceType;
	}__MY_PACKTED__	m_aItems[1];
}__MY_PACKTED__ DVBPSI_SERVICE_LIST_DESCRIPTOR;


///////////////////////////////////////////////////////////////
/// DVBPSI_DTID_CA_IDENTIFIER, 0x53
typedef struct tagDVBPSI_CA_IDENTIFIER_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE	m_byCount;
	WORD	m_awCASystemID[1];		//	变长结构
}__MY_PACKTED__ DVBPSI_CA_IDENTIFIER_DESCRIPTOR;

//////////////////////////////////////////////////////////////////////////
// DVBPSI Unknwon
typedef struct tagDVBPSI_UNDECODED_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE m_byDataLen;
	BYTE m_abyData[1];
}__MY_PACKTED__ DVBPSI_UNDECODED_DESCRIPTOR;

///////////////////////////////////////////////////////////////////////////
// DVBPSI Subtitle, 0x59
typedef struct tagDVBPSI_SUBTITLE_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE m_byCount;
	struct tagITEM
	{
		char ISO_639_LangCode[3];
		BYTE m_byType;
		WORD m_wCompositionPageID;
		WORD m_wAncillaryPageID;
	}__MY_PACKTED__ m_aItems[1];
}__MY_PACKTED__ DVBPSI_SUBTITLE_DESCRIPTOR;

///////////////////////////////////////////////////////////////////////////
// DVBPSI AC-3, 0x6A
#define DVBPSI_AC3_FLAGS_COMPONENT_TYPE_VALID	0x80		// m_byAC3Type valid
#define DVBPSI_AC3_FLAGS_BSID_VALID				0x40		// m_byBSID valid
#define DVBPSI_AC3_FLAGS_MAINID_VALID			0x20		// m_byMAINID valid
#define DVBPSI_AC3_FLAGS_ASVC_VALID				0x10		// m_byASVC valid
typedef struct tagDVBPSI_AC3_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	BYTE m_byFlags;			// see also #define DVBPSI_AC3_FLAGS_xxx
	BYTE m_byAC3Type;
	BYTE m_byBSID;
	BYTE m_byMAINID;
	BYTE m_byASVC;
}__MY_PACKTED__ DVBPSI_AC3_DESCRIPTOR;

//////////////////////////////////////////////////////////////////////////
// CYJ,2010-9-15
// DVBPSI_DTID_LOCAL_TIME_OFFSET
typedef struct tagDVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR : public DVBPSI_DECODED_DESCRIPTOR_BASE
{
	int m_nCount;
	struct
	{
		int  m_nLocalTimeOffsetMin;			// 本地时间偏移，单位：分钟
		time_t m_tTimeOfChange;				// 下次即将开始/结束（夏令时）的时间（包含日期）
		int  m_nNextTimeOffsetMin;			// 下次变化后的时间偏移，单位：分钟
		BYTE m_abyCountryCode[3];
		BYTE m_byRegionID;
		BYTE m_bPolarity;
	}__MY_PACKTED__ m_aLocalTimes[1];
}__MY_PACKTED__ DVBPSI_LOCAL_TIME_OFFSET_DESCRIPTOR;

#pragma pack(pop)

#endif // __DVB_DESCRIPTORS_H_20050111__
