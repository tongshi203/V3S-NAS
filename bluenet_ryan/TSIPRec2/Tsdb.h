//////////////////////////////////////////////////////////////////////////
//			TSDB 传输打包协议
//
//			西安通视数据有限责任公司
//			陈永健
//			1999.6.15
//
//			修改记录
//------------------------------------------------------------------------
// 2000.11.2	TSDBLOCKHEAD 添加成员变量 m_dwOrgCRC32 和 m_dwOffset 字段
//
//
/////////////////////////////////////////////////////////////////////////

#ifndef __TSDB_INCLUDE_19990615__
#define __TSDB_INCLUDE_19990615__

#include "crc.h"				//	CRC CLASS

// 2016.4.23 CYJ Add
#include <stdint.h>

#if _MSC_VER >= 1100
#undef  TSDB_DEFINE_GUID
#define TSDB_DEFINE_GUID EXTERN_GUID
#else
#define TSDB_DEFINE_GUID DEFINE_GUID
#endif

//	常数定义
//	多文件 {57A72F61-B84E-11d2-B30C-00C04FCCA334}
TSDB_DEFINE_GUID(CLSID_TSDBMULFILEHEAD, 0x57a72f61, 0xb84e, 0x11d2, 0xb3, 0xc, 0x0, 0xc0, 0x4f, 0xcc, 0xa3, 0x34);

//	单个文件 {57A72F62-B84E-11d2-B30C-00C04FCCA334}
TSDB_DEFINE_GUID(CLSID_TSDBFILEHEADER,  0x57a72f62, 0xb84e, 0x11d2, 0xb3, 0xc, 0x0, 0xc0, 0x4f, 0xcc, 0xa3, 0x34);

//	压缩数据头
TSDB_DEFINE_GUID(CLSID_TSDBCOMPRESSHEADER,0x7a70d342, 0x2232, 0x11d3, 0xb8, 0xe9, 0x0, 0x50, 0x4, 0x86, 0x8e, 0xaa);

//	解密数据头	{CEA772C7-7045-4f26-9973-7211C0377441}
TSDB_DEFINE_GUID(CLSID_TSDBLOCKHEADER,0xcea772c7, 0x7045, 0x4f26, 0x99, 0x73, 0x72, 0x11, 0xc0, 0x37, 0x74, 0x41);

//	每行数据字节
#define		DATABYTES_PERLINE		42

#pragma pack(push,1)						//	按一个字节对齐编译

///////////////////////////////////////////////////////////////////
//	通道结构定义
#ifndef __TONGSHI_TSDB_STRUCT__
#define __TONGSHI_TSDB_STRUCT__
	typedef union tagTSDBCHANNEL
	{
		struct
		{
			BYTE	m_nSubPage;						//	子页号
			BYTE	m_nPageNo;						//	页号
			BYTE	m_nMag;							//	杂志号
			BYTE	m_nVBINo;						//	等效 VBI 序号
		};
		DWORD		m_dwData;						//	数据
	}TSDBCHANNEL,*PTSDBCHANNEL;
#endif // __TONGSHI_TSDB_STRUCT__

////////////////////////////////////////////////////////////////////
//	多文件打包
typedef struct tagTSDBMULFILEHEAD
{
	GUID		m_CLSID;					//	GUID, CLSID_TSDBMULFILEHEAD
	WORD		m_cbSize;					//	文件头大小
	DWORD		m_dwHeaderCRC32;			//	文件头 CRC32
	WORD		m_wVersion;					//	版本号, 为以后兼容做准备
	BYTE		m_cFileNum;					//	文件的个数
	WORD		m_wFileDataOfs[2];			//	长度 = ?, 由 m_cbSize 确定, 但至少 2 文件
}TSDBMULFILEHEAD,*PTSDBMULFILEHEAD;

#define MULFILEHEAD_MAXSIZE					(sizeof(TSDBMULFILEHEAD)+sizeof(WORD)*100)
#define MULFILEHEAD_MINFILELENGATE		6144	//	< 6K 的文件需要用多文件
//////////////////////////////////////////////////////////////////////
#define TSDBFILEHEADER_VERSION			0x101
#define TSDB_NORMALFILELEN				10240

//	单个文件封装
typedef struct tagTSDBFILEHEADER
{
	GUID		m_CLSID;						//	GUID, CLSID_TSDBFILEHEADER
	WORD		m_cbSize;						//	文件头大小, 包含
	DWORD		m_dwHeaderCRC32;				//	文件头的 CRC32
	WORD		m_wVersion;						//	版本号, 为以后兼容做准备
	DWORD		m_dwFileCRC32;					//	文件 / 块的 CRC32
	DWORD		m_dwFileLen;					//	文件长度
	WORD		m_cbFileNameLenCount;			//	文件名字符个数
	union{
		struct
		{
			DWORD			m_bHugeFile:1;				//	是否大文件
			DWORD			m_bHasAttrib:1;				//	是否有文件属性
			DWORD			m_bWinSock:1;				//	是否有 WinSocket 地址
			DWORD			m_bHasMuiticast:1;			//	是否有 Multicast 地址
			DWORD			m_bFlagRes:28;
		};
		DWORD	m_dwFlags;
	};											//	在此后还有不定长度的文件名, 各个属性表
}TSDBFILEHEADER,*PTSDBFILEHEADER;

