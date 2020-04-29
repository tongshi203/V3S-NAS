// TSDBHugeFileObj.cpp: implementation of the CTSDBHugeFileObj class.
//
//						修改纪录
//	修改时间			内容
//////////////////////////////////////////////////////////////////////
//	2002.3.29			添加 AddRef 和 Release 函数
//	2001.4.5			tagFLAGFILE 添加数据项 m_bIsOpen
//						~CTSDBHugeFileObj 关闭大文件
//						SetOwnerHandle 增加判断大文件是否正常关闭
//						IsFileOK 添加对文件CRC32的判断
//	1999.12.19			添加清空大文件临时缓冲目录


#include "stdafx.h"
#include "resource.h"
#include "TSDBHugeFileObj.h"
#include "DirectroyHelp.h"

#ifdef _WIN32
  #include "MyRegKey.h"

  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBHugeFileObj::CTSDBHugeFileObj()
{
	m_nRef = 0;
	Init();
}

//	入口参数
//		pHeader						数据头
CTSDBHugeFileObj::CTSDBHugeFileObj(PTSDBHUGEFILEHEAD pHeader)
{
	Attach( pHeader );
}

//	关闭映射文件
//	关闭文件
CTSDBHugeFileObj::~CTSDBHugeFileObj()
{
	if( m_bIsOwner )							//	原来的主人, 释放使用权
	{
		m_pFlagBuf->m_bHasOwner = FALSE;			
		m_pFlagBuf->m_bCloseErr = FALSE;		//	2001.4.5 标记关闭
	}
	if( m_pFlagBuf )
		::UnmapViewOfFile(m_pFlagBuf);
	if( m_pDataBuf )
		::UnmapViewOfFile( m_pDataBuf );
	if( m_hmapFlagFile )
		::CloseHandle( m_hmapFlagFile );
	if( m_hmapDataFile )
		::CloseHandle( m_hmapDataFile );
	if( m_DataFile.m_hFile != CFile::hFileNull  )
		m_DataFile.Close();
	if( m_FlagFile.m_hFile != CFile::hFileNull  )
		m_FlagFile.Close();
}

//	初始化数据
void CTSDBHugeFileObj::Init()
{
	m_LastAccessTime = 0;
	m_bMsgSended = FALSE;						//	未曾发送过消息
	m_bIsOwner = FALSE;
	m_pFlagBuf = NULL;							//	标记文件缓冲区
	m_pDataBuf = NULL;							//	数据文件缓冲区
	m_hmapFlagFile = NULL;						//	标志文件映射句柄
	m_hmapDataFile = NULL;						//	数据文件映射句柄
}

//	将数据头附着到对象
//	入口参数
//		pHeader					数据头
//	返回参数
//		TRUE					成功
//		FALSE					失败
BOOL CTSDBHugeFileObj::Attach(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader );
	Init();
	if( !pHeader || pHeader->m_dwFileLen==0 || pHeader->m_szTmpFileName[0]==0 )
		return FALSE;					//	非法指针 或 文件长度=0 或 没有临时文件名
	int nFlagLen = (pHeader->m_wTotalBlock+7) / 8;		//	计算子文件记录标记缓冲区大小
	nFlagLen += sizeof( FLAGFILE );

	CMyRegKey regkey(HKEY_LOCAL_MACHINE,"Software");
	CString	strWorkPath =  regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
	strWorkPath += "\\TEMP\\HUGEFILE\\";
	CDirectroyHelp::Mkdir( strWorkPath );				//	创建目录

	m_pDataBuf = CreateAndMapFile(strWorkPath + pHeader->m_szTmpFileName,\
		pHeader->m_dwFileLen, m_DataFile, m_hmapDataFile );
	strWorkPath += pHeader->m_szTmpFileName;
	strWorkPath += ".REC";								//	标记
	m_pFlagBuf = (PFLAGFILE) CreateAndMapFile(strWorkPath,nFlagLen, m_FlagFile, m_hmapFlagFile );
	if( m_pDataBuf == NULL || m_pFlagBuf == NULL )
		return FALSE;
	memcpy( &m_pFlagBuf->m_Head, pHeader, sizeof(TSDBHUGEFILEHEAD) );
	if( m_pFlagBuf->m_bHasOwner == FALSE || m_pFlagBuf->m_bCloseErr )
	{
		m_pFlagBuf->m_bHasOwner = FALSE;
		m_bIsOwner = SetOwnerHandle();
	}
	return TRUE;
}

