///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			计算dvb上的 crc32 & crc16
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __DVB_CRC_H_20050111__
#define __DVB_CRC_H_20050111__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

DWORD DVB_GetCRC32( PBYTE pBuf, int nLen, DWORD dwLastCRC = 0xFFFFFFFF);
unsigned short DVB_GetCRC16(unsigned char * pBuf, int nLen, unsigned short wLastCRC=0xFFFF);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __DVB_CRC_H_20050111__
