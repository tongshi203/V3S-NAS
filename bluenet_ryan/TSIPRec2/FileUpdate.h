// FileUpdate.h: interface for the CFileUpdate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEUPDATE_H__D4F13931_9048_11D3_BD17_005004868EAA__INCLUDED_)
#define AFX_FILEUPDATE_H__D4F13931_9048_11D3_BD17_005004868EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct tagSOFTUPDATE
{
	char szFileName[_MAX_PATH];				//	文件名
	char szPath[_MAX_PATH];					//	路径
	char szOemName[_MAX_PATH];				//	OEM 厂商名称
	char szHelp[_MAX_PATH];					//	帮助说明
	int	 m_nBroTimes;						//	已经播出的次数
}SOFTUPDATE,*PSOFTUPDATE;

class CFileUpdate  
{
public:
	static BOOL IsUsing( LPCSTR pszFileName );
	static CString UpdateUnlockProc( DWORD dwFileLen, PBYTE pBuf, void * pExtDataBuf, DWORD dwExtLen );
	static void AddToDelList( LPCSTR pszFileName );
	static CString GetTSMainPath();
	static void AddToUpdateLog( LPCSTR pszLog );
	static CString GetUpdateLog();
	static BOOL AddFileNameToWinInit( LPCSTR pszSrcFileName, LPCSTR pszDstFileName );
	static void Update( DWORD dwFileLen, PBYTE pBuf, void * pExtDataBuf, DWORD dwExtLen );
	CFileUpdate();
	virtual ~CFileUpdate();

private:
	static BOOL IsAShortFileName( LPCSTR pszFileName );
	static void AddToUpdateTaskList( LPCSTR pszSrcFileName, LPCSTR pszDstFileName );
	static BOOL SetupInstall( LPCSTR pszSrcFileName, LPCSTR pszDstFileName );
	static CString GetRealPath ( LPCSTR pszPath );
};

#endif // !defined(AFX_FILEUPDATE_H__D4F13931_9048_11D3_BD17_005004868EAA__INCLUDED_)
