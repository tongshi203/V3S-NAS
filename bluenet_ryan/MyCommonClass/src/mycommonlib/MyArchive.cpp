//---------------------------------------------------------
//
//      Chen Yongjian @ Xi'an Tongshi Technology Limited
//
//      This file is implemented:
//			My Achive
//-----------------------------------------------------------

#include "stdafx.h"
#include "MyArchive.h"

////////////////////////////////////////////////////////////////////////////
// Archive object input/output

// minimum buffer size
enum { nBufSizeMin = 128 };

// default amount to grow m_pLoadArray upon insert
enum { nGrowSize = 64 };
// default size of hash table in m_pStoreMap when storing
enum { nHashSize = 137 };
// default size to grow collision blocks when storing
enum { nBlockSize = 16 };


CArchive::CArchive(CFile* pFile, UINT nMode, int nBufSize, void* lpBuf)
{
	// initialize members not dependent on allocated buffer
	m_nMode = nMode;
	m_pFile = pFile;

	// initialize the buffer.  minimum size is 128
	m_lpBufStart = (BYTE*)lpBuf;
	m_bUserBuf = TRUE;

	if (nBufSize < nBufSizeMin)
	{
		// force use of private buffer of minimum size
		m_nBufSize = nBufSizeMin;
		m_lpBufStart = NULL;
	}
	else
		m_nBufSize = nBufSize;

	nBufSize = m_nBufSize;
	if (m_lpBufStart == NULL)
	{
        // no support for direct buffering, allocate new buffer
        m_lpBufStart = new BYTE[m_nBufSize];
        m_bUserBuf = FALSE;
	}

	ASSERT(m_lpBufStart != NULL);

	m_lpBufMax = m_lpBufStart + nBufSize;
	m_lpBufCur = (IsLoading()) ? m_lpBufMax : m_lpBufStart;
}

CArchive::~CArchive()
{
	if (m_pFile != NULL && !(m_nMode & bNoFlushOnDelete))
		Close();

	Abort();    // abort completely shuts down the archive
}

BOOL CArchive::IsLoading() const
{
	return (m_nMode & load) != 0;
}

BOOL CArchive::IsStoring() const
{
	return (m_nMode & load) == 0;
}

BOOL CArchive::IsBufferEmpty() const
{
	return m_lpBufCur == m_lpBufMax;
}

CFile* CArchive::GetFile() const
{
	return m_pFile;
}

UINT CArchive::Read(void* lpBuf, UINT nMax)
{
	ASSERT( lpBuf && nMax );
    if( NULL == lpBuf || nMax == 0 )
		return 0;

	ASSERT( m_lpBufStart && m_lpBufCur );
	ASSERT( IsLoading() );

	// try to fill from buffer first
	UINT nMaxTemp = nMax;
	UINT nTemp = min(nMaxTemp, (UINT)(m_lpBufMax - m_lpBufCur));
    if( nTemp )
    {						  //	if Not empty
		memcpy(lpBuf, m_lpBufCur, nTemp);
		m_lpBufCur += nTemp;
        lpBuf = (BYTE*)lpBuf + nTemp;
		nMaxTemp -= nTemp;
    }

	if( 0 == nMaxTemp )
    	return nMax;		  //    Read succ

    ASSERT(m_lpBufCur == m_lpBufMax);

    // read rest in buffer size chunks
    nTemp = nMaxTemp - (nMaxTemp % m_nBufSize);
    UINT nRead = 0;

    UINT nLeft = nTemp;
    UINT nBytes;
    do
    {
        nBytes = m_pFile->Read(lpBuf, nLeft);
        lpBuf = (BYTE*)lpBuf + nBytes;
        nRead += nBytes;
        nLeft -= nBytes;
    }
    while ((nBytes > 0) && (nLeft > 0));

    nMaxTemp -= nRead;

    // read last chunk into buffer then copy
    if (nRead == nTemp)
    {
        ASSERT(m_lpBufCur == m_lpBufMax);
        ASSERT(nMaxTemp < (UINT)m_nBufSize);

        // fill buffer (similar to CArchive::FillBuffer, but no exception)
        UINT nLeft = max(nMaxTemp, (UINT)m_nBufSize);
        UINT nBytes;
        BYTE* lpTemp = m_lpBufStart;
        nRead = 0;
        do
        {
            nBytes = m_pFile->Read(lpTemp, nLeft);
            lpTemp = lpTemp + nBytes;
            nRead += nBytes;
            nLeft -= nBytes;
        }
        while ((nBytes > 0) && (nLeft > 0) && nRead < nMaxTemp);

        m_lpBufCur = m_lpBufStart;
        m_lpBufMax = m_lpBufStart + nRead;

        // use first part for rest of read
        nTemp = min(nMaxTemp, (UINT)(m_lpBufMax - m_lpBufCur));
        memcpy(lpBuf, m_lpBufCur, nTemp);
        m_lpBufCur += nTemp;
        nMaxTemp -= nTemp;
    }
	return nMax - nMaxTemp;
}

