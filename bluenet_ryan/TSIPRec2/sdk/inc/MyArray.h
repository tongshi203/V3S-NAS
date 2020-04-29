#ifndef __MYARRAY_H_INCLUDE20030820__
#define __MYARRAY_H_INCLUDE20030820__

#include "MyTemplateBase.h"

#pragma pack(push,4)

template<class TYPE>
class CMyArray
{
public:
// Construction
	CMyArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	bool SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	TYPE GetAt(int nIndex) const;
	void SetAt(int nIndex, TYPE & newElement);
	TYPE& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const TYPE* GetData() const;
	TYPE* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, TYPE & newElement);
	int Add(TYPE & newElement);
	int Append(const CMyArray& src);
	void Copy(const CMyArray& src);

	// overloaded operator helpers
	TYPE operator[](int nIndex) const;
	TYPE& operator[](int nIndex);
	void operator=(const CMyArray& src);

	// Operations that move elements around
	void InsertAt(int nIndex, TYPE & newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CMyArray* pNewArray);

// Implementation
protected:
	TYPE* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CMyArray();
};

/////////////////////////////////////////////////////////////////////////////
// CMyArray<TYPE> inline functions

template<class TYPE>
MY_INLINE int CMyArray<TYPE>::GetSize() const
	{ return m_nSize; }
template<class TYPE>
MY_INLINE int CMyArray<TYPE>::GetUpperBound() const
	{ return m_nSize-1; }
template<class TYPE>
MY_INLINE void CMyArray<TYPE>::RemoveAll()
	{ SetSize(0, -1); }
template<class TYPE>
MY_INLINE TYPE CMyArray<TYPE>::GetAt(int nIndex) const
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
template<class TYPE>
MY_INLINE void CMyArray<TYPE>::SetAt(int nIndex, TYPE & newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
template<class TYPE>
MY_INLINE TYPE& CMyArray<TYPE>::ElementAt(int nIndex)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
template<class TYPE>
MY_INLINE const TYPE* CMyArray<TYPE>::GetData() const
	{ return (const TYPE*)m_pData; }
template<class TYPE>
MY_INLINE TYPE* CMyArray<TYPE>::GetData()
	{ return (TYPE*)m_pData; }
template<class TYPE>
MY_INLINE int CMyArray<TYPE>::Add(TYPE & newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
template<class TYPE>
MY_INLINE TYPE CMyArray<TYPE>::operator[](int nIndex) const
	{ return GetAt(nIndex); }
template<class TYPE>
MY_INLINE TYPE& CMyArray<TYPE>::operator[](int nIndex)
	{ return ElementAt(nIndex); }
template<class TYPE>
MY_INLINE void CMyArray<TYPE>::operator=(const CMyArray& src)
	{ Copy( src); }

/////////////////////////////////////////////////////////////////////////////
// CMyArray<TYPE> out-of-line functions

template<class TYPE>
CMyArray<TYPE>::CMyArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE>
CMyArray<TYPE>::~CMyArray()
{
	if (m_pData != NULL)
	{
		MyDestructElements<TYPE>(m_pData, m_nSize);
		delete[] (unsigned char*)m_pData;
	}
}

template<class TYPE>
bool CMyArray<TYPE>::SetSize(int nNewSize, int nGrowBy)
{
	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		if (m_pData != NULL)
		{
			MyDestructElements<TYPE>(m_pData, m_nSize);
			delete[] (BYTE*)m_pData;
			m_pData = NULL;
		}
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		ASSERT(nNewSize <= SIZE_T_MAX/sizeof(TYPE));    // no overflow
#endif
		m_pData = (TYPE*) new BYTE[nNewSize * sizeof(TYPE)];
        if( NULL == m_pData )
        	return false;
		MyConstructElements<TYPE>(m_pData, nNewSize);
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements
			MyConstructElements<TYPE>(&m_pData[m_nSize], nNewSize-m_nSize);
		}
		else if (m_nSize > nNewSize)
		{
			// destroy the old elements
			MyDestructElements<TYPE>(&m_pData[nNewSize], m_nSize-nNewSize);
		}
		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = m_nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
		ASSERT(nNewMax <= SIZE_T_MAX/sizeof(TYPE)); // no overflow
#endif
		TYPE* pNewData = (TYPE*) new BYTE[nNewMax * sizeof(TYPE)];
        if( NULL == pNewData )
        	return false;

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);
		MyConstructElements<TYPE>(&pNewData[m_nSize], nNewSize-m_nSize);

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
    return true;
}

template<class TYPE>
int CMyArray<TYPE>::Append(const CMyArray& src)
{
	ASSERT(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	MyCopyElements<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
	return nOldSize;
}

template<class TYPE>
void CMyArray<TYPE>::Copy(const CMyArray& src)
{
	ASSERT(this != &src);   // cannot append to itself

	SetSize(src.m_nSize);
	MyCopyElements<TYPE>(m_pData, src.m_pData, src.m_nSize);
}

template<class TYPE>
void CMyArray<TYPE>::FreeExtra()
{
	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		ASSERT(m_nSize <= SIZE_T_MAX/sizeof(TYPE)); // no overflow
#endif
		TYPE* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

template<class TYPE>
void CMyArray<TYPE>::SetAtGrow(int nIndex, TYPE & newElement)
{
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1, -1);
	m_pData[nIndex] = newElement;
}

template<class TYPE>
void CMyArray<TYPE>::InsertAt(int nIndex, TYPE & newElement, int nCount /*=1*/)
{
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount, -1);   // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount, -1);  // grow it to new size
		// destroy intial data before copying over it
		MyDestructElements<TYPE>(&m_pData[nOldSize], nCount);
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(TYPE));

		// re-init slots we copied from
		MyConstructElements<TYPE>(&m_pData[nIndex], nCount);
	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

template<class TYPE>
void CMyArray<TYPE>::RemoveAt(int nIndex, int nCount)
{
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	MyDestructElements<TYPE>(&m_pData[nIndex], nCount);
	if (nMoveCount)
		memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(TYPE));
	m_nSize -= nCount;
}

template<class TYPE>
void CMyArray<TYPE>::InsertAt(int nStartIndex, CMyArray* pNewArray)
{
	ASSERT(pNewArray != NULL);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

#pragma pack(pop)

#endif // __MYARRAY_H_INCLUDE20030820__

