//---------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implemented:
//			My Achive
//-----------------------------------------------------------

#ifndef __MYARCHIVE_H_20030816__
#define __MYARCHIVE_H_20030816__

#include <stdio.h>
#include <stdlib.h>
#include <MyString.h>
#include <MyFile.h>

#pragma pack(push,4)

class CArchive
{
public:
// Flag values
	enum Mode { store = 0, load = 1, bNoFlushOnDelete = 2, bNoByteSwap = 4 };

	CArchive(CFile* pFile, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL);
	~CArchive();

// Attributes
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsBufferEmpty() const;

	CFile* GetFile() const;
// Operations
	UINT Read(void* lpBuf, UINT nMax);
	void Write(const void* lpBuf, UINT nMax);
	void Flush();
	void Close();
	void Abort();   // close and shutdown without exceptions

	// reading and writing strings
	void WriteString(LPCSTR lpsz);
	char *  ReadString(char *  lpsz, UINT nMax);
	BOOL ReadString(CMyString& rString);

// Implementation
public:
	bool FillBuffer(UINT nBytesNeeded);

	// public for advanced use
	CMyString m_strFileName;

protected:
	// archive objects cannot be copied or assigned
	CArchive(const CArchive& arSrc);
	void operator=(const CArchive& arSrc);

	UINT m_nMode;
    BOOL m_bUserBuf;
	int m_nBufSize;
	CFile* m_pFile;
	BYTE* m_lpBufCur;
	BYTE* m_lpBufMax;
	BYTE* m_lpBufStart;

//	operator
public:
	CArchive& operator<<(CMyString &strValue);
    CArchive& operator>>(CMyString &strValue);


	CArchive& operator<<(int i) { return operator<<((long)i); }
    CArchive& operator<<(unsigned u){ return operator<<((long)u); }
    CArchive& operator<<(short w)	{ return operator<<((WORD)w); }
    CArchive& operator<<(char ch)	{ return operator<<((BYTE)ch); }
    CArchive& operator<<(BYTE by)
		{ if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax) Flush();
		  *(BYTE*)m_lpBufCur = by; m_lpBufCur += sizeof(BYTE); return *this; }
    CArchive& operator<<(WORD w)
		{ if (m_lpBufCur + sizeof(WORD) > m_lpBufMax) Flush();
	    	*(WORD*)m_lpBufCur = w; m_lpBufCur += sizeof(WORD); return *this; }
	CArchive& operator<<(long l)
		{ if (m_lpBufCur + sizeof(long) > m_lpBufMax) Flush();
	    	*(LONG*)m_lpBufCur = l; m_lpBufCur += sizeof(long); return *this; }
#ifdef _WIN32
	CArchive& operator<<(DWORD dw)
		{ if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax) Flush();
			*(DWORD*)m_lpBufCur = dw; m_lpBufCur += sizeof(DWORD); return *this; }
#endif //_WIN32
	CArchive& operator<<(float f)
		{ if (m_lpBufCur + sizeof(float) > m_lpBufMax) Flush();
			*(DWORD*)m_lpBufCur = *(DWORD*)&f; m_lpBufCur += sizeof(float); return *this; }
	CArchive& operator<<(double d)
		{ Write( &d, sizeof(d) ); return *this; }

	CArchive& operator>>(int& i){ return operator>>((long&)i); }
	CArchive& operator>>(unsigned& u)	{ return operator>>((long&)u); }
	CArchive& operator>>(short& w)	{ return operator>>((WORD&)w); }
	CArchive& operator>>(char& ch)	{ return operator>>((BYTE&)ch); }
	CArchive& operator>>(BYTE& by)
	{ if (m_lpBufCur + sizeof(BYTE) > m_lpBufMax)
			FillBuffer(sizeof(BYTE) - (DWORD)(m_lpBufMax - m_lpBufCur));
		by = *(BYTE*)m_lpBufCur; m_lpBufCur += sizeof(BYTE); return *this; }
	CArchive& operator>>(WORD& w)
		{ if (m_lpBufCur + sizeof(WORD) > m_lpBufMax)
			FillBuffer(sizeof(WORD) - (DWORD)(m_lpBufMax - m_lpBufCur));
		w = *(WORD*)m_lpBufCur; m_lpBufCur += sizeof(WORD); return *this; }
#ifdef _WIN32
	CArchive& operator>>(DWORD& dw)
		{ if (m_lpBufCur + sizeof(DWORD) > m_lpBufMax)
			FillBuffer(sizeof(DWORD) - (DWORD)(m_lpBufMax - m_lpBufCur));
		dw = *(DWORD*)m_lpBufCur; m_lpBufCur += sizeof(DWORD); return *this; }
#endif //_WIN32
	CArchive& operator>>(float& f)
		{ if (m_lpBufCur + sizeof(float) > m_lpBufMax)
			FillBuffer(sizeof(float) - (DWORD)(m_lpBufMax - m_lpBufCur));
		*(DWORD*)&f = *(DWORD*)m_lpBufCur; m_lpBufCur += sizeof(float); return *this; }
	CArchive& operator>>(double& d)
		{ Read(&d, sizeof(d) ); return *this; }
	CArchive& operator>>(long& l)
		{ if (m_lpBufCur + sizeof(long) > m_lpBufMax)
			FillBuffer(sizeof(long) - (DWORD)(m_lpBufMax - m_lpBufCur));
		l = *(long*)m_lpBufCur; m_lpBufCur += sizeof(long); return *this; }
};

#pragma pack(pop)

#endif // __MYARCHIVE_H_20030816__