void CArchive::Write(const void* lpBuf, UINT nMax)
{
	if (nMax == 0)
		return;

	ASSERT(lpBuf != NULL);
	ASSERT(m_lpBufStart != NULL);
	ASSERT(m_lpBufCur != NULL);
	ASSERT(IsStoring());

	// copy to buffer if possible
	UINT nTemp = min(nMax, (UINT)(m_lpBufMax - m_lpBufCur));
	memcpy(m_lpBufCur, lpBuf, nTemp);
	m_lpBufCur += nTemp;
	lpBuf = (BYTE*)lpBuf + nTemp;
	nMax -= nTemp;

	if (nMax > 0)
	{
		Flush();    // flush the full buffer

		// write rest of buffer size chunks
		nTemp = nMax - (nMax % m_nBufSize);
		m_pFile->Write( lpBuf, nTemp );
		lpBuf = (BYTE*)lpBuf + nTemp;
		nMax -= nTemp;
		// copy remaining to active buffer
		ASSERT(nMax < (UINT)m_nBufSize);
		ASSERT(m_lpBufCur == m_lpBufStart);
		memcpy(m_lpBufCur, lpBuf, nMax);
		m_lpBufCur += nMax;
	}
}

void CArchive::Flush()
{
	ASSERT( m_lpBufStart != NULL );
	ASSERT( m_lpBufCur != NULL );
	if (IsLoading())
	{
		// unget the characters in the buffer, seek back unused amount
		if (m_lpBufMax != m_lpBufCur)
			m_pFile->Seek(-(m_lpBufMax - m_lpBufCur), CFile::current);
		m_lpBufCur = m_lpBufMax;    // empty
	}
	else
	{
        // write out the current buffer to file
        if (m_lpBufCur != m_lpBufStart)
            m_pFile->Write(m_lpBufStart, m_lpBufCur - m_lpBufStart);
		m_lpBufCur = m_lpBufStart;
	}
}

void CArchive::Close()
{
	Flush();
	m_pFile = NULL;
}

void CArchive::Abort()
{
	// disconnect from the file
	m_pFile = NULL;

	if (!m_bUserBuf)
	{
		delete[] m_lpBufStart;
		m_lpBufStart = NULL;
		m_lpBufCur = NULL;
	}
}

void CArchive::WriteString(LPCSTR lpsz)
{
	ASSERT( lpsz );
    if( NULL == lpsz )
    	return;
	Write(lpsz, strlen(lpsz));
}

//------------------------------------------
// Function:
//		read string
// Input Parameter:
//		lpsz		output buffer
//		nMax		output buffer size
// Output Parameter:
//		NULL		failed
char *  CArchive::ReadString(char *  lpsz, UINT nMax)
{
	// 2gb address space), then assume it to mean "keep the newline".
	int nStop = (int)nMax < 0 ? -(int)nMax : (int)nMax;
	ASSERT( lpsz && nMax );
    if( NULL == lpsz || 0 == nMax )
    	return NULL;

	char ch;
	int nRead = 0;

    while (nRead < nStop)
    {
        if( Read(&ch,1) != 1 )
        {
        	if( 0 == nRead )
            	return 0;
          	break;
        }

        // stop and end-of-line (trailing '\n' is ignored)
        if (ch == '\n' || ch == '\r')
        {
            if (ch == '\r')
                Read( &ch, 1 );
            // store the newline when called with negative nMax
            if ((int)nMax != nStop)
                lpsz[nRead++] = ch;
            break;
        }
        lpsz[nRead++] = ch;
    }

	lpsz[nRead] = '\0';
	return lpsz;
}

