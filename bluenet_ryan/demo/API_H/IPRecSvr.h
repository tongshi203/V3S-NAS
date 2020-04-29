/* this ALWAYS GENERATED file contains the definitions for the interfaces */

#ifndef __DVB_CPLUSPLUS_INTERFACE_H_20031201__
#define __DVB_CPLUSPLUS_INTERFACE_H_20031201__

class IIPFileMendHelper;

// 2016.1.20 CYJ Add
typedef void * HDATAPORT;

#pragma pack(push,8)

#if 0
#ifndef __MY_UNKNOWN_IMPLEMENT__
#define __MY_UNKNOWN_IMPLEMENT__
//--------------------------------------------------
//	{00000000-0000-0000-C000-000000000046}
static const GUID IID_IMyUnknown = { 0, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} };
//--------------------------------------------------
class IMyUnknown
{
public:
    IMyUnknown(){}
    virtual ~IMyUnknown(){}
public:
    virtual long AddRef(void) = 0;
    virtual long Release(void) = 0;
    virtual DWORD QueryInterface( REFIID iid, void **ppvObject) = 0;
};
#endif // __MY_UNKNOWN_IMPLEMENT__
#endif // 0

//MIDL_INTERFACE("9E99F19B-1B5D-46b0-8B01-7A03CE5AEC5F")
static const IID IID_IBufPacket = {0x9E99F19B,0x1B5D,0x46b0,{0x8B,0x01,0x7A,0x03,0xCE,0x5A,0xEC,0x5F}};
class IBufPacket : public IMyUnknown
{
public:
    IBufPacket(){}
    virtual ~IBufPacket(){}
public:
    virtual DWORD GetBufSize() = 0;
    virtual DWORD GetDataLen() = 0;
    virtual void PutDataLen( DWORD newVal) = 0;
    virtual DWORD GetReservedBytes()=0;
    virtual DWORD GetUserData()=0;
    virtual DWORD PutUserData(DWORD newVal) = 0;
    virtual PBYTE GetBuffer()=0;
    virtual BOOL SetBufSize( DWORD nNewVal ) = 0;
    virtual PBYTE AcquireHeadBuf( int nHeadLen ) = 0;
};

//************************************************************************************

static const IID IID_IFileObject = {0xC60599AC,0x3C99,0x4ef8,{0xB5,0xB6,0x90,0xC4,0x9B,0xB0,0x4C,0x06}};
//MIDL_INTERFACE("C60599AC-3C99-4ef8-B5B6-90C49BB04C06")
class IFileObject : public IBufPacket
{
public:
    IFileObject(){}
    virtual ~IFileObject(){}
public:
    virtual LPCSTR GetFileName()=0;
    virtual DWORD GetAttribute()=0;
    virtual time_t GetLastModifyTime()=0;
    virtual time_t GetCreatTime()=0;
    virtual time_t GetLastAccessTime()=0;
    virtual DWORD GetFilePurpose()=0;
    virtual time_t GetPacketTime()=0;
    virtual PBYTE GetAttributeExtData( PDWORD pdwLen=NULL )=0;
    virtual PBYTE GetExtData( PDWORD pdwLen=NULL )=0;
    virtual PBYTE GetHugeFileParam( PDWORD pdwLen=NULL )=0;
    virtual LPCSTR GetIPAddress()=0;
    virtual int GetPort()=0;
    virtual BOOL SaveTo( LPCSTR lpszPath, BOOL bIgnoreSubDirectory=FALSE, BOOL bRestoreTimes=TRUE)=0;
};

//************************************************************************************

class IDVBReceiverEvent : public IMyUnknown
{
public:
    IDVBReceiverEvent(){}
    virtual ~IDVBReceiverEvent(){}
public:
    virtual void OnFileOK( IFileObject * pObj, HDATAPORT hDataPort ) = 0;
    virtual void OnSubFileOK( IFileObject * pObj, HDATAPORT hDataPort ) = 0;
    virtual void OnProgress( HDATAPORT hDataPort, float fProgress, DWORD dwBroLoopCount, int dwFileCount, DWORD dwTotalLen, DWORD dwByteReceived, int nCountReceived, LPCSTR lpszFileName ) = 0;
};

//************************************************************************************


