#ifndef __IP_DATA_H_20040520__
#define __IP_DATA_H_20040520__

#include "IPRecSvr.h"
#include <BufPacket4C.h>
#include "SrcDataPort.h"
#include "BaseFileCombiner.h"
#include "FileObject.h"
#include "TSDBFileSystem.h"
#include "IPFileMendHelper.h"

#ifndef _WIN32
	#include <MyList.h>
#endif//_WIN32

class CIPData;

typedef struct tagONEASYNCREAD
{
	SDP_HANDLE	m_hSDP;
	CIPData	*	m_pIPData;
}ONEASYNCREAD,*PONEASYNCREAD;

#define IP_MAX_PACKET_SIZE	2048

#pragma pack(push,1)

class COneDataPortItem
{
public:
	COneDataPortItem()
	{
		m_pDataPort = NULL;
		m_pFileObjMgr = NULL;
		m_wPacketBufSize = IP_MAX_PACKET_SIZE;							//	默认包大小为 2K
		m_nPort = 0;
		m_pFileMendHelper = NULL;		
		memset( &m_ReceiveLog, 0, sizeof(m_ReceiveLog) );
	};

	~COneDataPortItem()
	{
		ASSERT( NULL == m_pFileMendHelper );
	}

	typedef struct tagFILERECEIVELOG
	{
		DWORD	m_dwID;					//	文件ID
		DWORD	m_dwFileCount;			//	总共字节数
		DWORD	m_dwTotalLen_16KB;		//	总共长度
		DWORD	m_dwOKFileCount;		//	成功接收的文件个数
		DWORD	m_dwOKLen_16KB;			//	成功接收的总长度，单位 16KB
		DWORD	m_dwOkLen_Below16KB;	//	辅助数据，用来准确结算16K
		float	m_fProgress;			//	进度
	}FILERECEIVELOG;

	CSrcDataPort		*	m_pDataPort;					//	数据源端口,一般有 ActiveX 创建
#ifdef _WIN32
	CList<ONEASYNCREAD,ONEASYNCREAD&> m_AsyncHandle;		//	等待处理的句柄对象
#else
	CMyList<ONEASYNCREAD> m_AsyncHandle;		//	等待处理的句柄对象
#endif //_WIN32
	CBaseFileCombiner	m_FileCombiner;						//	文件拼合对象
	CTSDBFileSystem *	m_pFileObjMgr;						//	文件对象管理，<==> ActiveX
	CIPFileMendHelper * m_pFileMendHelper;					//  2003-4-11 添加

	WORD	m_wPacketBufSize;								//	包大小
	long	m_nPort;
	CString m_strTargetIP;

	FILERECEIVELOG	m_ReceiveLog;
public:
	//////////////////////////////////////////////
	//功能:
	//		获取第一个异步操作句柄
	//入口参数:
	//		无
	//返回参数:
	//		>			句柄数目
	//		<0			失败
	SDP_HANDLE	GetHeadHandle()
	{
		if( m_AsyncHandle.IsEmpty() )
			return -1;
		ONEASYNCREAD & oneread = m_AsyncHandle.GetHead();
		return oneread.m_hSDP;
	};
};

class CIPData : public CBufPacket4C<IBufPacket>
{
public:
	CIPData() : CBufPacket4C<IBufPacket>( 0 ){ m_pDataPortItem = NULL; };
	COneDataPortItem * m_pDataPortItem;			//	附加参数	
	void DeleteHeadData( int nHeadLen )
	{
		ASSERT( nHeadLen < (int)GetDataLen() );
		Admin_AccessReservedBytes() += nHeadLen;
		PutDataLen( GetDataLen() - nHeadLen );
	}
    virtual void SafeDelete()
    {
    	delete this;
    };
};

#pragma pack( pop )

#endif // __IP_DATA_H_20040520__
