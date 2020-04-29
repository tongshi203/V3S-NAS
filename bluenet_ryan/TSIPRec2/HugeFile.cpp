///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-8-7
///
///		用途：
///			保存大文件
///=======================================================
//  2003-9-20	修改 NotifyOneSubFileOK，当大文件已经收弃，则关闭该文件

// HugeFile.cpp: implementation of the CHugeFile class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "IPRecSvr.h"
#include "HugeFile.h"

#ifdef _WIN32
  #include <io.h>
  #include <stdio.h>
  #include <stdlib.h>

  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif

#else
	#include <unistd.h>
    #include <utime.h>
    #define _access	access
    #define _unlink unlink
#endif //_WIN32  

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHugeFile::CHugeFile()
{
	Preset();
}

CHugeFile::~CHugeFile()
{
	OnHugeFileClose();
}

void CHugeFile::Close()
{
	if( m_hFile != hFileNull )
		CFile::Close();

	if( m_IsHugeFileAlreadOK )			//	has ready succ, and do not open the file
		return;							//	and this make the file shareable possible
	OnHugeFileClose();	
}

void CHugeFile::Abort()
{
	CFile::Abort();

	if( m_IsHugeFileAlreadOK )			//	has ready succ, and do not open the file
		return;							//	and this make the file shareable possible

	OnHugeFileClose();
}

#ifdef _WIN32
  BOOL CHugeFile::Open( LPCSTR lpszFileName, UINT nOpenFlags, CFileException* pError )
#else
  bool CHugeFile::Open( const char * lpszFileName, unsigned int nOpenFlags )
#endif //_WINew
{	
	m_IsHugeFileAlreadOK = FALSE;
	m_strFileName = lpszFileName;
	CString strFlagsFile = GetBitFlagsFileName();
    if( _access( lpszFileName, 0 ) != -1 )
	{									//	file exist, 可能已经成功接收，或数据有误，判断方法是
		if( _access( strFlagsFile, 0 ) != -1 )
			LoadRecFlags( strFlagsFile );	// exist
		else
			m_IsHugeFileAlreadOK = !IsHugeFileChanged();
	}

	if( !m_IsHugeFileAlreadOK )
	{
		TRY
		{
			CDWordArray & params = m_RecFlags.GetUserDefData();
			params.SetSize( 2 );
			PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
			pParam->m_dwFileLen = m_dwHugeFileLen;
			pParam->m_LastModifyTime = m_HugeFileLastModifyTime;
		}
		CATCH_ALL( e )
		{
			m_RecFlags.GetUserDefData().RemoveAll();
		}
		END_CATCH_ALL

		m_RecFlags.SaveToFile( strFlagsFile );		//	记录表示正在接收
	}

	if( m_IsHugeFileAlreadOK )						//	已经成功接收，不再存盘
		nOpenFlags = CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone;

#ifdef _WIN32
	return CFile::Open( m_strFileName, nOpenFlags, pError );
#else
	return CFile::Open( m_strFileName, nOpenFlags );
#endif //_WIN32
}

#ifdef _WIN32
  void CHugeFile::Write( const void* lpBuf, UINT nCount )
  {
	if( m_IsHugeFileAlreadOK )
		return;							//	已经存在了
	CFile::Write( lpBuf, nCount );
  }
#else
  unsigned int CHugeFile::Write( const void* lpBuf, UINT nCount )
  {
	if( m_IsHugeFileAlreadOK )
		return nCount;							//	已经存在了
	return CFile::Write( lpBuf, nCount );
  }
#endif //_WIN32

