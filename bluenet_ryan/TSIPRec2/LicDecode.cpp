// LicDecode.cpp: implementation of the CLicDecode class.
//
//////////////////////////////////////////////////////////////////////
//	2000.9.5	添加成员函数 GetVersion, 获取数据格式的版本号

#include "stdafx.h"
#include "LicDecode.h"
#include "Crc.h"

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

//	待解密的卡号
CLicDecode::CLicDecode()
{
	m_pDataBuf = NULL;
	m_pHeader = NULL;
	m_dwIDCodeReal = 0;
#ifdef _WIN32
	RtlZeroMemory( &m_UserID, sizeof(m_UserID) );
#else
	bzero( &m_UserID, sizeof(m_UserID) );
#endif //_WIN32
}

CLicDecode::~CLicDecode()
{
}

void CLicDecode::Init(DVB_USER_ID & UserID)
{
	m_UserID = UserID;
	m_dwIDCodeReal = UserID.m_RcvID.m_adwID[0];
	LockIDCode();
	m_pDataBuf = NULL;
	m_pHeader = NULL;
}

//	加绕卡号
void CLicDecode::LockIDCode()
{	
	DWORD dwIDCode = m_dwIDCodeReal;	// V1.00, V1.01 只使用低4字节用于授权
	_asm{
		mov eax,dwIDCode
		ror eax,3
		mov dl,al
		mov dh,al
		shl edx,16
		mov dh,al
		xor eax,edx
		mov dwIDCode,eax
	}
	m_dwIDCode = dwIDCode;
}

//	解绕卡号
//	入口参数
//		dwIDCode					卡号
DWORD CLicDecode::UnlockIDCode(DWORD dwIDCode)
{
	_asm{
		mov eax,dwIDCode
		mov dl,al
		mov dh,dl
		shl edx,16
		mov dh,al
		xor eax,edx
		rol eax,3
		mov dwIDCode,eax
	}
	return dwIDCode;
}

//	附着数据
//	入口参数
//		pLicBuf					授权数据缓冲区
//	返回参数
//		TRUE					CRC 校验成功
//		FALSE					CRC32 校验失败
BOOL CLicDecode::Attach(PBYTE pLicBuf)
{
	ASSERT( pLicBuf );
	m_pDataBuf = pLicBuf;
	m_pHeader = (PBLKHEADER)pLicBuf;
	if( m_pHeader->m_wVersion >= 0x102 )			//	目前只能解释 1.00 和 1.01 版本的格式
		return FALSE;
	if( CCRC::GetCRC32( TSLOCKCODECLEN-sizeof(DWORD), pLicBuf+sizeof(DWORD) ) != m_pHeader->m_dwCRC32 )
	{
		Detach();
		return FALSE;
	}
	return TRUE;
}

//	释放数据
void CLicDecode::Detach()
{
	m_pDataBuf = NULL;
	m_pHeader = NULL;
}

//	是否有该卡号的授权数据
//	返回参数
//		FALSE					肯定没有数据
//		TRUE					可能有数据
BOOL CLicDecode::IsInRange()
{
	ASSERT( m_pHeader );
	DWORD dwIDCode = UnlockIDCode( m_pHeader->m_dwMinIDCode );
	if( dwIDCode > m_dwIDCodeReal )
		return FALSE;
	dwIDCode = UnlockIDCode( m_pHeader->m_dwMaxIDCode );
	if( dwIDCode < m_dwIDCodeReal )
		return FALSE;
	return TRUE;
}

//	获取系统密码索引
DWORD CLicDecode::GetSysCodeIndex()
{
	ASSERT( m_pHeader );
	return m_pHeader->m_dwSysCodeIndex;
}

//	取下一个授权数据
//	返回参数
//		NULL					没有授权数据
//		数据号的指针
PDWORD CLicDecode::GetLicData()
{
register DWORD dwIDCode;
	ASSERT( m_pHeader );
	ASSERT( GetLicMethod() == LICOP_LIC );
	int nPos = sizeof(BLKHEADER);
	for(register int i=0; i<m_pHeader->m_wBlkNum && nPos<TSLOCKCODECLEN; i++)
	{									//	主循环
		PSUBBLKHEAD pSubHeader = (PSUBBLKHEAD)(m_pDataBuf+nPos);
		if( pSubHeader->m_dwIDCode == m_dwIDCode )
			return & pSubHeader->m_dwLicCode;					//	找到
		nPos += sizeof(SUBBLKHEAD);
		int nUserNum = pSubHeader->m_byUserNum - 1;
		DWORD dwBaseIDCode = UnlockIDCode( pSubHeader->m_dwIDCode );
		switch( pSubHeader->m_bySubType )
		{
		case LICTYPE_CONTINUE:
			{
				dwIDCode = dwBaseIDCode;
				PSUBBLK_CONTINUE	pSC = (PSUBBLK_CONTINUE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode ++;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSC->m_dwLicCode;				//	找到
					nPos += sizeof(SUBBLK_CONTINUE);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSC ++;
				}
			}
			break;

		case LICTYPE_LESS100:
			{
				PSUBBLK_LESS100		pSL1 = (PSUBBLK_LESS100)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL1->m_byIDCode + dwBaseIDCode;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSL1->m_dwLicCode;
					nPos += sizeof(SUBBLK_LESS100);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL1 ++;
				}
			}
			break;

		case LICTYPE_LESS10000:
			{
				PSUBBLK_LESS10000	pSL2 = (PSUBBLK_LESS10000)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL2->m_wIDCode + dwBaseIDCode;
					if( dwIDCode == m_dwIDCodeReal )
						return &pSL2->m_dwLicCode;
					nPos += sizeof(SUBBLK_LESS10000);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL2 ++;
				}
			}
			break;

		case LICTYPE_INDEPENDENCE:
			{
				PSUBBLK_INDEPENDENCE pSI = (PSUBBLK_INDEPENDENCE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					if( pSI->m_dwIDCode == m_dwIDCode )
						return &pSI->m_dwLicCode;
					pSI ++;
					nPos += sizeof(SUBBLK_INDEPENDENCE);
					ASSERT( nPos<TSLOCKCODECLEN );
				}
			}
			break;

		default:
			ASSERT( FALSE );
			return NULL;						//	失败
		}
	}
	return NULL;
}


