// MyMapFile.h: interface for the CMyMapFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYMAPFILE_H__C06351D8_CBEE_4A48_9B8A_2F44336787A9__INCLUDED_)
#define AFX_MYMAPFILE_H__C06351D8_CBEE_4A48_9B8A_2F44336787A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
   #include "MyFile.h"
#endif //_WIN32

#pragma pack(push,4)

class CMyMapFile  
{
public:	
	BOOL MapFile(LPCSTR lpszFileName, LPCSTR lpszShareName, int nMode, BOOL * pbIsExist = NULL, DWORD dwLowPos = 0, DWORD dwHightPos = 0 );
	DWORD GetFileLen();
	void Close();
	PBYTE GetBuffer();
	BOOL IsValid();
	BOOL MapFileForReadOnly( LPCSTR lpszFileName, LPCSTR lpszShareName = NULL );
	CMyMapFile();
	virtual ~CMyMapFile();

	enum {
		MAPFILE_MODE_READONLY = 0,			// read only
		MAPFILE_MODE_WRITEONLY,				// write only
		MAPFILE_MODE_READWRITE,				// read write
	};

private:
	PBYTE m_pMappedBuffer;
	BOOL m_bReadOnly;
	CFile	m_file;
};

#pragma pack(pop)

#endif // !defined(AFX_MYMAPFILE_H__C06351D8_CBEE_4A48_9B8A_2F44336787A9__INCLUDED_)
