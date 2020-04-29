///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-14
///
///=======================================================

// DVBFileReceiver.cpp : Implementation of CDVBFileReceiver
/////////////////////////////////////////////////////////////////
//  2003-11-18  允许 put_bstrAutoSavePath 为空字符串
//	2002.11.14	修改 OnOneFileOK，释放大文件情况下 Detatch 内存
//				修改 NotifyOnFileOnEvent和NotifyOnSubFileOKEvent，AddPacket 失败情况下的处理
//				组件的版本 => 1.01.001

#include "stdafx.h"
#include "IPRecSvr.h"
#include "DVBFileReceiver.h"
#include "DecoderThread.h"

#include "UDPDataPortLinux.h"
#include "UDPRecThread.h"
#include "FileWriterThread.h"

#include "DirectroyHelp.h"
#include <time.h>
#include <MyMapFile.h>

CDecoderThread			g_DecoderThread;
CUDPRecThread 			g_UDPRecThread;
CFileWriterThread		g_FileWriterThread;			//  2004-7-31 自动保存文件线程


#ifdef _WIN32
    #ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[]=__FILE__;
    #define new DEBUG_NEW
    #endif
#endif //_WIN32

#ifdef _WIN32
	extern "C" IDVBFileReceiver * WINAPI CreateDVBFileReceiver(void)
#else
	extern "C" IDVBFileReceiver * CreateDVBFileReceiver(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CDVBFileReceiver * pInstance = new CDVBFileReceiver;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();
	return static_cast<IDVBFileReceiver*>(pInstance);
}


#pragma pack( push, 1 )
typedef union tagEQUFILEATTRIBUTE	//	通过文件属性获取
{
	struct
	{
		DWORD m_dwValue:24;			//	文件长度/总个数
		DWORD m_dwID:6;				//	计数器表示进度
		DWORD m_dwIsFileCount:1;	//	是否为文件总个数，=0 文件总字节数
		DWORD m_dwHasExtData:1;		//	转移表示传输文件总字节数和文件总数
	};
	DWORD	m_dwData;
}EQUFILEATTRIBUTE,*PEQUFILEATTRIBUTE;
#pragma pack( pop )

/////////////////////////////////////////////////////////////////////////////
// CDVBFileReceiver

CDVBFileReceiver::CDVBFileReceiver()
{
	m_bSaveFileInBackgound = false;
	PresetVars();
}
CDVBFileReceiver::~CDVBFileReceiver()
{
    ExecDelayDeleteItems();			//	执行延迟的操作
    ASSERT( 0 == m_nTimer_2_Second );

	RegisterEventResponser( NULL );
}

DWORD CDVBFileReceiver::QueryInterface( REFIID iid,void ** ppvObject)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( IID_IDVBFileReceiver == iid )
	{
		AddRef();
		*ppvObject = static_cast<IDVBFileReceiver*>(this);
		return 0;	//	S_OK;
	}

	return CMyIUnknownImpl<IDVB_EPG_Receiver>::QueryInterface( iid, ppvObject );
}

long CDVBFileReceiver::GetMaxPacketSize()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_dwMaxPacketSize;
}

//////////////////////////////////////////////
//功能:
//		添加UDP/TCP一个数据端口
//入口参数:
//		bstrTargetIP			目标 IP 地址
//		nPort					端口
//		varLocalBindIP			本地绑定 IP，默认 ＝ NULL，即自动
//		barIsUDP				是否 UDP 数据包，缺省 ＝ TRUE
//		pVal					输出端口句柄
//								NULL			失败
//								其他			成功，删除端口需要引用该数值
//返回参数:
//		无
HDATAPORT CDVBFileReceiver::CreateDataPort( LPCSTR lpszTargetIP, int nPort, LPCSTR lpszBindIP, BOOL bIsUDP )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return NULL;			//	没有在线

	ASSERT( lpszTargetIP && nPort >= 1024 );
	if( NULL == lpszTargetIP || nPort < 1024 )
		return NULL;
	if( FALSE == bIsUDP )
		return NULL;					//	目前还没有实现

	CString strLocalBind = lpszBindIP;

	COneDataPortItem * pDataPortItem = NULL;
	TRY
	{
		CString strIP = lpszTargetIP;
		int nItemNo = FindDataPort( strIP, nPort );	//	看看是否曾经有过
		ASSERT( nItemNo < m_aDataPortItems.GetSize() );
		if( nItemNo >= 0 )
        	return NULL;				// exist already

        pDataPortItem = NewOneDataPortItem();
        if( NULL == pDataPortItem )
            return NULL;				// allocate memory failed

        pDataPortItem->m_strTargetIP = strIP;
        pDataPortItem->m_nPort = nPort;

		pDataPortItem->m_pDataPort = NULL;		// indicate this dataport is a UDP socket port

		pDataPortItem->m_pFileObjMgr = static_cast< CTSDBFileSystem * >(this);

#ifdef _DEBUG
		printf("To callg_UDPRecThread.AddDataPort\n");
#endif //_DEBUG

		g_UDPRecThread.AddDataPort( strIP, nPort, lpszBindIP, pDataPortItem );

#ifdef _DEBUG
		printf("return from add data port\n");
#endif //_DEBUG

		m_aDataPortItems.Add( pDataPortItem );
	}
	CATCH_ALL( e )
	{
		if( pDataPortItem )
		{
			if( pDataPortItem->m_pFileMendHelper )			//  2003-4-11 添加
				pDataPortItem->m_pFileMendHelper->Release();
			pDataPortItem->m_pFileMendHelper = NULL;

			delete pDataPortItem;
		}
		return NULL;
	}
	END_CATCH_ALL

	return (HDATAPORT)pDataPortItem;
}

