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

// dvbpsiscanprogramhelper.h: interface for the dvbpsiscanprogramhelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBPSISCANPROGRAMHELPER_H__4FCF36BD_7609_4B7F_B0D0_13F6B6F6A218__INCLUDED_)
#define AFX_DVBPSISCANPROGRAMHELPER_H__4FCF36BD_7609_4B7F_B0D0_13F6B6F6A218__INCLUDED_

#include <MyArray.h>
#include "dvbpsitables.h"
#include "tspacketdemux.h"

#ifndef _WIN32
	#include <MyString.h>
	#define CString CMyString
#endif //_WIN32

#pragma pack(push,1)

// support Max 4 language descriptor
#define ONE_DVB_PROGRAM_ES_MAX_LANGUAGE_COUNT	4
#define ONE_DVB_PROGRAM_MAX_CAID_NUMBER	16
// CYJ, 2010.3.5 Add
// Only for Audio, Subtitle, Teletext
typedef struct tagONE_DVB_PROGRAM_ES_PARAM
{
	BYTE m_byTagID;					// 0xA for Audio, 0x59 for subtitle, 0x56 for teletext
									// 0x6A for AC-3, and only indicates it's AC-3, other fields are meaningless
	char m_szISOLanguageCode[4];	// 3 characters ISO language + '\0'
	union
	{
		BYTE m_byAudioType;			// audio	
		struct 
		{
			BYTE m_byTeleTextType;
			BYTE m_byMagNo;	
			BYTE m_byPageNoBCD;
		}m_Teletext;				// teletext
		struct
		{
			BYTE m_bySubtitleType;
			WORD m_wCompositionPageID;
			WORD m_wAncillaryPageID;
		}m_SubTitle;				// subtitle
		struct
		{
			BYTE m_byFlags;			// see also #define DVBPSI_AC3_FLAGS_xxx
			BYTE m_byAC3Type;
			BYTE m_byBSID;
			BYTE m_byMAINID;
			BYTE m_byASVC;
		}m_AC3;
	};
}ONE_DVB_PROGRAM_ES_PARAM,*PONE_DVB_PROGRAM_ES_PARAM;

typedef struct tagONE_DVB_PROGRAM_PARAM
{								// 共 30 字节
	// PAT
	WORD	m_wSID;				// 节目号
	WORD	m_wPMT_PID;			// PMT 对应的 PID
	WORD	m_wPMTVersion;		// 2010.8.26 CYJ Add

	WORD	m_wTransportStreamID;
	WORD	m_wNetworkID;	
	
	// PMT
	WORD	m_wPCR_PID;
	WORD	m_wESCount;			// ES 个数
	struct 		
	{
		WORD m_wES_PID;
		BYTE m_byESType;
		BYTE m_byLanguageDescriptorCount;
		ONE_DVB_PROGRAM_ES_PARAM m_aESDescriptors[ONE_DVB_PROGRAM_ES_MAX_LANGUAGE_COUNT];	// CYJ, 2010.3.5 Add
	}m_aES_PID[64];				// CYJ,2010.3.5 Modify, 16=>64; 目前设计，最多不超过64个

	BYTE 	m_byCAIDCount;		// 2011.2.12, CA ID Count, can be used as m_byCADescriptorFound
	WORD	m_awCAIDs[ ONE_DVB_PROGRAM_MAX_CAID_NUMBER ];		// Max CAID Count

	// SDT
	BYTE	m_byServiceType;	// 由SDT提供的服务类型
	BYTE	m_byEIT_Schedule;
	BYTE	m_byEIT_Present_Following;
	BYTE	m_byRuningStatus;
	BYTE	m_byFreeCAMode;
	CString m_strProviderName;	// 提供商名称
	CString m_strServiceName;	// 服务名称（台名）
}ONE_DVB_PROGRAM_PARAM,*PONE_DVB_PROGRAM_PARAM;

#pragma pack(pop)

#pragma pack(push,4)

//-----------------------------------------------
class CDVBPSI_MultiPMT_Receiver : public CTSPacketResponser
{
public:
	CDVBPSI_MultiPMT_Receiver();
	~CDVBPSI_MultiPMT_Receiver();

	virtual void Reset();
	virtual void OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable ) = 0;
	bool AddSID( WORD wSID, WORD wPMTPID );
	void Remove( WORD wSID );

	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );	//	接收到一个TS分组

	class CMyPMT : public CDVBPSITable_PMT
	{
	public:
		CMyPMT( CDVBPSI_MultiPMT_Receiver * pResponser );
		virtual ~CMyPMT();
		virtual void OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable );
		CDVBPSI_MultiPMT_Receiver * m_pResponser;
		WORD m_wPMT_PID;
	};

protected:
	CMyArray<CMyPMT*> m_apPMTs;
};

//------------------------------------------------
class CDVBPSIScanProgramHelper : 
	public CDVBPSITable_PAT,
	public CDVBPSITable_SDT_BAT,
	public CDVBPSI_MultiPMT_Receiver
{
public:
	CDVBPSIScanProgramHelper(int nTimeOutSecond=10);
	virtual ~CDVBPSIScanProgramHelper();	

	virtual void OnProgramListReceived( PONE_DVB_PROGRAM_PARAM paProgram, int nCount ) = 0;
	virtual void OnEnablePID( WORD wPID ) = 0;

	// 2010.11.24 CYJ Add, Disable/Close One PID
	// 说明：由于本系统采用的时回调函数的方式，所以不能在OnDisablePID函数内删除TSInput对象
	// 而应该先记录该删除到PID，然后在 TSInput 之 PumpTSPacket 函数调用返回后再执行相应的操作。
	virtual void OnDisablePID( WORD wPID ) = 0;

	bool IsValid();
	void Initialize( CTSPacketDemux * pTSDemux );
	void Reset();

protected:
	virtual void OnPATReceived( const PDVB_PSI_TABLE_PAT pTable );
	virtual void OnSDTReceived( PDVB_PSI_TABLE_SDT pSDT );
	virtual void OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable );
	
protected:
	CMyArray<ONE_DVB_PROGRAM_PARAM>	m_aProgram;

private:	
	DWORD	m_dwPAT_TSID;
	time_t	m_nStartTime;				// 从接收到PAT开始计算超时时间
	int		m_nTimeOutSecond;			// 超时时间
	CDVBSectionReceivingLog	m_SDT_SectionRecStatus;	// SDT 接收状态
	CTSPacketDemux * m_pTSDemux;

protected:
	PONE_DVB_PROGRAM_PARAM FindProgram( WORD wSID, bool bInsertNewOne=false );
	bool IsSDT_PMTFullReceived();
	void TryToFind_ES6_Language( PONE_DVB_PROGRAM_PARAM pProgram, int nIndex, const PDVB_PSI_TABLE_PMT pTable );
	void TryToFind_Audio_Language( PONE_DVB_PROGRAM_PARAM pProgram, int nIndex, const PDVB_PSI_TABLE_PMT pTable );
	bool IsThisPMTPIDCanBeDisabled( WORD wPMT );
	void GetCAIDs( PONE_DVB_PROGRAM_PARAM pProgram, PDVBPSI_DECODED_DESCRIPTOR_BASE pDescriptor );
};

#pragma pack(pop)

#endif // !defined(AFX_DVBPSISCANPROGRAMHELPER_H__4FCF36BD_7609_4B7F_B0D0_13F6B6F6A218__INCLUDED_)
