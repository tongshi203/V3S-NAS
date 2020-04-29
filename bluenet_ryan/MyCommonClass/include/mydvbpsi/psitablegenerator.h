///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-2-8
///
///		用途：
///			DVB PSI 表生成器
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#if !defined(AFX_PATGENERATOR_H__7A6208DA_5C4A_4074_B7E5_D6C007201FB0__INCLUDED_)
#define AFX_PATGENERATOR_H__7A6208DA_5C4A_4074_B7E5_D6C007201FB0__INCLUDED_

#include <MyArray.h>
#include <MyMap.h>
#include <MyString.h>
#include "tspacketencapsulator.h"
#include <time.h>

#pragma pack(push,4)

class CMyBitStream;

class CDVBPSI_TableGeneratorBase : public CDVBPacketEncapsulatorBase
{
public:
	int Encapsulate( PBYTE pBuf, int nLen );
};

class CDVBPSI_PATGenerator : public CDVBPSI_TableGeneratorBase
{
public:
	CDVBPSI_PATGenerator();
	virtual ~CDVBPSI_PATGenerator();	
	void Preset();
	WORD SetTSID(WORD wTSID);
	BYTE SetSectionNumber( BYTE bySectionNumber );
	BYTE SetLastSectionNumber( BYTE bySectionNumber );
	BYTE GetVersionNumber() const;
	BYTE SetVersionNumber( BYTE byNumber );
	WORD SetNetworkID( WORD wNetworkID );
	void SetSID_PMT( WORD wSID, WORD wPMTPID );
	void RemoveSID_PMT( WORD wSID );
	void RemoveAllSID_PMT();
	void Build( bool bCurrentValid = true );

	enum{
		PAT_BUFFER_SIZE = 1024,
	};
private:
	BYTE	m_abyPATHeader[PAT_BUFFER_SIZE];
	WORD	m_wNetworkID;
	WORD	m_wSectionLen;
	bool	m_bModified;
	CMyMap<WORD,WORD,WORD,WORD>	m_mapSID_PID;
};

//////////////////////////////////////////////////////////////////////////
#ifndef __MY_SWAP_BYTE_ORDER__
#define __MY_SWAP_BYTE_ORDER__
#define SWAP_DWORD( a )		( DWORD((a&0xFF000000)>>24)|DWORD((a&0xFF0000)>>8)|DWORD((a&0xFF00)<<8)|DWORD((a&0xFF)<<24 ) )
#define SWAP_THREE_BYTE( a )	( DWORD((a&0xFFFF00)>>16) | DWORD(a&0xFF00) ) | DWORD((a&0xFF)<<16)
#define SWAP_WORD( a )		( WORD((a&0xFF00)>>8) | WORD((a&0xFF)<<8) )

#endif // __MY_SWAP_BYTE_ORDER__

#pragma pack(push,1)
typedef struct tagTSMPEG4_VIDEO_PARAM
{
	char	m_abyCompressor[4];	//	特征字符串
	WORD	m_wWidth;
	WORD	m_wHeight;
	DWORD	m_dwTimeScale;
	DWORD	m_dwRate;
	BYTE	m_byExtersionLen;
public:
	void SwapByteOrder()
	{
		m_wWidth = SWAP_WORD(m_wWidth);
		m_wHeight = SWAP_WORD( m_wHeight );
		m_dwTimeScale = SWAP_DWORD( m_dwTimeScale );
		m_dwRate = SWAP_DWORD( m_dwRate );
	}
}TSMPEG4_VIDEO_PARAM,*PTSMPEG4_VIDEO_PARAM;

typedef struct tagTSMPEG4_AUDIO_PARAM
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
	BYTE	m_byExtersionLen;
public:
	void SwapByteOrder()
	{
		m_wAudioFormat = SWAP_WORD( m_wAudioFormat );
		m_dwSamplesPerSecond = SWAP_DWORD( m_dwSamplesPerSecond );
		m_dwAverageBytesPerSecond = SWAP_DWORD( m_dwAverageBytesPerSecond );
		m_dwTimeScale = SWAP_DWORD( m_dwTimeScale );
		m_dwRate = SWAP_DWORD( m_dwRate );
		m_dwISOLanguageID = SWAP_THREE_BYTE(m_dwISOLanguageID);
		m_wPCM_BitsPerSample = SWAP_WORD(m_wPCM_BitsPerSample);		//	仅类型为PCM时，才会出现该字段
	}
}TSMPEG4_AUDIO_PARAM,*PTSMPEG4_AUDIO_PARAM;

#pragma pack(pop)

class CDVBPSI_PMTGenerator : public CDVBPSI_TableGeneratorBase
{
public:
	CDVBPSI_PMTGenerator();
	virtual ~CDVBPSI_PMTGenerator();

	enum{
		PMT_BUFFER_SIZE = 1024,
	};

