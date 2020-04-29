///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-14
///
///=======================================================

// FileObject.cpp: implementation of the CFileObject class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.14 修改 IP 和 Port 的表示方式，添加函数 MulticastParameter

#include "stdafx.h"
#include "FileObject.h"
#include "TSDB_Rec.h"
#include "DirectroyHelp.h"
#include "IPData.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#else
  #include <utime.h>
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
   
CFileObject::CFileObject() :
	CBufPacket4C<IFileObject>( 0, 4096 )			//	以4K为单位进行分配
{
	m_nMC_Port = 0;						//	多播端口
	m_pHugeFileParam = NULL;	//	大文件参数
	m_pExtData = NULL;			//	附加参数
	m_pAttributeData = NULL;	//	属性附加参数
	m_pFileHeader = NULL;		//	文件头
	m_PacketTime = 0;
	m_pDataPortItem = NULL;		//  2004-5-20 add
}

CFileObject::~CFileObject()
{

}

void CFileObject::SafeDelete()
{
	delete this;
}

//////////////////////////////////////////////
///功能:
///			获取文件名
///入口参数:
///			pVal		执行 BSTR 类型地缓存区
///返回参数:
///			S_OK 成功
LPCSTR CFileObject::GetFileName()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_strFileName;
}

//////////////////////////////////////////////
///功能:
///			获取文件属性
///入口参数:
///			pVal		输出属性
///返回参数:
///			
///注：
//		pVal 应按 DWORD 解释
DWORD CFileObject::GetAttribute()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_dwAttribute;
	return 0;
}

//////////////////////////////////////////////
///功能:
///			获取最后修改时间
time_t CFileObject::GetLastModifyTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_LastWriteTime;
	return 0;
}

//////////////////////////////////////////////
///功能:
///		创建创建时间
///入口参数:
///		
///返回参数:
///		
///注：
///		
time_t CFileObject::GetCreatTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_CreateTime;
	return 0;
}

//////////////////////////////////////////////
///功能:
///		创建最后访问时间
///入口参数:
///		
///返回参数:
///		
///注：
///	
time_t CFileObject::GetLastAccessTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_LastAccessTime;
	return 0;
}


//////////////////////////////////////////////
///功能:
///			获取文件用途属性
///入口参数:
///		
///返回参数:
///		
DWORD CFileObject::GetFilePurpose()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( m_pAttributeData )
		return m_pAttributeData->m_dwPurpose;
	else
		return 0;
}

//////////////////////////////////////////////
///功能:
///			获取播出时地打包时间
///入口参数:
///		
///返回参数:
///		
time_t CFileObject::GetPacketTime()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	return m_PacketTime;
}

//////////////////////////////////////////////
///功能:
///		获取文件属性附加数据
///入口参数:
///		pdwLen		output ExtData Len, default is NULL
///返回参数:
///		NULL		there is no extern data
PBYTE CFileObject::GetAttributeExtData( PDWORD pdwLen)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	if( pdwLen )
	{
		if( m_pAttributeData )
			*pdwLen = m_pAttributeData->m_cbSize - sizeof(TSDBFILEATTRIBHEAD) + 1;
		else
			*pdwLen = 0;
	}
	return (PBYTE)m_pAttributeData;
}

//////////////////////////////////////////////
///功能:
///			获取文件附加数据
///入口参数:
///		
///返回参数:
///		
PBYTE CFileObject::GetExtData( PDWORD pdwLen )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( pdwLen )
	{
		if( m_pExtData )
		{
			ASSERT( m_pFileHeader );
			CTSDBFileHeader Helper( m_pFileHeader );
			*pdwLen = Helper.ExtDataLen();
		}
		else
			*pdwLen = 0;
	}

	return m_pExtData;
}

//////////////////////////////////////////////
///功能:
///		获取大文件参数
///入口参数:
///		
///返回参数:
///		
PBYTE CFileObject::GetHugeFileParam( PDWORD pdwLen )
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32    

	if( pdwLen )
	{
		if( m_pHugeFileParam )
			*pdwLen = m_pHugeFileParam->m_cbSize;
		else
			*pdwLen = 0;
	}
	return PBYTE( m_pHugeFileParam );
}