BOOL CArchive::ReadString(CMyString& rString)
{
	rString.ReleaseBuffer( 0 );		//	release and set length = 0
	const int nMaxSize = 128;
	char * lpsz = rString.GetBuffer(nMaxSize+1);
	char * lpszResult;
	int nLen;
	for (;;)
	{
		lpszResult = ReadString(lpsz, (UINT)-nMaxSize); // store the newline
		rString.ReleaseBuffer();

		// if string is read completely or EOF
		if (lpszResult == NULL ||(nLen = strlen(lpsz)) < nMaxSize ||\
        	lpsz[nLen-1] == '\n')
		{
			break;
		}

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen+1) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen-1] == '\n')
		rString.ReleaseBuffer( nLen-1 );

	return lpszResult != NULL;
}

bool CArchive::FillBuffer(UINT nBytesNeeded)
{
	UINT nUnused = m_lpBufMax - m_lpBufCur;
	DWORD nTotalNeeded = ((DWORD)nBytesNeeded) + nUnused;

	// fill up the current buffer from file
    ASSERT(m_lpBufCur != NULL);
    ASSERT(m_lpBufStart != NULL);
    ASSERT(m_lpBufMax != NULL);

    if (m_lpBufCur > m_lpBufStart)
    {
        // copy unused
        if ((int)nUnused > 0)
        {
            memmove(m_lpBufStart, m_lpBufCur, nUnused);
            m_lpBufCur = m_lpBufStart;
            m_lpBufMax = m_lpBufStart + nUnused;
        }

        // read to satisfy nBytesNeeded or nLeft if possible
        UINT nRead = nUnused;
        UINT nLeft = m_nBufSize-nUnused;
       UINT nBytes;
        BYTE* lpTemp = m_lpBufStart + nUnused;
        do
        {
            nBytes = m_pFile->Read(lpTemp, nLeft);
            lpTemp = lpTemp + nBytes;
            nRead += nBytes;
            nLeft -= nBytes;
        }
        while (nBytes > 0 && nLeft > 0 && nRead < nBytesNeeded);

        m_lpBufCur = m_lpBufStart;
        m_lpBufMax = m_lpBufStart + nRead;
    }

	// not enough data to fill request?
	if ((DWORD)(m_lpBufMax - m_lpBufCur) < nTotalNeeded)
		return false;
    return true;
}

CArchive::CArchive(const CArchive& arSrc)
{
	ASSERT( false );
}

void CArchive::operator=(const CArchive& arSrc)
{
	ASSERT( false );
}

///------------------------------------------
/// Function:
///		serialize string
/// Input Parameter:
///
/// Output Parameter:
///
CArchive& CArchive::operator<<(CMyString &strValue)
{
	int nStrLen = strValue.GetLength();
	if(nStrLen < 255)
	{
		operator << ( (BYTE)nStrLen );
	}
	else if (nStrLen < 0xfffe)
	{
		operator << ( (BYTE)0xff );
		operator << ( (WORD)nStrLen );
	}
	else
	{
		operator << ( (BYTE)0xff );
		operator << ( (WORD)0xffff );
		operator << ( (DWORD)nStrLen );
	}
	Write( (LPCSTR)strValue, nStrLen );

	return *this;
}

// return string length or -1 if UNICODE string is found in the archive
static DWORD ReadStringLengthHelper(CArchive& ar)
{
	DWORD nNewLen;

	// attempt BYTE length first
	BYTE bLen;
	ar >> bLen;

	if (bLen < 0xff)
		return bLen;

	// attempt WORD length
	WORD wLen;
	ar >> wLen;
	if (wLen == 0xfffe)
	{
		// UNICODE string prefix (length will follow)
		return (UINT)-1;
	}
	else if (wLen == 0xffff)
	{
		// read DWORD of length
		ar >> nNewLen;
		return (UINT)nNewLen;
	}
	else
		return wLen;
}


CArchive& CArchive::operator>>(CMyString &strValue)
{
    strValue = "";
	DWORD nNewLen = ReadStringLengthHelper( *this );
	if (nNewLen == (DWORD)-1 || 0 == nNewLen )
    	return *this;

    char * pBufTmp = strValue.GetBuffer( nNewLen +1 );

	if( Read(pBufTmp, nNewLen ) != nNewLen )
		strValue = "";
    else
    	pBufTmp[nNewLen] = 0;

	return *this;
}
