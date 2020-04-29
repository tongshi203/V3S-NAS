///=======================================================
///    
///     作者: 王国雄
///    
///     niniryuhappy@gmail.com
///    
///     日期: 2006-4-17
///     文件: OneProgram_EITGenerator.h
///     版本: 
///     说明: 
///    
///========================================================
// OneProgram_EITGenerator.h: interface for the COneProgram_EITGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_)
#define AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_

#include <MyString.h>
#include <MyArray.h>
#include "bitstream.h"
#include "psitablegenerator.h"

#pragma pack(push,4)
//////////////////////////////////////////////////////////////////////////
// COneProgram_EITGenerator ：生成某一逻辑频道的EIT
//////////////////////////////////////////////////////////////////////////
class COneProgram_EITGenerator  
{
public:
	
	//EIT_BUFFER存放生成的EIT Section
	typedef struct tagEIT_Buffer
	{
		BYTE m_abyEITBuf[4000];
		WORD m_wBufSize;
	}EIT_BUFFER,*PEIT_BUFFER;

	//事件描述
	typedef struct tagOne_Event_Item
	{
		WORD		m_wEventID;
		time_t		m_tStartUTCTime;		//Universal Coordinated time
		DWORD		m_dwDuration;			// in seconds
		BYTE		m_byRunningStatus;		// 0-undefine,1-not running,2-start in a few second,3-pausing,4-running
		BYTE		m_bFreeCAMode;			//0-scrambled, 1-not scrambled
		BYTE		m_aISOLanguage[3];		// "chi"-Chinese, "eng"-English
		CMyString   m_strEventName;			//event name
		CMyString	m_strShortDescription;	//short event description, event_name+ short_event_description <= 250 bytes
		CMyString	m_strDetailDescription;	//detail event description , <= 2000bytes,
											//at the most 240 bytes in one extend descriptor, so the max count of descriptor is 9 	
	}ONE_EVENT_ITEM, *PONE_EVENT_ITEM;

	//某一逻辑频道EIT基本设置
	typedef struct tagOne_ProgramEIT_Base
	{
		WORD m_wSID;
		WORD m_wTSID;
		WORD m_wOriginalNetworkID;
		BYTE m_bCurrentNextIndicator;
		BYTE m_byEITScheduleVersion;
		BYTE m_byEITPFVersion;
	}ONE_PROGRAMEIT_BASE,*PONE_PROGRAMEIT_BASE;
	
	enum{
		ENABLE_SID  = 0x1,
		ENABLE_TSID = 0x02,
		ENABLE_CURRENT_NEXT_INDICATOR = 0x04,
		ENABLE_ORIGINAL_NETWORK_ID = 0x08,
		ENABLE_SCHEDULE_VERSION = 0x10,
		ENABLE_PF_VERSION = 0x20,
		ENABLE_ALL_FLAG = 0xFF
	};
	
	//EIT Segment:每三个小时事件目组成一个Segment（最多包含8个section）
	typedef struct tagOne_Segment_Item
	{
		WORD m_wFirstEventIndex;   //起始事件在m_aScheduleEvent中的索引
		WORD m_wEventCount;        //该segment中包含的事件个数
		WORD m_wFirstSectionIndex; //起始section在m_aScheduleSection中的索引
		WORD m_wSectionCount;      //section个数
	}ONE_SEGMENT_ITEM,*PONE_SEGMENT_ITEM;

	//EIT section 配置
	typedef struct tagOne_Section_Cfg
	{
		BYTE  m_byTableID;          //EIT Table ID
		BYTE  m_byLastTableID;      //该逻辑频道事件计划所使用的最后一个EIT Table ID
		BYTE  m_bySection_Num;     //该逻辑频道的当前Table中最后一个Section序号
		BYTE  m_byLastSection_Num; //该逻辑频道的当前Table中最后一个Section序号
		BYTE  m_bySegment_LastSection_Num; //所属segment中最后一个section序号
	}ONE_SECTION_CFG, *PONE_SECTION_CFG;

public:	
	COneProgram_EITGenerator();
	virtual ~COneProgram_EITGenerator();
	void Preset();
public:
	void RemovePFEvent();
	void SetCurrentDate(time_t  tUTCmidnight);
	void RemoveAllSchedule(); 
	
