//-----------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implement:
//			My StdioFile Object
//				2003.08.16
//		Note:
//			This is a File Object like MFC, but a subset.
//			This class can only be used in linux OS.
//-----------------------------------------------------------

#ifndef __MY_STDIOFILE_H_20030811__
#define __MY_STDIOFILE_H_20030811__

#include <MyFile.h>
#include <MyString.h>

#pragma pack(push,4)

class CStdioFile : public CFile
{
public:
	CStdioFile();
    CStdioFile( FILE * pOpenStream);
    ~CStdioFile();

    bool Open( const char * pszFileName, const char * pszFlags );

    virtual void Abort();
    virtual CFile * Duplicate() const;
    virtual bool Open( const char * pszFileName, unsigned int nOpenFlags );
    virtual void Close();
    virtual unsigned int Read( void * lpBuf, unsigned int nCount );
    virtual unsigned int Write( const void * lpBuf, unsigned int nCount );
    virtual void Flush();

    virtual long Seek( long lOff, unsigned int nFrom );
    virtual unsigned int GetPosition() const;

    char * ReadString( char * pszBuf, int nBufSize );
    bool ReadString( CMyString & rString );
    bool WriteString( const char * pszBuf );
public:
	FILE * m_pString;
};

#pragma pack(pop)

#endif // __MY_STDIOFILE_H_20030811__

