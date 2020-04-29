///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2003-7-18
///
///		用途：
///			IP File Receive DLL interface
///=======================================================

#ifndef __IP_FILE_REC_DLL_20030718__
#define __IP_FILE_REC_DLL_20030718__

#pragma pack(push,4)

// {EF80B1A3-34FC-4800-A855-541E34006F2E}
static const GUID IID_IMyBufPacket = { 0xef80b1a3, 0x34fc, 0x4800, {0xa8, 0x55, 0x54, 0x1e, 0x34, 0x0, 0x6f, 0x2e} };
// {F1646ED0-4A4A-4f1f-9B9A-D6E3CACC1C30}
static const GUID IID_IMyFileObject ={ 0xf1646ed0, 0x4a4a, 0x4f1f, {0x9b, 0x9a, 0xd6, 0xe3, 0xca, 0xcc, 0x1c, 0x30} };

//--------------------------------------------------
class IMyBufPacket : public IMyUnknown
{
public:
	virtual DWORD GetBufSize() = 0;
	virtual DWORD GetDataLen() = 0;
	virtual void  PutDataLen( DWORD dwNewValue ) = 0;
	virtual DWORD	GetReservedBytes() = 0;
	virtual DWORD	GetUserData() = 0;
	virtual DWORD	PutUserData( DWORD dwNewValue ) = 0;
	virtual PBYTE	GetBuffer() = 0;
	virtual BOOL	SetBufSize( DWORD dwNewValue ) = 0;
	virtual PBYTE	AcquireHeadBuf( int nHeadLen ) = 0;
};

//--------------------------------------------------
class IMyFileObject : public IMyBufPacket
{
public:
	virtual LPCSTR	GetFileName() = 0;
	virtual DWORD	GetAttribute() = 0;
	virtual time_t	GetLastModifyTime() = 0;
	virtual time_t	GetCreateTime() = 0;
	virtual time_t	GetLastAccessTime() = 0;
	virtual DWORD	GetFilePurpose() = 0;
	virtual time_t	GetPacketTime() = 0;
	virtual PBYTE	GetAttribExtData( int * pnDataLen ) = 0;
	virtual PBYTE	GetExtData( int * pnDataLen ) = 0;
	virtual PBYTE	GetHugeFileParam( int * pnDataLen ) = 0;
	virtual LPCSTR	GetIPAddress() = 0;
	virtual int		GetIPPort() = 0;
	virtual BOOL	SaveTo( LPCSTR lpszPath, BOOL bIgnoreSubDirectory = FALSE ) = 0;
};

#pragma pack(pop)

#endif // __IP_FILE_REC_DLL_20030718__

