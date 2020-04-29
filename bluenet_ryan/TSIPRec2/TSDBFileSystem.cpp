///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-14
///
///=======================================================

// TSDBFileSystem.cpp: implementation of the CTSDBFileSystem class.
//
//////////////////////////////////////////////////////////////////////
//
// 2002.11.14	修改 ProcessSingleFile 和 ProcessHugeFile，赋值多播 IP : Port
// 2002.5.22	添加，是否允许非 TSDB 文件通过
// 2001.9.21	修改 ProcessHugeFile，允许不保存大文件
// 2001.8.17	修改 ProcessSingleFile 函数，释放内存
// 2001.8.6		添加函数 ProcessOneFileInMem，处理已在内存中的文件
// 2001.3.16	添加成员数据m_dwByteRecevied，用来获取实际接收的速率
// 2000.10.28	从 CTSDBDrv 中分离出来
//		

#include "stdafx.h"
#include "Resource.h"
#include "TSDBFileSystem.h"
#include "TSDB_Rec.h"
#include "UnCmpMgr.h"
#include "FileUpdate.h"
#include "FilePurpose.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBFileSystem::CTSDBFileSystem()
{
	m_dwByteRecevied = 0;
	m_pOneBaseFile = NULL;
	m_bIsEnableNotTSDBFile = FALSE;
}

CTSDBFileSystem::~CTSDBFileSystem()
{
}

//	处理内存中的文件
void CTSDBFileSystem::ProcessOneFile( CMB_OneFile * pOneFile )
{
	ASSERT( pOneFile );
	if( FALSE == m_bIsEnableNotTSDBFile && pOneFile->m_dwByteRead < pOneFile->m_dwFileLen )
		return;										//	2002.5.22 添加，TSDB 封装的文件，一定需要正确的数据

	m_pOneBaseFile = pOneFile;
	PBYTE pBuf = pOneFile->GetBuffer();
	if( CTSDBMultiFileHeader::IsMultiFileHeader( pBuf ) )
	{
		if( pOneFile->m_dwFileLen == pOneFile->m_dwByteRead )
			ProcessMultiFile( (PTSDBMULFILEHEAD)pBuf, pOneFile );			//	是多个文件
	}
	else
		ProcessSingleFile( pBuf, pOneFile->GetDataLen(), pOneFile );	//	处理单个文件, 可能普通文件, 也可能是 TSDB 的单个文件文件
	m_pOneBaseFile = NULL;
}

//	处理多个文件
void CTSDBFileSystem::ProcessMultiFile(PTSDBMULFILEHEAD pMultiHeader, CMB_OneFile * pOneFile)
{
	ASSERT( CTSDBMultiFileHeader::IsMultiFileHeader( pMultiHeader ) );
	CTSDBMultiFileHeader	hdr( pMultiHeader );
	int nNum = hdr.GetFileNum();
	for(int i=0; i<nNum; i++)
	{
		ProcessSingleFile( (PBYTE) &hdr[i], hdr[i].m_dwFileLen, pOneFile );
	}
}

