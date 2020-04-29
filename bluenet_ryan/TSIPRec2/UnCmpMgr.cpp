// UnCmpMgr.cpp: implementation of the CUnCmpMgr class.
//
//////////////////////////////////////////////////////////////////////
//	2001.8.15	添加 GetSvr() 函数
//

#include "stdafx.h"
#include "Resource.h"
#include "UnCmpMgr.h"
#include "UnCompressObj.h"
#include "Tsdb.h"

#ifdef _WIN32
  #ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
  #endif
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnCmpMgr::CUnCmpMgr()
{
	m_pSvr = NULL;
}

CUnCmpMgr::~CUnCmpMgr()
{

}

//	判断是否压缩
//	入口参数
//		nLen								缓冲区大小
//		pBuffer								缓冲区
//	返回参数
//		TRUE								成功
//		FALSE								失败
BOOL CUnCmpMgr::IsCompress(int nLen, PBYTE pBuffer)
{
	return CUnCompressObj::IsCompress(nLen, pBuffer );
}

//	解压其中一个文件
//	入口参数
//		nFileNo						文件子序号, 不应超过 GetFileNum() 的值
//		outfStatus					输出文件参数
//		pDstBuf						用户指定的输出缓冲区, 若 pDstBuf = NULL, 则自动分配缓冲区, 调用者必须删除该内存
//									缓冲区大小必须根据 GetFileInfo 获取的长度进行分配
//	返回参数
//		NULL						失败
//		若 pDstBuf != NULL, 则返回 pDstBuf
PBYTE CUnCmpMgr::DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf)
{
	ASSERT( m_pSvr );
	return m_pSvr->DecodeOneFile( nFileNo, outfStatus, pDstBuf );
}

//	获取文件内容
//	入口参数
//		nFileNo						文件序号
//		outfStatus					输出文件状态
//	返回参数
//		文件长度
int CUnCmpMgr::GetFileInfo(int nFileNo,CFileStatus & outfStatus)
{
	ASSERT( m_pSvr );
	return m_pSvr->GetFileInfo( nFileNo, outfStatus );
}

//	获取压缩文件中的文件个数
//	返回参数
//		文件个数
int CUnCmpMgr::GetFileNum()
{
	ASSERT( m_pSvr );
	if( m_pSvr )
		return m_pSvr->GetFileNum();
	else
		return 0;
}

//	将指定长度及缓冲区的数据附着到对象中
//	入口参数
//		nFileLen					文件长度
//		pBuf						数据缓冲区
//	返回参数
//		文件个数
//	注:
//		经过 TSDB 编码后的 ARJ 和 PKZIP 附加数据必须去掉 TSDBCOMPRESSHEAD 后的数据, 即原来的数据
int CUnCmpMgr::Attach(int nFileLen,PBYTE pBuf)
{
	ASSERT( nFileLen && pBuf );
	PTSDBCOMPRESSHEAD pHeader = (PTSDBCOMPRESSHEAD) pBuf;
	switch( pHeader->m_dwMethod )
	{
	case TSDBCOMPRESS_METHOD_LZHUFV100:
		m_pSvr = static_cast<CUnCompressObj*>(&m_LzhufSvr);
		break;

	case TSDBCOMPRESS_METHOD_LZSSV100:
		m_pSvr = static_cast<CUnCompressObj*>(&m_lzss );
		break;
	
	default:
		ASSERT( FALSE );
		return 0;
	}
	return m_pSvr->Attach( nFileLen, pBuf );
}

//	2001.8.15	添加
//	获取服务器指针
CUnCompressObj * CUnCmpMgr::GetSvr()
{
	ASSERT( m_pSvr );
	return m_pSvr;
}