//	创建并映射
//	入口参数
//		pszFileName				文件名
//		dwFileLen				文件长度
//		file					CFile	对象
//		hOut					输出 CreateFileMapping 的句柄
//	输出参数
//		NULL					失败
PBYTE CTSDBHugeFileObj::CreateAndMapFile(LPCSTR pszFileName, DWORD dwFileLen, CFile &file, HANDLE &hOut)
{
BOOL	bIsCreate = FALSE;
	hOut = NULL;
	ASSERT( pszFileName && dwFileLen );
	if( file.Open(pszFileName,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::typeBinary|CFile::shareDenyNone) == FALSE )
		return NULL;
	for(int i=0; i<2; i++)
	{
		try
		{
			if( file.GetLength() != dwFileLen )
			{
				file.SetLength( dwFileLen );
				bIsCreate = TRUE;
			}
		}
		catch( CFileException * e )
		{
			e->Delete();
			ClearHugeFileTmpBuf();			//	没有空间,需要整理空间
			if( i )
			{
				file.Close();
				return NULL;				//	已经重试过了,只好放弃
			}
		}
	}
	hOut = ::CreateFileMapping( (HANDLE)file.m_hFile,NULL,PAGE_READWRITE,0,0,NULL );
	if( hOut == NULL )
		return NULL;
	PBYTE pRetVal = (PBYTE)::MapViewOfFile(hOut,FILE_MAP_ALL_ACCESS,0,0,0 );
	if( pRetVal && bIsCreate )
		memset( pRetVal,0,dwFileLen );						//	输出化数据
	return pRetVal;
}

//	设置 Owner 属性
BOOL CTSDBHugeFileObj::SetOwnerHandle()
{
	ASSERT(m_pFlagBuf);
	ASSERT( m_pFlagBuf->m_bHasOwner == FALSE );

	CString	strTmp = "Mutex_TS_HUGEFILE_";
	strTmp += m_pFlagBuf->m_Head.m_szTmpFileName;
	HANDLE hMutex = CreateMutex( NULL,FALSE, strTmp );
	if( hMutex == NULL )
		return FALSE;									//	创建失败
	BOOL bIsOwner = ( GetLastError() != ERROR_ALREADY_EXISTS );
	if( bIsOwner )
	{
		if( m_pFlagBuf->m_bCloseErr )					//	2001.4.5 添加
		{												//	说明上次没有正确关机
			m_pFlagBuf->m_dwBlockReceived = 0;
			int nFlagBytes = (m_pFlagBuf->m_Head.m_wTotalBlock+7) / 8;
			memset( m_pFlagBuf->m_abyFlags, 0, nFlagBytes );
		}
		m_pFlagBuf->m_bHasOwner = TRUE;					//	标记为主人地位
		m_pFlagBuf->m_bCloseErr = TRUE;					//	2001.4.5  标记已经打开
	}
	::CloseHandle( hMutex );
	return bIsOwner;
}

//	大文件是否接收 OK
BOOL CTSDBHugeFileObj::IsFileOK()
{
	ASSERT( m_pFlagBuf && m_pFlagBuf );
	if( NULL == m_pFlagBuf || NULL == m_pFlagBuf)
		return FALSE;
	ASSERT( m_pFlagBuf->m_dwBlockReceived <= m_pFlagBuf->m_Head.m_wTotalBlock );
	if( m_pFlagBuf->m_dwBlockReceived < m_pFlagBuf->m_Head.m_wTotalBlock )
		return FALSE;
													//	2001.4.5 添加对文件CRC32的判断
	if( CCRC::GetCRC32( m_pFlagBuf->m_Head.m_dwFileLen, GetDataBuf() ) == m_pFlagBuf->m_Head.m_dwFileCRC32 )
		return TRUE;
	if( m_bIsOwner )
	{												//	2001.4.5 若是主人，修改错误
		m_pFlagBuf->m_dwBlockReceived = 0;
		int nFlagBytes = (m_pFlagBuf->m_Head.m_wTotalBlock+7) / 8;
		memset( m_pFlagBuf->m_abyFlags, 0, nFlagBytes );
	}
	return FALSE;	
}

//	取大文件的接收百分比
float CTSDBHugeFileObj::GetPercentage()
{
	ASSERT( m_pFlagBuf );
	if( !m_pFlagBuf || m_pFlagBuf->m_Head.m_wTotalBlock == 0 )
		return 0.0f;
	float f0 = (float)m_pFlagBuf->m_dwBlockReceived;
	f0 /= m_pFlagBuf->m_Head.m_wTotalBlock;
	return f0;
}

//	该子文件是否接收
//	入口参数
//		nBlockNo				子文件序号
//	返回参数
//		TRUE					成功
//		FALSE					失败
BOOL CTSDBHugeFileObj::IsBlockOK(int nBlockNo)
{
	ASSERT( m_pFlagBuf );
	int nOffset = nBlockNo / 8;
	BYTE byMask = 1 << (nBlockNo & 7);
	return ( m_pFlagBuf->m_abyFlags[ nOffset ] & byMask );
}

