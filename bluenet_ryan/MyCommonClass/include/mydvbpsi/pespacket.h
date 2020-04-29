///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-12
///
///		用途：
///			PES 分组重新构造
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#if !defined(AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_)
#define AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_

#include <myheap.h>
#include <MyArray.h>
#include "tspacket.h"

#pragma pack(push,4)

//-----------------------------------------------
// ISO 标准定义的流类型
typedef enum
{
	DVBPESSI_PROGRAM_STREAM_MAP = 0xBC,
	DVBPESSI_PRIVATE_STREAM_1 = 0xBD,
	DVBPESSI_PADING_STREAM,
	DVBPESSI_PRIVATE_STREAM_2,
	DVBPESSI_AUDIO_13818_11172_START = 0xC0,
	DVBPESSI_AUDIO_13818_11172_END = 0xDF,
	DVBPESSI_VIDEO_13818_11172_START = 0xE0,
	DVBPESSI_VIDEO_13818_11172_END = 0xEF,
	DVBPESSI_ECM_STREAM = 0xF0,
	DVBPESSI_EMM_STREAM,
	DVBPESSI_DSM_CC_STREAM,
	DVBPESSI_IEC_13522_STREAM,
	DVBPESSI_PROGRAM_STREAM_DIRECTORY = 0xFF,
}DVB_PES_STREAM_ID;

//-------------------------------------------------
// Stream Type
typedef enum
{
	DVBPES_STREAM_TYPE_VIDEO_MPEG1 = 1,
	DVBPES_STREAM_TYPE_VIDEO_MPEG2,
	DVBPES_STREAM_TYPE_AUDIO_MPEG1,
	DVBPES_STREAM_TYPE_AUDIO_MPEG2,
	DVBPES_STREAM_TYPE_H222_ISO13818_PRIVATE_SEGMENT,
	DVBPES_STREAM_TYPE_H222_ISO13818_PEC_PACKET,
	DVBPES_STREAM_TYPE_ISOIEC13522_MHEG,
	DVBPES_STREAM_TYPE_H222_ISO13818_1_DSMCC,
	DVBPES_STREAM_TYPE_H222_ISO13818_1_11172_1,

	DVBPES_STREAM_TYPE_AUDIO_MPEG4 = 0xF,
	DVBPES_STREAM_TYPE_VIDEO_MPEG4 = 0x10,
	DVBPES_STREAM_TYPE_VIDEO_H264 = 0x1B,

}DVB_PES_STREAM_TYPE;

//----------------------------------------------
//	根据 PES 流处理方式，为处理方便而定义的处理类型
typedef enum
{
	PES_PROCESS_MODE_AV_STREAM_COMPATIBLE = 0,
	PES_PROCESS_MODE_EMM_EMC_COMPATIBLE,
	PES_PROCESS_MODE_PADING,
}SELFDEFINE_PES_PROCESS_MODE;

#ifndef MY_LONG64
	#ifdef _WIN32
		typedef __int64	MY_LONG64;
	#else
		typedef long long MY_LONG64;
	#endif //_WIN32
#endif // MY_LONG64

#pragma pack(push,1)

typedef struct tagDVB_PES_PACKET_HEADER
{
	BYTE	m_abyStartCodePrefix[3];		// 0x00 00 01
	BYTE	m_byStreamID;
	BYTE	m_abyPacketLength[2];
	BYTE	m_abyData[1];
public:
	//	获取 PES 包大小
	inline WORD	GetPacketLength(){ return (m_abyPacketLength[0]<<8) | m_abyPacketLength[1]; }
	//  根据前3个字节，判断是否为 PES 包
	inline bool	IsPESPacket()
	{ return  m_abyStartCodePrefix[0] == 0 && m_abyStartCodePrefix[1] == 0 && m_abyStartCodePrefix[2] == 1; }
	//  获取PES的处理方式
	SELFDEFINE_PES_PROCESS_MODE GetProcessMode();
	//  获取PES头大小
	int GetHeaderLen();
	//  获取有效数据地址及其长度
	PBYTE GetPayloadData( int & nPayloadDataLen );
	//	获取加扰控制
	BYTE  GetScrambling_Control()
	{
		ASSERT(PES_PROCESS_MODE_AV_STREAM_COMPATIBLE == GetProcessMode() );
		return (( m_abyData[0]>>4)&3 );
	};
}__MY_PACKTED__ DVB_PES_PACKET_HEADER,*PDVB_PES_PACKET_HEADER;

#pragma pack(pop)

class CPCR_TSPacketResponser;

