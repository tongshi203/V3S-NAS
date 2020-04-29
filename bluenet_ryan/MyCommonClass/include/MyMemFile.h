//---------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implemented:
//			Memory File
//-----------------------------------------------------------

#ifndef __MY_MEM_FILE_H_20030815__
#define __MY_MEM_FILE_H_20030815__

#include <MyComDef.h>
#include <MyFile.h>

#pragma pack(push,4)

class CMemFile : public CFile
{
public:
// Constructors
	CMemFile(UINT nGrowBytes = 1024);
	CMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);

// Operations
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
	BYTE* Detach();

// Advanced Overridables
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void Free(BYTE* lpMem);
	virtual bool GrowFile(DWORD dwNewLen);

// Implementation
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE* m_lpBuffer;
	BOOL m_bAutoDelete;

public:
	virtual ~CMemFile();
	virtual DWORD GetPosition() const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
    virtual bool SeekToBegin();
    virtual long SeekToEnd();
	virtual bool SetLength( unsigned long dwNewLen );
    virtual unsigned long GetLength();
    virtual unsigned int Write(const void * lpBuf, unsigned int nCount );
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

#pragma pack(pop)

#endif //__MY_MEM_FILE_H_20030815__

