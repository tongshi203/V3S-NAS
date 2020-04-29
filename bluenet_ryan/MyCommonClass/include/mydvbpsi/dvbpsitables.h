///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			DVB PSI Tables 解析
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#if !defined(AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_)
#define AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_

#include <myheap.h>
#include "tspacket.h"
#include "dvbpsitablesdefine.h"


#pragma pack(push,4)
#define INVALID_ID	0xFFFF

class CDVBSectionReceivingLog
{
public:
	CDVBSectionReceivingLog();
	~CDVBSectionReceivingLog();

	void ClearSectionLog();					// 清除section记录状态
	void SetSectionNoStatus(int nSectionNo, bool bValue=true);	// 设置接收状态
	bool IsSectionReceived( int nSectionNo );	// 判断释放接收到
	bool IsAllSectionReceived();			// 判断全部接收
	void SetSectionCount(int nSectionCount);	// 设置section个数

	void Reset();
	int GetSectionCount()const{return m_nSectionCount;}

protected:
	DWORD	m_adwSectionRecLog[8];			// 8*4*8 = 256，用于记录Section是否接收到
	int		m_nSectionCount;				// section 个数，最大256
};

class CDVBPSITablesBase :
	public CTSPacketResponser,
	public CMyHeap,
	public CDVBSectionReceivingLog
{
public:
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	CDVBPSITablesBase(int nMaxTableSize=0);
	virtual ~CDVBPSITablesBase();

	enum{
		RESULT_CONSTRUCT_SUCC = 0,			//	表构造成功
		RESULT_CONSTRUCT_CANCELD,			//	表改变，但没有完整
		RESULT_TABLE_NOT_EXIST,				//	表不存在
	};

	virtual void OnTableReceived() = 0;			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived() = 0;		//	读取上次接收到的表
	virtual bool IsPSIPacketIntegral()=0;		//	判断一个分组是否接收完全

	virtual void Dump(FILE*fOutput=NULL) = 0;

protected:
	bool	m_bHeaderReceived;				//	表头接收到了
	WORD	m_wErrorCounter;				//	TS packet 发生错误的次数
	BYTE	m_byExpectTSContinuityCounter;	//	上次 TS packet 的continuity counter
};

//------------------------------------------------
//	PAT 表
class CDVBPSITable_PAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_PAT();
	virtual ~CDVBPSITable_PAT();
	enum{
		PAT_TABLE_MAX_SIZE = 1024+256,
		PAT_TABLE_DECODED_MAX_SIZE = 4096,	// 最多可以有：1000个节目
		PAT_TABLE_MAX_PROGRAM = 1000,
	};

	virtual void OnTableReceived();			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnPATReceived( const PDVB_PSI_TABLE_PAT pTable );
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_PatTable;
};

//------------------------------------------------
//	CAT 表
class CDVBPSITable_CAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_CAT();
	virtual ~CDVBPSITable_CAT();

	enum
	{
		CAT_TABLE_MAX_SIZE = 1024+256,
		CAT_TABLE_DECODED_MAX_SIZE = 4096,	// 最多可以有：1000个节目
	};

	virtual void OnTableReceived();			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnCATReceived( const PDVB_PSI_TABLE_CAT pTable );
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_CatTable;
};

//------------------------------------------------
//	PMT 表
class CDVBPSITable_PMT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_PMT();
	virtual ~CDVBPSITable_PMT();
	enum{
		PMT_TABLE_MAX_SIZE = 1024+512,
		PMT_TABLE_DECODED_MAX_SIZE = 4096,	//  CYJ,2007-3-21 增加到4KB
	};

	virtual void OnTableReceived();			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual void OnPMTReceived( const PDVB_PSI_TABLE_PMT pTable );
	void SetSID( WORD wSID ){ m_wDstSID = wSID; }
	WORD GetSID() const{ return m_wDstSID;  }
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	int DecodeStreamInfo( PBYTE pBuf, int nLen );
	int GetStreamCount( PBYTE pBuf, int nLen );

private:
	CMyHeap	m_PmtTable;
	WORD	m_wDstSID;				//	目标SID，若为 0x1FFF，则全部通过
};

//------------------------------------------------
// SDT, BAT
class CDVBPSITable_SDT_BAT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_SDT_BAT();
	virtual ~CDVBPSITable_SDT_BAT();
	enum{
		TABLE_MAX_SIZE = 1024 + 256,
		DECODED_SDT_MAX_SIZE = 2048,
		DECODED_ST_MAX_SIZE = 2048,
		DECODED_BAT_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnSDTReceived( PDVB_PSI_TABLE_SDT pSDT );
	virtual void OnBATReceived( PDVB_PSI_TABLE_BAT pBAT );

	PDVB_PSI_TABLE_SDT	GetLastReceivedSDT();
	PDVB_PSI_TABLE_BAT	GetLastReceivedBAT();

	virtual void Dump(FILE*fOutput=NULL);

private:
	void DecodeSDT();
	void DecodeBAT();
	WORD SDT_CalculateProgramCoumt(PBYTE pBuf, int nLen);
	int  BAT_GetBatItemsCount( PBYTE pBuf, int nLen );
	int BAT_DecodeBATItems( PBYTE pBuf, int nLen );

private:
	CMyHeap	m_SDT;
	CMyHeap	m_BAT;
};

