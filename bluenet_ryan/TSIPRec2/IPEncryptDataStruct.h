///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2002-11-13
///
///=======================================================

#ifndef __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__
#define __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__

///--------------------------------------------------------
///	一下为通视文件接收专用的 IP 加密加密。
///	该情况下，一定为多播/UDP包方式。
///	此时加密后的数据可以通过UDP的方式再次播出

#define TS_IP_ENCRYPT_DATA_TAG	0x45495354		// 'TSIE', TongShi IP Encrypt
#define TS_IP_ENCRYPT_VERSION	0x100			//	当前的加密版本

#pragma pack( push,1 )							//	一个字节对齐

typedef struct tagTS_IP_ENCRYPT_STRUCT
{
	DWORD	m_dwTag;							//	应该等于 TS_IP_ENCRYPT_DATA_TAG，否则一定不是通视的
	DWORD	m_dwHeadCRC32;						//	数据头的 CRC32
	WORD	m_wVersion;							//	版本号，当前为 0x100
	WORD	m_wSrcDataLen;						//	原始数据
	DWORD	m_dwSrcDataCRC32;					//	原始数据的CRC32
	DWORD	m_dwSysCodeIndex;					//	加密用的系统密码索引
	DWORD	m_dwDrvSN:24;						//	加密驱动使用的序列号，3 字节的HEX数字
	DWORD	m_dwReserved:8;						//	保留，必须为 0
}TS_IP_ENCRYPT_STRUCT,*PTS_IP_ENCRYPT_STRUCT;






#pragma pack( pop )

#endif // __IP_ENCRYPT_DATA_STRUCT_INCLUDE_20021113__