// UNLZSS32.cpp: implementation of the CUNLZSS32 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "unlzss32.h"

#include "crc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUNLZSS32::CUNLZSS32()
{
	m_pOutBufAutoAlloc = NULL;					//	自动分配的内存
}

CUNLZSS32::~CUNLZSS32()
{
	if( m_pOutBufAutoAlloc )
		delete[] m_pOutBufAutoAlloc;
}

//	将一个压缩文件附着到CUNLZSS 对象
//	入口参数
//		nFileLen				文件长度
//		pBuf					缓冲区
//	返回参数
//		文件个数
int CUNLZSS32::Attach(int nFileLen, PBYTE pBuf)
{
	CUnCompressObj::Detach();
	if( CUnCompressObj::IsCompress(nFileLen, pBuf) == FALSE )
		return 0;					//	不是压缩文件
	CUnCompressObj::Attach( nFileLen, pBuf );
	return 1;
}

//	取文件个数
int CUNLZSS32::GetFileNum()
{	
	if( GetHeader() )							
		return 1;
	else
		return 0;					//	没有数据
}

//	解压一个文件
//	入口参数
//		nFileNo					文件序号
//		outfStatus				输出文件状态
//		pDstBuf					用户指定缓冲区,若NULL,自动分配内存
//	返回参数
//		解压后的文件缓冲区
//		NULL					失败
PBYTE CUNLZSS32::DecodeOneFile(int nFileNo, CFileStatus &outfStatus, PBYTE pDstBuf)
{
	ASSERT( nFileNo == 0 );						//	因为只有一个文件

	FreeMemory();	

	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );
	if( CCRC::GetCRC32( pHeader->m_dwFileLen,GetDataBuf() ) != pHeader->m_dwFileCRC32 )
		return NULL;							//	CRC 错误

	SetDstBuffer( pHeader->m_dwOrgFileLen, pDstBuf );		//	设置输出文件缓冲区
	Decode();									//	解压文件
	ASSERT( pHeader->m_dwOrgFileLen == (DWORD)m_nOutDataLen );		//	测试解压是否正确
	ASSERT( pHeader->m_dwOrgFileCRC32 == CCRC::GetCRC32(pHeader->m_dwOrgFileLen,m_pDstDataBuf) );
	m_pOutBufAutoAlloc = ReleaseDstBuffer( outfStatus.m_size );
	if( NULL == pDstBuf )
		return m_pOutBufAutoAlloc;

	m_pOutBufAutoAlloc = NULL;					//	使用外部的内存
	return pDstBuf;
}

//	取一个文件状态
//	入口参数
//		nFileNo					文件序号
//		outfStatus				状态
//	返回参数
//		文件长度
//		0						失败
int CUNLZSS32::GetFileInfo(int nFileNo, CFileStatus &outfStatus)
{
	ASSERT( nFileNo == 0 );					//	只支持 1 个文件
	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );
	outfStatus.m_szFullName[0] = 0;			//	文件名,属性和时间参见 TSDBFILEHEAD
	outfStatus.m_size = pHeader->m_dwOrgFileLen;
	return pHeader->m_dwOrgFileLen;
}


//	取解压方法常数
DWORD CUNLZSS32::GetCompressMethod()
{
	return TSDBCOMPRESS_METHOD_LZSSV100;
}

//	获取版本号
int CUNLZSS32::GetDecoderVersion()
{
	return CURRENT_UNLZSS_VERSION*0x100 + UNLZSS_MINOR_VER;
}

//	释放内存
void CUNLZSS32::FreeMemory()
{
	if( m_pOutBufAutoAlloc )
		delete m_pOutBufAutoAlloc;
	m_pOutBufAutoAlloc = NULL;
}

//	解码
void CUNLZSS32::Decode()
{
	short i, j, k, r, c;
	WORD flags;
	BYTE text_buf[N + F - 1];
	BYTE out_text_buf[ OUT_TEXT_BUF_SIZE ];		//	输出缓冲区
	register int out_text_ptr = 0;						//	输出的字节数

	PTSDBCOMPRESSHEAD pHeader = GetHeader();
	ASSERT( pHeader );

	int nOrgFileLen = pHeader->m_dwOrgFileLen;
	int nCmpFileLen = pHeader->m_dwFileLen;
	int OutFileLen = 0;

	memset( text_buf,0x20,N - F);				//	预置内容

	r = N - F;  
	flags = 0;

	for(int nByteDecode=0; ; ) 
	{
		if ( ((flags >>= 1) & 256) == 0 ) 
		{
			c = ReadOneByte();
			nByteDecode ++;
			flags = c | 0xff00;					/* uses higher byte cleverly */
		}										/* to count eight */
		if( nByteDecode >= nCmpFileLen )
			break;
		if (flags & 1)							//	输出单个文件
		{
			c = ReadOneByte();
			nByteDecode ++;
			out_text_buf[out_text_ptr++] = (BYTE)c;	//	存一个字节
			text_buf[r++] = (BYTE)c;  
			r &= (N - 1);
		} 
		else 
		{
			i = ReadOneByte();
			j = ReadOneByte();
			nByteDecode += 2;
			i &= 0xff;
			i |= ((j & 0xf0) << 4);  
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) 
			{
				c = text_buf[(i + k) & (N - 1)];
				out_text_buf[out_text_ptr++] = (BYTE)c;	//	存一个字节
				text_buf[r++] = (BYTE)c; 
				r &= (N - 1);
			}
		}												
		if( out_text_ptr >= ( OUT_TEXT_BUF_SIZE - 2 * F ) )
		{											//	将缓冲的文件存盘
			Write( out_text_buf, out_text_ptr );
			OutFileLen += out_text_ptr;
			out_text_ptr = 0;
			if( OutFileLen >= nOrgFileLen )			//	文件满
				break;
		}
	}

	if( out_text_ptr )					//	将剩下的字节保存
		Write( out_text_buf, out_text_ptr );
}