//	设置指定文件
//	入口参数
//		nBlockNo				子文件序号
//	返回参数
//		文件是否接收成功
BOOL CTSDBHugeFileObj::SetBlockNo(int nBlockNo)
{
	ASSERT( m_pFlagBuf && m_bIsOwner);
	ASSERT( nBlockNo < m_pFlagBuf->m_Head.m_wTotalBlock );
	int nOffset = nBlockNo / 8;
	BYTE byMask = 1 << (nBlockNo & 7);
	if( (m_pFlagBuf->m_abyFlags[ nOffset ] & byMask ) == 0 )
	{												//	未曾接收到
		m_pFlagBuf->m_abyFlags[ nOffset ] |= byMask;
		m_pFlagBuf->m_dwBlockReceived ++;
	}
	ASSERT( m_pFlagBuf->m_dwBlockReceived <= m_pFlagBuf->m_Head.m_wTotalBlock );
	return IsFileOK();								//	2001.4.5 修改
}

//	保存一个子文件
//	入口参数
//		pHead					数据头
//		pDataBuf				数据缓冲区
//	返回参数
//		文件是否接收成功
BOOL CTSDBHugeFileObj::SaveBlock(PTSDBHUGEFILEHEAD pHead, PBYTE pDataBuf)
{
	ASSERT( pHead && pDataBuf );
	ASSERT( m_pDataBuf );
	ASSERT( pHead->m_dwFileCRC32 == m_pFlagBuf->m_Head.m_dwFileCRC32 );
	if( IsFileOK() )
		return TRUE;
	m_LastAccessTime = CTime::GetCurrentTime().GetTime();			//	更新访问时间
	if( m_bIsOwner == FALSE && m_pFlagBuf->m_bHasOwner == FALSE )	//	原来的主人退出, 要新建立主人
		m_bIsOwner = SetOwnerHandle();
	if( m_bIsOwner )
	{								//	是主人, 有权限更改数据
		memcpy( m_pDataBuf + pHead->m_dwFilePosition, pDataBuf, pHead->m_wBlockSize );
		return SetBlockNo( pHead->m_wBlockNo );
	}
	else
		return IsFileOK();			//	非主人, 只有查询的份了
}

//	取文件缓冲区
//	若 m_pDataBuf != NULL 表示创建成功
PBYTE CTSDBHugeFileObj::GetDataBuf()
{
	return m_pDataBuf;
}

//	取文件长度
DWORD CTSDBHugeFileObj::GetFileLen()
{
	ASSERT( m_pFlagBuf );
	return m_pFlagBuf->m_Head.m_dwFileLen;
}

//	是否同一个大文件
//	入口参数
//		pHeader					数据头
BOOL CTSDBHugeFileObj::IsSameObj(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader && m_pFlagBuf );
	if( pHeader->m_dwFileCRC32 != m_pFlagBuf->m_Head.m_dwFileCRC32 )
		return FALSE;
	return( stricmp( pHeader->m_szTmpFileName, m_pFlagBuf->m_Head.m_szTmpFileName ) == 0 );
}

//	清空大文件临时缓冲目录
void CTSDBHugeFileObj::ClearHugeFileTmpBuf()
{
	CFileFind	finder;
	CMyRegKey	regkey( HKEY_LOCAL_MACHINE, "Software" );
	CString strPath = regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
	strPath += "\\Temp\\HugeFile";
	if( finder.FindFile( strPath+"\\*.*" ) == FALSE )
		return;
	BOOL bFindNext = TRUE;
	CString strTmp;
	do
	{
		bFindNext = finder.FindNextFile();
		if( finder.IsDots() == FALSE )
		{
			strTmp = finder.GetFilePath();
			if( strTmp.IsEmpty() == FALSE )
				remove( strTmp );						//	删除文件
		}
	}while( bFindNext );
}

//////////////////////////////////////////////
// 2002.3.29	添加
//功能:
//		增加引用计数器
//入口参数:
//		无
//返回参数:
//		引用次数
long CTSDBHugeFileObj::AddRef()
{
	return ::InterlockedIncrement( &m_nRef );
}

//////////////////////////////////////////////
// 2002.3.29	添加
//功能:
//		减少引用计数器，当减至 0 的时候，删除自己
//入口参数:
//		无
//返回参数:
//		当前引用计数器
long CTSDBHugeFileObj::Release()
{
	if( ::InterlockedDecrement( &m_nRef ) )
		return m_nRef;
	delete this;
	return 0;
}
