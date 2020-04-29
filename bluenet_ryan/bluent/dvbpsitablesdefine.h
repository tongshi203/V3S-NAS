///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			DVB PSI 表定义
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __DVB_PSI_TABLE_DEFINE_H_20050111__
#define __DVB_PSI_TABLE_DEFINE_H_20050111__

#include "dvbdescriptorsdefine.h"
#include <time.h>

typedef enum
{
	DVBPSI_TBLID_PAT = 0,
	DVBPSI_TBLID_CA,
	DVBPSI_TBLID_PMT,
	DVBPSI_TBLID_DATABROCAST_MPE = 0x3E,		// private data
	DVBPSI_TBLID_NIT = 0x40,
	DVBPSI_TBLID_NIT_OTHER,					// 2010.4.7 Other network
	DVBPSI_TBLID_SDT_ACTUAL = 0x42,
	DVBPSI_TBLID_SDT_OTHER = 0x46,
	DVBPSI_TBLID_BAT = 0x4A,
	DVBPSI_TBLID_EIT_ACTUAL = 0x4E,
	DVBPSI_TBLID_EIT_OTHER,
	DVBPSI_TBLID_EIT_ACTUAL_SCHEDULE_START,
	DVBPSI_TBLID_EIT_ACTUAL_SCHEDULE_END = 0x5F,
	ACTUAL_SCHEDULE_OTHER_SCHEDULE_START,
	ACTUAL_SCHEDULE_OTHER_SCHEDULE_END = 0x6F,
	
	DVBPSI_TBLID_TDT = 0x70,
	DVBPSI_TBLID_TOT = 0x73,

	DVBPSI_TBLID_PROGRAM_LIST_TABLE = 0x80,
	DVBPSI_TBLID_PROGRAM_INTRODUCTION_TABLE,
	DVBPSI_TBLID_PROGRAM_REMOTE_CONTROL_TABLE,
	DVBPSI_TBLID_TONGSHI_VOD_IDENTITY,

	DVBPSI_TBLID_RESERVED = 0xFF
}DVB_PSI_TABLE_ID;

#ifdef _WIN32
  #pragma pack(push,1)
#endif //_WIN32

typedef struct tagDVB_PSI_TABLE_BASE
{
	BYTE	m_byTableID;				//	表ID，参见 DVB_PSI_TABLE_ID
	DWORD	m_dwTableSize;				//	表大小，包括 DVB_PSI_TABLE_BASE 结构
}__MY_PACKTED__ DVB_PSI_TABLE_BASE,*PDVB_PSI_TABLE_BASE;

//--------------------------------------------
//	PAT 表, Table ID = 0 = DVBPSI_TBLID_PAT
typedef struct tagDVB_PSI_TABLE_PAT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wTSID;					// transport stream id
	BYTE	m_byVersionNumber;			// 0～31之间循环使用,每次改变PAT，都将＋1
	BYTE	m_bCurrentNextIndicator;	// 
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;		//	
	WORD	m_wCount;					//	后面解码的个数
	struct
	{
		WORD	m_wSID;					// SID
		WORD	m_wPMT_PID;				// 对于 SID=0,则解释为  Network PID
	}m_aPrograms[1];
public:
	//	get the PID of one SID, 0x1FFF is not found
	WORD GetPID( WORD wSID )
	{
		for( int i=0; i<m_wCount; i++)
		{
			if( m_aPrograms[i].m_wSID == wSID )
				return m_aPrograms[i].m_wPMT_PID;
		}
		return 0x1FFF;		//	not found
	}
	int FindPMT_PID(WORD wSID)
	{
		for( int i=0; i<m_wCount; i++)
		{
			if( m_aPrograms[i].m_wSID == wSID )
				return i;
		}
		return -1;
	}
}__MY_PACKTED__ DVB_PSI_TABLE_PAT,*PDVB_PSI_TABLE_PAT;

//--------------------------------------------
//	CAT 表, Table ID = 1 = DVBPSI_TBLID_PAT
typedef struct tagDVB_PSI_TABLE_CAT : public tagDVB_PSI_TABLE_BASE
{
	BYTE	m_byVersionNumber;			// 0～31之间循环使用,每次改变PAT，都将＋1
	BYTE	m_bCurrentNextIndicator;	// 
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;		//	
	WORD	m_wCount;					//	后面解码的个数
	PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
}__MY_PACKTED__ DVB_PSI_TABLE_CAT,*PDVB_PSI_TABLE_CAT;


//---------------------------------------------
//	PMT 表，Table ID = 2 = DVBPSI_TBLID_PMT
typedef struct tagDVB_PSI_TABLE_PMT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wSID;
	BYTE	m_byVersionNumber;			// 0～31之间循环使用,每次改变PAT，都将＋1
	BYTE	m_bCurrentNextIndicator;	// 
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;		//	
	WORD	m_wPCR_PID;
	WORD	m_wCount;					//	Program descriptor count
	PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;	// descriptor list, might be NULL
	WORD	m_wStreamCount;				//	节目流个数
	struct tagSTREAMINFO
	{
		WORD	m_wES_PID;				//	PID
		BYTE	m_byStreamType;			//	stream type, 参见：DVB_STREAM_TYPE_ENUM		
		BYTE	m_byCount;				//	stream descriptor count
		PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	}__MY_PACKTED__ m_aStreams[1];
