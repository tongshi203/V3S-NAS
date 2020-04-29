// TSDBHugeFileMgr.cpp: implementation of the CTSDBHugeFileMgr class.
//
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
//	2002.3.29	为了共享大文件对象，修改 UpdateObj & ClearBuf 函数，delete pObj ==> pObj->Release

#include "stdafx.h"
#include "resource.h"
#include "TSDBHugeFileMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTSDBHugeFileMgr::CTSDBHugeFileMgr()
{
	m_pLastFind = NULL;
}

CTSDBHugeFileMgr::~CTSDBHugeFileMgr()
{
	ClearBuf();
}

//	保存数据
//	入口参数
//		pHeader							大文件头
//		pDataBuf						数据缓冲区
//	返回参数
//		TRUE							大文件接收成功
//		FALSE							接收中
BOOL CTSDBHugeFileMgr::SaveBlock(PTSDBHUGEFILEHEAD pHeader, PBYTE pDataBuf)
{
	ASSERT( pHeader && pDataBuf );
	CTSDBHugeFileObj * pObj = FindObj( pHeader );
	if( pObj == NULL )
	{									//	该大文件为新的大文件, 需要创建
		pObj = AddHelpObj( pHeader );
		if( !pObj )
		{
			UpdateObj();				//	删除最早的对象
			pObj = AddHelpObj( pHeader );
			if( !pObj )
				return FALSE;			//	缓冲区中太多的对象使用中
		}
	}
	return pObj->SaveBlock( pHeader, pDataBuf );
}

//	生成并添加对象
//	入口参数
//		pHeader							大文件辅助对对象
//	返回参数
//		对象指针
CTSDBHugeFileObj * CTSDBHugeFileMgr::AddHelpObj(PTSDBHUGEFILEHEAD pHeader)
{
	ASSERT( pHeader );
	if( m_HelpObj.GetSize() >= MAX_HUGEOBJ_NUM )
		return NULL;
	CTSDBHugeFileObj * pObj = new CTSDBHugeFileObj();
	if( !pObj )
		return NULL;					//	申请内存失败
	if( pObj->Attach( pHeader ) == FALSE )
	{
		delete pObj;
		return NULL;
	}
	pObj->AddRef();						//	2002.3.29 确定可以使用，增加引用计数器
	m_HelpObj.Add( pObj );
	return pObj;
}

//	查找数据头
//	入口参数
//		pHeader							数据头
//	返回参数
//		对象指针
CTSDBHugeFileObj * CTSDBHugeFileMgr::FindObj(PTSDBHUGEFILEHEAD pHeader)
{
	int nNum = m_HelpObj.GetSize();
	if( m_pLastFind && m_pLastFind->IsSameObj( pHeader ) )
		return m_pLastFind;
	CTSDBHugeFileObj * pObj = NULL;
	for(int i=0; i<nNum; i++)
	{
		pObj = (CTSDBHugeFileObj *)m_HelpObj[i];
		ASSERT( pObj );
		if( pObj->IsSameObj( pHeader ) )
			return pObj;
	}
	return NULL;
}

//	整理对象
//	删除时间最小, 即最早的对象
void CTSDBHugeFileMgr::UpdateObj()
{
	m_pLastFind = NULL;
	int nNum = m_HelpObj.GetSize();
	CTSDBHugeFileObj * pObj;
	int nNoToDelete = -1;
	time_t	MinTime = CTime::GetCurrentTime().GetTime();
	for(int i=0; i<nNum; i++)
	{
		pObj = (CTSDBHugeFileObj *)m_HelpObj[i];
		ASSERT( pObj );
		if( MinTime >= pObj->m_LastAccessTime )
		{
			MinTime = pObj->m_LastAccessTime ;
			nNoToDelete = i;
		}
	}
	ASSERT( nNoToDelete != -1 );
	pObj = (CTSDBHugeFileObj *)m_HelpObj[nNoToDelete];
	pObj->Release();				//	2002.3.29, delete pObj ==> pObj->Release，为了共享对象
	m_HelpObj.RemoveAt( nNoToDelete );
}

//	清除所有数据
void CTSDBHugeFileMgr::ClearBuf()
{
	int nNum = m_HelpObj.GetSize();
	for( int i=0; i<nNum; i ++ )
	{
		CTSDBHugeFileObj * pObj = (CTSDBHugeFileObj *)m_HelpObj[0];
		ASSERT( pObj );
		pObj->Release();			//	2002.3.29, delete pObj ==> pObj->Release，为了共享对象
		m_HelpObj.RemoveAt(0);
	}
	m_pLastFind = NULL;
}
