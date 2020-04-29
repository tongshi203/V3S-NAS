/**************************************************************************
 *
 *	Auto delete buffer helper class
 *
 *	Chen Yongjian @ Zhoi
 *	2015.1.27 @ Xi'an
 *
 *
 ***************************************************************************/

#ifndef __MY_RUDP_AUTO_DELETE_BUF_H_20150127__
#define __MY_RUDP_AUTO_DELETE_BUF_H_20150127__

class CMyRUDP_AutoDeleteBuf
{
public:
	CMyRUDP_AutoDeleteBuf( int nBufSize = 0 )
	{
		if( nBufSize > 0 )
			m_pBuf = new unsigned char [ nBufSize ];
		else
			m_pBuf = NULL;
		m_nBufSize = nBufSize;
		m_nDataLen = 0;
	}

	CMyRUDP_AutoDeleteBuf( unsigned char * pBuf, int nBufSize )
	{
		m_pBuf = pBuf;
		m_nBufSize = nBufSize;
		m_nDataLen = 0;
	}

	virtual ~CMyRUDP_AutoDeleteBuf()
	{
		if( m_pBuf )
			delete m_pBuf;
	}

	bool Allocate( int nBufSize )
	{
		if( m_pBuf )
			delete m_pBuf;
		m_pBuf = new unsigned char [ nBufSize ];
		m_nBufSize = nBufSize;
		return m_pBuf != NULL;
	}

	bool Copy( const unsigned char *pBuf, int nLen )
	{
		if( NULL == pBuf || nLen <= 0 )
			return false;

        if( false == Allocate( nLen ) )
			return false;

		memcpy( m_pBuf, pBuf, nLen );
		m_nDataLen = nLen;

		return true;
	}

	bool IsValid() { return ( m_pBuf != NULL ); }

	unsigned char *GetBuffer() { return m_pBuf; }
	int GetBufSize() { return m_nBufSize; }
	int GetDataLen() { return m_nDataLen; }
	void SetDataLen( int nNewLen ) { m_nDataLen = nNewLen; }

	operator unsigned char *() { return m_pBuf; }
	unsigned char & operator [](int nIndex ) { return m_pBuf[nIndex]; }

protected:
	unsigned char * m_pBuf;
	int 			m_nBufSize;
	int 			m_nDataLen;
};

#endif // __MY_RUDP_AUTO_DELETE_BUF_H_20150127__
