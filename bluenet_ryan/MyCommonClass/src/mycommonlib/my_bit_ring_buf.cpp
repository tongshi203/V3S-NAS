/**********************************************************************
 *
 *
 * My bit ring buffer object
 *
 * Chen Yongjian @ Tongshi
 * 2012.01.30 @ Xi'an
 *
 * @note
 *		These object is thread safe ( support multi-thread case ).
 * There is a ring buffer with a write pointer and a read pointer.
 * a 32 bits read cache register and a 32 bits write cache register,
 * so may cause a 32 bits delay.
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _DEBUG
	#include <assert.h>
#endif // _DEBUG

#include "my_bit_ring_buf.h"

CMyBitRingBuf::CMyBitRingBuf()
{
	m_pBuf = NULL;				// ring data buffer
	m_nBufSize = 0;				// buffer size
	m_nReadPtr = 0;				// read ptr
	m_nWritePtr = 0;			// write ptr

	m_byReadRegister = 0;		// read cache register
	m_nBitsInReadReg = 0;

	m_byWriteRegister = 0;		// write cache register
	m_nBitsInWriteReg = 0;
}

CMyBitRingBuf::~CMyBitRingBuf()
{
	Invalidate();
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Initialize bit ring buffer
 *
 * @param [in]	nBitCount		buffer size, in bits
 *
 * @return		0				succ
 *				other			error code
 */
