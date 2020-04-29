
#ifndef __CRC_CLS_H_990130__
#define __CRC_CLS_H_990130__

#define __ENABLE_CRC16__

class CCRC
{
public:
	static unsigned int GetCRC32(int nBytes, const unsigned char * pSrcBuf, unsigned int dwOrgCRC=0);
#ifdef __ENABLE_CRC16__
	static unsigned short GetCRC16(int nBytes, const unsigned char * pSrcBuf, unsigned short dwOrgCRC=0);
#endif // __ENABLE_CRC16__
};

#endif // __CRC_CLS_H_990130__