//////////////////////////////////////////////
//功能:
//		通知文件接收 OK
//入口参数:
//		pFileObject				待处理的文件对象
//返回参数:
//		无
//注：
//		当文件不是一个大文件时，pHugeFileObject NULL
//修改记录：
//	2002.11.14 处理 AddPacket 失败的情况
void CDVBFileReceiver::NotifyOnFileOKEvent( CFileObject * pFileObject )
{
	CFileEventObject * pFileEvent = m_FileNotifyEventList.AllocatePacket();
	ASSERT( pFileEvent );
	if( NULL == pFileEvent )
		return;
	pFileEvent->m_pFileObject = pFileObject;
	ASSERT( FALSE == pFileEvent->m_bIsSubFile );
	if( false == m_FileNotifyEventList.AddPacket( pFileEvent ) )
	{
		ASSERT( FALSE );						//	发生错误，放弃
		m_FileNotifyEventList.DeAllocate( pFileEvent );
	}

	pFileEvent->Release();
}

//////////////////////////////////////////////
//功能:
//		提交一个子文件成功接收 OK
//入口参数:
//		pFileObject				子文件对象
//返回参数:
//		无
//修改记录：
//	2002.11.14 处理 AddPacket 失败的情况
void CDVBFileReceiver::NotifySubFileOKEvent( CFileObject * pFileObject )
{
	ASSERT( pFileObject && pFileObject->m_pHugeFileParam );

	CFileEventObject * pFileEvent = m_FileNotifyEventList.AllocatePacket();
	ASSERT( pFileEvent );
	if( NULL == pFileEvent )
		return;
	pFileEvent->m_pFileObject = pFileObject;
	pFileEvent->m_bIsSubFile = TRUE;
	if( false == m_FileNotifyEventList.AddPacket( pFileEvent ) )
	{
		ASSERT( FALSE );				//	发生错误
		m_FileNotifyEventList.DeAllocate( pFileEvent );
	}

	pFileEvent->Release();
}

//////////////////////////////////////////////
//功能:
//		删除数据端口
//入口参数:
//		hDataPort			句柄，由CreateDataPort 或 AddDataPort 返回值
//		pVal				输出成功与否
//返回参数:
//
BOOL CDVBFileReceiver::DeleteDataPort( HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return FALSE;			//	没有在线

	int nItemNo = FindDataPort( hDataPort );
	if( nItemNo < 0 )
		return FALSE;

	COneDataPortItem * pItem = m_aDataPortItems[nItemNo];
	ASSERT( pItem );

    if( NULL == pItem->m_pDataPort )	// a Socket data port,
	    g_UDPRecThread.DeleteDataPort( pItem->m_strTargetIP, pItem->m_nPort );
    else
    {
    	ASSERT( FALSE );		// deal with MyDataPort
    }

	m_aDataPortItems.RemoveAt( nItemNo );

	if( pItem->m_pFileMendHelper )
	{										//  2003-4-11 添加，删除修补对象
		pItem->m_pFileMendHelper->Release();
		pItem->m_pFileMendHelper = NULL;
	}

	m_aDelayDeleteItems.Add( pItem );		//	延迟操作的删除

	if( pItem->m_pDataPort )				//	删除端口
	{
		pItem->m_pDataPort->Release();
		pItem->m_pDataPort = NULL;
	}

	return TRUE;
}

//////////////////////////////////////////////
//功能:
//		查找是否曾经有过该端口
//入口参数:
//		strIP		目标 IP
//		nPort		端口
//返回参数:
//		-1			失败
//		>=0			序号
int CDVBFileReceiver::FindDataPort(CString &strIP, long nPort)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
		if( pItem->m_nPort != nPort )
			continue;
		if( pItem->m_strTargetIP != strIP )
			continue;

		return i;
	}
	return -1;
}

//////////////////////////////////////////////
//功能:
//		获取对象的版本
//入口参数:
//		pVal			输出版本字符串
//返回参数:
//
LPCSTR CDVBFileReceiver::GetVersion()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_strVersion.Format("%d.%02d.%03d", MAJOR_VERSION,MINOR_VERSION,BUILD_NO  );

	return (LPCSTR)m_strVersion;
}

BOOL CDVBFileReceiver::GetSendSubFileEvent()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_bSendSubFileEvent;

}

void CDVBFileReceiver::PutSendSubFileEvent( BOOL bNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_bSendSubFileEvent = bNewVal;
}

long CDVBFileReceiver::GetIPPacketBPS()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	DWORD dwBPS = 0;
	for(int i=0; i<nCount; i++)
	{
		dwBPS += m_aDataPortItems[i]->m_FileCombiner.GetIPBPS();
	}

	return dwBPS;
}

long CDVBFileReceiver::GetFileBPS()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	DWORD dwNow = (DWORD)time( NULL );
	DWORD dwDelta = dwNow - m_dwLastTickCount;
	if( dwDelta && (dwDelta >= 10 || 0 == m_dwLastFileBPS) )			//	间隔太小，使用上一次的速率
	{
		m_dwLastFileBPS = ( m_dwByteReceived * 8 ) / dwDelta;		//	125*64 = 8 * 1000
		m_dwLastTickCount = ( m_dwLastTickCount + dwNow ) / 2;
		m_dwByteReceived /= 2;
	}

	return m_dwLastFileBPS;
}

