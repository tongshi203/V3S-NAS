#ifndef __CODECFORMAT_INCLUDE_H_19991105__
#define __CODECFORMAT_INCLUDE_H_19991105__

#pragma pack(push,1)

	#define  TSLOCKCODECLEN 6600 
	typedef enum {
		LICTYPE_CONTINUE = 0,						//	n 个连续的卡号
		LICTYPE_LESS100,							//	n 个卡号差值小于 100 的卡号组合
		LICTYPE_LESS10000,							//	n 个卡号差值小于 10000 的卡号组合
		LICTYPE_INDEPENDENCE,						//	n 个相互独立的卡号
	}LICTYPE_ENUM;
	typedef enum {									//	授权方法
		LICOP_LIC = 0,
		LICOP_DEL,
	}LICOP_METHOD;
	typedef struct tagBLKHEADER						//	数据块数据头
	{
		DWORD	m_dwCRC32;							//	数据的 CRC32
		WORD	m_wVersion;							//	版本号
		DWORD	m_dwMinIDCode;						//	最小卡号
		DWORD	m_dwMaxIDCode;						//	最大卡号
		WORD	m_wBlkNum;							//	子块数
		DWORD	m_dwSysCodeIndex;					//	系统密码索引
		BYTE	m_byLicOpCode;						//	授权操作指令
	}BLKHEADER,*PBLKHEADER;

	typedef struct tagSUBBLKHEAD
	{
		BYTE	m_bySubType;						//	子类型
		BYTE	m_byUserNum;						//	该子类型的用户数
		DWORD	m_dwIDCode;							//	用户卡号
		DWORD	m_dwLicCode;						//	授权码
	}SUBBLKHEAD,*PSUBBLKHEAD;

	typedef struct tagSUBBLK_CONTINUE				//	n 个连续卡号的组合
	{
		DWORD	m_dwLicCode;						//	授权号
	}SUBBLK_CONTINUE,*PSUBBLK_CONTINUE;

	typedef struct tagSUBBLK_LESS100
	{												//	n 个卡号差值不超过100的卡号
		BYTE	m_byIDCode;
		DWORD	m_dwLicCode;
	}SUBBLK_LESS100,*PSUBBLK_LESS100;

	typedef struct tagSUBBLK_LESS10000
	{												//	n 个卡号差值不超过 10000
		WORD	m_wIDCode;							//	卡号
		DWORD	m_dwLicCode;						//	授权号
	}SUBBLK_LESS10000,*PSUBBLK_LESS10000;

	typedef struct tagSUBBLK_INDEPENDENCE
	{												//	n 个独立的卡号
		DWORD	m_dwIDCode;							//	卡号
		DWORD	m_dwLicCode;						//	授权号
	}SUBBLK_INDEPENDENCE,*PSUBBLK_INDEPENDENCE;

#pragma pack(pop)

#endif // __CODECFORMAT_INCLUDE_H_19991105__