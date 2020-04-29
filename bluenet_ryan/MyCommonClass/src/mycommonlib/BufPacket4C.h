// IpBroPacket.h: interface for the CIpBroPacket class.
//
//////////////////////////////////////////////////////////////////////
//	2002.11.15	添加函数，IsBufAttached
//	2002.7.5	修改 SetBufSize，添加属性 m_dwAlignment，提高内存分配方式

#if !defined(AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_)
#define AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_

#pragma pack(push,4)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdlib.h>
#include "IPRecDrvInterface.h"

template < class TBase >
class CBufPacket4C : public TBase
{
//	interface of IUnknown
public:
	virtual long AddRef(void)
	{
		return InterlockedIncrement( &m_nRef );
	}

	virtual long Release(void)
	{
		if( 0 == InterlockedDecrement( &m_nRef ) )
		{
			ASSERT( FALSE == m_bIsAttached );
			if( m_pBuf )
				free( m_pBuf );
			m_pBuf = NULL;
			SafeDelete();
			return 0;
		}
		return m_nRef;
	}

	virtual void SafeDelete() = 0;

	virtual DWORD QueryInterface( REFIID iid, void **ppvObject)
	{
		if( IID_IMyUnknown == iid || IID_IMyBufPacket == iid )
		{
			AddRef();
			*ppvObject = static_cast<IMyUnknown*>(this);
			return 0;         // S_OK
		}
		return 0x80004002;    //  E_NOINTERFACE;
	}

	void Preset();

//	interface of IpBroPacket
public:		
	//	申请使用内存，nHeadLen 申请的内存字节数；返回地址，为NULL表示失败
	virtual PBYTE	AcquireHeadBuf( int nHeadLen )
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		ASSERT( DWORD(nHeadLen) <= m_dwReservedBytes );

		if( nHeadLen > long(m_dwReservedBytes) )
			return NULL;

		m_dwReservedBytes -= nHeadLen;
		m_dwDataLen += nHeadLen;
		return ( m_pBuf + m_dwReservedBytes );
	}


	virtual BOOL	SetBufSize( DWORD dwNewValue )
	{
		ASSERT( FALSE == m_bIsAttached );
		if( m_bIsAttached )
			return FALSE;

		ASSERT( 0 == m_dwBufSize || m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( 0 == m_dwBufSize || (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		Preset();

		DWORD dwMemSize = dwNewValue + m_dwDefReservedBytes;
														//  2003-4-1 添加一个条件 m_pBuf = NULL，则也重新分配
		if( dwMemSize > m_dwBufSize || 0 == dwMemSize || NULL == m_pBuf ||\
			( 2*dwMemSize < m_dwBufSize && m_dwBufSize > 20*1024 ) )	//	超过20K就释放内存
		{												//	需要重新分配
			m_dwBufSize = 0;
			if( 0 == dwMemSize )
            {
                if( m_pBuf )
                    free( m_pBuf );
                m_pBuf = NULL;
				return TRUE;
            }

			dwMemSize += m_dwMemAlignment;				//	规整内存的分配块，m_dwMemAlignment 已经减一
			dwMemSize &= (~m_dwMemAlignment);
            m_pBuf = (PBYTE)realloc( m_pBuf, dwMemSize );
			if( NULL == m_pBuf )
				return FALSE;							//	没有内存		
			m_dwBufSize = dwMemSize;
		}

		ASSERT( m_pBuf && m_dwBufSize );
		
		return TRUE;
	}

	virtual PBYTE	GetBuffer()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		return ( m_pBuf + m_dwReservedBytes );
	}

	//	获取用户自定义数据
	virtual DWORD	GetUserData()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );		

		return m_nUserData;
	}

	//	设置新的用户数据，同时返回原来的用户数据
	virtual DWORD	PutUserData( DWORD dwNewValue )
	{
		DWORD dwOldUserData = m_nUserData;

		m_nUserData = dwNewValue;

		return dwOldUserData;
	}

	//	获取保留的数据头长度
	virtual DWORD	GetReservedBytes()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );

		return m_dwReservedBytes;
	}

	//	获取数据大小
	virtual DWORD GetDataLen()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );		

		return m_dwDataLen;
	}

	//	设置数据大小
	virtual void  PutDataLen( DWORD dwNewValue )
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		ASSERT( dwNewValue <= m_dwBufSize );
		ASSERT( (dwNewValue+m_dwReservedBytes) <= m_dwBufSize );

		m_dwDataLen = dwNewValue;
	}

	virtual DWORD GetBufSize()
	{
		ASSERT( m_dwReservedBytes <= m_dwBufSize );		//	保留字节不可能超过总的大小
		ASSERT( (m_dwDataLen+m_dwReservedBytes) <= m_dwBufSize );
		return m_dwBufSize - m_dwReservedBytes;
	}

	CBufPacket4C(DWORD dwDefReservedByte = 4096, DWORD dwAlignment = 64 );
	virtual ~CBufPacket4C();

	//////////////////////////////////////////////
	//功能:
	//		外部设定内存
	//入口参数:
	//		pBuf				缓冲区地址
	//		dwBufSize			缓存区大小
	//返回参数:
	//		无
	void	Attach( PBYTE pBuf, DWORD dwBufSize )
	{
		ASSERT( pBuf && dwBufSize );
		if( m_pBuf )
			free( m_pBuf );
		m_pBuf = pBuf;		
		m_dwBufSize = dwBufSize;
		Preset();
		m_bIsAttached = TRUE;
	}

	//////////////////////////////////////////////
	//功能:
	//		取消内存
	//入口参数:
	//		dwBufSize			输出缓冲区大小
	//返回参数:
	//		缓冲区地址，可能为 0
	PBYTE Detach( DWORD & dwBufSize )
	{
		PBYTE pBuf = m_pBuf;
		dwBufSize = m_dwBufSize;
		m_pBuf = NULL;
		m_dwBufSize = 0;		//  2003-4-1 删除内存时，同时置内存大小为 0
		Preset();
		m_bIsAttached = FALSE;
		return pBuf;
	}

	///-------------------------------------------------------
	/// 2002-11-14
	/// 功能：
	///		获取缓冲区是否是 Attached
	/// 入口参数：
	///		无 
	/// 返回参数：
	///		TRUE			通过 Attached
	///		FALSE			不是 Attached
	BOOL IsBufAttaced()
	{
		return m_bIsAttached;
	}