long  CDVBFileReceiver::GetDataPortCount()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_aDataPortItems.GetSize();
}

LPCSTR CDVBFileReceiver::GetAutoSavePath()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return (LPCSTR)m_strAutoSavePath;
}

//  2003-11-18 允许使用空字符串
void  CDVBFileReceiver::SetAutoSavePath( LPCSTR lpszNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( lpszNewVal )
		m_strAutoSavePath = lpszNewVal;
	else
		m_strAutoSavePath = "";
	if( m_strAutoSavePath.IsEmpty() )
	{										//  2003-11-18 允许空字符串
		m_bAutoSave = FALSE;
		return;						//	不自动接收
	}

	int nLen = m_strAutoSavePath.GetLength();
#ifdef _WIN32
	m_strAutoSavePath.Replace( '/', '\\');
	if( nLen && m_strAutoSavePath[nLen-1] != '\\' )
		m_strAutoSavePath += '\\';					//	归一化成 '\' 结尾
#else
	m_strAutoSavePath.Replace( '\\', '/');
	if( nLen && m_strAutoSavePath[nLen-1] != '/' )
		m_strAutoSavePath += '/';					//	归一化成 '\' 结尾
#endif //_WIN32

	m_bAutoSave = ( 0 != nLen );
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		Initialize
/// Input parameter:
///		varMaxPacketSize			max packet size, ignore currently, use 2K instead
///		bSaveFileInBackground		save file data in background thead( CFileWriterThread )
///									default is faluse, in the case, the notification file object's databuf = NULL
/// Output parameter:
///		None
BOOL CDVBFileReceiver::Init(bool bSaveFileInBackground,int varMaxPacketSize)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( FALSE == m_bIsOnLine );

	if( m_bIsOnLine )
		return TRUE;
	PresetVars();

	m_strAutoSavePath = "";				//	先不自动存盘

	m_dwMaxPacketSize = varMaxPacketSize;
	if( 0 == m_dwMaxPacketSize )
		m_dwMaxPacketSize = 2048;
	m_bIsOnLine = TRUE;

	if( false == IPRD_Init() )
		return FALSE;

	m_bSaveFileInBackgound = bSaveFileInBackground;
	if( m_bSaveFileInBackgound )			//  2004-7-31 自动写文件，添加到工作线程中
		g_FileWriterThread.Add( this );

	return TRUE;
}

void CDVBFileReceiver::Close()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	RegisterEventResponser( NULL );

	if( FALSE == m_bIsOnLine )
		return;

	if( m_bSaveFileInBackgound )			//  2004-7-31 从写文件线程中删除
		g_FileWriterThread.Remove( this );

	DeleteAllDataPort();

	m_bIsOnLine = FALSE;

	CleanFileOKQueue();				//	删除还没有发送的文件

	if( m_HugeFile.m_hFile != CFile::hFileNull )
	{
		TRY
		{
			m_HugeFile.Close();
		}
		CATCH( CFileException, e )
		{
			m_HugeFile.Abort();
		}
		END_CATCH
	}

	IPRD_Close();
}

//////////////////////////////////////////////
//功能:
//		添加一个数据源端口
//入口参数:
//		pSrcDataPort		数据源端口
//		pVal				数据句柄
//	注：
//		pSrcDataPort		已经初始化过，可以接收程序
HDATAPORT CDVBFileReceiver::AddDataPort( void * pDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
/*
	CSrcDataPort * pSrcDataPort = (CSrcDataPort *)pDataPort;
	ASSERT( pSrcDataPort );
	if( NULL == pSrcDataPort )
		return 0;

	ASSERT( m_bIsOnLine );
	if( FALSE == m_bIsOnLine )
		return 0;			//	没有在线

	COneDataPortItem * pDataPortItem = NewOneDataPortItem();
	if( NULL == pDataPortItem )
		return 0;

	CSrcDataPort * pSrcPort = (CSrcDataPort*)pSrcDataPort;

	pDataPortItem->m_strTargetIP = pSrcPort->m_szIPAddress;
	pDataPortItem->m_nPort = pSrcPort->m_wPort;

	pDataPortItem->m_pDataPort = pSrcPort;
	pSrcPort->AddRef();							//	增加引用计数器
	pDataPortItem->m_pFileObjMgr = static_cast< CTSDBFileSystem * >(this);

	g_ThreadMgr.m_pReadThread->AddDataPort( pDataPortItem );

	m_aDataPortItems.Add( pDataPortItem );

	return (long)pDataPortItem;
*/
	ASSERT( FALSE );		// do more here
	return NULL;
}

//////////////////////////////////////////////
//功能:
//		查找端口句柄
//入口参数:
//		hDataPort			数据源端口
//返回参数:
//		-1			失败
//		>=0			序号
int CDVBFileReceiver::FindDataPort(HDATAPORT hDataPort)
{
	COneDataPortItem * pDataPort = (COneDataPortItem *)hDataPort;

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
		if( (HDATAPORT)pItem == hDataPort )
			return i;
	}
	return -1;
}

//////////////////////////////////////////////
//功能:
//		删除所有的数据源端口，因为即将退出
//入口参数:
//		无
//返回参数:
//		无
void CDVBFileReceiver::DeleteAllDataPort()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	int nCount = m_aDataPortItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDataPortItems[i];
		ASSERT( pItem );
        if( NULL == pItem->m_pDataPort )	// UDP socket
        	g_UDPRecThread.DeleteDataPort( pItem->m_strTargetIP, pItem->m_nPort );
		else
        {
        	ASSERT(FALSE);		// do more here
		}

		m_aDelayDeleteItems.Add( pItem );		//	延迟删除操作

		if( pItem->m_pFileMendHelper )
		{										//  2003-4-11 添加，删除修补对象
			pItem->m_pFileMendHelper->Release();
			pItem->m_pFileMendHelper = NULL;
		}

		if( pItem->m_pDataPort )				//	删除端口
		{
			pItem->m_pDataPort->Release();
			pItem->m_pDataPort = NULL;
		}
	}
	m_aDataPortItems.RemoveAll();
}