static const IID IID_IDVBFileReceiver = {0x77035EDB,0x54B1,0x46EC,{0x89,0xB8,0x24,0xA0,0x78,0x70,0x80,0xB9}};
//MIDL_INTERFACE("77035EDB-54B1-46EC-89B8-24A0787080B9")
class IDVBFileReceiver : public IMyUnknown
{
public:
    IDVBFileReceiver(){}
    virtual ~IDVBFileReceiver(){}
public:
    virtual long GetMaxPacketSize() = 0;
    virtual LPCSTR GetVersion()=0;
    virtual BOOL GetSendSubFileEvent()=0;
    virtual void PutSendSubFileEvent( BOOL bNewVal ) = 0;
    virtual long GetIPPacketBPS()=0;
    virtual long GetFileBPS()=0;
    virtual long GetDataPortCount()=0;
    virtual LPCSTR GetAutoSavePath()=0;
    virtual void SetAutoSavePath( LPCSTR lpszNewVal )=0;
    virtual BOOL GetSendNotTSDBFileEvent()=0;
    virtual void SetSendNotTSDBFileEvent( BOOL bNewVal )=0;
    virtual void Close( void) = 0;
    virtual IIPFileMendHelper * GetIPFileMendHelper( HDATAPORT hDataPort ) = 0;
    virtual BOOL Init(bool bSaveFileInBackground=false, int varMaxPacketSize = 2048 ) = 0;
    virtual HDATAPORT CreateDataPort( LPCSTR lpszTargetIP, int nPort, LPCSTR lpszBindIP=NULL, BOOL bIsUDP = TRUE ) = 0;
    virtual HDATAPORT AddDataPort( void * pDataPort ) = 0;
    virtual BOOL DeleteDataPort( HDATAPORT hDataPort ) = 0;
    virtual void RegisterEventResponser( IDVBReceiverEvent * pObject ) = 0;
    virtual void DoMessagePump( void ) = 0;
    virtual float GetProgressInfo( HDATAPORT hDataPort, DWORD & dwBroLoopCount, int & dwFileCount, DWORD & dwTotalLen, DWORD & dwByteReceived, int & nCountReceived ) = 0;
    virtual BOOL PutSendProgressEvent( BOOL bNewValue ) = 0;
    virtual BOOL GetSendProgressEvent() = 0;
    virtual BOOL GetDotMapFileOnFileOKEvent() = 0;
    virtual void SetDoMapFileOnfileOKEvent( BOOL bNewValue ) = 0;
};

//----------------------------------------------------------------
//	File EPG Receiver
// {6CF52BF6-D87D-4ed5-B7FA-D1BE42B5C1D5}
static const IID IID_IDVBEPGReceiver = { 0x6cf52bf6, 0xd87d, 0x4ed5, { 0xb7, 0xfa, 0xd1, 0xbe, 0x42, 0xb5, 0xc1, 0xd5 } };

class IDVB_EPG_Receiver : public IDVBFileReceiver
{
public:
    IDVB_EPG_Receiver(){}
    virtual ~IDVB_EPG_Receiver(){}
public:
    virtual bool GetEnableOldEPG()=0;
    virtual void SetEnableOldEPG( bool bNewValue ) = 0;
};

//************************************************************************************

static const IID IID_IIPFileMendHelper = {0xDA4BCA9C,0x66B3,0x42EC,{0xAE,0x8D,0x3B,0x36,0xC0,0x77,0xC4,0xEA}};
//MIDL_INTERFACE("DA4BCA9C-66B3-42EC-AE8D-3B36C077C4EA")
class IIPFileMendHelper : public IMyUnknown
{
public:
    IIPFileMendHelper(){}
    virtual ~IIPFileMendHelper(){}
public:
    virtual long GetSubFileHasReceived()=0;
    virtual long GetTotalSubFileCount()=0;
    virtual BOOL GetIsSubFileOK( long nIndex ) = 0;
    virtual PBYTE GetDataBufferVC()=0;
    virtual BOOL SetTotalSubFileCount(long nNewValue) = 0;
    virtual long SetBitValue( int nIndex,int nBitValue ) = 0;
    virtual void Prepare( void) = 0;
    virtual long GetNextFileID( int nBitValue ) = 0;
    virtual long ReStat()=0;
    virtual long Combine( IIPFileMendHelper *pSrcObj ) = 0;
    virtual BOOL LoadFromFile( LPCSTR lpszFileName ) = 0;
    virtual BOOL SaveToFile( LPCSTR lpszFileName ) = 0;
    virtual IIPFileMendHelper * Clone() = 0;
};

////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//	Initialize the IPR driver, must call before any operate on the IPRD
#ifdef _WIN32
bool WINAPI IPRD_Init(void);
typedef bool (WINAPI * PFN_IPRD_INIT)(void);

void WINAPI IPRD_Close(void);
typedef void (WINAPI * PFN_IPRD_CLOSE)(void);

IDVBFileReceiver * WINAPI CreateDVBFileReceiver(void);
typedef IDVBFileReceiver * (WINAPI * PFN_CREATEDVBFILERECEIVER)(void);

IDVB_EPG_Receiver * WINAPI Create_DVB_EPG_Receiver(void);
typedef IDVB_EPG_Receiver * ( WINAPI * PFN_CREATE_DVB_EPG_RECEIVER)(void);

#else

bool IPRD_Init(void);
typedef bool (* PFN_IPRD_INIT)(void);

void IPRD_Close(void);
typedef void (* PFN_IPRD_CLOSE)(void);

IDVBFileReceiver * CreateDVBFileReceiver(void);
typedef IDVBFileReceiver * (* PFN_CREATEDVBFILERECEIVER)(void);

IDVB_EPG_Receiver * Create_DVB_EPG_Receiver(void);
typedef IDVB_EPG_Receiver * ( * PFN_CREATE_DVB_EPG_RECEIVER)(void);
#endif //_WIn32

#ifdef __cplusplus
}
#endif //__cplusplus

#pragma pack(pop)

#endif // __DVB_CPLUSPLUS_INTERFACE_H_20031201__
