///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-9
///
///=======================================================
//  2003-6-9 修改函数 SetFilePathParameters 中，强制将 m_nLogFileNo = -1

// LogFileHelper.cpp: implementation of the CLogFileHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFileHelper.h"

#ifdef _WIN32
	#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
	#endif
#else
	#include <sys/stat.h>
    #include <sys/types.h>
#endif //_WIN32

#define _MAX_PATH	260    

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///-------------------------------------------------------
/// 2002-11-9
/// 功能：
///		
/// 入口参数：
///		lpszFullPath			全路径，包括log文件名，不能包括扩展名。
///								扩展名为 .LOG
///								缺省为 NULL，则默认为当前目录的 log 子目录下
///		nMaxCount				文件数，缺省为 10
///		nLenGate				切换文件的门槛长度，缺省为 1M
/// 返回参数：
///
CLogFileHelper::CLogFileHelper(LPCSTR lpszFullPath, int nMaxCount, int nLenGate)
{
	m_nLogFileNo = -1;
	SetFilePathParamters( lpszFullPath, nMaxCount, nLenGate );
}

CLogFileHelper::~CLogFileHelper()
{

}

///-------------------------------------------------------
/// 2002-11-9
/// 功能：
///		
/// 入口参数：
///		lpszFullPath			全路径，包括log文件名，不能包括扩展名。
///								扩展名为 .LOG
///								缺省为 NULL，则默认为当前目录的 log 子目录下
///		nMaxCount				文件数，缺省为 10
///		nLenGate				切换文件的门槛长度，缺省为 1M
/// 返回参数：
///		无
void CLogFileHelper::SetFilePathParamters(LPCSTR lpszFullPath, int nMaxCount, int nLenGate)
{
	ASSERT( nMaxCount && nLenGate );

	m_nLogFileNo = -1;			//  2003-6-9 从头开始记录

	if( lpszFullPath && strlen(lpszFullPath) )
	{
		m_strFullPath = lpszFullPath;
		int nLen = m_strFullPath.GetLength();
#ifdef _WIN32
		if( nLen && m_strFullPath[nLen-1] == '\\' )
			m_strFullPath.ReleaseBuffer( nLen-1 );		//	最后不能以'\\'结尾
#else
		if( nLen && m_strFullPath[nLen-1] == '/' )
			m_strFullPath.ReleaseBuffer( nLen-1 );		//	最后不能以'\\'结尾
#endif //_WIN32
	}
	else
	{
		char szDir[_MAX_PATH];
		char szPath[_MAX_PATH];
		char szFileName[_MAX_PATH];
#ifdef _WIN32
		_splitpath( __argv[0], szDir, szPath, szFileName, NULL );
		m_strFullPath.Format( "%s%sLog\\%s", szDir, szPath, szFileName );

        CString strTmp;
		strTmp.Format("%s%sLog", szDir, szPath );
		::CreateDirectory( strTmp, NULL );
#else
		m_strFullPath = "Log/LogData";
        mkdir( "Log", 0 );
#endif //_WIN32
	}	

	if( nLenGate > 10*1024 )
		m_nLenGate = nLenGate;
	else
		m_nLenGate = 10*1024;

	if( nMaxCount > 1 )
		m_nMaxCount = nMaxCount;
	else
		m_nMaxCount = 10;

	Abort();							//	关闭当前文件
}

///-------------------------------------------------------
/// 2002-10-22
/// 功能：
///		打开日志文件
/// 入口参数：
///		无
/// 返回参数：
///		TRUE				成功
///		FALSE				失败
BOOL CLogFileHelper::OpenLogFile()
{
#ifdef _WIN32
	TRY
	{
#endif //_WIN32
		if( m_hFile != CFile::hFileNull )
			Close();
#ifdef _WIN32
	}
	CATCH( CFileException, e )
	{
		Abort();
	}
	END_CATCH
#endif //_WIN32    

	m_nLogFileNo ++;
	if( m_nLogFileNo >= m_nMaxCount || m_nLogFileNo < 0 )
		m_nLogFileNo = 0;

	CString strTmp = GetCurrentLogFileName();
	return Open( strTmp, CFile::modeCreate|CFile::modeWrite|CFile::typeText|CFile::shareDenyWrite );
}

///-------------------------------------------------------
/// 2002-11-9
/// 功能：
///		写入 Log 文件中
/// 入口参数：
///		strLog			待写入的字符串
/// 返回参数：
///		无
///	注：
///		strLog 中需要加入换行回车
void CLogFileHelper::WriteToLogFile(LPCSTR lpszLog)
{
	if( m_hFile == CFile::hFileNull )
	{
		if( FALSE == OpenLogFile() )
			return;
	}
#ifdef _WIN32
	TRY
	{
#endif //_WIN32
		WriteString( lpszLog );
		if( GetLength() >= 1024*1024 )	//	1M
			OpenLogFile();			//	关闭当前Log文件，并打开新的Log文件
#ifdef _WIN32
	}
	CATCH( CFileException, e)
	{
		return;
	}
	END_CATCH
#endif //_WIN32    
}

///-------------------------------------------------------
/// 2002-11-9
/// 功能：
///		获取当前log文件名
/// 入口参数：
///		无
/// 返回参数：
///		当前文件名
CString CLogFileHelper::GetCurrentLogFileName()
{
	CString strTmp;
	strTmp.Format("%s%d.LOG", (LPCSTR)m_strFullPath, m_nLogFileNo );
	return strTmp;
}