	long Release();
	long AddRef();
	void RemoveEvent(WORD wEventID);

	int AddScheduleEvent(ONE_EVENT_ITEM& OneEvent);	
	void SetBaseCfg(ONE_PROGRAMEIT_BASE& Basecf, DWORD dwFlags = 0xffffffff);
	void SetFollowingEvent(ONE_EVENT_ITEM &OneEvent);
	void SetPresentEvent(ONE_EVENT_ITEM& OneEvent);
	void Build();

	CMyArray<EIT_BUFFER>	  m_aScheduleEITBuf;
	CMyArray<EIT_BUFFER>	  m_aPFEITBuf;
	//该逻辑频道EIT基本设置
	ONE_PROGRAMEIT_BASE m_BaseCfg;
protected:
	long m_nRefCount;
	int FindOneEvent(WORD wEventID);
	WORD SplitSegmentEdge();
	void SplitScheduleSectionEdge(ONE_SEGMENT_ITEM& segmentItem);
	
	void BuildScheduleTable();
	void BuildScheduleOneSection(ONE_SECTION_CFG& SecCfg, int nNo);
	void BuildPFEITSection();
	WORD BuildOneEvent(CMyBitStream* pStream,ONE_EVENT_ITEM& pEvent);
	WORD BuildExtendDescriptor(CMyBitStream *pStream, CMyString& strDescriptor, DWORD dwISOlanguage);
	WORD BuildShortDescriptor(CMyBitStream *pStream, CMyString strEventName,CMyString strDescriptor, DWORD dwISOlanguage);
	void UTCTimetoMJD(time_t UTCtime, PBYTE pBuf);
	
	CMyArray<ONE_SEGMENT_ITEM> m_aScheduleSegment;//
	CMyArray<DWORD>			   m_aScheduleSection;// higher 16 bit : event count, lower 16 bit :start event index;
	CMyArray<ONE_EVENT_ITEM>   m_aScheduleEvent;//按照时间顺序排列(从当天UTC午夜开始)，eventid连续递增
	
	ONE_EVENT_ITEM		m_aPFEventBuffer[2];	//  CYJ,2006-12-31 用于当前/下一个事件缓冲区，这样不用老申请/释放内存	
	//当前事件
	PONE_EVENT_ITEM		m_pPresentEvent;   
	//下一个事件
	PONE_EVENT_ITEM     m_pFollowingEvent;
	

	//ScheduleEIT  起始时间, 一般为UTC零点
	time_t	m_tScheduleStartTime;
	bool	m_bEventPFModified;
	bool	m_bEventScheduleModified;
};

//////////////////////////////////////////////////////////////////////////
// CEITGenerator : 整合逻辑频道的EIT，并将其封装成TS包
//////////////////////////////////////////////////////////////////////////
class CDVBPSI_EITGenerator : public CDVBPSI_TableGeneratorBase
{
public:
	enum
	{
		BUILD_FLAG_PF_SECTION = 1,
		BUILD_FLAG_SCHEDULE = 2,

		BUILD_FLAG_ALL = 0xFFFFFFFF,
	};

	void Build( DWORD dwBuildMask = BUILD_FLAG_ALL );
	void RemoveAll();
	void DeregisterProgramEIT(WORD wSID);
	void RegisterOneProgramEIT(WORD wSID, COneProgram_EITGenerator* pProgramEIT);
	virtual void EncapsulateOneSection(PBYTE pBuf, int nLen );
	CDVBPSI_EITGenerator();
	virtual ~CDVBPSI_EITGenerator();
	
protected:
	CMyMap<WORD, WORD, COneProgram_EITGenerator*, COneProgram_EITGenerator*> m_MapSID_ProgarmEIT;
};

#pragma pack(pop)

#endif // !defined(AFX_ONEPROGRAM_EITGENERATOR_H__D90BA713_5534_4D88_B2D0_45BFE2A8E63C__INCLUDED_)
