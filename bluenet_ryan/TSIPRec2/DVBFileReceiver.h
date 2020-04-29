// DVBFileReceiver.h : Declaration of the CDVBFileReceiver

#ifndef __DVBFILERECEIVER_H_
#define __DVBFILERECEIVER_H_

#include "Resource.h"       // main symbols
#include "IPData.h"
#include "TSDBFileSystem.h"
#include "HugeFile.h"
#include "MyIUnknownImpl.h"
#include "FileDelayEventDispatcher.h"

#pragma pack(push,8)

class CFileEventObject
{
public:
	CFileEventObject(){  m_nRef = 0;  };

public:
	CFileObject *	m_pFileObject;
	BOOL	m_bIsSubFile;					//	是否为大文件之子文件

public:
	long AddRef()
	{
		return ::InterlockedIncrement( &m_nRef );
	};

	long Release()
	{
		if( ::InterlockedDecrement( &m_nRef ) )
			return m_nRef;
		delete this;
		return 0;
	};

	void Preset()
	{
		m_pFileObject = NULL;
		m_bIsSubFile = FALSE;
	};

private:
	long	m_nRef;
};

/////////////////////////////////////////////////////////////////////////////
// CDVBFileReceiver
class CDVBFileReceiver : public CMyIUnknownImpl<IDVB_EPG_Receiver>,	public CTSDBFileSystem
{
public:
	CDVBFileReceiver();
	virtual ~CDVBFileReceiver();

	enum {
		MAJOR_VERSION = 1,				//	主版本
		MINOR_VERSION = 1,				//	次版本
		BUILD_NO = 13,					//	编译号
	};

	void WriteThread_DoCheckAndSaveFiles();


	virtual void  NotifyOnFileOKEvent( CFileObject * pFileObject );
	virtual void  NotifySubFileOKEvent( CFileObject * pFileObject );

	virtual void SafeDelete()
	{
		Close();
		delete this;
	}

// IDVBFileReceiver
public:
	bool SetDataPortReceiveLog(COneDataPortItem * pDataPortItem, DWORD dwAttrib, DWORD dwFileLen, BOOL bIncFileCount, LPCSTR lpszFileName);
	virtual IIPFileMendHelper * GetIPFileMendHelper( HDATAPORT hDataPort );
	virtual BOOL GetSendNotTSDBFileEvent();
	virtual void SetSendNotTSDBFileEvent( BOOL bNewVal );
	virtual HDATAPORT AddDataPort( void * pDataPort );
	virtual void Close();
	virtual BOOL Init(bool bSaveFileInBackground=false,int varMaxPacketSize=2048);
	virtual LPCSTR GetAutoSavePath();
	virtual void SetAutoSavePath( LPCSTR lpszNewVal );
	virtual long GetDataPortCount();
	virtual long GetFileBPS();
	virtual long GetIPPacketBPS();
	virtual BOOL GetSendSubFileEvent();
    virtual void PutSendSubFileEvent( BOOL bNewVal );
	virtual LPCSTR GetVersion();
	virtual BOOL DeleteDataPort( HDATAPORT hDataPort );
	virtual HDATAPORT CreateDataPort( LPCSTR lpszTargetIP, int nPort, LPCSTR lpszBindIP=NULL, BOOL bIsUDP = TRUE );
	virtual long GetMaxPacketSize();
	virtual void RegisterEventResponser( IDVBReceiverEvent * pObject );
	virtual void DoMessagePump(void);
	virtual float GetProgressInfo( HDATAPORT hDataPort, DWORD & dwBroLoopCount, int & dwFileCount, DWORD & dwTotalLen, DWORD & dwByteReceived, int & nCountReceived );
	virtual BOOL PutSendProgressEvent( BOOL bNewValue );
	virtual BOOL GetSendProgressEvent();
	virtual BOOL GetDotMapFileOnFileOKEvent();
	virtual void SetDoMapFileOnfileOKEvent( BOOL bNewValue );

//	IUnknwon
	virtual DWORD QueryInterface( REFIID iid,void ** ppvObject);

// EPG Recevier
	virtual bool GetEnableOldEPG(){ return false;};
	virtual void SetEnableOldEPG( bool bNewValue ){ bNewValue = false; };

private:
	BOOL m_bMapFileOnFireFileOKEvent;				//	触发Hugefile On File OK event 时，是否映射文件，缺省为 TRUE
	BOOL m_bSendProgressEvent;
	BOOL m_bAutoSave;										//	是否自动存盘
	BOOL m_bIsOnLine;
	long m_bFileOKEventDone;								//	File OK 事件通知是否已经处理
	CString m_strAutoSavePath;
	BOOL m_bSendSubFileEvent;
	DWORD m_dwMaxPacketSize;
#ifdef _WIN32
	CArray<COneDataPortItem*,COneDataPortItem*> m_aDataPortItems;		//	记录当前所有的端口
	CArray<COneDataPortItem*,COneDataPortItem*> m_aDelayDeleteItems;	//	延迟删除的 DataPortItem 内存
#else
	CMyArray<COneDataPortItem*> m_aDataPortItems;		//	记录当前所有的端口
	CMyArray<COneDataPortItem*> m_aDelayDeleteItems;	//	延迟删除的 DataPortItem 内存
#endif //_WIN32
	CLookaheadPacketMgr<CFileEventObject> m_FileNotifyEventList;		//	文件接收成功的列表

///////////////////////////////////////////////////////////
//	大文件
private:
	CHugeFile m_HugeFile;					//	文件对象
	time_t	m_HugeFileLastModifyTime;		//	最后访问时间，判断是否更新
	CString	m_strHugeFileName;				//	文件名
	DWORD	m_dwHugeFileLen;				//	大文件文件长度
	int		m_nSavePathLen;					//	记录保存路径的长度
	DWORD	m_dwHugeFileByteReceived;		//	已经成功接收的字节数

//////////////////////////////////////////////////////////////
//	统计速率
private:
	DWORD	m_dwByteReceived;				//	已经接收到的文件字节数
	DWORD	m_dwLastTickCount;				//	上次统计的时间
	DWORD	m_dwLastFileBPS;				//	上次统计的速率

protected:
	IDVBReceiverEvent * m_pFileEventObject;			// file event

protected:
	virtual void Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort );
	virtual void Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort );
	virtual void Fire_OnProgressEvent( COneDataPortItem * pDataPortItem , LPCSTR lpszFileName);

private:
	CString m_strVersion;
	int m_nTimer_2_Second;
	bool m_bSaveFileInBackgound;		//  2004-7-31 是否使用文件保存线程保存数据
	CFileDelayEventDispatcher	m_DelayEventDispatcher;

private:
	void PresetVars();
	void OnHugeFileFullyReceived(const char * pszFileName, CFileObject *pFileObject);
	COneDataPortItem * NewOneDataPortItem();
	void CheckAndSendFileEvent();
	bool SaveHugeFile( CFileObject * pFileObject );
	BOOL IsHugeFileChanged(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen);
	BOOL PreprareHugeFile(LPCSTR lpszFileName, time_t LastModifyTime,DWORD dwFileLen, int nSubFileCount);
	BOOL MoveHugeToOffset(DWORD dwOffset);

	void DeleteAllDataPort();
	void CleanFileOKQueue();
	void ExecDelayDeleteItems();
	int FindDataPort( HDATAPORT hDataPort );
	int FindDataPort( CString & strIP, long nPort );
};

#pragma pack(pop)

#endif //__DVBFILERECEIVER_H_