//	判断是否已经更新
//	入口参数
//		lpszFileName			文件名
//		LastModifyTime			最后修改时间
//		dwFileLen				文件长度
//	返回参数
//		TRUE					更新，需要重新打开文件
//		FALSE					没有更新，可以继续写数据
BOOL CDVBFileReceiver::IsHugeFileChanged(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen)
{
	ASSERT( lpszFileName );
#ifdef _WIN32
    ASSERT( lpszFileName[1] == ':' );
#else
	ASSERT( lpszFileName[0] == '/' );
#endif //_WIN32
	if( LastModifyTime == m_HugeFileLastModifyTime && \
		m_dwHugeFileLen == dwFileLen &&\
		0 == m_strHugeFileName.CompareNoCase( lpszFileName ) &&\
		m_HugeFile.m_hFile != CFile::hFileNull )
	{
		return FALSE;
	}
	if( m_HugeFile.m_hFile != CFile::hFileNull  )
	{
		TRY
		{
			m_HugeFile.Close();
		}
		CATCH( CFileException, e )
		{
#if defined(_DEBUG) && defined(_WIN32)
			e->ReportError();
#endif // _DEBUG
			m_HugeFile.Abort();
			return TRUE;
		}
		END_CATCH
	}
	return TRUE;
}

//	准备大文件环境
//	入口参数
//		lpszFileName			文件名，全路径
//		LastModifyTime			最后访问时间
//		dwFileLen				文件长度
//		nSubFileCount			子文件总数，2003-8-7 添加
//	返回参数
//		TRUE					成功
//		FALSE					创建文件失败
//	修改记录
//		2003-8-7 添加入口参数 nSubFileCount
BOOL CDVBFileReceiver::PreprareHugeFile(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen, int nSubFileCount)
{
	ASSERT( lpszFileName );
#ifdef _WIN32
    ASSERT( lpszFileName[1] == ':' );
#else
	ASSERT( lpszFileName[0] == '/' );
#endif //_WIN32

	ASSERT( m_HugeFile.m_hFile == CFile::hFileNull );
	m_HugeFileLastModifyTime = LastModifyTime;
	m_strHugeFileName = lpszFileName;
	m_dwHugeFileLen = dwFileLen;
	m_dwHugeFileByteReceived = 0;				//	准备开始接收

	CDirectroyHelp::Mkdir( lpszFileName );

	m_HugeFile.SetHugeFileParameter( dwFileLen, LastModifyTime, nSubFileCount );

	if( FALSE == m_HugeFile.Open( lpszFileName, CFile::modeNoTruncate|CFile::modeCreate|\
                        CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite ) )
	{
		return FALSE;
	}
	if( m_HugeFile.GetLength() > dwFileLen )
		m_HugeFile.SetLength( dwFileLen );


	return TRUE;
}

//	移动文件指针到指定位置，若不够，则创建
BOOL CDVBFileReceiver::MoveHugeToOffset(DWORD dwOffset)
{
	BOOL bRetVal = TRUE;
	TRY
	{
		DWORD dwCurPos = m_HugeFile.GetPosition();
		if( dwOffset == dwCurPos )
			return TRUE;
		else if( dwOffset > m_HugeFile.GetLength() )
			m_HugeFile.SetLength( dwOffset );
		else
			m_HugeFile.Seek( dwOffset, CFile::begin );
	}
	CATCH( CFileException,e)
	{
#ifdef _DEBUG_
		e->ReportError();
#endif // _DEBUG
		bRetVal = FALSE;
		m_HugeFile.Abort();
	}
	END_CATCH
	return bRetVal;
}

//////////////////////////////////////////////
//功能:
//		保存大文件，只有自动保存的情况下才调用该函数
//入口参数:
//		pFileObject		(大)文件对象
//返回参数:
//		true			整个大文件成功接收
///		false			没有接收成功
//修改记录：
//  2004-5-26		若大文件成功接收，发送 OnFileOK 事件
bool CDVBFileReceiver::SaveHugeFile(CFileObject *pFileObject)
{
	ASSERT( FALSE == m_strAutoSavePath.IsEmpty() );
	ASSERT( m_bSendSubFileEvent );
#ifdef _WIN32
	ASSERT( m_strAutoSavePath[ m_strAutoSavePath.GetLength()-1 ] == '\\' );
#else
	ASSERT( m_strAutoSavePath[ m_strAutoSavePath.GetLength()-1 ] == '/' );
#endif //_WIN32
	ASSERT( pFileObject->m_pHugeFileParam );

	time_t LastModifyTime = 0;
	if( pFileObject->m_pAttributeData )
		LastModifyTime = pFileObject->m_pAttributeData->m_LastWriteTime;
	DWORD FileLen = pFileObject->m_pHugeFileParam->m_dwFileLen;

	CString strTmp = m_strAutoSavePath;
    strTmp += pFileObject->m_strFileName;
	bool bIncreaseFileCount = false;

	if( IsHugeFileChanged( strTmp, LastModifyTime, FileLen ) )
	{
		int nSubFileCount = pFileObject->m_pHugeFileParam->m_wTotalBlock;
		if( FALSE == PreprareHugeFile( strTmp, LastModifyTime, FileLen, nSubFileCount ) )
			return false;
		bIncreaseFileCount = true;
	}

	if( FALSE == MoveHugeToOffset( pFileObject->m_pHugeFileParam->m_dwFilePosition ) )
		return false;							//	移动指针失败

	bool bRetVal = false;
	TRY
	{
		ASSERT( m_HugeFile.m_hFile != CFile::hFileNull );
		m_HugeFile.Write( pFileObject->GetBuffer(), pFileObject->m_pHugeFileParam->m_wBlockSize );
		bRetVal = m_HugeFile.NotifyOneSubFileOK( pFileObject->m_pHugeFileParam->m_wBlockNo );
	}
	CATCH(CFileException,e)
	{
#if defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		m_HugeFile.Abort();
		return false;
	}
	END_CATCH

	SetDataPortReceiveLog(  pFileObject->m_pDataPortItem, pFileObject->GetAttribute(), \
	pFileObject->GetDataLen(), bIncreaseFileCount, pFileObject->GetFileName() );

	return bRetVal;
}

