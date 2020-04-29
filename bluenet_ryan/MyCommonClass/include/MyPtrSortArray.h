///////////////////////////////////////////////////////////////
//
//	MyPtrSortArray.H	
//	
//
//	支持一个有序的结构指针数组
//
//		
///////////////////////////////////////////////////////////////
#ifndef __MY_PTRSORTARRAY_INCLUDE_20101016__
#define __MY_PTRSORTARRAY_INCLUDE_20101016__

#include <MyArray.h>

// 2016.12.17 CYJ Add
#pragma pack(push,8)

template < class T, int OFS, class SORTTYPE > 
class CMyPtrSortArray : public CMyArray<T>
{
public:
	CMyPtrSortArray(int nMaxCount = 0)
	{
		m_nMaxCount = nMaxCount;		//	＝ 0，不作溢出判断
		m_bIsBufAttached = FALSE;
	}

	~CMyPtrSortArray()
	{
		if( m_bIsBufAttached )			//	内存是通过 Attach 的，不能被删除
			Detach();
	}

	int Add( T& newElement )
	{
		ASSERT( FALSE == m_bIsBufAttached );
		if( m_bIsBufAttached )
			return -1;

		int nNo = Find( GetSortValue(newElement) );
		if( nNo >= 0 )
		{
			CMyArray<T>::ElementAt(nNo) = newElement;		//	覆盖
			return nNo;							//	已经存在
		}
		if( m_nMaxCount && CMyArray<T>::GetSize() >= m_nMaxCount )
		{
            throw "Not enough memory";
     		return -1;
		}

		nNo = -nNo-1;
		InsertAt( nNo, newElement );
		return nNo;
	}

	int Find( SORTTYPE ValueToFind );

#ifdef _DEBUG
	void Dump();
	void Test();
#endif // _DEBUG

	//-----------------------------------------------
	//	通过 Attach 的内存为固定，不能被删除，但可以读写
	void	Attach( T * pBuf, int nCount )
	{
		ASSERT( pBuf && nCount );
		if( NULL == pBuf || 0 == nCount )
			return;
		m_bIsBufAttached = TRUE;
		CMyArray<T>::m_pData = pBuf;
		CMyArray<T>::m_nSize = nCount;
		CMyArray<T>::m_nMaxSize = nCount;
	}

	T * Detach()
	{
		ASSERT( m_bIsBufAttached );
		if( FALSE == m_bIsBufAttached )
			return NULL;
		m_bIsBufAttached = FALSE;
		T * pRetVal = CMyArray<T>::m_pData;
		CMyArray<T>::m_pData = NULL;
		CMyArray<T>::m_nSize = 0;
		CMyArray<T>::m_nMaxSize = 0;
		return pRetVal;
	}

protected:
	inline SORTTYPE & GetSortValue( const T * pSrc )
	{
		PBYTE pBuf = (PBYTE)*pSrc;
		return * (SORTTYPE*)(pBuf + OFS);
	}

	inline SORTTYPE & GetSortValue( const T & src )
	{
		PBYTE pBuf = (PBYTE)src;
		return * (SORTTYPE*)(pBuf + OFS);
	}
private:
	int	m_nMaxCount;					//	允许的最大值，防止意外导致溢出
	BOOL	m_bIsBufAttached;
};

//--------------------------------------------------------
//	查找一个 DWORD 是否在列表中
//	入口参数
//		ValueToFind			数值
//	返回参数
//		<0					失败
//		>=0					成功
//	注：
//		当错误返回时，(-nRetVal) - 1 表示应当插入的位置
template < class T, int OFS, class SORTTYPE > 
int CMyPtrSortArray<T,OFS,SORTTYPE>::Find( SORTTYPE ValueToFind )
{
	int nCount = CMyArray<T>::GetSize();
	if( 0 == nCount )
		return -1;
	const T * pBuf = CMyArray<T>::GetData();
	SORTTYPE * pSortField = &GetSortValue( pBuf );
	if( ValueToFind < *pSortField )
		return -1;									//	判断最小
	else if( ValueToFind == *pSortField )
		return 0;
	
	pSortField = &GetSortValue( pBuf[nCount-1] );	//	判断最大	
	if( ValueToFind > *pSortField )
		return -nCount-1;
	else if( ValueToFind == *pSortField )
		return nCount -1;

	int nMin = 0;
	int nMax = nCount-1;
	for(int i=0; i<nCount-1; i++)
	{
		int nTmp = ( nMin + nMax + 1 ) / 2;
		ASSERT( nTmp >= nMin && nTmp <= nMax );
		pSortField = &GetSortValue( pBuf[nTmp] );
		if( ValueToFind == *pSortField )
			return nTmp;							//	发现
		if( nTmp == nMin || nTmp == nMax )
			break;									//	退出循环
		if( ValueToFind < *pSortField )
			nMax = nTmp;
		else
			nMin = nTmp;
	}
	return -nMax-1;
}

#ifdef _DEBUG
template < class T, int OFS, class SORTTYPE >
void CMyPtrSortArray<T,OFS,SORTTYPE>::Dump()
{
	int nCount = CMyArray<T>::GetSize();
	fprintf( stderr, "nCount = %d\n", nCount );
	for(int i=0; i<nCount; i++)
	{
		SORTTYPE & value = GetSortValue( CMyArray<T>::ElementAt(i) );
		fprintf( stderr, "Sort Field[%d] = %d\n", i,value );
	}
}
template < class T, int OFS, class SORTTYPE >
void CMyPtrSortArray<T,OFS,SORTTYPE>::Test()
{
	int nCount = CMyArray<T>::GetSize();
	for(int i=0; i<nCount; i++)
	{
		SORTTYPE & currentValue = GetSortValue( CMyArray<T>::ElementAt(i) );
		for(int j=0; j<i; j++)
		{
			SORTTYPE & Tmp = GetSortValue( CMyArray<T>::ElementAt(j) );
			ASSERT( Tmp < currentValue );
		}
	}	
}

#endif // _DEBUG

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // __MY_PTRSORTARRAY_INCLUDE_20101016__
