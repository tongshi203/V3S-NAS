/**************************************************************************************
 *
 *	Mapped file
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.7 @ Xi'an
 *
 **************************************************************************************/

#define __USE_LARGEFILE64
#define __USE_FILE_OFFSET64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

#include "my_map_file.h"

CMyMappedFile::CMyMappedFile()
{
	m_hFile = -1;
	m_bIsReadOnly = true;
	m_llFileLength = 0;
	m_pBufferMapped = NULL;
}

CMyMappedFile::~CMyMappedFile()
{
	Close();
}

//--------------------------------------------------------------
/** CYJ 2015-04-07
 *
 *	Open file name and map it
 *
 * @param [in]	pszFileName			data file name
 * @param [in]	llFileSizeNeed		file size needed
 * @param [in]	bReadOnly			read only
 * @param [in]	dwCreateMode		if file not exist, create mode
 *
 * @return		0					succ
 *				other				error code
 * @note
 *	1. If file not exist, create it
 *	2. If file size if small than llFileSizeNeed, file it with 0
 *	3. If file size if large then llFileSizeNeed, only [ 0, llFileSizeNeed) is mapped
 */
int CMyMappedFile::Open( const char *pszFileName, int64_t llFileSizeNeed, bool bReadOnly, uint32_t dwCreateMode )
{
	Close();

	if( 0 == dwCreateMode )
		dwCreateMode = S_IRUSR|S_IWUSR|S_IRGRP;

	m_bIsReadOnly = bReadOnly;
	m_llFileLength = llFileSizeNeed;

	int nRetVal = PrepareFile( pszFileName, llFileSizeNeed, dwCreateMode );
	if( nRetVal )
	{
		nRetVal = errno;
	#ifdef _DEBUG
		fprintf(stderr, "CMyMappedFile::Open, PrepareFile failed. errno=%d\n", nRetVal);
	#endif // _DEBUG

		return nRetVal;
	}

	int nFlags = bReadOnly ? O_RDONLY : O_RDWR;
	m_hFile = open64( pszFileName, nFlags|O_LARGEFILE, dwCreateMode );
	if( m_hFile < 0 )
	{
		nRetVal = errno;

	#ifdef _DEBUG
		fprintf(stderr, "CMyMappedFile::Open, open(%s) failed, \n", pszFileName, nRetVal );
	#endif // _DEBUG

		return nRetVal;
	}

	int nProtected = bReadOnly ? PROT_READ : PROT_READ|PROT_WRITE;
	m_pBufferMapped = (uint8_t*) mmap64( NULL, m_llFileLength, nProtected, MAP_SHARED, m_hFile, 0 );
    if( NULL == m_pBufferMapped )
    {
    	nRetVal = errno;

	#ifdef _DEBUG
		fprintf( stderr, "CMyMappedFile::Open, map file failed (%d).\n", nRetVal );
	#endif // _DEBUG

        Close();
        return nRetVal;
    }

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-07
 *
 *	Prepare file size
 *
 * @param [in]	pszFileName			file name
 * @param [in]	llFileSizeNeed		file size need
 *
 * @return		0					succ
 *				other				failed
 */
int CMyMappedFile::PrepareFile( const char *pszFileName, int64_t llFileSizeNeed, uint32_t dwCreateMode )
{
	if( access( pszFileName, F_OK ) )
	{	// file not exist
		int hFile = open( pszFileName, O_RDWR|O_CREAT, dwCreateMode );
		if( hFile < 0 )
			return errno;
		close( hFile );

		return truncate64(pszFileName, llFileSizeNeed );
	}

	struct stat64 stat_data;
	int nRetVal = stat64( pszFileName, &stat_data );
	if( nRetVal )
	{	// stat failed.
	#ifdef _DEBUG
		assert( false );			// should not be here
	#endif // _DEBUG
		return truncate64(pszFileName, llFileSizeNeed );
	}

    if( stat_data.st_size >= llFileSizeNeed )
		return 0;					// large enough

	// no enough size, extended it, filled with 0
	return truncate64( pszFileName, llFileSizeNeed );
}

//--------------------------------------------------------------
/** CYJ 2015-04-07
 *
 *	Unmap and close the file
 */
void CMyMappedFile::Close()
{
	if( m_pBufferMapped )
	{	// has been mapped, unmap
		munmap( (void*)m_pBufferMapped, m_llFileLength );
		m_pBufferMapped = NULL;
	}

	if( m_hFile >= 0 )
	{
		close( m_hFile );
		m_hFile = -1;
	}
}

//--------------------------------------------------------------
/** CYJ 2015-04-07
 *
 *	Get Mapped memory address
 *
 * @param [in]	llOffset			offset
 *
 * @return		NULL				failed
 *				other				mapped address
 */
uint8_t * CMyMappedFile::GetBuffer( int64_t llOffset )
{
	if( NULL == m_pBufferMapped || llOffset >= m_llFileLength )
		return NULL;

	return m_pBufferMapped + llOffset;
}

//--------------------------------------------------------------
/** CYJ 2015-04-07
 *
 *	Sync data from memory to file.
 *
 * @param [in]	llOffset			offset
 * @param [in]	llLength			length to be commit
 *
 * @return		0					succ
 *				other				error code
 */
int CMyMappedFile::SyncToFile( int64_t llOffset, int64_t llLength )
{
	if( NULL == m_pBufferMapped )
		return 0;
	if( llOffset >= m_llFileLength )
		return EINVAL;
	if( llOffset + llLength >= m_llFileLength )
		llLength = m_llFileLength - llOffset;
	return msync( m_pBufferMapped + llOffset, llLength, MS_ASYNC );
}

