// BaseFileCombiner.cpp: implementation of the CBaseFileCombiner class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14  修改 OnFileOK，给 pOneFile 赋 IP:Port 值
//	2002.6.29	修改，GetIPBPS 的 deDelta 可能为 0
//	2002.6.16	修改 GetIPBPS，统计方法，可能会导致时间溢出

#include "stdafx.h"
#include "BaseFileCombiner.h"
#include "crc.h"
#include "IPData.h"
#include <time.h>

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

class COneDataPortItem;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseFileCombiner::CBaseFileCombiner()
{
	m_dwByteReceived = 0;						//	总共接收到的字节数
	m_dwLastTickCount = time(NULL);					//	上次计算的时间
	m_dwLastBPS = 0;
}

CBaseFileCombiner::~CBaseFileCombiner()
{
	FreeActiveOne_FileObject();
}
//////////////////////////////////////////////
//功能:
//		处理输入的一页数据
//入口参数:
//		pDPItem			数据端口参数
//		pBuf			缓冲区地址
//		dwLen			长度
//返回参数:
//		无
void CBaseFileCombiner::DoInputOnePage(COneDataPortItem *pDPItem, PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pDPItem && pBuf && dwLen );
	ASSERT( &pDPItem->m_FileCombiner == this );

	m_dwByteReceived += dwLen;					//	以字节为单位，统计 IP包 速率
	if( m_dwByteReceived >= 0x7FFFFFFF )
	{											//	超过 2G 字节，折中考虑
		m_dwLastTickCount = ( m_dwLastTickCount + time(NULL) ) / 2;
		m_dwByteReceived /= 2;
	}

	PTSDVBMULTICASTPACKET0 pHeader = (PTSDVBMULTICASTPACKET0)pBuf;

	if( pHeader->m_cbSize >= dwLen || pHeader->m_cbSize < offsetof(TSDVBMULTICASTPACKET0,m_PacketTime) )
		return;									//	错误的数据头
	if( CCRC::GetCRC32(pHeader->m_cbSize-offsetof(TSDVBMULTICASTPACKET0,m_PacketTime),\
		(PBYTE)&pHeader->m_PacketTime ) != pHeader->m_dwHeaderCRC32 )
	{
		TRACE("CBaseFileCombiner::DoInputOnePage, Header CRC32 error.\n");
		return;									//	数据头 CRC 错误
	}
#ifdef _DEBUG_
	TRACE("Receive One page %d.%02d %s %d\n",pHeader->m_chFile.m_nMag, pHeader->m_chFile.m_nPageNo,\
		pHeader->m_cFileName, pHeader->GetPageOfsInFile()/pHeader->GetPageLen() );
#endif // _DEBUG

	CMB_OneFile * pOneFile=NULL;
	if( FALSE == m_OneFileMgr.Lookup( pHeader->m_chFile.m_dwData, pOneFile ) )
	{											//	没有找到
		pOneFile = AllocOneFile( pHeader );
		if( NULL == pOneFile )					//	分配内存失败
			return;
	}
	else
	{
		if( pOneFile->IsFileChanged(pHeader->m_PacketTime,pHeader->m_dwFileLen) )
		{
			pOneFile->CollectDataUseXorChksum();	//	2001.10.12 试图纠正错误
			if( pOneFile->m_dwByteRead )
			{
				TRACE("File changed.\n");
				pOneFile->m_Time = RestoreBroTime( pOneFile->m_Time );
				OnFileOK( pDPItem, pOneFile );				//	pOneFile 处理方需要调用 Release 方法
			}
			else
			{
				TRACE("File changed and none page received.\n");
				DeAllocate( pOneFile, FALSE );		//	释放文件对象
				pOneFile->Release();
			}
			pOneFile = AllocOneFile( pHeader );
			if( NULL == pOneFile )
			{
				m_OneFileMgr.RemoveKey( pHeader->m_chFile.m_dwData );
				return;								//	分配失败
			}
		}
	}

	int nRetVal = pOneFile->AddOnePage( pBuf, dwLen );
	ASSERT( nRetVal != CMB_OneFile::MBROF_FILE_CHANGED );
	if( nRetVal == CMB_OneFile::MBROF_FILE_OK )
	{
		ASSERT( pOneFile->m_dwByteRead );
		pOneFile->m_Time = RestoreBroTime( pOneFile->m_Time );
		OnOneSubFileOK( pDPItem, pBuf, dwLen );					//  2003-4-11 记录子文件接收OK
		OnFileOK( pDPItem, pOneFile );							//	pOneFile 处理方需要调用 Release 方法
		m_OneFileMgr.RemoveKey( pHeader->m_chFile.m_dwData );	//	删除
	}
}

//////////////////////////////////////////////
//功能:
//		一个基本文件接收成功
//入口参数:
//		pDPItem			数据源参数
//		pOneFile		基本文件
//返回参数:
//		无
//修改记录：
//		给 pOneFile 赋 IP:Port 值
void CBaseFileCombiner::OnFileOK(COneDataPortItem *pDPItem, CMB_OneFile *pOneFile)
{
	ASSERT( pOneFile );
	ASSERT( pDPItem && pDPItem->m_pFileObjMgr);

#ifdef _DEBUG_
	TRACE("One base file %08X is received, FileLen=%d, Byte received=%d, PacketTime=%d, Ch=%X\n",
		pOneFile, pOneFile->GetDataLen(), pOneFile->m_dwByteRead,
		pOneFile->m_Time, pOneFile->m_chFile.m_dwData );
#endif // _DEBUG_

	ASSERT( pDPItem->m_nPort );			//	2002.11.14 添加
	pOneFile->SetMulticastParameter( pDPItem->m_strTargetIP, (WORD)pDPItem->m_nPort, pDPItem );

	pDPItem->m_pFileObjMgr->ProcessOneFile( pOneFile );

	DeAllocate( pOneFile, FALSE );
	pOneFile->Release();
}

