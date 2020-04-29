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

#ifndef __MY_BIT_RING_BUF_H_20120130__
#define __MY_BIT_RING_BUF_H_20120130__

// 2016.12.17 CYJ Add
#pragma pack(push,8)

class CMyBitRingBuf
{
public:
	CMyBitRingBuf();
	virtual ~CMyBitRingBuf();

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
	int Initialize( int nBitCount );

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Invalidate, and free resource
	 */
	void Invalidate();

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Get buffer size in Bytes
	 *
	 * @return		buffer size in bits
	 */
	const int GetBufferSize()const { return m_nBufSize; }

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Get bit count in cache buffer
	 *
	 * @return		bits count in cache buffer
	 *
	 * @note		the return value not include the bits in write cache register
	 */
	const int GetBitsInBuf()const;

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * get available cache buffer to putting bits
	 *
	 * @return		available bits
	 */
	const int GetAvailableBits()const;

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Flush write cache register to ring buffer by padding 0
	 */
	void Flush();

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Clean and reset all data
	 */
	void Reset();

	//--------------------------------------------------
	/** CYJ,2012-01-30
	 *
	 * Get ring data buffer
	 *
	 * @return	data buffer
	 */
	const unsigned char * GetDataBuffer(){ return m_pBuf;}

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
	void PutBits( unsigned int dwData, int nBits );
	void PutBits_1( unsigned int dwData ){ PutBits(dwData,1); }
	void PutBits_2( unsigned int dwData ){ PutBits(dwData,2); }
	void PutBits_8( unsigned int dwData ){ PutBits(dwData,8); }
	void PutBits_16( unsigned int dwData ){ PutBits(dwData,16); }
	void PutBits_32( unsigned int dwData ){ PutBits(dwData, 32); }

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
	unsigned int GetBits( int nBits );
	unsigned int GetBits_1(){ return GetBits(1); }
	unsigned int GetBits_2(){ return GetBits(2); }
	unsigned int GetBits_8(){ return GetBits(8); }
	unsigned int GetBits_16(){ return GetBits(16); }
	unsigned int GetBits_32(){ return GetBits(32); }

protected:
	int IncreasePtr( int nPtr ){ return (nPtr+1)%m_nBufSize; }

protected:
	unsigned char * m_pBuf;				// ring data buffer
	int m_nBufSize;							// buffer size
	int m_nReadPtr;							// read ptr
	int m_nWritePtr;						// write ptr

	unsigned int m_byReadRegister;			// read cache register
	int	m_nBitsInReadReg;

	unsigned char m_byWriteRegister;		// write cache register
	int m_nBitsInWriteReg;
};

#pragma pack(pop)

#endif // __MY_BIT_RING_BUF_H_20120130__