//////////////////////////////////////////////
//功能:
//		执行延迟的删除操作
//入口参数:
//		无
//返回参数:
//		无
void CDVBFileReceiver::ExecDelayDeleteItems()
{
	int nCount = m_aDelayDeleteItems.GetSize();
	for(int i=0; i<nCount; i++)
	{
		COneDataPortItem * pItem = m_aDelayDeleteItems[i];
		ASSERT( NULL == pItem->m_pFileMendHelper );	//  2003-4-11 添加
		delete pItem;
	}
	m_aDelayDeleteItems.RemoveAll();
}

//////////////////////////////////////////////
//功能:
//		清除 OnFileOK 中的文件缓冲区
//入口参数:
//		无
//返回参数:
//		无
void CDVBFileReceiver::CleanFileOKQueue()
{
#ifdef _DEBUG
	BOOL bShown = FALSE;
#endif // _DEBUG
	for(int i=0; i<0xFFFF; i++)								//	防止死循环
	{
		CFileEventObject * pFileEvent = m_FileNotifyEventList.PeekData( 0 );
		if( NULL == pFileEvent )
			return;

#ifdef _DEBUG
		if( FALSE == bShown )
			TRACE("CleanFileOKQueue, last active file is release.\n");
		bShown = TRUE;
#endif // _DEBUG

		DeAllocate( pFileEvent->m_pFileObject );			//	释放文件对象
		pFileEvent->m_pFileObject->Release();				//	释放

		m_FileNotifyEventList.DeAllocate( pFileEvent );		//	回归对象
		pFileEvent->Release();
	}
}

//----------------------------------------------------------
//	是否触发非 TSDB 文件事件
BOOL CDVBFileReceiver::GetSendNotTSDBFileEvent()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_bIsEnableNotTSDBFile;
}

//----------------------------------------------------------
//	是否触发非 TSDB 文件事件
void CDVBFileReceiver::SetSendNotTSDBFileEvent( BOOL bNewVal )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	m_bIsEnableNotTSDBFile = bNewVal;

}

//-----------------------------------------------
//	2002.12.28
//	检测并触发文件OK事件
void CDVBFileReceiver::CheckAndSendFileEvent()
{
	for(int i=0; i<1000; i++)				//	2002.7.5，10次太少了，大约 1000 次，不一定非要 10 次，防止错误
	{
		CFileEventObject * pFileEvent = m_FileNotifyEventList.PeekData( 0 );
		if( NULL == pFileEvent )
			return;

		ASSERT( pFileEvent->m_pFileObject );

		if( m_dwByteReceived >= 0x7FFFFFFF )
		{														//	可能越界，减半处理
			m_dwLastTickCount = ( m_dwLastTickCount + time(NULL) ) / 2;
			m_dwByteReceived /= 2;
		}

		//	在此调用事件函数，事件中必须处理完数据
		if( FALSE == pFileEvent->m_bIsSubFile )
		{
			if( m_bAutoSave )
				pFileEvent->m_pFileObject->SaveTo( m_strAutoSavePath, FALSE, TRUE );	//	保存文件
			SetDataPortReceiveLog( pFileEvent->m_pFileObject->m_pDataPortItem,
				pFileEvent->m_pFileObject->GetAttribute(), pFileEvent->m_pFileObject->GetDataLen(),
				true, pFileEvent->m_pFileObject->GetFileName() );

			Fire_OnFileOK( static_cast<IFileObject*>( pFileEvent->m_pFileObject), \
				(HDATAPORT)pFileEvent->m_pFileObject->m_pDataPortItem );
			m_dwByteReceived += pFileEvent->m_pFileObject->GetDataLen();		//	大文件不增加，由大文件的子文件增加
		}
		else
		{		//	SetDataPortReceiveLog will be called in function SaveHugeFile
			bool bFullHugeFileReceived = false;
			if( m_bAutoSave )
				bFullHugeFileReceived = SaveHugeFile( pFileEvent->m_pFileObject );
			if( m_bSendSubFileEvent )
			{
				Fire_OnSubFileOK( static_cast<IFileObject*>( pFileEvent->m_pFileObject),\
					(HDATAPORT)( pFileEvent->m_pFileObject->m_pDataPortItem ) );
			}
			m_dwByteReceived += pFileEvent->m_pFileObject->GetDataLen();			//	计算接收到的字节数
			if( bFullHugeFileReceived )
			{									//  2004-7-5 huge file is received
				OnHugeFileFullyReceived(  m_strHugeFileName, pFileEvent->m_pFileObject );
			}
		}

		int nFreeItemCount = GetItemCountInFreeList( FALSE );	//	2002.7.5 添加，当分配太多时，释放内存
		if( pFileEvent->m_pFileObject->IsBufAttaced() )
		{														//	2002.11.14 释放大文件的 Attached
			ASSERT( pFileEvent->m_pFileObject->m_pHugeFileParam );	//	一般来说，就是大文件
			DWORD dwBufLenTmp;
			pFileEvent->m_pFileObject->Detach( dwBufLenTmp );
		}
		pFileEvent->m_pFileObject->PresetVar();				//	2002.5.22 添加
		if( nFreeItemCount >= 30 )
		{														//	释放不必要的内存。
			pFileEvent->m_pFileObject->SetBufSize( 0 );
			DeAllocateEx( pFileEvent->m_pFileObject, TRUE );			//	释放文件对象
			pFileEvent->m_pFileObject->Release();
		}
		else
		{
			DeAllocate( pFileEvent->m_pFileObject );			//	释放文件对象
#ifdef _DEBUG
			ASSERT( pFileEvent->m_pFileObject->Release() > 0 );
#else
			pFileEvent->m_pFileObject->Release();
#endif //_DEBUG
		}

		m_FileNotifyEventList.DeAllocate( pFileEvent );		//	回归对象
		pFileEvent->Release();
	}
}


