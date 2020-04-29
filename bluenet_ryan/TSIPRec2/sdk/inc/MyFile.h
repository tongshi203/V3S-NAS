//-----------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implement:
//			My File Object
//
//		Note:
//			This is a File Object like MFC, but a subset.
//			This class can only be used in linux OS.
//-----------------------------------------------------------

#ifndef __MY_FILE_H_20030811__
#define __MY_FILE_H_20030811__

#include "MyTime.h"

#pragma pack(push,4)

class CFileStatus
{
public:
	CFileStatus();

public:
	CTime	m_ctime;
    CTime	m_mtime;
    CTime 	m_atime;
    long	m_size;
    BYTE	m_attribute;
    BYTE	m_padding;
    char	m_szFullName[260];
};

class CFile
{
public:
	CFile();
    virtual ~CFile();

    virtual void Abort();
    virtual CFile * Duplicate() const;
    virtual bool Open( const char * pszFileName, unsigned int nOpenFlags );
    virtual void Close();
    virtual unsigned int Read( void * lpBuf, unsigned int nCount );
    virtual unsigned int Write( const void * lpBuf, unsigned int nCount );
    virtual void Flush();

    virtual long Seek( long lOff, unsigned int nFrom );
    virtual bool SeekToBegin();
    virtual long SeekToEnd();

    virtual unsigned long GetLength();
    virtual unsigned long GetLength(const char * pszFileName) const;
    virtual bool SetLength( unsigned long dwNewLen );

    virtual unsigned int GetPosition() const;
    virtual const char * GetFileName() const;
    virtual const char * GetFilePath() const;
	const char * SetFilePath(const char * lpszFileName);

    static bool Rename( const char * lpszOldName, const char * lpszNewName );
    static bool Remove( const char * lpszFileName );

    bool GetStatus( CFileStatus & rStatus ) const;
    static bool GetStatus( const char * lpszFileName, CFileStatus & rStatus);

	enum{ hFileNull = 0xFFFFFFFF };			//	open file failed.
    enum OpenFlags{	 				//	file open mode
    	modeCreate = 0x1000,
        modeNoTruncate = 0x2000,
        modeRead = 0,
        modeWrite = 1,
        modeReadWrite = 2,
        shareDenyNone = 0x40,
        shareDenyRead = 0x30,
        shareDenyWrite = 0x20,
        shareExclusive = 0x10,
		typeText = 0x4000,
        typeBinary = 0x8000,
    };

    enum SeekPosition{ begin = 0,current = 1,end = 2 };
    enum BufferCommand{ bufferRead, bufferWrite, bufferCommit, bufferCheck };

public:
    unsigned int m_hFile;

protected:
    char * m_pszFileName;		//	pointer to file name buffer
};

#pragma pack(pop)

#endif // __MY_FILE_H_20030811__

