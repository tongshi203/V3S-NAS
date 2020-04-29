#include "stdafx.h"

#include "MyFile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

CFileStatus::CFileStatus()
{
	m_size = 0;
    m_attribute = 0;
    m_padding = 0;
    bzero( m_szFullName, sizeof(m_szFullName) );
}

CFile::CFile()
{
    m_hFile = hFileNull;
    m_pszFileName = NULL;		//	pointer to file name buffer
}

CFile::~CFile()
{
	if( hFileNull != m_hFile )
    	Close();

	if( m_pszFileName )
    	delete m_pszFileName;
}

void CFile::Abort()
{
	if( hFileNull != m_hFile )
		Close();
}

//------------------------------------------
// Function:
//		duplicate file handle
// Input Parameter:
//		none
// Output Parameter:
//		NULL			failed.
//		!NULL			new file object
CFile * CFile::Duplicate() const
{
	ASSERT( hFileNull != m_hFile && m_pszFileName && *m_pszFileName );
    if( hFileNull == m_hFile )
    	return NULL;
	CFile * pFile = new CFile;
    if( NULL == pFile )
    	return NULL;
    pFile->m_hFile = dup( m_hFile );
    if ( pFile->m_hFile < 0 )
    {
    	delete pFile;
        pFile = NULL;
    }
    else
    	pFile->SetFilePath( m_pszFileName );

    return pFile;
}

//------------------------------------------
// Function:
//		open file by pszFileName and nOpenFlags
// Input Parameter:
//		pszFileName		the file to be opened
//		nOpenFlags		open flags
// Output Parameter:
//		true			succ
//		false			false
bool CFile::Open( const char * pszFileName, unsigned int nOpenFlags )
{
	ASSERT( pszFileName && *pszFileName );
    if( NULL == pszFileName || 0 == *pszFileName )
    	return false;
	int nReadWriteFlags = nOpenFlags & 3;		// the low 2 bits
    if( nOpenFlags & modeCreate )
	    nReadWriteFlags |= O_CREAT;
    if( (nReadWriteFlags != modeRead) && (0 == (nOpenFlags & modeNoTruncate)) )
    	nReadWriteFlags |= O_TRUNC;

	// 2012.1.16 CYJ Add
	const int nMode = S_IRWXU|S_IRGRP|S_IROTH;

	m_hFile = open( pszFileName, nReadWriteFlags, nMode );
    if( long(m_hFile) < 0 )
    {
    	m_hFile = hFileNull;
    	return false;
    }

    SetFilePath( pszFileName );		//	setup the file name

	return true;
}

//------------------------------------------
// Function:
//		Close file object
// Input Parameter:
//		None
// Output Parameter:
//		None
void CFile::Close()
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return;

    close( m_hFile );
    m_hFile = hFileNull;
}

//------------------------------------------
// Function:
//		Read data
// Input Parameter:
//		lpBuf			the buffer to be fill with data
//		nCount			bytes to read
// Output Parameter:
//		the byte count that actually read
unsigned int CFile::Read( void * lpBuf, unsigned int nCount )
{
    ASSERT( lpBuf );
    if( NULL == lpBuf || 0 == nCount )
    	return 0;
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return 0;

    return read( m_hFile, lpBuf, nCount );
}

//------------------------------------------
// Function:
//		write data
// Input Parameter:
//		lpBuf			the data buffer to be writen
//		nCount			bytes to be writen
// Output Parameter:
//		actual count that wrote
unsigned int CFile::Write( const void * lpBuf, unsigned int nCount )
{
	ASSERT( lpBuf && nCount );
    if( NULL == lpBuf || 0 == nCount )
    	return 0;
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return 0;
    return write( m_hFile, lpBuf, nCount );
}

//------------------------------------------
// Function:
//		flush data to disk
// Input Parameter:
//		none
// Output Parameter:
//		none
void CFile::Flush()
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return;

    fsync( m_hFile );
}

//------------------------------------------
// Function:
//		Seek position
// Input Parameter:
//		lOff			offset to seek
//		nFrom			seek method, begin, current, or end
// Output Parameter:
//		<0				failed
//		>=0				new position after seeked
long CFile::Seek( long lOff, unsigned int nFrom )
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return -1;

	int nSeekMethod;
    switch( nFrom )
    {
    case begin:
    	nSeekMethod = SEEK_SET;
    	break;
    case current:
    	nSeekMethod = SEEK_CUR;
        break;
    case end:
    	nSeekMethod = SEEK_END;
        break;
    default:
    	ASSERT( false );
        return -1;
    }

	return lseek( m_hFile, lOff, nSeekMethod );
}

//------------------------------------------
// Function:
//		Seek to begin
// Input Parameter:
//		none
// Output Parameter:
//		true			succ
//		false			failed
bool CFile::SeekToBegin()
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return false;
	return lseek( m_hFile, 0, SEEK_SET );
}

//------------------------------------------
// Function:
//		seek to the end
// Input Parameter:
//		none
// Output Parameter:
//		<0				failed
//		the file len
long CFile::SeekToEnd()
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    	return 0;
    return lseek( m_hFile, 0, SEEK_END );
}

