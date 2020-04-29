// FileUpdate.cpp: implementation of the CFileUpdate class.
//
//				升级文件
//	方法:	通过 修改 WinInit.Ini 的内容, 在启动 Windows 时更新文件
//			升级纪录在 <TS>\Receive\目录下的 Update.Log 文件中
//	
//////////////////////////////////////////////////////////////////////
//	2001.6.14	修改 SetupInstall 中的 FORCE_IN_USE 标志
//	2001.4.6	修改 SetInstall 升级文件但不删除源文件
//	2000.11.9	修改 Update 函数，修改 SetupInstallFile 入口参数

#include "stdafx.h"
#include "resource.h"
#include "FileUpdate.h"
#include "MyRegKey.h"
#include "DirectroyHelp.h"
#include "io.h"
#include "setupapi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileUpdate::CFileUpdate()
{

}

CFileUpdate::~CFileUpdate()
{

}

//	升级文件
//	入口参数
//		dwFileLen					文件长度
//		pBuf						数据缓冲区
//		pExtDataBuf					附加数据缓冲区地址
//		dwExtLen					附加数据长度
void CFileUpdate::Update( DWORD dwFileLen, PBYTE pBuf, void *pExtDataBuf, DWORD dwExtLen )
{
CString strSrcFileName;
	ASSERT( dwFileLen && pBuf && pExtDataBuf && dwExtLen && dwExtLen == sizeof(SOFTUPDATE) );
	PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
	CString strDstFile = GetRealPath( pItem->szPath ) + pItem->szFileName;

	strSrcFileName = strDstFile + ".1";
	CDirectroyHelp::Mkdir( strDstFile );			//	创建临时目录

	if( _access( strSrcFileName,0 ) == 0 )
		return;										//	已经存在
	else
	{
		CFile f;
		try
		{
			if( f.Open( strSrcFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary ) == FALSE )
				return;
			f.Write( pBuf, dwFileLen );
			f.Close();
		}
		catch(CFileException * e)
		{
			TRACE("MSG CFile::Update, file IO error, filename=%s\n",strSrcFileName);
			e->Delete();
			return;								
		}
	}
	BOOL bFileInUse = IsUsing( strDstFile );
	if( bFileInUse )
		AddToUpdateTaskList( strSrcFileName, strDstFile );		//	正在使用中,且长文件名
	else
	{
		if( SetupInstall( strSrcFileName, strDstFile ) )		//	不在使用中, 进行升级
			AddToDelList( strSrcFileName );						//	添加到删除的文件列表中
		else
			AddToUpdateTaskList( strSrcFileName, strDstFile );		//	正在使用中,且长文件名
	}

	strSrcFileName = strDstFile;
	strSrcFileName += "  ---------  ";
	strSrcFileName += pItem->szHelp;
	CTime now = CTime::GetCurrentTime();
	strSrcFileName.Insert(0, now.Format("%Y.%m.%d %H:%M:%S 接收到升级文件 ==> ") );
	AddToUpdateLog( strSrcFileName );
}

//	解密程序的升级
//	入口参数
//		dwFileLen						文件长度
//		pBuf							缓冲区
//		pExtDataBuf						扩展缓冲区地址
//		dwExtLen						扩展缓冲区大小
//	返回参数
//		长度 = 0						失败
//		临时文件的文件名
CString CFileUpdate::UpdateUnlockProc(DWORD dwFileLen, PBYTE pBuf, void *pExtDataBuf, DWORD dwExtLen)
{
CString strSrcFileName;
CString strRetVal;
	ASSERT( dwFileLen && pBuf && pExtDataBuf && dwExtLen == sizeof(SOFTUPDATE) );
	PSOFTUPDATE pItem = (PSOFTUPDATE) pExtDataBuf;
	ASSERT( stricmp(pItem->szPath,"<WINSYSDIR>" ) == 0 );
	CString strDstFile = GetRealPath( "<WINSYSDIR>" ) + pItem->szFileName;

	strSrcFileName = strDstFile + ".Dll";
	strRetVal = strSrcFileName;									//	返回的文件名
	if( _access( strSrcFileName,0 ) == 0 )
		return strRetVal;										//	已经存在
	else
	{
		try	
		{
			CFile f;
			if( f.Open( strSrcFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary ) == FALSE )
			{
				strRetVal.Empty();
				return strRetVal;									//	失败
			}
			f.Write( pBuf, dwFileLen );
			f.Close();
		}
		catch( CFileException * e)
		{
			strRetVal.Empty();
			e->Delete();
			TRACE("MSG CFileUpdate::UpdateUnlockProc,  File IO failed,filename=%s\n",strSrcFileName);
			return strRetVal;
		}
	}
	BOOL bFileInUse = IsUsing( strDstFile );
	if( bFileInUse )
		AddToUpdateTaskList( strSrcFileName, strDstFile );		//	正在使用中,且长文件名
	else
	{
		if( SetupInstall( strSrcFileName, strDstFile ) )				//	不在使用中, 进行升级
			AddToDelList( strSrcFileName );							//	添加到删除的文件列表中
		else
			AddToUpdateTaskList( strSrcFileName, strDstFile );		//	正在使用中,且长文件名
		strRetVal = strDstFile;									//	升级成功, 用目标文件名
	}

	strSrcFileName = strDstFile;
	strSrcFileName += "  ---------  ";
	strSrcFileName += pItem->szHelp;
	CTime now = CTime::GetCurrentTime();
	strSrcFileName.Insert(0, now.Format("%Y.%m.%d %H:%M:%S 接收到解密升级程序 ==> ") );
	AddToUpdateLog( strSrcFileName );
	return strRetVal;
}