//////////////////////////////////////////////
///功能:
///			获取 IP Address
///入口参数:
///		
///返回参数:
///		
LPCSTR CFileObject::GetIPAddress()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
	return (LPCSTR)m_strMC_DstIP;
}

//////////////////////////////////////////////
///功能:
///			获取端口
///入口参数:
///		
///返回参数:
///		
int CFileObject::GetPort()
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif//_WIN32
	return m_nMC_Port;
}

//////////////////////////////////////////////
//功能:
//		保存文件
//入口参数:
//		lpszPath				主目录
//		bIgnorSubDirectroy		是否忽略子目录
//		bRestoreTime			是否还原时间
//返回参数:
//		
BOOL CFileObject::SaveTo(LPCSTR lpszPath, BOOL bIgnoreSubDirectory, BOOL bRestoreTimes)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	ASSERT( lpszPath && *lpszPath );
	if( NULL == lpszPath || 0 == *lpszPath )
		return FALSE;

	CString strFileName = lpszPath;
	int nLen = strFileName.GetLength();
#ifdef _WIN32
	if( strFileName[nLen-1] != '\\' )
		strFileName += '\\';				//	归一化
#else
	if( strFileName[nLen-1] != '/' )
		strFileName += '/';				//	归一化
#endif //_WIN32

	if( bIgnoreSubDirectory )
	{
		const char * pszFileName = strrchr( m_strFileName, '\\' );		//	从右至左
		if( NULL == pszFileName )
			pszFileName = strrchr( m_strFileName, '/' );				//	从右至左

		if( NULL == pszFileName )
			pszFileName = m_strFileName;
		else
			pszFileName ++;					//	跳过 '\\' or '/'
		strFileName += pszFileName;
	}
	else
		strFileName += m_strFileName;

	CDirectroyHelp::Mkdir( strFileName );

	UINT uCreateFlags = CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyWrite;

	BOOL bIsHugeSubFile = FALSE;	
	if( m_pFileHeader && m_pHugeFileParam && GetDataLen() != m_pHugeFileParam->m_dwFileLen )
	{										//	大文件，若是子文件，则需要定位
		bIsHugeSubFile = TRUE;				//	不是一个完整的大文件，是子文件
		uCreateFlags |= CFile::modeNoTruncate;		//	不截
	}

	CFile f;
	if( FALSE == f.Open( strFileName, uCreateFlags ) )
		return FALSE;
	TRY
	{
		if( bIsHugeSubFile )
		{
			ASSERT( m_pHugeFileParam );
			f.SetLength( m_pHugeFileParam->m_dwFileLen );
			f.Seek( m_pHugeFileParam->m_dwFilePosition, CFile::begin );
		}
		f.Write( GetBuffer(), GetDataLen() );
		
		if( bRestoreTimes && m_pAttributeData )
		{													//	还原时间
#ifdef _WIN32
			FILETIME fileTime[3];
			time_t attribtime[3] = { m_pAttributeData->m_CreateTime, m_pAttributeData->m_LastAccessTime, m_pAttributeData->m_LastWriteTime };
			FILETIME * pFileTime[3];
			SYSTEMTIME sysTmpTime;

			for(int i=0; i<3; i++)
			{
				CTime t = attribtime[i];
				if( t.GetAsSystemTime( sysTmpTime) )
				{
					pFileTime[i] = fileTime + i;
					::SystemTimeToFileTime( &sysTmpTime, pFileTime[i] );
				}
				else
					pFileTime[i] = NULL;
			}

			::SetFileTime( (HANDLE) f.m_hFile, pFileTime[0], pFileTime[1], pFileTime[2] );
            f.Close();
#else
			f.Close();
			struct utimbuf filet;
            filet.actime = m_pAttributeData->m_LastAccessTime;
            filet.modtime = m_pAttributeData->m_LastWriteTime;
			utime( strFileName, &filet );
#endif //_WIN32
		}
		else
			f.Close();
	}
	CATCH_ALL( e )
	{
#if	defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		f.Abort();
	}
	END_CATCH_ALL

	return TRUE;
}

