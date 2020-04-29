#ifndef __IP_SRC_DATA_PORT_INCLUDE_20020320__
#define __IP_SRC_DATA_PORT_INCLUDE_20020320__

typedef int SDP_HANDLE;

class CSrcDataPort
{
public:
	CSrcDataPort() :
	  m_nRef(0)
	{
		  m_nLastError = 0;
#ifdef _WIN32
		  ZeroMemory( m_szIPAddress, sizeof(m_szIPAddress) );
#else
 		  bzero( m_szIPAddress, sizeof(m_szIPAddress) );
#endif //_WIN32
		  m_wPort = 0;						//	端口
	};

	~CSrcDataPort(){};

public:
	virtual BOOL	Initialize( LPCSTR lpszIP, UINT nPort, LPCSTR lpszLocalBind = NULL, int nCount = -1 ) = 0;
	virtual void	Invalidate() = 0;
	virtual HANDLE 	GetEventHandle(SDP_HANDLE hNo)=0;
	virtual SDP_HANDLE	ReadAsyn( PBYTE pBuf, DWORD dwBufSize,PDWORD pdwByteRead ) = 0;
	virtual BOOL	ReadSync( PBYTE pBuf, DWORD dwBufSize, PDWORD pdwByteRead ) = 0;
	virtual BOOL	GetOverlappedResult( SDP_HANDLE hReadNo, PDWORD pdwByteRead, BOOL bWait ) = 0;
	virtual void	CancelAsynRead(SDP_HANDLE hReadNo) = 0;
	virtual int		GetItemCount() = 0;
	virtual void	SafeDelete() = 0;
	virtual	BOOL	CanIDoReadAsync() = 0;

public:
	long	AddRef()
	{
		return ::InterlockedIncrement( &m_nRef );
	};

	long	Release()
	{
		long nResult = ::InterlockedDecrement( &m_nRef );
		if( 0 == nResult )
			SafeDelete();
		return nResult;
	};

public:
	long	m_nLastError;
	char	m_szIPAddress[50];				//	2002.12.23 添加 IP 地址和端口参数
	WORD	m_wPort;						//	端口

private:
	long	m_nRef;
};


#endif // __IP_SRC_DATA_PORT_INCLUDE_20020320__
