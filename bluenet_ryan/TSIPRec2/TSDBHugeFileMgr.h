// TSDBHugeFileMgr.h: interface for the CTSDBHugeFileMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDBHUGEFILEMGR_H__7B40D442_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_TSDBHUGEFILEMGR_H__7B40D442_74C2_11D3_B1F1_005004868EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Tsdb.h"
#include "TSDBHugeFileObj.h"

class CTSDBHugeFileMgr  
{
public:
	void ClearBuf();
	BOOL SaveBlock ( PTSDBHUGEFILEHEAD pHeader, PBYTE pDataBuf );
	CTSDBHugeFileObj * FindObj( PTSDBHUGEFILEHEAD pHeader );
	CTSDBHugeFileMgr();
	virtual ~CTSDBHugeFileMgr();

	enum { MAX_HUGEOBJ_NUM = 8 };				//	最多允许使用 8 个大文件对象

private:
	CTSDBHugeFileObj * m_pLastFind;				//	上次查找的对象
	void UpdateObj();
	CTSDBHugeFileObj * AddHelpObj( PTSDBHUGEFILEHEAD pHeader );
	CObArray m_HelpObj;
};

#endif // !defined(AFX_TSDBHUGEFILEMGR_H__7B40D442_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