//-----------------------------------------------
//	PES 分组解复用
class CPESPacket :
	public CTSPacketResponser,
	public CMyHeap
{
public:
	bool GetPCR(MY_LONG64 & scr, WORD & wExtension );
	void SetPCR(MY_LONG64 & scr, WORD wExtension );
	bool HandlePCR( PDVB_TS_PACKET pTSPacket );
	bool IsRequireCompletePESPacket() const{ return m_bRequireCompletePacket; }
	// 仅在要求完整输出的情况下，才作CRC检测
	bool DoCRCCheck(bool bDoCheck = true);
	bool IsDoCRCCheck()const {return m_bDoCRCCheck;}
	bool SetRequireCompletePESPacket( bool bComplete = true );
	int GetOutputMethod()const	{ return m_nOutputMethod; }
	int SetOutputMethod( int nMethod );
	CPCR_TSPacketResponser * GetAssociatedPCRResponser();
	CPCR_TSPacketResponser * SetAssociatedPCRResponser( CPCR_TSPacketResponser * pResponser);

	CPESPacket(int nCacheBufSize = 0x10100);
	virtual ~CPESPacket();

	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	//	接收到PES分组，pData,nDataLen 数据，nOffset 数据在整个PES分组的中的偏移，nErrorTimes 该PES发生错误的次数
	virtual void OnPESReceived(PBYTE pData, int nDataLen, int nOffset, int nErrorTimes ) = 0;

	enum{
		RESULT_CONSTRUCT_SUCC = 0,			//	表构造成功
		RESULT_CONSTRUCT_CANCELD,			//	表改变，但没有完整
	};
	enum{
		OUTPUT_METHOD_AS_ELEMENTARY_STREAM = 0,	//	输出元素流
		OUTPUT_METHOD_AS_PES_PACKET,			//	输出PES分组
		OUTPUT_METHOD_AS_PROGRAM_STREAM,		//	以节目流方式输出
	};

	void Dump(FILE*fOutput=NULL);

protected:
	bool	m_bHeaderReceived;				//	表头接收到了
	WORD	m_wErrorCounter;				//	TS packet 发生错误的次数
	BYTE	m_byExpectTSContinuityCounter;	//	上次 TS packet 的continuity counter
	bool	m_bRequireCompletePacket;		//	要求输出完整的PES分组，对于视频，允许输出不完整的分组，后面处理器再拼接
	DWORD	m_dwPESBytesReceived;			//  PES 分组已经接收到的字节数

	int		m_nOutputMethod;				//	输出方式，缺省方式输出为元素流

	BYTE	m_byStreamID;
	WORD	m_wPacketLength;				//	可能为0
	int		m_nPayloadLen;					//	有效数据大小，仅在输出元素流时起作用，且非视频类才有意义

	bool	m_bHasSCR;
	WORD	m_wPS_SCR_Base_0_14;			//	system clock reference
	WORD	m_wPS_SCR_Base_15_29;
	WORD	m_wPS_SCR_Base_30_32;
	WORD	m_wPS_SCR_Extension;			// system clock reference extension
	MY_LONG64	m_PCR;						// Program clock reference

	bool	m_bHasPTS;
	bool	m_bHasDTS;
	bool	m_bHasESMuxRate;
	MY_LONG64	m_PTS;						// 播放时间
	MY_LONG64	m_DTS;						// 解码时间
	DWORD		m_dwProgramMuxRate;

	bool	m_bRandomAccessPoint;			// 是否为随机访问点
	bool	m_bTSPacketDiscontinuity;		// TS 分组不连续

	CPCR_TSPacketResponser	*	m_pPCRResponser;		// 当 PCR_PID = 本PES's PID 时，需要用该对象去驱动其他的PES处理器

	bool m_bDoCRCCheck;						//  CYJ,2006-3-2, 进行CRC检测，缺省为检测
	BYTE m_byLastHasCRCFlags;				//  上次有CRC，所以若这一次没有CRC，则认为PES本身的错误;bit0 最后一帧，bit1上一帧

private:
	PBYTE GetCRC( PBYTE pBuf, int nLen ) const;
	void DecodePESHeader(PBYTE pBuf, int nLen);
	void ConstructPSPackHeader( bool bOutputSystemHeader = false );
	void DecodeTSAdaptionField( PDVB_TS_PACKET pTSPacket );
};


class CPESResponserArray;

//---------------------------------------------------
//	PCR TS 分组解释
class CPCR_TSPacketResponser : public CTSPacketResponser
{
public:
	CPCR_TSPacketResponser();
	virtual ~CPCR_TSPacketResponser();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );

	bool IsValid();
	void RemoveAll();
	void Remove( CPESPacket * pResponser );
	void Add( CPESPacket * pResponser );

private:
	CPESResponserArray * m_paPESResponser;

private:
	int	Find( CPESPacket * pResponser );
};

#pragma pack(pop)

#endif // !defined(AFX_PESPACKET_H__7C4FE69C_BE2C_4FA9_88A2_60CC90418602__INCLUDED_)