//////////////////////////////////////////////
//功能:
//		从缓冲区中分析 TSDB 封装的参数
//入口参数:
//		无
//返回参数:
//		无
//注：
//		一般在调用该函数后，将触发 OnFileOK 事件
void CFileObject::DoTSDBSingleFile()
{
	m_pHugeFileParam = NULL;	//	大文件参数
	m_pExtData = NULL;			//	附加参数
	m_pAttributeData = NULL;	//	属性参数

	m_pFileHeader = (PTSDBFILEHEADER)GetBuffer();
	CTSDBFileHeader	hdr( m_pFileHeader );
	ASSERT( hdr.IsFileHead() );

	ASSERT( (m_pFileHeader->m_dwFileLen + m_pFileHeader->m_cbSize) == GetDataLen() );
	PutDataLen( m_pFileHeader->m_dwFileLen );
	Admin_AccessReservedBytes() += m_pFileHeader->m_cbSize;		//	跳到真正的数据

	if( hdr.HasFileAttarib() )
		m_pAttributeData = hdr.GetFileAttribHead();
	if( hdr.HasExtData() )
		m_pExtData = GetBuffer() + hdr.ExtDataLen();
	if( hdr.IsHugeFile() )
		m_pHugeFileParam = hdr.GetHugeFileHead();

	m_strFileName = hdr.GetFileName();
#ifdef _WIN32
	m_strFileName.Replace( '/', '\\' );
#else
	m_strFileName.Replace( '\\', '/' );
#endif //_WIN32
}

//////////////////////////////////////////////
//功能:
//		大文件接收成功时，特别设置文件头参数
//入口参数:
//		pFileHead			文件头参数块
//返回参数:
//		TRUE				成功
//		FALSE				失败
BOOL CFileObject::SetHugeFileFileHeader(PTSDBFILEHEADER pFileHead)
{
	ASSERT( pFileHead );

	m_pHugeFileParam = NULL;	//	大文件参数
	m_pExtData = NULL;			//	附加参数
	m_pAttributeData = NULL;	//	属性参数

	TRY
	{
		m_FileHeadBuf.SetSize( pFileHead->m_cbSize );
	}
	CATCH( CMemoryException, e )
	{
#if defined(_DEBUG) && defined(_WIN32)
		e->ReportError();
#endif // _DEBUG
		m_pFileHeader = NULL;
		return FALSE;
	}
	END_CATCH
#ifdef _WIN32
	RtlCopyMemory( m_FileHeadBuf.GetData(), pFileHead, pFileHead->m_cbSize );
#else
	memcpy( m_FileHeadBuf.GetData(), pFileHead, pFileHead->m_cbSize );
#endif //_WIN32
	m_pFileHeader = (PTSDBFILEHEADER) m_FileHeadBuf.GetData();

	CTSDBFileHeader	hdr( m_pFileHeader );
	ASSERT( hdr.IsFileHead() );

	if( hdr.HasFileAttarib() )
		m_pAttributeData = hdr.GetFileAttribHead();
	if( hdr.HasExtData() )
		m_pExtData = m_FileHeadBuf.GetData() + m_pFileHeader->m_cbSize - hdr.ExtDataLen();
	if( hdr.IsHugeFile() )
		m_pHugeFileParam = hdr.GetHugeFileHead();

	m_strFileName = hdr.GetFileName();
	return TRUE;
}

//////////////////////////////////////////////
// 2002.5.22 添加
// 功能:
//		预置参数值
// 入口参数:
//		无
// 返回参数:
//		无
void CFileObject::PresetVar()
{
	m_pHugeFileParam = NULL;	//	大文件参数
	m_pExtData = NULL;			//	附加参数
	m_pAttributeData = NULL;	//	属性参数
	m_pFileHeader = NULL;
	m_strMC_DstIP = "";			//	2002.11.14 修改，与多播相关的参数
	m_nMC_Port = 0;				//	多播端口 
	m_pDataPortItem = NULL;
}

///-------------------------------------------------------
/// 2002-11-14
/// 功能：
///		设置多播 IP 和端口
/// 入口参数：
///		lpszIP				多播 IP 地址
///		wPort				端口
/// 返回参数：
///		无
void CFileObject::SetMulticastParameter(LPCSTR lpszIP, WORD wPort, COneDataPortItem * pDataPortItem)
{
	ASSERT( lpszIP && wPort && pDataPortItem );
	if( lpszIP )
		m_strMC_DstIP = lpszIP;
	else
		m_strMC_DstIP = "";
	m_nMC_Port = wPort;
	m_pDataPortItem = pDataPortItem;
}