///-------------------------------------------------------
/// 2003-4-10
/// 功能：
///		获取 IP File Mend helper
/// 入口参数：
///
/// 返回参数：
///
IIPFileMendHelper * CDVBFileReceiver::GetIPFileMendHelper( HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( m_aDataPortItems.GetSize() );

	int nItemNo;
	int nDataItemCount = m_aDataPortItems.GetSize();
	if( 0 == nDataItemCount )
		return NULL;

	if( 1 == nDataItemCount )
		nItemNo = 0;					// 只有一个数据端口，不用进行查询
	else
	{									//	存在多个数据端口，放弃
		long nDataPortIndex = (long)hDataPort;
		if( nDataPortIndex < 100 && nDataPortIndex >= 0 && nDataPortIndex < nDataItemCount )
			nItemNo = (int)nDataPortIndex;		//	当成数据下标来使用
		else if( m_aDataPortItems.GetSize() > 1 )
			nItemNo = FindDataPort( hDataPort );
		if( nItemNo < 0 )
			return NULL;
	}

	ASSERT( nItemNo < m_aDataPortItems.GetSize() );
	COneDataPortItem * pDataPortItem = m_aDataPortItems[ nItemNo ];
	ASSERT( pDataPortItem );
#ifdef _WIN32
	ASSERT( FALSE == ::IsBadWritePtr( pDataPortItem, sizeof(COneDataPortItem) ) );
#endif //_WIN32

	if( NULL == pDataPortItem->m_pFileMendHelper )
		return NULL;

	IIPFileMendHelper * pRetVal = NULL;

	if( 0 != pDataPortItem->m_pFileMendHelper->QueryInterface(IID_IIPFileMendHelper, (void**)&pRetVal ) )
		return NULL;
	return pRetVal;
}

///-------------------------------------------------------
/// 2003-4-11
/// 功能：
///		创建一个数据端口对象
/// 入口参数：
///		无
/// 返回参数：
///		NULL				失败
///		其他				成功
COneDataPortItem * CDVBFileReceiver::NewOneDataPortItem()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	COneDataPortItem * pDataPortItem = NULL;
	if( m_aDelayDeleteItems.GetSize() )
	{										//	存在延迟删除的对象，我们可以重新使用
		pDataPortItem = m_aDelayDeleteItems[0];
#ifdef _WIN32
		ASSERT( FALSE == ::IsBadWritePtr(pDataPortItem,sizeof(COneDataPortItem) ) );
#endif //_WIN32
		m_aDelayDeleteItems.RemoveAt( 0 );
#ifdef _WIN32
		ConstructElements( pDataPortItem,1 );
#else
		MyConstructElements( pDataPortItem,1 );
#endif //_WIN32
	}
	else
		pDataPortItem = new COneDataPortItem;
	if( NULL == pDataPortItem )
		return NULL;

	CIPFileMendHelper * pInstance = new CIPFileMendHelper;
	if( NULL == pInstance )
			return NULL;
	pInstance->AddRef();
	pDataPortItem->m_pFileMendHelper = pInstance;

	return pDataPortItem;
}

//-----------------------------------------------
// on file oK
void CDVBFileReceiver::Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

#ifdef _DEBUG
	TRACE("Fire_OnFileOK is called. FileName=%s,Len=%d\n", pObject->GetFileName(), pObject->GetDataLen() );
#endif //_DEBUG

	if( NULL == m_pFileEventObject )
		return;

	pObject->AddRef();
	m_pFileEventObject->OnFileOK( pObject, hDataPort );
	pObject->Release();
}

//------------------------------------------------
// on sub-file oK
void CDVBFileReceiver::Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

#ifdef _DEBUG
	TRACE("Fire_OnSubFileOK is called. FileName=%s,Len=%d\n", pObject->GetFileName(), pObject->GetDataLen() );
#endif //_DEBUG

	if( NULL == m_pFileEventObject )
		return;

	pObject->AddRef();
	m_pFileEventObject->OnSubFileOK( pObject, hDataPort );
	pObject->Release();
}