#ifdef __CYJ_TEST_LICCODE__
//	列表卡号
//	入口参数
//		pBuf				输出缓冲区, 大小 = 10K*sizeof(DWORD)
//	返回参数
//		该文件包含的授权卡号数
int CLicDecode::ListIDCode(PDWORD pBuf)
{
register DWORD dwIDCode;
int nNo = 0;
	ASSERT( m_pHeader );
	int nPos = sizeof(BLKHEADER);
	for(register int i=0; i<m_pHeader->m_wBlkNum && nPos<TSLOCKCODECLEN; i++)
	{									//	主循环
		PBLKHEADER pHead = (PBLKHEADER)m_pDataBuf;
		if( pHead->m_byLicOpCode != LICOP_LIC )
			return 0;
		PSUBBLKHEAD pSubHeader = (PSUBBLKHEAD)(m_pDataBuf+nPos);
		nPos += sizeof(SUBBLKHEAD);
		int nUserNum = pSubHeader->m_byUserNum - 1;
		DWORD dwBaseIDCode = UnlockIDCode( pSubHeader->m_dwIDCode );
		pBuf[nNo++] = dwBaseIDCode;
		switch( pSubHeader->m_bySubType )
		{
		case LICTYPE_CONTINUE:
			{
				dwIDCode = dwBaseIDCode;
				PSUBBLK_CONTINUE	pSC = (PSUBBLK_CONTINUE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode ++;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_CONTINUE);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSC ++;
				}
			}
			break;

		case LICTYPE_LESS100:
			{
				PSUBBLK_LESS100		pSL1 = (PSUBBLK_LESS100)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL1->m_byIDCode + dwBaseIDCode;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_LESS100);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL1 ++;
				}
			}
			break;

		case LICTYPE_LESS10000:
			{
				PSUBBLK_LESS10000	pSL2 = (PSUBBLK_LESS10000)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					dwIDCode = pSL2->m_wIDCode + dwBaseIDCode;
					pBuf[nNo++] = dwIDCode;
					nPos += sizeof(SUBBLK_LESS10000);
					ASSERT( nPos<TSLOCKCODECLEN );
					pSL2 ++;
				}
			}
			break;

		case LICTYPE_INDEPENDENCE:
			{
				PSUBBLK_INDEPENDENCE pSI = (PSUBBLK_INDEPENDENCE)( m_pDataBuf+nPos);
				for(register int j=0; j<nUserNum; j++)
				{
					pBuf[nNo++] = UnlockIDCode( pSI->m_dwIDCode );
					pSI ++;
					nPos += sizeof(SUBBLK_INDEPENDENCE);
					ASSERT( nPos<TSLOCKCODECLEN );
				}
			}
			break;

		default:
			ASSERT( FALSE );
			return 0;						//	失败
		}
	}
	return nNo;
}

#endif // __CYJ_TEST_LICCODE__

//	获取授权操作
//	如:		添加,	删除
LICOP_METHOD CLicDecode::GetLicMethod()
{
	LICOP_METHOD opcode = (LICOP_METHOD)m_pHeader->m_byLicOpCode;
	return opcode;
}

//	是否该删除该段号
//	返回参数
//		TRUE					该删除
//		FALSE					不删除
BOOL CLicDecode::IsToDelete()
{
	ASSERT( m_pHeader );
	ASSERT( GetLicMethod() == LICOP_DEL );
	register PDWORD pdwCloseID = (PDWORD)( m_pDataBuf + sizeof(BLKHEADER) );
	int nCount = m_pHeader->m_wBlkNum;
	for(register int i=0; i<nCount; i++)
	{
		if( *pdwCloseID == m_dwIDCode )
			return TRUE;
		pdwCloseID ++;
	}
	return FALSE;
}

//	获取版本号
WORD CLicDecode::GetVersion()
{
	ASSERT( m_pHeader );
	if( m_pHeader == NULL )
		return 0;
	return m_pHeader->m_wVersion;
}
