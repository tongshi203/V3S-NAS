// UnCmpMgr.h: interface for the CUnCmpMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNCMPMGR_H__190D1AC1_726C_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_UNCMPMGR_H__190D1AC1_726C_11D3_B1F1_005004868EAA__INCLUDED_

#include "Lzhuf.h"	// Added by ClassView
#include "unlzss32.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnCmpMgr  
{
public:
	CUnCompressObj * GetSvr();
	static BOOL IsCompress(int nLen, PBYTE pBuffer );
	PBYTE DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf=NULL);
	int GetFileInfo(int nFileNo,CFileStatus & outfStatus);
	int GetFileNum();
	int Attach(int nFileLen,PBYTE pBuf);

	CUnCmpMgr();
	virtual ~CUnCmpMgr();

private:
	CUNLZSS32 m_lzss;
	CUnCompressObj * m_pSvr;
	CLzhuf32 m_LzhufSvr;
};

#endif // !defined(AFX_UNCMPMGR_H__190D1AC1_726C_11D3_B1F1_005004868EAA__INCLUDED_)