void CHugeFile::Preset()
{
	m_IsHugeFileAlreadOK = FALSE;
	m_HugeFileLastModifyTime = 0;		//	最后访问时间，判断是否更新
	m_dwHugeFileLen = 0;				//	大文件文件长度
	m_strFileName = "";
	m_nTotalSubFileCount = 0;
	m_RecFlags.Reset();
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		Test whether current Huge file is changed
/// Input parameter:
///		None
/// Output parameter:
///		TRUE				changed
///		FALSE				same
BOOL CHugeFile::IsHugeFileChanged()
{
	CFileStatus fsta;
	if( FALSE == GetStatus( m_strFileName, fsta ) )
		return TRUE;				//	不存在，所以可以认为改变了

	if( fsta.m_mtime.GetTime() == m_HugeFileLastModifyTime && m_dwHugeFileLen == (DWORD)fsta.m_size  )
		return FALSE;				// same, not changed

	return TRUE;
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		获取记录文件名，形成方法，原文件名后＋".$$$"；若已经成功接收，则删除该记录文件
/// Input parameter:
///		None
/// Output parameter:
///		None
CString CHugeFile::GetBitFlagsFileName()
{
	CString strRetVal = m_strFileName;
    strRetVal += ".$HF$$$";
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		Huge file will be closed, check if the file is receive ok, 
///		if OK, then delete the record flags file and set the file last modify time to the original time of broadcast side
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHugeFile::OnHugeFileClose()
{
	if( m_IsHugeFileAlreadOK )
		return;
	
	CString strFlagFileName = GetBitFlagsFileName();

//	ASSERT( m_RecFlags.GetTotalSubFileCount() );
	if( 0 == m_RecFlags.GetTotalSubFileCount() )
		return;

	if( m_RecFlags.GetSubFileHasReceived() < m_RecFlags.GetTotalSubFileCount() )
	{								// Not received OK		
		m_RecFlags.SaveToFile( strFlagFileName );
		Preset();
		return;
	}

	m_IsHugeFileAlreadOK = TRUE;
	_unlink( strFlagFileName );		//	已经成功接收

#ifdef _WIN32
	CFileStatus fstat;
	GetStatus( m_strFileName, fstat );
	fstat.m_mtime = m_HugeFileLastModifyTime;
	fstat.m_atime = m_HugeFileLastModifyTime;
	ASSERT( fstat.m_size == (long)m_dwHugeFileLen );
	SetStatus( m_strFileName, fstat );			//	设置最后修改时间
#else
	struct utimbuf filet;
    filet.actime = m_HugeFileLastModifyTime;
    filet.modtime = m_HugeFileLastModifyTime;
	utime( m_strFileName, &filet );
#endif //_WIN32

	Preset();
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		设置接收参数
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHugeFile::SetHugeFileParameter(DWORD dwFileLen, time_t LastModifyTime, int nSubFileCount)
{
	Preset();

	ASSERT( dwFileLen && nSubFileCount >= 0 );
	m_dwHugeFileLen = dwFileLen;

	m_HugeFileLastModifyTime = LastModifyTime;
	m_nTotalSubFileCount = nSubFileCount;
	m_RecFlags.SetTotalSubFileCount( nSubFileCount );
}

///-------------------------------------------------------
/// CYJ,2003-8-7
/// Function:
///		记录子文件接收成功
/// Input parameter:
///		nSubFileNo				子文件序号
/// Output parameter:
///		true					full huge file received
///		false					not fully received
///	Modify log
//		2004-7-5, return true if huge file is fully recieved
//		2003-9-20, If the huge file is received succ, close the file, and the CDVBFileReceiver will reopen it if needed
bool CHugeFile::NotifyOneSubFileOK(int nSubFileNo)
{
	ASSERT( nSubFileNo >= 0 && nSubFileNo < m_RecFlags.GetTotalSubFileCount() );
	if( nSubFileNo >= 0 && nSubFileNo < m_RecFlags.GetTotalSubFileCount() )
	{
		m_RecFlags.SetBitValue( nSubFileNo, 1 );
		int nTotalSubFileCount = m_RecFlags.GetTotalSubFileCount();
		if( !m_IsHugeFileAlreadOK && nTotalSubFileCount && m_RecFlags.GetSubFileHasReceived() >= nTotalSubFileCount )
		{
			Close();				//  2003-9-20 已经接收成功，关闭文件		
			return true;
		}
	}
	return false;
}

///-------------------------------------------------------
/// CYJ,2003-8-8
/// Function:
///		Load Rec Flags data from file
/// Input parameter:
///		strFlagsFile		rec falgs data file name
/// Output parameter:
///		None
void CHugeFile::LoadRecFlags(CString &strFlagsFile)
{
	m_RecFlags.LoadFromFile( strFlagsFile );		//	存在
	m_RecFlags.SetTotalSubFileCount( m_nTotalSubFileCount );
	CDWordArray & params = m_RecFlags.GetUserDefData();
	if( params.GetSize() >= 2 )
	{
		PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
		if( pParam->m_dwFileLen == m_dwHugeFileLen &&\
			pParam->m_LastModifyTime == m_HugeFileLastModifyTime )
		{											//	相同的文件
			return;
		}
	}

	TRY
	{	
		m_RecFlags.CleanDataOnly();			//	重新清成 0 

		params.SetSize( 2 );
		PUSERDEFPARAMETER pParam = (PUSERDEFPARAMETER)params.GetData();
		pParam->m_dwFileLen = m_dwHugeFileLen;
		pParam->m_LastModifyTime = m_HugeFileLastModifyTime;
	}
	CATCH_ALL( e )
	{
		params.RemoveAll();
	}
	END_CATCH_ALL
}

#ifdef _WIN32
 void CHugeFile::SetLength( DWORD dwNewLen )
 {
	if( m_IsHugeFileAlreadOK )
		return;

	CFile::SetLength( dwNewLen );
 }
#else
  bool CHugeFile::SetLength( DWORD dwNewLen )
  {
	if( m_IsHugeFileAlreadOK )
		return true;

	return CFile::SetLength( dwNewLen );
}
#endif //_WIN32