	typedef struct tagONE_STREAM_INFO
	{
		BYTE	m_byStreamID;
		WORD	m_wPID;
		CMyArray<BYTE>	m_abyDescriptor;
	public:
		struct tagONE_STREAM_INFO & operator=( struct tagONE_STREAM_INFO & src)
		{
			m_byStreamID = src.m_byStreamID;
			m_wPID = src.m_wPID;
			m_abyDescriptor.Copy( src.m_abyDescriptor);
			return *this;
		};
	}ONE_STREAM_INFO,*PONE_STREAM_INFO;
public:
	void CreateStream( BYTE byStreamID, WORD wPID, PBYTE pDescriptor, int nLen );
	void CreateProgramInfo(PBYTE pDescriptor, int nLen);
	void CreateTongshiMPEG4Bro(WORD wPID, PTSMPEG4_VIDEO_PARAM pVideo, PTSMPEG4_AUDIO_PARAM pAudio);
	void DeleteStream( BYTE byStreamID );
	void RemoveAllStream();
	void RemoveProgramInfo();
	void Preset();
	BYTE SetSectionNumber( BYTE bySectionNumber );
	BYTE SetLastSectionNumber( BYTE bySectionNumber );
	BYTE GetVersionNumber() const;
	BYTE SetVersionNumber( BYTE byNumber );
	WORD SetSID(WORD wSID );
	WORD GetSID() const;
	WORD SetPCR_PID( WORD wPID );
	WORD GetPCR_PID()const;	

	void Build( bool bCurrentValid = true );


private:
	CMyArray<ONE_STREAM_INFO> m_aStreams;
	CMyArray<BYTE> m_ProgramInfo;
	BYTE	m_abyPMTBuf[PMT_BUFFER_SIZE];
	WORD	m_wSectionLen;
	bool	m_bModified;

private:
	int FindStream( BYTE byStreamID );
};

///////////////////////////////////////////////////////////////////////
/// SDT, Service description table
class CDVBPSI_SDTGenerator : public CDVBPSI_TableGeneratorBase
{
public:
	CDVBPSI_SDTGenerator();
	virtual ~CDVBPSI_SDTGenerator();	

	typedef struct tagSDT_BASE_CFG
	{
		WORD	m_wTSID;						//	传输流ID
		BYTE	m_byVersionNumber;				//	版本 0 ～ 31
		BYTE	m_bCurrentNextIndicator;		//	当前是否有效		
		WORD	m_wOriginalNetworkID;
	}SDT_BASE_CFG,*PSDT_BASE_CFG;

	enum{
		ENABLE_TSID = 1,
		ENABLE_VERSION = 2,
		ENABLE_CURRENT_NEXT_INDICATOR = 4,
		ENABLE_ORIGINAL_NETWORK_ID = 8,

		ENABLE_ALL_FLAG = 0xFF
	};

	typedef struct tagONE_SDT_ITEM
	{
		WORD	m_wSID;						//	节目ID，也称作 ProgramNumber
		BYTE	m_bEITScheduleFlag;			// 为 1
		BYTE	m_bEITPresentFollowingFlag;	// 为 1
		BYTE	m_byRunningStatus;			// 0-undfine,1-not running,2-start in a few second,3-pausing,4-running
		BYTE	m_bFreeCAMode;				// 为 0 表示加密节目，1 为免费节目
		CMyString m_strProviderName;			// 提供商名称，应小于250字节
		CMyString	m_strName;					// 节目名称，应小于250字节
	}ONE_SDT_ITEM,*PONE_SDT_ITEM;

	typedef struct tagSDT_BUFFER
	{
		BYTE	m_abyBuf[1000];
		WORD	m_wBufSize;
	}SDT_BUFFER,*PSDT_BUFFER;

public:
	void RemoveAll();
	void Remove( WORD wSID );
	void Add( ONE_SDT_ITEM & Program );
	void SetBaseCfg( SDT_BASE_CFG & Cfg, DWORD dwFlags );
	void Build();
	void Preset();

private:
	int FindProgram( WORD wSID );
	void BuildOneProgram( CMyBitStream * pOutBS, int nNo );
	void BuildOneSection( int nIndex );
	int  SplitSectionEdge();
	
private:
	CMyArray<ONE_SDT_ITEM>	m_aPrograms;
	CMyArray<SDT_BUFFER>	m_aSDTBuf;
	CMyArray< DWORD > m_aSectionEdge;	//	用于记录各个项的起止边界条件，高16位为个数，低16位为起始序号
	SDT_BASE_CFG	m_BaseCfg;
	bool			m_bModified;	
	BYTE	m_bySectionNumber;
	BYTE	m_byLastSectionNumber;
};

#pragma pack(pop)

#endif // !defined(AFX_PATGENERATOR_H__7A6208DA_5C4A_4074_B7E5_D6C007201FB0__INCLUDED_)