public:
	//	find stream by the stream type, -1 = not found, else array index
	int FindStream( BYTE byStreamType )
	{
		for(int i=0; i<m_wStreamCount; i++)
		{
			if( m_aStreams[i].m_byStreamType == byStreamType )
				return i;
		}
		return -1;
	}
}__MY_PACKTED__ DVB_PSI_TABLE_PMT, *PDVB_PSI_TABLE_PMT;

//----------------------------------------------------------
// SDT 表
typedef struct tagDVB_PSI_TABLE_SDT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wTSID;						//	传输流ID
	BYTE	m_byVersionNumber;				//	版本 0 ～ 31
	BYTE	m_bCurrentNextIndicator;		//	当前是否有效
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;
	WORD	m_wOriginalNetworkID;
	WORD	m_wCount;						//	节目个数
	struct tagSERVICELIST
	{
		WORD	m_wSID;						//	节目ID，也称作 ProgramNumber
		BYTE	m_bEITScheduleFlag;
		BYTE	m_bEITPresentFollowingFlag;
		BYTE	m_byRunningStatus;
		BYTE	m_bFreeCAMode;
		BYTE	m_byCount;					//	program descriptor count
		PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	}__MY_PACKTED__ m_aPrograms[1];
public:
	//	find program by SID, return -1 = not found, else array index
	int FindProgram( WORD wSID )
	{
		for(int i=0; i<m_wCount; i++)
		{
			if( wSID == m_aPrograms[i].m_wSID )
				return i;
		}
		return -1;
	}
}__MY_PACKTED__ DVB_PSI_TABLE_SDT, *PDVB_PSI_TABLE_SDT;

//////////////////////////////////////////////////////////////////////////
// BAT
typedef struct tagDVB_PSI_TABLE_BAT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wBouquetID;
	BYTE	m_byVersionNumber;				//	版本 0 ～ 31	
	BYTE	m_bCurrentNextIndicator;		//	当前是否有效
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;
	WORD	m_wCount;						// 总体描述个数
	PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;	// descriptor list, might be NULL
	
	WORD m_wItemCount;
	struct tagONE_BAT_ITEM
	{
		WORD	m_TransportStreamID;
		WORD	m_wOriginalNetworkID;
		WORD	m_wCount;					//	program descriptor count
		PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	}__MY_PACKTED__ m_aBATs[1];	
}__MY_PACKTED__ DVB_PSI_TABLE_BAT, *PDVB_PSI_TABLE_BAT;

//////////////////////////////////////////////////////////////////////////
///	EIT
#define DVB_EIT_RUNSTATUS_UNDEFINED					0
#define DVB_EIT_RUNSTATUS_NOT_RUN					1
#define DVB_EIT_RUNSTATUS_START_IN_A_FEW_SECONDS	2
#define DVB_EIT_RUNSTATUS_PAUSING					3
#define DVB_EIT_RUNSTATUS_RUNNING					4
typedef struct tagDVB_PSI_TABLE_EIT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wSID;							//	对应的节目ID
	BYTE	m_byVersionNumber;
	BYTE	m_bCurrentNextIndicator;
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;
	WORD	m_wTSID;
	WORD	m_wOriginalNetworkID;
	BYTE	m_bySegmentLastSectionNumber;
	BYTE	m_byLastTableID;
	WORD	m_wCount;
	struct  tagEVENTMSG
	{
		WORD		m_wEventID;
		time_t		m_tStartTimeUTC;		// Universal Coordinated Time 
		DWORD		m_dwDuration;			// in seconds
		BYTE		m_byRunningStatus;		// please see also DVB_EIT_RUNSTATUS_XXX
		BYTE		m_bFreeCAMode;
		BYTE		m_byCount;
		PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	}__MY_PACKTED__ m_aEvents[1];
public:
	//	find Event by the EventID, -1 = not found, else array index
	int FindEvent( WORD wEventID )
	{
		for(int i=0; i<m_wCount; i++ )
		{
			if( m_aEvents[i].m_wEventID == wEventID )
				return i;
		}
		return -1;
	}	
}__MY_PACKTED__ DVB_PSI_TABLE_EIT, *PDVB_PSI_TABLE_EIT;

//////////////////////////////////////////////////////////////////
//	NIT
typedef struct tagDVB_PSI_TABLE_NIT : public tagDVB_PSI_TABLE_BASE
{
	WORD	m_wNetworkID;
	BYTE	m_byVersionNumber;
	BYTE	m_bCurrentNextIndicator;
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;
	WORD	m_wNetworkDrCount;
	PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	WORD	m_wItemCount;
	struct tagITEM
	{
		WORD	m_wTSID;
		WORD	m_wOriginalNetworkID;
		BYTE	m_byCount;
		PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
	}__MY_PACKTED__ m_aItems[1];
}__MY_PACKTED__ DVB_PSI_TABLE_NIT, *PDVB_PSI_TABLE_NIT;

//////////////////////////////////////////////////////////////////////////
// TOT
typedef struct tagDVBPSI_TABLE_TOT : public tagDVB_PSI_TABLE_BASE
{
	time_t	m_tUTCTime;
	PDVBPSI_DECODED_DESCRIPTOR_BASE m_pDescriptor;
}__MY_PACKTED__ DVBPSI_TABLE_TOT, *PDVBPSI_TABLE_TOT;


#ifdef _WIN32
  #pragma pack(pop)
#endif //_WIN32


#endif // __DVB_PSI_TABLE_DEFINE_H_20050111__

