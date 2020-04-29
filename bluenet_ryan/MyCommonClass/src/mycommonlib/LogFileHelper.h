// LogFileHelper.h: interface for the CLogFileHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGFILEHELPER_H__DBB6059A_193D_490F_B52B_A2E48596BE19__INCLUDED_)
#define AFX_LOGFILEHELPER_H__DBB6059A_193D_490F_B52B_A2E48596BE19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
  #include <stdio.h>
  #include <stdlib.h>
  #include <MyStdioFile.h>
  #undef CString
  #define CString CMyString
#endif //_WIN32

#pragma pack(push,4)

class CLogFileHelper : public CStdioFile
{
public:
	CString GetCurrentLogFileName();
	void WriteToLogFile( LPCSTR lpszLog );
	void SetFilePathParamters( LPCSTR lpszFullPath = NULL, int nMaxCount = 10, int nLenGate = 1024*1024 );
	CLogFileHelper( LPCSTR lpszFullPath = NULL, int nMaxCount = 10, int nLenGate = 1024*1024 );
	virtual ~CLogFileHelper();

private:
	CString m_strFullPath;
	int m_nLenGate;
	int m_nMaxCount;
	int m_nLogFileNo;

private:
	BOOL OpenLogFile();	
};

#pragma pack(pop)

#endif // !defined(AFX_LOGFILEHELPER_H__DBB6059A_193D_490F_B52B_A2E48596BE19__INCLUDED_)