int CMyBitRingBuf::Initialize( int nBitCount )
{
	Invalidate();

	m_nBufSize = ( nBitCount + 7 ) / 8;
	m_pBuf = (unsigned char * ) malloc( m_nBufSize );
	if( NULL == m_pBuf )
		return ENOMEM;

	m_nReadPtr = 0;				// read ptr
	m_nWritePtr = 0;			// write ptr

	m_byReadRegister = 0;		// read cache register
	m_nBitsInReadReg = 0;

	m_byWriteRegister = 0;		// write cache register
	m_nBitsInWriteReg = 0;

	return 0;
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Invalidate, and free resource
 */
void CMyBitRingBuf::Invalidate()
{
	if( m_pBuf )
	{
		free( m_pBuf );
		m_pBuf = NULL;
	}
	m_nBufSize = 0;
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Get bit count in cache buffer
 *
 * @return		bits count in cache buffer
 *
 * @note		the return value not include the bits in write cache register
 */
const int CMyBitRingBuf::GetBitsInBuf()const
{
	int nRetVal = ( m_nWritePtr + m_nBufSize - m_nReadPtr ) % m_nBufSize;
	return nRetVal * 8 + m_nBitsInReadReg;
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * get available cache buffer to putting bits
 *
 * @return		available bits
 */
const int CMyBitRingBuf::GetAvailableBits()const
{
	return ( m_nBufSize - 1 ) * 8 - GetBitsInBuf();
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Flush write cache register to ring buffer by padding 0
 *
 * @note
 *		after put bits, only 0 ~ 7 bits in the cache register
 */
void CMyBitRingBuf::Flush()
{
	if( 0 == m_nBitsInWriteReg )
		return;
	m_byWriteRegister <<= ( 8 - m_nBitsInWriteReg );
	m_pBuf[ m_nWritePtr ] = m_byWriteRegister;
	m_nWritePtr = IncreasePtr( m_nWritePtr );
	m_nBitsInWriteReg = 0;
	m_byWriteRegister = 0;
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Clean and reset all data
 */
void CMyBitRingBuf::Reset()
{
	m_nReadPtr = 0;				// read ptr
	m_nWritePtr = 0;			// write ptr

	m_byReadRegister = 0;		// read cache register
	m_nBitsInReadReg = 0;

	m_byWriteRegister = 0;		// write cache register
	m_nBitsInWriteReg = 0;
}

//  CYJ,2005-1-14 add
static unsigned int s_bitstream_bit_msk[33] =
{
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
  0x000000ff,
};

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Put bits to cache buffer
 *
 * @param [in]	dwData		data to put, low nBits to be put
 * @param [in]	nBits		bit count to put
 *							nBits should <= 16
 * @note
 *		to simplify operation, these functions do not check available buffer
 *	the caller should check the available bits buffer
 */
void CMyBitRingBuf::PutBits( unsigned int dwData, int nBits )
{
#ifdef _DEBUG
	assert( nBits <= 32 );
#endif //_DEBUG

	while( nBits > 0 )
	{
	#ifdef _DEBUG
		assert( m_nBitsInWriteReg < 8 );		
	#endif //_DEBUG

		int nBitsNeed = 8 - m_nBitsInWriteReg;
		if( nBitsNeed > nBits )
			nBitsNeed = nBits;

		unsigned char byTmp;
		if( nBits == nBitsNeed )
			byTmp = (unsigned char)( dwData & s_bitstream_bit_msk[ nBits ] );
		else
		{
			byTmp = (unsigned char)( dwData >> (nBits-nBitsNeed) );
			byTmp &= s_bitstream_bit_msk[ nBitsNeed ];
		}

		nBits -= nBitsNeed;
		m_nBitsInWriteReg += nBitsNeed;

		if( 8 == m_nBitsInWriteReg )
		{
			m_pBuf[ m_nWritePtr ] = m_byWriteRegister | byTmp;
			m_nWritePtr = IncreasePtr( m_nWritePtr );
			m_nBitsInWriteReg = 0;
			m_byWriteRegister = 0;
		#ifdef _DEBUG
			assert( m_nWritePtr < m_nBufSize );
		#endif // #ifdef _DEBUG
		}
		else
		{
		#ifdef _DEBUG
			assert( m_nBitsInWriteReg < 8 );
			assert( m_nWritePtr < m_nBufSize );
		#endif //_DEBUG
			m_byWriteRegister |= ( byTmp << (8-m_nBitsInWriteReg) );
		}
	}
}

//--------------------------------------------------
/** CYJ,2012-01-30
 *
 * Get bits from cache buffer
 *
 * @param [in]	nBits		bit count to get
 *
 * @return		N bits data
 * @note
 *		to simplify operation, these functions do not check available buffer
 *	the caller should check the data bits in buffer
 */
unsigned int CMyBitRingBuf::GetBits( int nBits )
{
	unsigned int dwRetVal = 0;

	while( nBits > 0 )
	{
		if( 0 == m_nBitsInReadReg )
		{	// reload
			while( nBits >= 8 )
			{	// copy bytes
				nBits -= 8;
				if( nBits )
					dwRetVal |= ( m_pBuf[ m_nReadPtr ] << nBits );
				else
					dwRetVal |= ( m_pBuf[ m_nReadPtr ] );
				m_nReadPtr = IncreasePtr( m_nReadPtr );
			}
			if( 0 == nBits )
				break;

			m_nBitsInReadReg = 8;
			m_byReadRegister = m_pBuf[ m_nReadPtr ];
			m_nReadPtr = IncreasePtr( m_nReadPtr );
		}

	#ifdef _DEBUG
		assert( m_nBitsInReadReg <= 8 );
	#endif //_DEBUG

		int nBitToCopy = ( m_nBitsInReadReg > nBits ) ? nBits : m_nBitsInReadReg;
		nBits -= nBitToCopy;
		m_nBitsInReadReg -= nBitToCopy;

		unsigned int dwTmp;
		if( m_nBitsInReadReg )
		{
			dwTmp = m_byReadRegister >> m_nBitsInReadReg;
			dwTmp &= s_bitstream_bit_msk[ nBitToCopy ];
		}
		else
			dwTmp = m_byReadRegister & s_bitstream_bit_msk[ nBitToCopy ];

		if( nBits )
			dwRetVal |= ( dwTmp << nBits );
		else
			dwRetVal |= dwTmp;
	}

	return dwRetVal;
}



