//	DVB 播出协议

#ifndef __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__
#define __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__

// 2016.4.23 CYJ Add
#include <stdint.h>

typedef enum
{
	TSDVB_OFFSET = 0,						//	通过 Offset 确定偏移
	TSDVB_PAGELEN_864,						//	每页长度 = 864
	TSDVB_PAGELEN_1024,						//	每页长度 = 1024
}TSDVBMULTICASTBRO_MODE;

#pragma pack(push,1)						//	按一个字节对齐编译

typedef enum
{
	EPFI_SUBFILE_ID = 0,					//	子文件序号
}EXTPARAM_FINCTION_INDEX;

typedef struct tagEXTPARAM_SUBFILEID
{											//	子文件参数，m_wFunctionIndex = 0
	DWORD	m_dwSubFileID;					//	子文件序号，从 0 开始
	DWORD	m_dwTotalSubFileCount;			//	子文件总个数，为0无效
	BYTE	m_abyReserved[2];				//	保留，必须 ＝ 0
}EXTPARAM_SUBFILEID,*PEXTPARAM_SUBFILEID;

//	DVB Multicast 播出
// 2016.4.23 CYJ Modify, changed time_t to uint32_t
typedef struct tagTSDVBMULTICASTPACKET0
{
	DWORD		m_dwHeaderCRC32;				//	数据头 CRC32
	WORD		m_cbSize;						//	包大小
	uint32_t	m_PacketTime;					//	打包时间, 解释时间方法:标记  Bit31 = 1  当月所有秒 * 256 + 编码计数器
	TSDBCHANNEL	m_chFile;
	union
	{
		char		m_cFileName[13];			//	文件名, 包括字符串结束符 '\0'
		struct
		{
			BYTE	m_byReserved_0;				//	保留，必须为0，因为 0 表示空字符串
			BYTE	m_byXorChkSum;				//	后11字节的XOR Check Sum
			BYTE	m_byFunctionIndex;			//	功能索引，参见 EXTPARAM_FINCTION_INDEX
			union
			{									//	只能有10字节
				EXTPARAM_SUBFILEID	m_SubFileID;		// 子文件序号
			};
		}m_ExtParam;
	};
	WORD		m_wPageLen:12;					//	页长度，最大 4K
	WORD		m_wMode:4;						//	文件长度表示方式
	union
	{
		DWORD	m_dwFileLen;					//	文件长度, m_dwFileLenMode = TSDVB_OFFSET
		struct
		{
			DWORD	m_dwFTotalLen:21;			//	总共的文件页数,m_wMode 确定，m_wMode != TSDVB_OFFSET
			DWORD	m_dwPageNo:11;				//	固定的页编码
		};
	};
	union
	{
		DWORD	m_dwPageOfs;					//	该页在整个文件中的偏移,TSDVB_OFFSET
		DWORD	m_dwSysCodeIndex;				//	加密系统密码,m_wMode != TSDVB_OFFSET
	};
	DWORD		m_dwPageCRC32;					//	页 CRC32
public:
	DWORD	GetFileLen()
	{
		if( TSDVB_OFFSET == m_wMode )
			return m_dwFileLen;
		else
			return m_dwFTotalLen;
		ASSERT( FALSE );
		return 0;
	};
	DWORD	GetPageOfsInFile()
	{
		switch( m_wMode )
		{
		case TSDVB_OFFSET:
			return m_dwPageOfs;
		case TSDVB_PAGELEN_864:
			return m_dwPageNo * 864;
		case TSDVB_PAGELEN_1024:
			return m_dwPageNo * 864;
		default:
			ASSERT( FALSE );
		}
		return 0;
	};
	WORD	GetPageLen()
	{
		switch( m_wMode )
		{
		case TSDVB_OFFSET:
			return 1024;
		case TSDVB_PAGELEN_864:						//	每页长度 = 864
			return 864;
		case TSDVB_PAGELEN_1024:					//	每页长度 = 1024
			return 1024;
		default:
			ASSERT( FALSE );
			return 1024;
		}
	};

	///-------------------------------------------------------
	/// 2003-4-3
	/// 功能：
	///		获取扩展参数序号
	/// 入口参数：
	///		无
	/// 返回参数：
	///		>=0					扩展功能序号
	///		<0					失败
	int GetExtParamFunctionIndex()
	{
		if( m_ExtParam.m_byReserved_0 )
			return -1;			//	不是扩展
		BYTE byXorValue = 0;
		for(int i=2; i<13; i++)
		{
			byXorValue ^= (BYTE)m_cFileName[i];
		}
		if( byXorValue != m_ExtParam.m_byXorChkSum )
			return -2;
		return m_ExtParam.m_byFunctionIndex;
	}
}TSDVBMULTICASTPACKET0,*PTSDVBMULTICASTPACKET0;					//	48 字节

#pragma pack(pop)						//	按一个字节对齐编译



#endif // __TSDVB_MULTICAST_BRO_PROTOCOL_20010903__
