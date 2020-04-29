// DPRecThread.h: interface for the CDPRecThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DPRECTHREAD_H__78C8D161_E9BE_4543_920D_E23D34DE13F6__INCLUDED_)
#define AFX_DPRECTHREAD_H__78C8D161_E9BE_4543_920D_E23D34DE13F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DecoderThread.h"
#include "IPData.h"

class CDPRecThread : public CMyThread  
{
public:
	void DeleteDataPort( COneDataPortItem * pDataPortItem );
	int AddDataPort( COneDataPortItem * pDataPortItem );
	void Close();
	BOOL Init( CDecoderThread * pDecoderThread );
	CDPRecThread();
	virtual ~CDPRecThread();

	virtual int ExitInstance();					//	返回参数
	virtual BOOL InitInstance();				//	是否成功
	virtual void Run();							//	运行主体
	virtual void Delete();						//	删除自己

	enum { MAX_DATA_PORT_COUNT = 96 };			//	最多支持 96 路数据源端口

private:	
	void DoReadSyncDataPort();
	void CleanUpIPData();
	void OnDataOK( COneDataPortItem * pDataPortItem, CIPData * pIPData, DWORD dwByteCount  );
	void DoReadAsync();
	int FindDataPortItem( COneDataPortItem * pDataPortItem );
	CDecoderThread * m_pDecoderThread;
	CArray< COneDataPortItem *, COneDataPortItem * >	m_anPorts;	//	所有地对象
	CCriticalSection	m_SynObj;				//	同步对象
};

#endif // !defined(AFX_DPRECTHREAD_H__78C8D161_E9BE_4543_920D_E23D34DE13F6__INCLUDED_)
