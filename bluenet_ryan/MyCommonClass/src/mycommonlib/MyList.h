///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-7-25
///
///		用途：
///			MyList
///=======================================================

#ifndef __MYLIST_20030726__
#define __MYLIST_20030726__

#ifdef _DEBUG
	#include <assert.h>
#endif //_DEBUG
#include "MyTemplateBase.h"

#pragma pack(push,4)

template<class TYPE>
class CMyList
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		TYPE data;
	};
public:
	CMyList();
	~CMyList();

	int GetCount() const
	{
		return m_nCount;
	}

	bool IsEmpty() const
	{
		return 0 == m_nCount;
	}

	TYPE& GetHead()
	{
#ifdef _DEBUG
		assert( m_pNodeHead && m_nCount );
#endif //_DEBUG
		return m_pNodeHead->data;
	}

	TYPE GetHead() const
	{
#ifdef _DEBUG
		assert( m_pNodeHead && m_nCount );
#endif //_DEBUG
		return m_pNodeHead->data;
	}

	TYPE& GetTail()
	{
#ifdef _DEBUG
		assert( m_pNodeTail && m_nCount );
#endif //_DEBUG
		return m_pNodeTail->data;
	}

	TYPE GetTail() const
	{
#ifdef _DEBUG
		assert( m_pNodeTail && m_nCount );
#endif //_DEBUG
		return m_pNodeTail->data;
	}

	TYPE RemoveHead()
	{
#ifdef _DEBUG
		assert(m_pNodeHead != NULL);
#endif //_DEBUG

		CNode* pOldNode = m_pNodeHead;
		TYPE returnValue = pOldNode->data;

		m_pNodeHead = pOldNode->pNext;
		if (m_pNodeHead != NULL)
			m_pNodeHead->pPrev = NULL;
		else
			m_pNodeTail = NULL;
		FreeNode(pOldNode);
		return returnValue;
	}

	TYPE RemoveTail()
	{
#ifdef _DEBUG
		assert(m_pNodeTail != NULL);
#endif //_DEBUG

		CNode* pOldNode = m_pNodeTail;
		TYPE returnValue = pOldNode->data;

		m_pNodeTail = pOldNode->pPrev;
		if (m_pNodeTail != NULL)
			m_pNodeTail->pNext = NULL;
		else
			m_pNodeHead = NULL;
		FreeNode(pOldNode);
		return returnValue;
	}

	POSITION AddHead(TYPE & newElement)
	{
		CNode* pNewNode = NewNode(NULL, m_pNodeHead);
        if( NULL == pNewNode )
        	return NULL;
		pNewNode->data = newElement;
		if (m_pNodeHead != NULL)
			m_pNodeHead->pPrev = pNewNode;
		else
			m_pNodeTail = pNewNode;
		m_pNodeHead = pNewNode;
		return (POSITION) pNewNode;
	}

	POSITION AddTail(TYPE & newElement)
	{
		CNode* pNewNode = NewNode(m_pNodeTail, NULL);
        if( NULL == pNewNode )
        	return NULL;
		pNewNode->data = newElement;
		if (m_pNodeTail != NULL)
			m_pNodeTail->pNext = pNewNode;
		else
			m_pNodeHead = pNewNode;
		m_pNodeTail = pNewNode;
		return (POSITION) pNewNode;
	}

	void AddHead(CMyList* pNewList)
	{
#ifdef _DEBUG
		assert( pNewList );
#endif //_DEBUG
		if( NULL == pNewList )
			return;
		POSITION pos = pNewList->GetTailPosition();
		while (pos != NULL)
			AddHead(pNewList->GetPrev(pos));
	}

	void AddTail(CMyList* pNewList)
	{
#ifdef _DEBUG
		assert( pNewList );
#endif //_DEBUG
		if( NULL == pNewList )
			return;
		POSITION pos = pNewList->GetHeadPosition();
		while (pos != NULL)
			AddTail(pNewList->GetNext(pos));
	}

	void RemoveAll()
	{
		CNode* pNode;
		for( pNode = m_pNodeHead; pNode != NULL; )
		{
			MyDestructElements<TYPE>(&pNode->data, 1);

			CNode * pNodeTmp = pNode->pNext;
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
			pNode = pNodeTmp;
		}

		while( m_pNodeFree )
		{
			char* pBufTmp = (char* )m_pNodeFree;
			m_pNodeFree = m_pNodeFree->pNext;
			if( pBufTmp )
				delete pBufTmp;
		}

		m_nCount = 0;
		m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	}

	POSITION GetHeadPosition() const
	{
		return (POSITION) m_pNodeHead;
	}

	POSITION GetTailPosition() const
	{
		return (POSITION) m_pNodeTail;
	}

	TYPE& GetNext(POSITION& rPosition)
	{
		CNode* pNode = (CNode*) rPosition;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		rPosition = (POSITION) pNode->pNext;
		return pNode->data;
	}

	TYPE GetNext(POSITION& rPosition) const
	{
		CNode* pNode = (CNode*) rPosition;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		rPosition = (POSITION) pNode->pNext;
		return pNode->data;
	}

	TYPE& GetPrev(POSITION& rPosition)
	{
		CNode* pNode = (CNode*) rPosition;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data;
	}

	TYPE GetPrev(POSITION& rPosition) const
	{
		CNode* pNode = (CNode*) rPosition;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data;
	}

	TYPE& GetAt(POSITION position)
	{
		CNode* pNode = (CNode*) position;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		return pNode->data;
	}

	TYPE GetAt(POSITION position) const
	{
		CNode* pNode = (CNode*) position;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		return pNode->data;
	}

	void SetAt(POSITION pos, TYPE & newElement)
	{
		CNode* pNode = (CNode*) pos;
#ifdef _DEBUG
		assert( pNode );
#endif //_DEBUG
		pNode->data = newElement;
	}

	void RemoveAt(POSITION position)
	{
		CNode* pOldNode = (CNode*) position;
#ifdef _DEBUG
		assert( pOldNode );
#endif //_DEBUG

		// remove pOldNode from list
		if (pOldNode == m_pNodeHead)
		{
			m_pNodeHead = pOldNode->pNext;
		}
		else
		{
#ifdef _DEBUG
			assert( pOldNode->pPrev );
#endif //_DEBUG
			pOldNode->pPrev->pNext = pOldNode->pNext;
		}
		if (pOldNode == m_pNodeTail)
		{
			m_pNodeTail = pOldNode->pPrev;
		}
		else
		{
#ifdef _DEBUG
			assert( pOldNode->pNext );
#endif //_DEBUG
			pOldNode->pNext->pPrev = pOldNode->pPrev;
		}
		FreeNode(pOldNode);
	}

	POSITION InsertBefore(POSITION position, TYPE & newElement)
	{
		if (position == NULL)
			return AddHead(newElement); // insert before nothing -> head of the list

		// Insert it before position
		CNode* pOldNode = (CNode*) position;
		CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
		if( NULL == pNewNode )
        	return NULL;
		pNewNode->data = newElement;

		if (pOldNode->pPrev != NULL)
			pOldNode->pPrev->pNext = pNewNode;
		else
		{
#ifdef _DEBUG
			assert(pOldNode == m_pNodeHead);
#endif //_DEBUG
			m_pNodeHead = pNewNode;
		}
		pOldNode->pPrev = pNewNode;
		return (POSITION) pNewNode;
	}

	POSITION InsertAfter(POSITION position, TYPE & newElement)
	{
		if (position == NULL)
			return AddTail(newElement); // insert after nothing -> tail of the list

		// Insert it before position
		CNode* pOldNode = (CNode*) position;
		CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
        if( NULL == pNewNode )
        	return NULL;
		pNewNode->data = newElement;

		if (pOldNode->pNext != NULL)
			pOldNode->pNext->pPrev = pNewNode;
		else
		{
#ifdef _DEBUG
			assert(pOldNode == m_pNodeTail);
#endif //_DEBUG
			m_pNodeTail = pNewNode;
		}
		pOldNode->pNext = pNewNode;
		return (POSITION) pNewNode;
	}

	POSITION Find(TYPE & searchValue, POSITION startAfter = NULL) const
	{
		CNode* pNode = (CNode*) startAfter;
		if (pNode == NULL)
			pNode = m_pNodeHead;  // start at head
		else
			pNode = pNode->pNext;  // start after the one specified

		for (; pNode != NULL; pNode = pNode->pNext)
		{
			if( pNode->data == *(&searchValue) )
				return (POSITION)pNode;
		}
		return NULL;
	}

	POSITION FindIndex(int nIndex) const
	{
		if (nIndex >= m_nCount || nIndex < 0)
			return NULL;  // went too far
#ifdef _DEBUG
		assert( m_pNodeHead );
#endif //_DEBUG
		if( NULL == m_pNodeHead )
			return NULL;

		CNode* pNode = m_pNodeHead;
		while (nIndex--)
		{
#ifdef _DEBUG
			assert( pNode );
#endif //_DEBUG
			pNode = pNode->pNext;
		}
		return (POSITION) pNode;
	}

protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	CNode* m_pNodeFree;
	int m_nCount;

protected:
	CNode* NewNode(CMyList::CNode* pPrev, CMyList::CNode* pNext)
        {
            if (m_pNodeFree == NULL)
            {
                    // chain them into free list
//                    m_pNodeFree = (CMyList::CNode*)( new char[ sizeof(CMyList::CNode) ] ); ???
					m_pNodeFree = (CNode*)( new char[ sizeof(CNode) ] );
                    if( NULL == m_pNodeFree )
                            return NULL;
                    m_pNodeFree->pNext = NULL;
                    m_pNodeFree->pPrev = NULL;
            }
#ifdef _DEBUG
            assert(m_pNodeFree != NULL);  // we must have something
#endif //_DEBUG

//            CMyList::CNode* pNode = m_pNodeFree; ???
			CNode* pNode = m_pNodeFree;
            m_pNodeFree = m_pNodeFree->pNext;
            pNode->pPrev = pPrev;
            pNode->pNext = pNext;
            m_nCount++;

#ifdef _DEBUG
            assert(m_nCount > 0);  // make sure we don't overflow
#endif //_DEBUG

           MyConstructElements<TYPE>(&pNode->data, 1);
            return pNode;
        }
	void FreeNode(CNode*);
};

template<class TYPE>
void CMyList<TYPE>::FreeNode(CMyList::CNode* pNode)
{
	MyDestructElements<TYPE>(&pNode->data, 1);
	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;

#ifdef _DEBUG
	assert(m_nCount >= 0);  // make sure we don't underflow
#endif //_DEBUG

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

template<class TYPE>
CMyList<TYPE>::CMyList()
{
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
}

template<class TYPE>
CMyList<TYPE>::~CMyList()
{
	RemoveAll();
}

#pragma pack(pop)

#endif //__MYLIST_20030726__
