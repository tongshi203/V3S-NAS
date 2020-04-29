#ifndef _BITSTRM_H
#define _BITSTRM_H
#include <stdlib.h>

// this class will increment Err if you proceed past the end of the buffer
// the buffer is padded with zeros if the input buffer length is not modulo 
// 4 bytes
class CMyBitStream
{
private:
	int				m_incnt;
	unsigned char *	m_rdbfr;
	unsigned long	m_rdbfr_length;
	unsigned char *	m_rdptr;
	unsigned char * m_rdmax;
	int				m_bitcnt;
	unsigned int	m_bfr;

	//  CYJ,2005-1-14 cyj add
	unsigned char * m_pWritePtr;
	int	m_nWriteBitsInCache;
	unsigned int m_dwWriteCache;
	int m_nTotalWriteBits;
	
	void initbits (int *Err=NULL, bool bRssetWriteParam = false );

public:
	CMyBitStream (unsigned char *pBuffer, unsigned long Length, int *Err=NULL);
	~CMyBitStream ();

	long			m_nbits;

	// get n bits without advancing
	unsigned int showbits (int n);
	// advance n bits
	void flushbits (int n, int *Err=NULL);
	// advance 32 bits
	void flushbits32 (int *Err=NULL);
	// get n bits + advance
	unsigned int getbits (int n, int *Err=NULL);
	// get 32bits + advance
	unsigned int getbits32 (int *Err=NULL);
	// align to the next byte
	void align (int *Err=NULL);
	// see how many bits have been processed
	long getnbits ();

	//  CYJ,2005-1-14 add
	//	get total write bits
	long GetTotalWriteBits(){ return m_nTotalWriteBits; }
	// put n bits
	void PutBits( register unsigned long dwData, int nBits );
	// put 1 bit
	void PutBit( unsigned char byData );
	// put bit 32
	void PutBits32( unsigned long dwData );	
	void PutBits16( unsigned short wData );	
	void PutBits8( unsigned char byData );	
	// flush write bits
	void FinishWrite();

private:
	bool SaveCacheByteToBuf();
};

#endif	// _BITSTRM_H

