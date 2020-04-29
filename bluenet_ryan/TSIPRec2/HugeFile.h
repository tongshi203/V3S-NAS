// HugeFile.h: interface for the CHugeFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUGEFILE_H__E6126F6E_85E2_49DC_A11E_FAC3869140A2__INCLUDED_)
#define AFX_HUGEFILE_H__E6126F6E_85E2_49DC_A11E_FAC3869140A2__INCLUDED_

#include "BitArrayObject.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CHugeFile : public CFile
{
public:	
	CHugeFile();
	virtual ~CHugeFile();

	virtual void Abort();
	virtual void Close();
#ifdef _WIN32
	virtual BOOL Open( LPCSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL );
    virtual void Write( const void* lpBuf, UINT nCount );
    virtual void SetLength( DWORD dwNewLen );
#else
	virtual bool Open( const char * lpszFileName, unsigned int nOpenFlags );
    virtual unsigned int Write( const void* lpBuf, UINT nCount );
    virtual bool SetLength( DWORD dwNewLen );
#endif //_WIN32

	typedef struct tagUSERDEFPARAMETER
	{
		DWORD	m_dwFileLen;				//	文件长度
		time_t	m_LastModifyTime;			//	最后修改时间
	}USERDEFPARAMETER,*PUSERDEFPARAMETER;

public:
	bool NotifyOneSubFileOK( int nSubFileNo );
	void SetHugeFileParameter( DWORD dwFileLen, time_t LastModifyTime, int nSubFileCount );

public:
	CBitArrayObject m_RecFlags;
	time_t	m_HugeFileLastModifyTime;		//	最后访问时间，判断是否更新
	DWORD	m_dwHugeFileLen;				//	大文件文件长度
	CString m_strFileName;
	BOOL m_IsHugeFileAlreadOK;
	int		m_nTotalSubFileCount;

private:
	void OnHugeFileClose();
	void Preset();

private:
	void LoadRecFlags( CString & strFlagsFile );
	CString GetBitFlagsFileName();

	BOOL IsHugeFileChanged();	
};

#endif // !defined(AFX_HUGEFILE_H__E6126F6E_85E2_49DC_A11E_FAC3869140A2__INCLUDED_)