//	调用安装程序,直接拷贝数据
//	入口参数
//		pszSrcFileName				数据源文件名
//		pszDstFileName				目标文件名
BOOL CFileUpdate::SetupInstall(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
	ASSERT( pszSrcFileName && pszDstFileName );

	HINSTANCE hDll = ::LoadLibrary( "SetupAPI.DLL" );
	if( hDll == NULL )
		return FALSE;

	BOOL ( WINAPI* pFunc)(HINF,PINFCONTEXT,LPCSTR,LPCSTR,LPCSTR,DWORD,PSP_FILE_CALLBACK ,PVOID);

	pFunc = \
		(BOOL(WINAPI*)(HINF,PINFCONTEXT,LPCSTR,LPCSTR,LPCSTR,DWORD,PSP_FILE_CALLBACK ,PVOID))::GetProcAddress( hDll,"SetupInstallFileA" );
	if( pFunc )
	{																		//	2001.6.14 取消 FORCE_IN_USE 标志
		pFunc( NULL, NULL, pszSrcFileName, NULL, pszDstFileName,\
			SP_COPY_SOURCE_ABSOLUTE|SP_COPY_NEWER_OR_SAME, NULL, NULL );	// 进行版本检测

	}
	::FreeLibrary( hDll );
	return pFunc ? TRUE : FALSE;
}

