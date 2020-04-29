/***********************************************************************************************
 *
 *	My Memory Block manager
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.8 @ Xi'an
 *
 *	@note
 *		For a large memory ( a large memory block or a mapped file ),
 *	splitted into serveral memory block with same size,
 *	manager the allocation / free of these memory blocks
 *
 ***********************************************************************************************/

#ifndef __MY_MEMORY_BLOCK_MANAGER_H_20150408__
#define __MY_MEMORY_BLOCK_MANAGER_H_20150408__

#include <stdint.h>
#include <MySyncObj.h>
#include <list>

#pragma pack(push,8)

class CMyMemoryBlockManager
{
public:
	CMyMemoryBlockManager();
	virtual ~CMyMemoryBlockManager();

	//--------------------------------------------------------------
	/** CYJ 2015-04-08
	 *
	 *	Initialize MMBM object
	 *
	 * @param [in]	pBuf					working memory area
	 * @param [in]	llBufSize				working memory size
	 * @param [in]	llHeadSize				Header size
	 * @param [in]	llBlockSize				block size
	 *
	 * @return		0						succ
	 *				other					error code
	 * @note
	 *		Head area is using for save the memory block allocation
	 *	When Invalidate is call, save the allocation data to the head area
	 */
	int Initialize( uint8_t *pBuf, int64_t llBufSize, int64_t llHeadSize, int64_t llBlockSize );

	//--------------------------------------------------------------
	/** CYJ 2015-04-08
	 *
	 *	Head area is using for save the memory block allocation
	 *	When Invalidate is call, save the allocation data to the head area
	 *
	 * @note
	 *		not destory data of the memory
	 */
	void Invalidate();

	//--------------------------------------------------------------
	/** CYJ 2015-04-08
	 *
	 *	Allocate one block
	 *
	 * @return		0						failed, no enough memory
	 *				other					memory block
	 */
	uint32_t Allocate();

	//--------------------------------------------------------------
	/** CYJ 2015-04-08
	 *
	 *	Free one block
	 *
	 * @param [in]	dwBlockIndex			memory block to be free
	 */
	void Free( uint32_t dwBlockIndex );

	//--------------------------------------------------------------
	/** CYJ 2015-04-08
	 *
	 *	Get memory block address by Block Index
	 *
	 * @param [in]	dwBlockIndex			block index
	 *
	 * @return		NULL					error block index
	 *				other					block address
	 */
	uint8_t * GetBuffer( uint32_t dwBlockIndex );

	//--------------------------------------------------------------
	/** CYJ 2015-04-09
	 *
	 *	Sync working data to head area
	 */
	void Flush();

	//------------------------------------------------------------------
	// Get Total Item count
	uint32_t GetTotalItemCount(){ return m_nBlockNumber; }
	uint32_t GetUsedItemCount(){ return m_dwUsedBlockCount; }
	uint32_t GetFreeItemCount(){ return m_dwFreeBlockCount; }

	enum{
		MMBM_INVALID_BLOCK_ID = 0,				// NULL
		MMBM_IN_USING_BLOCK_ID = 0xFFFFFFFF,	// in using
	};

	void DebugCheckUsedFreeItems( bool bDump = false );

protected:
	void SaveWorkingDataToHeadArea( bool bDoLock );
	int LoadWorkingDataFromHeadArea();
	void PresetWorkingData();

protected:
	uint8_t *	m_pBuf;					// saved memory pointer
	int64_t		m_llBufSize;			// buffer size
	int64_t		m_llHeadSize;			// header area buffer size
	int64_t		m_llBlockSize;			// block size

	uint32_t	m_nBlockNumber;			// block number
	uint8_t *	m_pFirstBlock;			// first block address

    uint32_t	m_dwFirstFreeBlock;
    uint32_t	m_dwUsedBlockCount;
    uint32_t	m_dwFreeBlockCount;
    uint32_t *	m_pdwBlockList;			// point to block info

	CMutex		m_SyncObj;				// sync object
};

#pragma pack(pop)

#endif // __MY_MEMORY_BLOCK_MANAGER_H_20150408__

