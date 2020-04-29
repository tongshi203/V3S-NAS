// MB_OneFile.h: interface for the CMB_OneFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_)
#define AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPRecSvr.h"
#include <BufPacket4C.h>
#include "Tsdb.h"

class COneDataPortItem;


#pragma pack(push,1)						//	按一个字节对齐编译

typedef struct tagMYMEMALLOCFLAG
{
	DWORD	m_bAllocUseHeadAlloc:1;		//	是否从 PagedHeap 分配，反之用 new
	DWORD	m_bAutoDelete:1;			//	需要自动删除，用PagedHeap删除，反之用delete
	DWORD	m_dwRes:30;					//	保留，必须＝0
}MYMEMALLOCFLAG,*PMYMEMALLOCFLAG;

#pragma pack(pop)						//	按一个字节对齐编译

//	一个文件
class CMB_OneFile : public CBufPacket4C<IBufPacket>
{
public:
	CMB_OneFile();		
	~CMB_OneFile();

	int AddOnePage( PBYTE pBuf, DWORD dwLen );
	BOOL IsFileChanged( time_t t,DWORD dwFileLen);
	BOOL Initialize( TSDBCHANNEL chFile, LPCSTR lpszFileName, DWORD dwLen, time_t FileTime );
	BOOL CollectDataUseXorChksum();

    virtual void SafeDelete();

public:
	enum {  PRS_MAX_PAGENUM = 256,			//	最大记录的页状态

			MBROF_DATA_ERR = -2,			//	数据错误
			MBROF_FILE_CHANGED = -1,		//	文件改变
			MBROF_DATAOK_FILENOTOK = 0,		//	数据OK，但文件还没有完成
			MBROF_FILE_OK = 1,				//	文件成功接收
	};
//	数据区
public:		
	void SetMulticastParameter( LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem );
	CString m_strMC_DstIP;					//	2002.11.14 添加，多播IP地址
	WORD	m_wMC_Port;						//	2002.11.14 添加，多播端口
	COneDataPortItem *	m_pDataPortItem;	//  2004-5-20 data port item

	time_t	m_Time;							//	文件时间
	TSDBCHANNEL	m_chFile;
	char	m_szFileName[13];				//	文家名
	DWORD	m_dwFileLen;					//	文件长度
	DWORD	m_dwByteRead;					//	已经读取的字节长度
	DWORD	m_dwSysCodeIndex;				//	系统密码索引
	DWORD	m_adwPageRecFlags[PRS_MAX_PAGENUM/32];		//	只记前8*32=32*8=256页，每比特表示一页是否接收
	DWORD	m_adwPageErrFlags[PRS_MAX_PAGENUM/32];		//	只记前8*32=32*8=256页，每比特表示该页是否错误
	union
	{
		struct
		{
			DWORD	m_bIsReceived:1;		//	是否有（部分）数据
			DWORD	m_bIsFileErr:1;			//	是否有误
			DWORD	m_dwRes:30;				//	保留，＝0
		};
		DWORD	m_dwData;
	}m_dwResultFlags;

private:
	void SetPageReceived( int nPageNo, BOOL bIsErr );	
	enum { XORCHKSUMBUFLEN = 2048 };
	BYTE	m_abyXorChkSum[XORCHKSUMBUFLEN];	//	XOR CHK SUM
	int		m_nXorChkSumDataLen;				//	非0，有校验；反之，没有校验
};

#endif // !defined(AFX_MB_ONEFILE_H__AE83B1CC_79BC_444A_861A_B368DFF9BE3E__INCLUDED_)
