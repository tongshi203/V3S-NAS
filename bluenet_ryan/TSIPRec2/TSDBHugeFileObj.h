// TSDBHugeFileObj.h: interface for the CTSDBHugeFileObj class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
#define AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Tsdb.h"

class CTSDBHugeFileObj
{
public:
	BOOL IsSameObj( PTSDBHUGEFILEHEAD pHeader );
	DWORD GetFileLen();
	PBYTE GetDataBuf();
	BOOL Attach( PTSDBHUGEFILEHEAD pHeader );
	BOOL SaveBlock( PTSDBHUGEFILEHEAD pHead, PBYTE pDataBuf );
	float GetPercentage();
	BOOL IsFileOK();
	CTSDBHugeFileObj( PTSDBHUGEFILEHEAD pHeader );
	CTSDBHugeFileObj();
	virtual ~CTSDBHugeFileObj();

public:	
	long Release();
	long AddRef();
	static void ClearHugeFileTmpBuf();
	time_t	m_LastAccessTime;					//	上次访问时间
	BOOL m_bMsgSended;							//	已经发送过消息

private:
	BOOL IsBlockOK( int nBlockNo );
	BOOL SetBlockNo( int nBlockNo );
	void Init();
	BOOL SetOwnerHandle();
	static PBYTE CreateAndMapFile( LPCSTR pszFileName, DWORD dwFileLen, CFile & file, HANDLE & hOut );

private:
	typedef struct tagFLAGFILE
	{
		BOOL				m_bHasOwner;		//	有主人正在写数据,若该位 = FASLE, 有主人, 其他客户可以只能查询
		TSDBHUGEFILEHEAD	m_Head;				//	大文件数据头
		DWORD				m_dwBlockReceived;	//	成功接收到的子文件数
		BOOL				m_bCloseErr;		//	大文件是否正常关闭
		BYTE				m_abyFlags[1];		//	子文件接收标记, 对应位 = 1 表示接收成功,每一比特对应1个子文件
	}FLAGFILE, *PFLAGFILE;

private:
	long m_nRef;
	BOOL	m_bIsOwner;							//	是否为主人
	PFLAGFILE	m_pFlagBuf;						//	标记文件缓冲区
	PBYTE	m_pDataBuf;							//	数据文件缓冲区
	HANDLE m_hmapFlagFile;						//	标志文件映射句柄
	HANDLE m_hmapDataFile;						//	数据文件映射句柄
	CFile m_FlagFile;							//	标记文件
	CFile m_DataFile;							//	数据文件
};

#endif // !defined(AFX_TSDBHUGEFILEOBJ_H__7B40D441_74C2_11D3_B1F1_005004868EAA__INCLUDED_)