protected:
	long m_nRef;	
	//	只有特殊情况才能使用该函数进行访问/修改 m_dwReservedBytes
	DWORD & Admin_AccessReservedBytes()
	{
		return m_dwReservedBytes;
	}

private:
	long m_nUserData;
	PBYTE m_pBuf;		
	DWORD	m_dwReservedBytes;			//	保留的字节数
	DWORD	m_dwBufSize;				//	缓冲区大小
	DWORD	m_dwDataLen;				//	数据长度
	DWORD	m_dwDefReservedBytes;		//	默认的保留字节长度
	BOOL	m_bIsAttached;				//	是否用 Attach 设置内存
	DWORD	m_dwMemAlignment;			//	2002.7.5 添加，内存分配方式,缺省为 1 字节
};

template <class TBase>
CBufPacket4C<TBase>::CBufPacket4C(DWORD dwDefReservedByte/*=4096*/,DWORD dwAlignment /*= 64*/ )
 : m_nRef( long(0) ),
   m_pBuf( NULL )
{
	ASSERT( dwDefReservedByte <= 0xFFFF );
	ASSERT( dwAlignment > 0 && dwAlignment < 128*1024 );		//	假设不超过128K
	m_dwDefReservedBytes = dwDefReservedByte;		//	默认的保留字节

	m_dwReservedBytes = 0;			//	保留的字节数
	m_dwBufSize = 0;				//	缓冲区大小
	m_dwDataLen = 0;				//	数据长度
	m_nUserData = 0;	
	m_bIsAttached = FALSE;				//	Attach 方式设置的内存，不能重新设置
	m_dwMemAlignment = dwAlignment-1;	//	1 字节对齐，即没有要求
}

template <class TBase>
CBufPacket4C<TBase>::~CBufPacket4C()
{
	ASSERT( FALSE == m_bIsAttached );
    if( m_pBuf )
        free( m_pBuf );
    m_pBuf = NULL;
};

//	设置初始值
//	1、保留字节为 4K
template <class TBase>
void CBufPacket4C<TBase>::Preset()
{
	m_dwReservedBytes = m_dwDefReservedBytes;
	m_dwDataLen = 0;				//	没有数据
};

#pragma pack(pop)

#endif // !defined(AFX_IPBROPACKET_H__4BE7CB66_4619_446C_9902_66795779B44B__INCLUDED_)