//	处理单个文件
//	修改记录：
//		2002.11.14 赋值多播 IP : Port
void CTSDBFileSystem::ProcessSingleFile(PBYTE pBuffer,DWORD dwLen, CMB_OneFile * pBaseFile)
{
PBYTE	pUncmpAutoAllocBuf = NULL;
//	if( CTSDBUnlock::IsLock( pBuffer ) ) 
//	{
//	在此调用解密
//		UnlockData();		
//	}
	ASSERT( m_pOneBaseFile );
	CFileObject * pOneFile = AllocatePacket();
	if( NULL == pOneFile )									//	分配文件对象失败
		return;

	ASSERT( pBaseFile->m_wMC_Port && FALSE == pBaseFile->m_strMC_DstIP.IsEmpty() );
	pOneFile->SetMulticastParameter( pBaseFile->m_strMC_DstIP, \
		pBaseFile->m_wMC_Port, pBaseFile->m_pDataPortItem );	// 2002.11.14 添加
	
	BOOL bIsCompleteReceived = ( pBaseFile->m_dwByteRead == pBaseFile->m_dwFileLen );
#ifdef _DEBUG
	if( FALSE == bIsCompleteReceived )
		TRACE("One file %08X is changed.\n", pBaseFile );
#endif // _DEBUG

	int	nSrcFileLen = dwLen;

	if( CUnCmpMgr::IsCompress( nSrcFileLen, pBuffer ) )
	{														//	判断是否 TSDB 的压缩文件
		if( FALSE == bIsCompleteReceived || m_UnCompressSvr.Attach( nSrcFileLen,pBuffer ) == 0 )
		{													//	既然压缩，没有完整接收一定是失败
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;											//	解压失败
		}
		ASSERT( m_UnCompressSvr.GetFileNum() == 1 );
		CFileStatus outfStatus;
		if( 0 == m_UnCompressSvr.GetFileInfo( 0, outfStatus ) ||\
			FALSE == pOneFile->SetBufSize( outfStatus.m_size ) )
		{													//	失败
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;
		}
		pBuffer = m_UnCompressSvr.DecodeOneFile( 0,outfStatus, pOneFile->GetBuffer() );
		if( pBuffer == NULL )
		{
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;											//	解压失败
		}
		pOneFile->PutDataLen( outfStatus.m_size );
		nSrcFileLen = outfStatus.m_size;
	}
	else
	{
		if( FALSE == pOneFile->SetBufSize( nSrcFileLen ) )
		{
			DeAllocate( pOneFile );
			pOneFile->Release();
			return;
		}
		memcpy( pOneFile->GetBuffer(), pBuffer, nSrcFileLen );
		pOneFile->PutDataLen( nSrcFileLen );
	}

	if( CTSDBFileHeader::IsFileHead( pBuffer, pOneFile->GetDataLen() ) )
	{
		if( FALSE == bIsCompleteReceived || FALSE == ProcessTSDBSingleFile( pOneFile ) )		//	处理单个文件
		{						//	没有完整接收，一定失败，也可能由于其他原因，放弃处理该文件，需要释放
			DeAllocate( pOneFile );
			pOneFile->Release();
		}
	}
	else
	{														//	普通文件
		if( m_bIsEnableNotTSDBFile )
		{
			pOneFile->m_strFileName = pBaseFile->m_szFileName;
			NotifyOnFileOKEvent( pOneFile );		
		}
	}
}

//////////////////////////////////////////////
//功能:
//		处理 TSDB 的单个文件
//入口参数:
//		pOneFile		文件对象，缓冲区中为文件数据
//返回参数:
//		TRUE			该文件对象已被提交处理，不用 DeAllocate 释放
//		FALSE			由于某种原因，没有被处理，调用者需要 DeAllocate 来释放对象
BOOL CTSDBFileSystem::ProcessTSDBSingleFile( CFileObject * pOneFile )
{
	ASSERT( pOneFile );
	PTSDBFILEHEADER pFileHead = (PTSDBFILEHEADER)pOneFile->GetBuffer();
	CTSDBFileHeader	hdr( pFileHead );
	if( hdr.IsHugeFile() )
		return ProcessHugeFile( pOneFile, hdr );								//	大文件

	pOneFile->DoTSDBSingleFile();				//	提取参数

	if( hdr.HasFileAttarib() && IsSysReservFile(hdr,pOneFile->m_pAttributeData->m_dwPurpose) )
	{														//	发送文件属性
		PTSDBFILEATTRIBHEAD		pAttrib = hdr.GetFileAttribHead();
		CFileStatus fsta;
		int nFilePurpose = pAttrib->m_dwPurpose;
		fsta.m_attribute = (BYTE)pAttrib->m_dwAttribute;
		fsta.m_mtime = CTime( pAttrib->m_LastWriteTime );
		strcpy(fsta.m_szFullName, hdr.GetFileName() );
		PBYTE pDataBuf = hdr.GetDataBuf();						//	获取数据文件
		int nFileLen = hdr.GetFileLen();
		if( ProcessSysReservFile( hdr, pDataBuf, fsta, nFilePurpose ) )
			return FALSE;										//	不再通知应用程序, 内部已经处理
	}
	NotifyOnFileOKEvent( pOneFile );
	return TRUE;
}

//	处理升级程序
//	入口参数
//		hdr						文件头
//		pBuf					数据缓冲区
//		fsta					文件状态结构
//		nFilePurpose			用途
//	返回参数
//		TRUE					系统的升级文件,不须再通知应用程序
//		FALSE					其他文件
BOOL CTSDBFileSystem::ProcessUpdateFile(CTSDBFileHeader &hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose)
{
BOOL bRetVal = FALSE;
	int nExtLen = hdr.ExtDataLen();
	PBYTE pExtDataBuf = new BYTE[ (nExtLen+4095)&(~4095) ];
	if( pExtDataBuf )								//	升级软件
	{
		hdr.CopyExtData( pExtDataBuf, nExtLen );
		switch( nFilePurpose )
		{
		case TSDB_FP_UPDATE_SYSFILE:				//	系统文件的升级
        	ASSERT( FALSE );
//			CFileUpdate::Update( fsta.m_size, pBuf, pExtDataBuf , nExtLen );
			bRetVal = TRUE;
			break;

		case TSDB_FP_UPDATE_UPDATENOW:				//	立即升级的程序, 即解密程序
			{
/*				CString strFileName = CFileUpdate::UpdateUnlockProc( fsta.m_size, pBuf, pExtDataBuf , nExtLen );
				if( strFileName.IsEmpty() == FALSE )
					gUnlockSvr.GetProcMgr().AddObj( strFileName );				
*/
				bRetVal = TRUE;
			}
			break;

		case TSDB_FP_UPDATE_APP:					//	OEM 等应用程序的升级
			{
				PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
				CString strTmp;
				strTmp.Format("%s\\%s /%s",pItem->szPath, pItem->szFileName, pItem->szOemName );
				strncpy( fsta.m_szFullName, strTmp, _MAX_PATH );
			}
			break;
		}
		delete pExtDataBuf;
	}
	return bRetVal;
}

