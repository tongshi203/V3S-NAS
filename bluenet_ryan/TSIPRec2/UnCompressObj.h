// UnCompressObj.h: interface for the CUnCompressObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_

#include "Tsdb.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32			// linux
#include <MyFile.h>
#endif //_WIN32

class CUnCompressObj  
{
public:
	virtual void FreeMemory() =0;
	void Write( PBYTE pBuf, int nCount=1);
	void OutputOneByte( BYTE byData );
	int ReadOneByte();
	static BOOL IsCompress( int nLen, PBYTE pBuffer );
	PBYTE ReleaseDstBuffer(long & outfLen );
	void SetDstBuffer(int nDstBufLen,PBYTE pBuffer );
	PBYTE GetDataBuf();
	PTSDBCOMPRESSHEAD GetHeader();
	virtual void Detach();
	virtual PBYTE DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf=NULL);
	virtual int GetFileInfo(int nFileNo,CFileStatus & outfStatus);
	virtual int GetFileNum();
	virtual int Attach(int nFileLen,PBYTE pBuf);
	virtual DWORD GetCompressMethod();
	virtual int GetDecoderVersion();

	CUnCompressObj();
	virtual ~CUnCompressObj();

public:	
	PTSDBCOMPRESSHEAD m_pTSDBCmpHead;					//	数据头
	int		m_nSrcDataLen;								//	原始数据头大小
	PBYTE	m_pSrcDataBuf;								//	数据区地址
	int		m_nDataRead;								//	已经读取的数据字节数
	PBYTE	m_pDstDataBuf;								//	目标数据
	PBYTE	m_pOutDataBufPtr;							//	输出的数据缓冲区指针
	int		m_nOutDataLen;								//	实际输出缓冲区大小
	int		m_nDstBufSize;								//	输出缓冲区大小
};

#endif // !defined(AFX_UNCOMPRESSOBJ_H__73F868A1_736F_11D3_B1F1_005004868EAA__INCLUDED_)