//	添加到我设计的安装升级程序中
//	入口参数
//		pszSrcFileName				数据源文件名
//		pszDstFileName				目标文件名
//	注:
//		升级文件的列表在 TS\UPDATE 目录下的 UPDATE.INI
//		[Rename]
//		ITEMNUM = 总数
//		ItemSrc1 = 源文件名
//		ItemDst1 = 目标文件名
void CFileUpdate::AddToUpdateTaskList(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
char * pszSection =  "Rename";			//	升级文件
	ASSERT( pszSrcFileName && pszDstFileName );
	CString strFileName = GetTSMainPath() + "\\UPDATE\\UPDATE.INI";
	int nItem = ::GetPrivateProfileInt( pszSection, "ItemNum",0,strFileName );

	CString strTmp;
	strTmp.Format("%d",nItem+1);
	::WritePrivateProfileString( pszSection,"ItemNum",strTmp,strFileName );

	CString strKey;
	strKey.Format("ItemSrc%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszSrcFileName, strFileName );
	strKey.Format("ItemDst%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszDstFileName, strFileName );
}

//	添加文件升级任务到 WinInit.Ini
//	入口参数
//		pszSrcFileName				数据源文件名
//		pszDstFileName				目标文件名
//	返回参数
//		FALSE						已经存在升级任务
//		TRUE						成功
//	注:
//		这种升级方法只适合于 8.3 格式的短文件名
BOOL CFileUpdate::AddFileNameToWinInit(LPCSTR pszSrcFileName, LPCSTR pszDstFileName)
{
	ASSERT( pszSrcFileName && pszDstFileName );
	CString strDst = pszDstFileName;
	strDst.MakeUpper();
	CString strTmp;
	::GetPrivateProfileSection( "rename", strTmp.GetBuffer( 32000 ), 32000, "WinInit.Ini" );
	strTmp.ReleaseBuffer();
	if( strTmp.Find( strDst + "=" ) >= 0 )
		return FALSE;				//	已经存在
	::WritePrivateProfileString( "rename",strDst,pszSrcFileName,"WinInit.Ini" );
	return TRUE;
}

//	获取升级纪录
CString CFileUpdate::GetUpdateLog()
{
	CString strFileName = GetTSMainPath() + "\\Update\\Update.Log";
	CString strLog;
	try
	{
		CFile f;
		if( f.Open( strFileName, CFile::modeRead|CFile::typeBinary ) == FALSE )
			return strLog;
		int nLen = f.GetLength();
		f.Read( strLog.GetBuffer( nLen + 10 ), nLen );
		strLog.ReleaseBuffer( nLen );
		f.Close();
	}
	catch( CFileException * e)
	{
		strLog.Empty();
		TRACE("MSG CFileUpdate::GetUpdateLog, file IO failed, filename=%s\n",strFileName);
		e->Delete();
	}
	return strLog;
}

//	添加一条升级纪录到升级纪录文件中
void CFileUpdate::AddToUpdateLog(LPCSTR pszLog)
{
	CString strFileName = GetTSMainPath() + "\\Update\\Update.Log";
	CString strLog;
	FILE * fp ;
	fp = fopen( strFileName, "r+t" );
	if( fp == NULL )
		fp = fopen( strFileName, "wt" );
	if( fp )
	{
		fseek( fp,0,SEEK_END );
		fprintf( fp, "%s\n", pszLog );
		fclose( fp );
	}
}

//	添加到删除的文件列表中
//	入口参数
//		pszFileName				文件名
//	注:
//		在 Update\Update.Ini 中的 [Delete] 项中
//		ItemNum = 数据项数
//		Item0 = 待删除的文件名
void CFileUpdate::AddToDelList(LPCSTR pszFileName)
{
	ASSERT( pszFileName );
	const char * pszSection = "Delete";
	CString strFileName = GetTSMainPath() + "\\UPDATE\\UPDATE.INI";
	int nItem = ::GetPrivateProfileInt( pszSection, "ItemNum",0,strFileName );

	CString strTmp;
	strTmp.Format("%d",nItem+1);
	::WritePrivateProfileString( pszSection,"ItemNum",strTmp,strFileName );

	CString strKey;
	strKey.Format("Item%d",nItem);
	::WritePrivateProfileString( pszSection, strKey, pszFileName, strFileName );
}

//	获取通视的工作主目录 
CString CFileUpdate::GetTSMainPath()
{
	CMyRegKey	regkey( HKEY_LOCAL_MACHINE, "Software" );
	return regkey.GetProfileString( "Tongshi","mainPath","C:\\TS" );
}

//	获取真正的目录
CString CFileUpdate::GetRealPath(LPCSTR pszPath)
{
	CString strRet;
	int nSkipLen = 0;
	if( strnicmp( pszPath, "<TS>", 4 ) == 0 )
	{
		nSkipLen = 4;
		strRet.Format("%s%s", GetTSMainPath(), pszPath+nSkipLen );
	}
	else if( strnicmp( pszPath, "<WINDIR>", 8 ) == 0 )
	{
		nSkipLen = 8;
		GetWindowsDirectory( strRet.GetBuffer(_MAX_PATH),_MAX_PATH );
		strRet.ReleaseBuffer();
		strRet += (pszPath+nSkipLen);
	}
	else if( strnicmp( pszPath, "<WINSYSDIR>", 11 ) == 0 )
	{
		nSkipLen = 11;
		GetSystemDirectory( strRet.GetBuffer(_MAX_PATH),_MAX_PATH );
		strRet.ReleaseBuffer();
		strRet += (pszPath+nSkipLen);
	}
	else
	{
		strRet = pszPath;
		if( strRet.IsEmpty() )
			strRet = GetTSMainPath();
	}
	int nLen = strRet.GetLength();
	if( nLen && strRet[nLen-1] != '\\' )
		strRet += '\\';
	return strRet;
}

//	是否
BOOL CFileUpdate::IsAShortFileName(LPCSTR pszFileName)
{
char szName[_MAX_PATH*4];		//  CYJ,2005-8-1 修改缓冲区长度
char szExtName[_MAX_PATH];
	_splitpath( pszFileName, NULL, NULL, szName, szExtName );
	if( strlen( szName ) > 8 )			//	长文件名
		return FALSE;
	if( strstr( szName, "." ) )			//	长文件名
		return FALSE;
	if( strlen( szExtName ) > 4 )
		return FALSE;
	strcat( szName, szExtName );
	if( strlen(szName) >= 13 )
		return FALSE;
	if( strstr( szName," " ) )
		return FALSE;
	return TRUE;
}

//	判断一个文件是否使用中
//	入口参数
//		pszFileName						文件名
//	返回参数
//		TRUE							使用中
//		FALSE							可以升级
BOOL CFileUpdate::IsUsing(LPCSTR pszFileName)
{
	CFile f;
	if( _access( pszFileName,0 ) != 0 )
	{
		if( f.Open( pszFileName, CFile::modeCreate|CFile::shareExclusive|CFile::modeWrite|CFile::typeBinary ) )
			f.Close();							//	创建
		return FALSE;							//	不存在, 一定不在使用中
	}
	if( f.Open( pszFileName, CFile::modeCreate|CFile::modeNoTruncate|CFile::shareExclusive|CFile::modeWrite|CFile::typeBinary ) == FALSE )
		return TRUE;
	f.Close();
	return FALSE;
}
