// TSDBFileSystem.h: interface for the CTSDBFileSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDBFILESYSTEM_H__E0B45181_A9DC_11D4_A976_0001022D5536__INCLUDED_)
#define AFX_TSDBFILESYSTEM_H__E0B45181_A9DC_11D4_A976_0001022D5536__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
	#include <MyList.h>
#endif //_WIN32

#include "UnCmpMgr.h"
#include "Tsdb.h"
#include "TSDB_Rec.h"
#include "MB_OneFile.h"
#include "LookaheadPacketMgr.h"
#include "FileObject.h"

class CMB_OneFile;

class CTSDBFileSystem : public CLookaheadPacketMgr<CFileObject>
{
public:
	CTSDBFileSystem();
	virtual ~CTSDBFileSystem();
	virtual void NotifyOnFileOKEvent( CFileObject * pFileObject ) = 0;
	virtual void NotifySubFileOKEvent( CFileObject * pFileObject ) = 0;

public:
	DWORD m_dwByteRecevied;				//	从启动对象以来接收到的字节数
	DWORD m_dwOneFileBufSize;

public:
	void ProcessOneFile( CMB_OneFile * pOneFile );	
	static PBYTE UnlockData( );	

protected:
	BOOL ProcessHugeFile( CFileObject * pOneFile,CTSDBFileHeader &FileHead );
	BOOL ProcessTSDBSingleFile( CFileObject * pOneFile );
	void ProcessSingleFile( PBYTE pBuffer, DWORD dwLen, CMB_OneFile * pBaseFile = NULL);
	void ProcessMultiFile( PTSDBMULFILEHEAD pMultiHeader,CMB_OneFile * pOneFile = NULL );	
	BOOL ProcessSysReservFile( CTSDBFileHeader &hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose );
	BOOL ProcessUpdateFile( CTSDBFileHeader & hdr, PBYTE pBuf, CFileStatus &fsta, DWORD nFilePurpose );
protected:
	BOOL m_bIsEnableNotTSDBFile;						//	2002.5.22 添加，是否允许非 TSDB 文件通过
	CUnCmpMgr m_UnCompressSvr;							//	解压服务程序
	CMB_OneFile * m_pOneBaseFile;
private:
	BOOL IsSysReservFile( CTSDBFileHeader & hdr, DWORD nFilePurpose );
};

#endif // !defined(AFX_TSDBFILESYSTEM_H__E0B45181_A9DC_11D4_A976_0001022D5536__INCLUDED_)
