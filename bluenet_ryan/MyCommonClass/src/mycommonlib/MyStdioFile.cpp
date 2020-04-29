//---------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implemented:
//				My Stdio File
//				2003-08-16
//		This class can only be used in linux OS.
//-----------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <fcntl.h>
#include "MyStdioFile.h"

CStdioFile::CStdioFile()
{
	m_pString = NULL;
}

CStdioFile::CStdioFile( FILE * pOpenStream)
{
	ASSERT( pOpenStream );
    if( pOpenStream )
    {
    	m_pString = pOpenStream;
        m_hFile = fileno( pOpenStream );
    }
}

CStdioFile::~CStdioFile()
{

}

void CStdioFile::Abort()
{
	if( m_pString )
    	fclose( m_pString );
    m_pString = NULL;
    m_hFile = hFileNull;
}

CFile * CStdioFile::Duplicate() const
{
	ASSERT( false );
    return NULL;
}

bool CStdioFile::Open( const char * pszFileName, unsigned int nOpenFlags )
{
	char szFlags[4];
    int nPtr= 0;

    if( nOpenFlags & modeCreate )
    {
    	if( nOpenFlags & modeNoTruncate )
        	szFlags[ nPtr ] = 'a';
        else
        	szFlags[ nPtr ] = 'w';
    }
    else if( nOpenFlags & modeWrite )
    	szFlags[nPtr] = 'a';
    else
    	szFlags[nPtr] = 'r';
    nPtr ++;

	if( szFlags[0] == 'r' && (nOpenFlags&modeReadWrite) || \
    	(szFlags[0] != 'r' && !(nOpenFlags&modeWrite) ) )
    {
    	szFlags[nPtr++] = '+';
    }
    if( nOpenFlags & typeBinary )
    	szFlags[nPtr++] = 'b';
	szFlags[nPtr] = 0;

	return Open( pszFileName, szFlags );
}

bool CStdioFile::Open( const char * pszFileName, const char * pszFlags )
{
	m_pString = fopen( pszFileName, pszFlags );
    if( NULL == m_pString )
    	return false;

	SetFilePath( pszFileName );
	m_hFile = fileno( m_pString );

    return true;
}

void CStdioFile::Close()
{
	Abort();
}

unsigned int CStdioFile::Read( void * lpBuf, unsigned int nCount )
{
	ASSERT( m_pString );
	if( NULL == m_pString )
    	return 0;
    ASSERT( lpBuf && nCount );
    if( NULL == lpBuf || 0 == nCount )
    	return 0;
	return fread( lpBuf, 1, nCount, m_pString );
}

unsigned int CStdioFile::Write( const void * lpBuf, unsigned int nCount )
{
	ASSERT( m_pString && lpBuf && nCount );
    if( NULL == m_pString || NULL == lpBuf || 0 == nCount )
    	return 0;
    return fwrite( lpBuf, 1, nCount, m_pString );
}

void CStdioFile::Flush()
{
	ASSERT( m_pString );
    if( m_pString )
    	fflush( m_pString );
}

long CStdioFile::Seek( long lOff, unsigned int nFrom )
{
	ASSERT( m_pString );
    if( NULL == m_pString )
    	return -1;
    ASSERT( nFrom == begin || nFrom == end || nFrom == current );
    if( 0 == fseek( m_pString, lOff, nFrom ) )
		return ftell( m_pString );
	return -1;
}

unsigned int CStdioFile::GetPosition() const
{
	ASSERT( m_pString );
    if( NULL == m_pString )
    	return 0;
    return ftell( m_pString );
}

//------------------------------------------
// Function:
//		Get one line string
// Input Parameter:
//		pszBuf			out buffer
//		nBufSize		out buffer size
// Output Parameter:
//		NULL			failed
//		else			string
char * CStdioFile::ReadString( char * pszBuf, int nBufSize )
{
	ASSERT( m_pString );
    if( NULL == m_pString )
    	return NULL;
    return fgets( pszBuf, nBufSize, m_pString );
}

bool CStdioFile::ReadString( CMyString & rString )
{
	ASSERT( m_pString );
    if( NULL == m_pString )
    	return false;

	int nSize = 128;
    int nOldSize = 0;
    while( !feof( m_pString ) )
    {
    	char * pszBuf = rString.GetBuffer( nSize + 1 );
        if( NULL == pszBuf )
        	return false;
        pszBuf += nOldSize;
		if( fgets( pszBuf, nSize, m_pString ) )
        {
			rString.ReleaseBuffer();
            int nLen = rString.GetLength();
            if( nLen )
            {
				if( rString[nLen-1] == '\n')
                {
                	rString.ReleaseBuffer( nLen - 1 );
		        	return true;
                }
                else if( feof( m_pString ) )
                	return true;
            }
        }
        nOldSize = rString.GetLength();
		nSize += 128;
    }
    return false;
}

bool CStdioFile::WriteString( const char * pszBuf )
{
	ASSERT( m_pString );
    if( NULL == m_pString )
    	return false;
    int nLen = strlen( pszBuf );
    return ( nLen == (int)fwrite( pszBuf, 1, nLen, m_pString ) );
}