//	处理系统保留的文件
//	入口参数
//		hdr						文件头
//		pBuf					数据缓冲区
//		fsta					文件状态结构
//		nFilePurpose			用途
//	返回参数
//		TRUE					系统内部处理文件,不须再通知应用程序
//		FALSE					其他文件
BOOL CTSDBFileSystem::ProcessSysReservFile(CTSDBFileHeader &hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose)
{
	ASSERT( IsSysReservFile( hdr, nFilePurpose ) );

	if( nFilePurpose >= 0x80000000 || nFilePurpose  == 0 )		//	用户自定义类型
		return FALSE;
	switch( nFilePurpose )
	{
	case TSDB_FP_UPDATE_SYSFILE:
	case TSDB_FP_UPDATE_UPDATENOW:
	case TSDB_FP_UPDATE_APP:
		if( hdr.HasExtData() )
			ProcessUpdateFile( hdr, pBuf, fsta, nFilePurpose );
		else
			return FALSE;						//	错误的类型
		break;

	case TSDB_FP_LICENCE_MSG:					//	授权文件
//		gUnlockSvr.ProcessLicData( pBuf, fsta.m_size );
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////
//功能:
//		判断是否为系统保留的文件
//入口参数:
//		hdr				文件头
//		nFilePurpose	文件属性
//返回参数:
//		TRUE			系统保留的文件，需要处理
//		FALSE			应用程序
BOOL CTSDBFileSystem::IsSysReservFile(CTSDBFileHeader &hdr, DWORD nFilePurpose)
{
	if( nFilePurpose >= 0x80000000 || nFilePurpose  == 0 )		//	用户自定义类型
		return FALSE;
	switch( nFilePurpose )
	{
	case TSDB_FP_UPDATE_SYSFILE:
	case TSDB_FP_UPDATE_UPDATENOW:
	case TSDB_FP_UPDATE_APP:
		if( hdr.HasExtData() )
			return TRUE;
		else
			return FALSE;						//	错误的类型

	case TSDB_FP_LICENCE_MSG:					//	授权文件
		return TRUE;

	default:
		return FALSE;
	}
	return FALSE;
}

//	处理大文件
//	入口参数
//		FileHead						TSDB 文件
//	修改记录：
//		2002.11.14 赋值多播 IP : Port
BOOL CTSDBFileSystem::ProcessHugeFile( CFileObject * pOneFile,CTSDBFileHeader &FileHead )
{
	PTSDBHUGEFILEHEAD pHugeHead = FileHead.GetHugeFileHead();
	ASSERT( pHugeHead );
	BOOL bIsFileOK = FALSE;	

	BOOL bIsSysReservedFile = FALSE;
	if( FileHead.HasFileAttarib() && IsSysReservFile(FileHead, FileHead.GetFileAttribHead()->m_dwPurpose) )
		bIsSysReservedFile = TRUE;

	BOOL bOneFileIsSummit = FALSE;					//	标识该文件对象是否提交，若没有提交，则调用者需要 DeAllocate
	if( FALSE == bIsSysReservedFile )
	{
		pOneFile->DoTSDBSingleFile();				//	提取参数
		ASSERT( pOneFile->GetDataLen() == pHugeHead->m_wBlockSize );
		NotifySubFileOKEvent( pOneFile );			//	只有非系统保留文件，才通知子文件接收到子文件
		bOneFileIsSummit = TRUE;
	}

	return bOneFileIsSummit;
}

//	解密数据
//	入口参数
//		pBuffer				文件缓冲区
//		outLen				输出长度
//		dwSysCodeIndex		输出系统密码索引
//	返回参数
//		NULL				失败
//		其它				缓冲区指针
PBYTE CTSDBFileSystem::UnlockData()
{	
	ASSERT( FALSE );
	return NULL;
}


