// TSDBMultiFileHeader.h: interface for the CTSDBMultiFileHeader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDBMULTIFILEHEADER_H__5390AAE1_22F8_11D3_B8E9_005004868EAA__INCLUDED_)
#define AFX_TSDBMULTIFILEHEADER_H__5390AAE1_22F8_11D3_B8E9_005004868EAA__INCLUDED_

#include "Tsdb.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
#include <MyString.h>
#endif //_WIN32

class CTSDBMultiFileHeader  
{
public:
	static BOOL IsMultiFileHeader( PTSDBMULFILEHEAD pRefHead );
	static BOOL IsMultiFileHeader( PBYTE pBuf );
	CTSDBMultiFileHeader( PTSDBMULFILEHEAD pRefHead);
	CTSDBMultiFileHeader( CTSDBMultiFileHeader & RefMulHead);
	int GetFileOfs(int nIndex);
	PTSDBFILEHEADER GetFileHeader(int nIndex);
	int GetFileNum();
	BOOL IsMultiFileHeader();
	CTSDBMultiFileHeader();
	virtual ~CTSDBMultiFileHeader();
	
	operator TSDBMULFILEHEAD&(){return *m_pHeader;}
	operator PTSDBMULFILEHEAD(){return m_pHeader;}
	TSDBFILEHEADER& operator []( int nIndex );
	CTSDBMultiFileHeader & operator=(CTSDBMultiFileHeader& );
	CTSDBMultiFileHeader & operator=(PTSDBMULFILEHEAD pRefMulHead);

private:
	PTSDBMULFILEHEAD m_pHeader;
};


class CTSDBFileHeader  
{
public:
	static BOOL IsFileHead( PTSDBFILEHEADER pHeader, int nBufLen );
	static BOOL IsFileHead( PBYTE pBuf, int nBufLen );
	int CopyExtData( PBYTE pDstBuf, int nBufSize);
	PTSDBSOCKETHEAD GetSocketHead();
	PTSDBMULTICAST GetMulticastHead();
	PTSDBFILEATTRIBHEAD GetFileAttribHead();
	PTSDBHUGEFILEHEAD GetHugeFileHead();
	int CopyData( PBYTE pDstBuf, int nBufSize );
	PBYTE GetDataBuf();
	int ExtDataLen();
	BOOL HasExtData();
	BOOL HasMulticast();
	BOOL HasSocket();
	BOOL HasFileAttarib();
	BOOL IsHugeFile();
	CString GetFileName();
	int GetFileLen();
	BOOL IsFileHead();
	CTSDBFileHeader( TSDBFILEHEADER & RefHeader);
	CTSDBFileHeader( PTSDBFILEHEADER pRefHeader);
	CTSDBFileHeader(CTSDBFileHeader & refHeader );
	CTSDBFileHeader();
	virtual ~CTSDBFileHeader();

	operator TSDBFILEHEADER&() { return * m_pHeader; }
	operator PTSDBFILEHEADER() { return m_pHeader; }
	operator TSDBHUGEFILEHEAD&() { return (*GetHugeFileHead()); }
	operator PTSDBHUGEFILEHEAD() { return GetHugeFileHead(); }
	operator TSDBFILEATTRIBHEAD&() { return (*GetFileAttribHead()); }
	operator PTSDBFILEATTRIBHEAD() { return GetFileAttribHead(); }
	operator TSDBSOCKETHEAD&() { return (*GetSocketHead()); }
	operator PTSDBSOCKETHEAD() { return GetSocketHead(); }
	operator TSDBMULTICAST&() { return (*GetMulticastHead());}
	operator PTSDBMULTICAST() { return GetMulticastHead(); }
	CTSDBFileHeader & operator = ( CTSDBFileHeader & RefHead);
	CTSDBFileHeader & operator = ( PTSDBFILEHEADER pRefHeader );
	CTSDBFileHeader & operator = ( TSDBFILEHEADER & RefHeader );

private:
	int GetParamHeader( int nBitOfs );
	PTSDBFILEHEADER m_pHeader;
};

#endif // !defined(AFX_TSDBMULTIFILEHEADER_H__5390AAE1_22F8_11D3_B8E9_005004868EAA__INCLUDED_)