//////////////////////////////////////////////////////////////////////////
//	大文件
//	大文件接收记录扩展名
#define HUGEFILEMSG_EXTNAME			".TS"
#define HUGEFILE_SIZEGATE			20480		//	20K

//	数据结构
typedef struct tagTSDBHUGEFILEHEAD
{
	WORD		m_cbSize;						//	数据包大小
	WORD		m_wVersion;						//	版本
	DWORD		m_dwFileLen;					//	文件长度
	DWORD		m_dwFileCRC32;					//	文件 CRC32
	WORD		m_wTotalBlock;					//	总块数
	WORD		m_wBlockNo;						//	块序号
	DWORD		m_dwFilePosition;				//	起始偏移
	WORD		m_wBlockSize;					//	块大小
	char		m_szTmpFileName[25];			//	临时文件名
//	1.02 及其以上版本
	struct										//	1.02 以上版本才处理
	{
		DWORD	m_bNotSaveTmpFile:1;			//	不保存临时文件，由应用程序处理
		DWORD	m_dwRes0:31;					//	保留必须 ＝ 0
	}m_Flags;
}TSDBHUGEFILEHEAD,*PTSDBHUGEFILEHEAD;

///////////////////////////////////////////////////////////////////////////
//  文件属性
// 2016.4.23 CYJ Modify, change time_t to int32_t, since linux-64, time_t is 64 bits
typedef struct tagTSDBFILEATTRIBHEAD
{
	WORD		m_cbSize;						//	数据包大小
	DWORD		m_dwAttribute;					//	文件属性
	int32_t		m_CreateTime;					//	文件创建时间
	int32_t		m_LastAccessTime;				//	上次访问时间
	int32_t		m_LastWriteTime;				//	上次更新时间
	DWORD		m_dwPurpose;					//	文件用途
	BYTE		m_ExtData[1];					//	附加数据
}TSDBFILEATTRIBHEAD,*PTSDBFILEATTRIBHEAD;

////////////////////////////////////////////////////////////////////////////
//	SOCKET 地址
typedef struct tagTSDBSOCKETHEAD
{
	WORD		m_cbSize;						//	Socket 地址
}TSDBSOCKETHEAD,*PTSDBSOCKETHEAD;

///////////////////////////////////////////////////////////////////////////
//	多地址传送
typedef struct tagTSDBMULTICAST
{
	WORD		m_cbSize;						//	数据头大小
	WORD		m_cbUnitSize;					//	数据单元大小
	WORD		m_nPacketNum;					//	数据包数
	BYTE		m_Data[1];						//	数据
}TSDBMULTICAST,*PTSDBMULTICAST;

///////////////////////////////////////////////////////////////////////////
//	数据压缩
typedef struct tagTSDBCOMPRESSHEAD
{
	GUID		m_CLSID;						//	GUID, CLSID_TSDBFILEHEADER
	WORD		m_cbSize;						//	文件头大小, 包含
	DWORD		m_dwHeaderCRC32;				//	文件头的 CRC32
	DWORD		m_dwMethod;						//	压缩方法
	WORD		m_wVersion;						//	压缩版本
	DWORD		m_dwFileLen;					//	压缩后文件长度
	DWORD		m_dwFileCRC32;					//	压缩后文件CRC32
	DWORD		m_dwOrgFileLen;					//	压缩前的文件长度
	DWORD		m_dwOrgFileCRC32;				//	压缩前的文件 CRC32
}TSDBCOMPRESSHEAD,*PTSDBCOMPRESSHEAD;			//	还有附加数据

//	压缩方法长度定义
typedef enum
{
	TSDBCOMPRESS_METHOD_NOCMP =	0,				//	不压缩
	TSDBCOMPRESS_METHOD_AUTO,					//	自动压缩
	TSDBCOMPRESS_METHOD_ARG241 = 0x20,			//	ARJ	系列
	TSDBCOMPRESS_METHOD_PKZIP204 = 0x30,		//	PKZIP 系列
	TSDBCOMPRESS_METHOD_LZSSV100 = 0x40,		//	LZSS
	TSDBCOMPRESS_METHOD_LZHUFV100 = 0x50,		//	LZHUF
} TSDBCOMPRESSMETHODENUM;

///////////////////////////////////////////////////////////////////////////
//	数据加密
typedef struct tagTSDBLOCKHEAD
{
	GUID	m_CLSID;							//	GUID
	WORD	m_wHeadSize;						//	数据头大小
	DWORD	m_dwHeadCrc32;						//	数据头 CRC32
	DWORD	m_dwSysCodeIndex;					//	系统密码索引
	DWORD	m_dwDataLen;						//	数据长度
	DWORD	m_dwOffset;							//	数据块在文件中的偏移
	DWORD	m_dwOrgCRC32;						//	原始 CRC32
}TSDBLOCKHEAD, *PTSDBLOCKHEAD;

#pragma pack(pop)						//	按一个字节对齐编译

#endif // __TSDB_INCLUDE_19990615__