//------------------------------------------
// Function:
//		get file length
// Input Parameter:
//		none
// Output Parameter:
//		the file length
unsigned long CFile::GetLength()
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
    {
		if( m_pszFileName )
			return GetLength( m_pszFileName );
        ASSERT( false );
		return 0;
    }

	long nCurPos = GetPosition();
    long nFileLen = SeekToEnd();
    Seek( nCurPos, begin );

    return nFileLen;
}

unsigned long CFile::GetLength(const char * pszFileName) const
{
	ASSERT( pszFileName && *pszFileName );
    if( NULL == pszFileName )
    	return 0;

	struct stat status;
    if( lstat( pszFileName, &status ) != 0 )
    	return 0;
    return status.st_size;
}

//------------------------------------------
// Function:
//		set file length
// Input Parameter:
//		dwNewLen			new file len
// Output Parameter:
//		true				succ
//		false				failed
bool CFile::SetLength( unsigned long dwNewLen )
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
	   	return 0;
#ifdef _WIN32
	ASSERT( FALSE );		// to be implement
#else
	return 0 == ftruncate( m_hFile, dwNewLen );
#endif //_WIN32
}

//------------------------------------------
// Function:
//		Get current postion
// Input Parameter:
//      none
// Output Parameter:
//      current position
unsigned int CFile::GetPosition() const
{
	ASSERT( hFileNull != m_hFile );
    if( hFileNull == m_hFile )
	   	return 0;

	return lseek( m_hFile, 0, SEEK_CUR );
}

//------------------------------------------
// Function:
//		Get File name
// Input Parameter:
//		none
// Output Parameter:
//		null		failed
//		!NULL		file name only, not include the path
const char * CFile::GetFileName() const
{
	if( NULL == m_pszFileName )
    	return NULL;
#ifdef _WIN32
    char * pszFileName = strrchr( m_pszFileName, '\\' );
#else  // Linux
	char * pszFileName = strrchr( m_pszFileName, '/' );
#endif // _WIN32
	if( NULL == pszFileName )
    	return m_pszFileName;
    else
    	return (pszFileName + 1);
}

//------------------------------------------
// Function:
//		get full path file name
// Input Parameter:
//		none
// Output Parameter:
//		full file path
const char * CFile::GetFilePath() const
{
	return m_pszFileName;
}

//------------------------------------------
// Function:
//		rename a file
// Input Parameter:
//		lpszOldName		the exist file name
//		lpszNewName		the file name to be renamed
// Output Parameter:
//		true			succ
//		false			failed
bool CFile::Rename( const char * lpszOldName, const char * lpszNewName )
{
	ASSERT( lpszOldName && lpszNewName );
    if( NULL == lpszOldName || NULL == lpszNewName )
    	return false;

    rename( lpszOldName, lpszNewName );

	return false;
}

//------------------------------------------
// Function:
//		delete one file
// Input Parameter:
//		lpszFileName	the file name to be deleted
// Output Parameter:
//		true			succ
//		fals			failed
bool CFile::Remove( const char * lpszFileName )
{
	ASSERT( lpszFileName && *lpszFileName );
    if( NULL == lpszFileName || 0 == *lpszFileName )
    	return false;
	return 0 == unlink( lpszFileName );
}

//------------------------------------------
// Function:
//		Set file path only
// Input Parameter:
//		lpszFileName		file name to be copied
// Output Parameter:
//		NULL				failed
//		!NULL				succ and copy the file name to m_pszFileName buffer
const char * CFile::SetFilePath(const char * lpszFileName)
{
    int nLen = strlen( lpszFileName );
    ASSERT( nLen > 0 );
    if( 0 == nLen )
    	return NULL;

	if( m_pszFileName )
	    delete m_pszFileName;

    m_pszFileName = new char[ nLen + 1 ];
    if( NULL == m_pszFileName )
    	return NULL;
	strcpy( m_pszFileName, lpszFileName );

    return m_pszFileName;
}

///------------------------------------------
/// Function:
///		Get file status
/// Input Parameter:
///		rStatus			output status
/// Output Parameter:
///		true			succ
///		false			failed
bool CFile::GetStatus( CFileStatus & rStatus ) const
{
	if( NULL == m_pszFileName )
    	return false;
	return GetStatus( m_pszFileName, rStatus );
}

///------------------------------------------
/// Function:
///		Get file status
/// Input Parameter:
///		lpszFileName	file name
///		rStatus			output status
/// Output Parameter:
///		true			succ
///		false			failed
bool CFile::GetStatus( const char * lpszFileName, CFileStatus & rStatus)
{
	ASSERT( lpszFileName );
    if( NULL == lpszFileName )
    	return false;

	struct stat staTmp;
    if( stat( lpszFileName, &staTmp ) < 0 )
    	return false;

    rStatus.m_ctime = staTmp.st_ctime;
    rStatus.m_mtime = staTmp.st_mtime;
    rStatus.m_atime = staTmp.st_atime;
    rStatus.m_size = staTmp.st_size;
    strncpy( rStatus.m_szFullName, lpszFileName, sizeof(rStatus.m_szFullName)-1 );
    rStatus.m_szFullName[ sizeof(rStatus.m_szFullName)-1 ]=0;
    rStatus.m_attribute = 0;
    rStatus.m_padding = 0;

    return true;
}