//	分配并初始化一个对象
//	入口参数
//		pHeader				数据头
//	返回参数
//		NULL				失败
CMB_OneFile * CBaseFileCombiner::AllocOneFile(PTSDVBMULTICASTPACKET0 pHeader)
{
	CMB_OneFile * pOneFile = AllocatePacket(0,FALSE);
	if( NULL == pOneFile )
		return NULL;

	if( FALSE == pOneFile->Initialize( pHeader->m_chFile, pHeader->m_cFileName, \
		pHeader->m_dwFileLen, pHeader->m_PacketTime ) )
	{										//	初始化失败
		DeAllocate( pOneFile, FALSE );
		pOneFile->Release();
		return NULL;
	}
	m_OneFileMgr.SetAt( pHeader->m_chFile.m_dwData, pOneFile );
	return pOneFile;
}

//	还原时间
//	入口参数
//		BroTime						待解码的播出时间
//	返回参数
//		UTC	时间
time_t CBaseFileCombiner::RestoreBroTime(time_t BroTime)
{
	if( BroTime & (1L<<31) )
	{
		BroTime >>= 8;
		BroTime &= 0x7FFFFF;
		BroTime += Y2KSECOND;
	}
	else
	{
		if( BroTime < 24*3600*10L )
		{							//	老的打包时间,归一化成 2000.1.1  00:00:00
			BroTime /= 10;
			BroTime += Y2KSECOND;
		}
	}
	return BroTime;
}

//////////////////////////////////////////////
//功能:
//		释放正在接收的文件对象
//入口参数:
//		无
//返回参数:
//		无
void CBaseFileCombiner::FreeActiveOne_FileObject()
{
	POSITION pos = m_OneFileMgr.GetStartPosition();
#ifdef _DEBUG
	if( pos )
		TRACE("CBaseFileCombiner::FreeActiveOne_FileObject,  Free Active OneFile Object.\n");
#endif // _DEBUG
	while( pos )
	{
		DWORD dwKey;
		CMB_OneFile * pOneFile;
		m_OneFileMgr.GetNextAssoc( pos, dwKey, pOneFile );
		ASSERT( pOneFile );
		pOneFile->Release();
	}
}

//////////////////////////////////////////////
//功能:
//		获取 IP 链路层的速率
//入口参数:
//		无
//返回参数:
//		链路层速率
DWORD CBaseFileCombiner::GetIPBPS()
{
	DWORD dwNow = time(NULL);
	DWORD dwDelta = dwNow - m_dwLastTickCount;
	if( dwDelta >= 10 || 0 == m_dwLastBPS )			//	间隔太小，使用上一次的速率，每10秒统计一次
	{													//	以后每5秒统计一次
		if( 0 == dwDelta )								//	2002.6.29，添加，dwDelta 可能为 0
			return 0;
		m_dwLastBPS = ( m_dwByteReceived * 8) / dwDelta;		//	125*64 = 8 * 1000
		m_dwLastTickCount = m_dwLastTickCount + dwDelta / 2;
		m_dwByteReceived /= 2;
	}
	return m_dwLastBPS;
}

///-------------------------------------------------------
/// 2003-4-11
/// 功能：
///		一个子文件接收成功
/// 入口参数：
///		pDPItem				数据端口对象
///		pBuf				缓冲区
///		dwLen				缓冲区长度
/// 返回参数：
///		无
void CBaseFileCombiner::OnOneSubFileOK(COneDataPortItem *pDPItem, PBYTE pBuf, DWORD dwLen)
{
	ASSERT( pDPItem && pBuf && dwLen );
	if( NULL == pDPItem->m_pFileMendHelper )
		return;

	PTSDVBMULTICASTPACKET0 pPacket = (PTSDVBMULTICASTPACKET0)pBuf;
	if( pPacket->m_ExtParam.m_byReserved_0 )
		return;				//	不是扩展命令方式
	BYTE byXorValue = 0;
	for(int i=2; i<13; i++)
	{
		byXorValue ^= (BYTE)pPacket->m_cFileName[i];
	}
	if( byXorValue != pPacket->m_ExtParam.m_byXorChkSum )
		return;				//	数据校验错误
	if( EPFI_SUBFILE_ID != pPacket->m_ExtParam.m_byFunctionIndex )
		return;
	int nSubFileID = pPacket->m_ExtParam.m_SubFileID.m_dwSubFileID;
	int nTotalFileCount = pPacket->m_ExtParam.m_SubFileID.m_dwTotalSubFileCount;

	if( nTotalFileCount )
		pDPItem->m_pFileMendHelper->SetTotalSubFileCount( nTotalFileCount );

	long nTotalBit1Count = pDPItem->m_pFileMendHelper->SetBitValue( nSubFileID, 1 );

#ifdef _DEBUG
	CString strTmp;
	strTmp.Format("SubFile %d/%d is OK.\n", nSubFileID, nTotalFileCount );
#endif // _DEBUG

}