//------------------------------------------------
// register event responeser object
void CDVBFileReceiver::RegisterEventResponser( IDVBReceiverEvent * pObject )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( FALSE == m_bSaveFileInBackgound )
	{
		if( m_pFileEventObject )
			m_pFileEventObject->Release();
		m_pFileEventObject = pObject;
		if( pObject )
			pObject->AddRef();
	}
	else
	{
		m_DelayEventDispatcher.SetHandler( pObject );
		if( pObject )
			m_pFileEventObject = static_cast<IDVBReceiverEvent*>( &m_DelayEventDispatcher );
		else
			m_pFileEventObject = NULL;
	}
}

void CDVBFileReceiver::DoMessagePump(void)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( FALSE == m_bSaveFileInBackgound )
		CheckAndSendFileEvent();
	else
		m_DelayEventDispatcher.DispatchEvents();
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		set ont channel receive status
/// Input parameter:
///		None
/// Output parameter:
///		TRUE				changed
///		false				not changed
bool CDVBFileReceiver::SetDataPortReceiveLog(COneDataPortItem * pDataPortItem, DWORD dwAttrib, DWORD dwFileLen, BOOL bIncFileCount, LPCSTR lpszFileName )
{
	ASSERT( pDataPortItem );
	if( NULL == pDataPortItem )
		return false;
	EQUFILEATTRIBUTE	equLen;
	equLen.m_dwData = (DWORD)dwAttrib;
	if( FALSE == equLen.m_dwHasExtData )
		return false;				//	不包含数据

	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;

	bool bIsChanged = FALSE;
	if( ReceiveLog.m_dwID != equLen.m_dwID )				//	更新
	{
#if defined(_DEBUG) || defined(_MYDEBUG)
		TRACE("LogID Changed, %X==>%X, NewFileLen=%d,%s=%d\n",\
			ReceiveLog.m_dwID, equLen.m_dwID, dwFileLen, \
			equLen.m_dwIsFileCount ? "Len" : "Count", equLen.m_dwValue );
#endif
		memset( &ReceiveLog, 0, sizeof(ReceiveLog) );
		bIsChanged = TRUE;
	}

	ReceiveLog.m_dwID = equLen.m_dwID;

	if( equLen.m_dwIsFileCount )
		ReceiveLog.m_dwFileCount = equLen.m_dwValue;
	else
		ReceiveLog.m_dwTotalLen_16KB = equLen.m_dwValue;

	ReceiveLog.m_dwOkLen_Below16KB += dwFileLen;
	ReceiveLog.m_dwOKLen_16KB += (ReceiveLog.m_dwOkLen_Below16KB >> 14);
	ReceiveLog.m_dwOkLen_Below16KB &= 0x3FFF;
	float fOldProgress = ReceiveLog.m_fProgress;
	if( ReceiveLog.m_dwTotalLen_16KB )
	{
		ReceiveLog.m_fProgress = float(ReceiveLog.m_dwOKLen_16KB) / ReceiveLog.m_dwTotalLen_16KB ;
		if( ReceiveLog.m_fProgress > 1 )
			ReceiveLog.m_fProgress = 1;
	}
	else
		ReceiveLog.m_fProgress = 0;

	if( bIncFileCount )
	{
		ReceiveLog.m_dwOKFileCount ++;
		if( ReceiveLog.m_dwOKFileCount > ReceiveLog.m_dwFileCount )
		{										//	防止溢出
			ReceiveLog.m_dwOKFileCount = 0;
			ReceiveLog.m_dwOKLen_16KB = 0;
		}
	}

	if( m_bSendProgressEvent && fOldProgress != ReceiveLog.m_fProgress )
		Fire_OnProgressEvent( pDataPortItem, lpszFileName );

	return bIsChanged;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		get one data port information
/// Input parameter:
///		hDataPort			IN		data port handle
///		dwBroLoopCount		out		output broadcast loop count
///		dwFileCount			OUT		total file count
///		dwTotalLen			OUT		total length, KB
///		dwByteReceived		OUT		byte received, KB
///		nCountReceived		OUT		file count received
/// Output parameter:
///		>=0					progress
///		<0					failed
float CDVBFileReceiver::GetProgressInfo( HDATAPORT hDataPort, DWORD & dwBroLoopCount, int & dwFileCount, DWORD & dwTotalLen, DWORD & dwByteReceived, int & nCountReceived )
{
	ASSERT( hDataPort );
	if( NULL == hDataPort )
		return -1;
	COneDataPortItem * pDataPortItem = (COneDataPortItem *) hDataPort;
	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;
	dwBroLoopCount = ReceiveLog.m_dwID;
	dwFileCount = ReceiveLog.m_dwFileCount;
	dwTotalLen = (ReceiveLog.m_dwTotalLen_16KB << 4);	// * 16K
	dwByteReceived = (ReceiveLog.m_dwOKLen_16KB << 4) + (ReceiveLog.m_dwOkLen_Below16KB>>10);
	nCountReceived = ReceiveLog.m_dwOKFileCount;

	return ReceiveLog.m_fProgress;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		put Send progress event
/// Input parameter:
///		bNewValue			new value
/// Output parameter:
///		None
BOOL CDVBFileReceiver::PutSendProgressEvent( BOOL bNewValue )
{
	BOOL bRetVal = m_bSendProgressEvent;
	m_bSendProgressEvent = bNewValue;
	return bRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		Get send progress event
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBFileReceiver::GetSendProgressEvent()
{
	return m_bSendProgressEvent;
}

///-------------------------------------------------------
/// CYJ,2004-5-20
/// Function:
///		call on progress event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::Fire_OnProgressEvent( COneDataPortItem * pDataPortItem, LPCSTR lpszFileName )
{
	ASSERT( pDataPortItem );
	if( NULL == pDataPortItem || FALSE == m_bSendProgressEvent )
		return;
	if( NULL == m_pFileEventObject )
		return;

	COneDataPortItem::FILERECEIVELOG & ReceiveLog = pDataPortItem->m_ReceiveLog;
	DWORD dwBroLoopCount = ReceiveLog.m_dwID;
	DWORD dwFileCount = ReceiveLog.m_dwFileCount;
	DWORD dwTotalLen = (ReceiveLog.m_dwTotalLen_16KB << 4);	// * 16K
	DWORD dwByteReceived = (ReceiveLog.m_dwOKLen_16KB << 4) + (ReceiveLog.m_dwOkLen_Below16KB>>10);
	DWORD nCountReceived = ReceiveLog.m_dwOKFileCount;
	float fProgress = ReceiveLog.m_fProgress;

	m_pFileEventObject->OnProgress( (HDATAPORT)pDataPortItem, fProgress, dwBroLoopCount,\
		dwFileCount, dwTotalLen, dwByteReceived, nCountReceived, lpszFileName );
}

////-------------------------------------------------------
/// CYJ,2004-5-26
/// Function:
///		On huge file fully received
/// Input parameter:
///		pszFileName					huge file name, full path
///		pFileObject					reference file object
/// Output parameter:
///		None
void CDVBFileReceiver::OnHugeFileFullyReceived(const char * pszFileName, CFileObject *pFileObject)
{
//	TRACE("OnHugeFileFullyReceived is called,%s\n", pszFileName);

	CMyMapFile	mapfile;
	if( m_bMapFileOnFireFileOKEvent && \
		FALSE == mapfile.MapFileForReadOnly( pszFileName ) )
	{
		return;
	}

	pFileObject->AddRef();		//	不能被释放，因为TmpFileObj引用其中的参数
	CFileObject TmpFileObj;

	TmpFileObj.m_strMC_DstIP = pFileObject->m_strMC_DstIP;					//	2002.11.14 修改，与多播相关的参数
	TmpFileObj.m_nMC_Port = pFileObject->m_nMC_Port;						//	多播端口
	TmpFileObj.m_pDataPortItem = pFileObject->m_pDataPortItem;	//  2004-5-20 data port item

	TmpFileObj.m_pHugeFileParam = pFileObject->m_pHugeFileParam;	//	大文件参数
	TmpFileObj.m_pExtData = pFileObject->m_pExtData;			//	附加参数
	TmpFileObj.m_pAttributeData = pFileObject->m_pAttributeData;	//	属性参数
	TmpFileObj.m_pFileHeader = pFileObject->m_pFileHeader;		//	TSDB 文件头
	TmpFileObj.m_strFileName = pFileObject->m_strFileName;		//	文件名
	TmpFileObj.m_PacketTime = pFileObject->m_PacketTime ;		//	文件播出打包时间
	TmpFileObj.m_FileHeadBuf.Copy( pFileObject->m_FileHeadBuf );		//	文件头缓冲区

	TmpFileObj.AddRef();			//	防止被释放
	DWORD dwFileLen = 0;
	if( m_bMapFileOnFireFileOKEvent )
	{
		dwFileLen = mapfile.GetFileLen();
		TmpFileObj.Attach( mapfile.GetBuffer(), dwFileLen );
		TmpFileObj.PutDataLen( dwFileLen );
	}

	Fire_OnFileOK( static_cast<IFileObject*>(&TmpFileObj), (HDATAPORT)pFileObject->m_pDataPortItem );

	if( m_bMapFileOnFireFileOKEvent )
		TmpFileObj.Detach( dwFileLen );

	pFileObject->Release();
}

///-------------------------------------------------------
/// CYJ,2004-5-26
/// Function:
///		Preset all vars
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::PresetVars()
{
	m_dwMaxPacketSize = 2048;		// 2K bytes
    m_bSendSubFileEvent = TRUE;
    m_bFileOKEventDone = TRUE;
    m_bIsOnLine = FALSE;
    m_bAutoSave = FALSE;			//	是否自动存盘

    m_dwByteReceived = 0;						//	已经接收到的文件字节数
    m_dwLastTickCount = time(NULL);			//	上次统计的时间, second
    m_dwLastFileBPS = 0;						//	上次统计的速率
    m_nTimer_2_Second = 0;						//	每 2 秒检测一次事件缓冲区
    m_pFileEventObject = NULL;
	m_bSendProgressEvent = FALSE;

	m_bMapFileOnFireFileOKEvent = TRUE;
}

///-------------------------------------------------------
/// CYJ,2004-7-31
/// Function:
///		Write thread 检测信号事件并保存到文件中
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::WriteThread_DoCheckAndSaveFiles()
{
	CheckAndSendFileEvent();
	m_DelayEventDispatcher.FlushAddCatch();
}

///-------------------------------------------------------
/// CYJ,2004-7-6
/// Function:
///		Get not map file on file OK event
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBFileReceiver::GetDotMapFileOnFileOKEvent()
{
	return m_bMapFileOnFireFileOKEvent;
}

///-------------------------------------------------------
/// CYJ,2004-7-6
/// Function:
///		set not map file on file ok event
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBFileReceiver::SetDoMapFileOnfileOKEvent( BOOL bNewValue )
{
	m_bMapFileOnFireFileOKEvent = bNewValue;
}