//////////////////////////////////////////////////////////
/// EIT
class CDVBPSITable_EIT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_EIT();
	virtual ~CDVBPSITable_EIT();
	enum{
		TABLE_MAX_SIZE = 4096 + 256,			// 多留256字节是为了能够保存最后一个TS分组
		DECODED_TABLE_MAX_SIZE = 2048+4096,
	};
	virtual void OnTableReceived();							//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	WORD	GetDstSID()const { return m_wDstSID; }
	WORD	SetDstSID( WORD wSID );

	virtual void OnEITReceived( PDVB_PSI_TABLE_EIT pEIT );

private:
	WORD GetEventsCount( PBYTE pBuf, int nLen );
	CMyHeap	m_EIT;
	WORD	m_wDstSID;
};

//////////////////////////////////////////////////////////
/// NIT
class CDVBPSITable_NIT : public CDVBPSITablesBase
{
public:
	CDVBPSITable_NIT();
	virtual ~CDVBPSITable_NIT();
	enum{
		TABLE_MAX_SIZE = 1024 + 256,
		DECODED_TABLE_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();							//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	virtual void OnNITReceived( PDVB_PSI_TABLE_NIT pNIT );
	void SetDstNID( WORD wNID ){ m_wDstNID = wNID; }

private:
	CMyHeap	m_NIT;
	WORD	m_wDstNID;

private:
	int GetTSItemDescriptorCount( PBYTE pBuf, int nLen );
};

//////////////////////////////////////////////////////////////////////////
//	TDT		Time and Date Table
//	TOT		Time Offset Table
//	ST		Stuffing Table
class CDVBPSITable_TDT_TOT_ST :  public CDVBPSITablesBase
{
public:
	CDVBPSITable_TDT_TOT_ST();
	virtual ~CDVBPSITable_TDT_TOT_ST();

	enum{
		TABLE_MAX_SIZE = 1024 + 256,			// 1KB
		DECODED_TABLE_MAX_SIZE = 2048,
	};
	virtual void OnTableReceived();							//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();
	virtual void Dump(FILE*fOutput=NULL);

	virtual void OnDateTimeReceived( time_t tUTCDateTime, const PDVBPSI_TABLE_TOT pTOTTable ) = 0;		// 接收到 TDT
	virtual void OnTimeOffsetTableReceived( const PDVBPSI_TABLE_TOT pTOTTable ) = 0;		// 接收到 TDT

protected:
	void DecodeTDT();
	void DecodeTOT();

protected:
	CMyHeap	m_TOT;
};

///////////////////////////////////////////////////////////////////////
// ECM/EMM table
class CDVBPSITable_ECM_EMM_Message : public CDVBPSITablesBase
{
public:
	CDVBPSITable_ECM_EMM_Message();
	virtual ~CDVBPSITable_ECM_EMM_Message();

	enum{
		TABLE_MAX_SIZE = 4096,			// 4 KB
	};
	virtual void OnTableReceived();							//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnECM_EMMMessageReceived( BYTE byTableID, PBYTE pSectionData, int nSectionLen ) = 0;

	virtual void Dump(FILE*fOutput=NULL);
};

///////////////////////////////////////////////////////////////////////
// Tongshi VOD UniversalTable
class CDVBPSI_TSVOD_UniversalTable : public CDVBPSITablesBase
{
public:
	CDVBPSI_TSVOD_UniversalTable();
	CDVBPSI_TSVOD_UniversalTable(PBYTE pRawBuf,int nRawBufLen, PBYTE pDstBuf, int nDstBufLen);
	virtual ~CDVBPSI_TSVOD_UniversalTable();

	enum{
		TABLE_MAX_SIZE = ( 512*1024 + 256 ),
		DECODED_TABLE_MAX_SIZE = ( TABLE_MAX_SIZE * 2 + 2048 ),
	};

	virtual void OnPrivateDataReceived( PBYTE pBuf, int nLen, BYTE byBuildCounter, BYTE byTableID );

	virtual void OnTableReceived();							//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_Tbl;
	BYTE	m_byBuildCounter;
};

#pragma pack(pop)

#endif // !defined(AFX_DVBPSITABLES_H__FD159F36_C595_4D3F_AC09_15406E7B5F8E__INCLUDED_)
