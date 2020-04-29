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


#include <stdafx.h>
#include <assert.h>
#include <errno.h>
#include <algorithm>

#include "my_memory_block_mgr.h"

///////////////////////////////////////////////////////////////////
//	8c45ed1e-202c-43f3-b859-769ba8f65fc1
static uint8_t	s_aMMBMTagID[]={ 0x8c,0x45,0xed,0x1e,0x20,0x2c,0x43,0xf3,0xb8,0x59,0x76,0x9b,0xa8,0xf6,0x5f,0xc1 };

#define COUNTER_OFFSET		16
#define BLOCKID_OFFSET		32

///////////////////////////////////////////////////////////////////
CMyMemoryBlockManager::CMyMemoryBlockManager()
{
	m_pBuf = NULL;				// saved memory pointer
	m_llBufSize = 0;			// buffer size
	m_llHeadSize = 0;			// header area buffer size
	m_llBlockSize = 0;			// block size

	m_nBlockNumber = 0;			// block number
	m_pFirstBlock = NULL;		// first block address

    m_dwFirstFreeBlock = 0;
    m_dwUsedBlockCount = 0;
    m_dwFreeBlockCount = 0;
    m_pdwBlockList = NULL;
}

CMyMemoryBlockManager::~CMyMemoryBlockManager()
{
	Invalidate();
}

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
int CMyMemoryBlockManager::Initialize( uint8_t *pBuf, int64_t llBufSize, int64_t llHeadSize, int64_t llBlockSize )
{
#ifdef _DEBUG
	assert( pBuf && llBufSize && llHeadSize && llBlockSize );
#endif //_DEBUG
	if( NULL == pBuf || 0 == llBufSize || 0 == llHeadSize || 0 == llBlockSize )
		return EINVAL;

	Invalidate();

	m_pBuf = pBuf;						// saved memory pointer
	m_llBufSize = llBufSize;			// buffer size
	m_llHeadSize = llHeadSize;			// header area buffer size
	m_llBlockSize = llBlockSize;		// block size

	m_dwFirstFreeBlock = MMBM_INVALID_BLOCK_ID;
    m_dwUsedBlockCount = 0;
    m_dwFreeBlockCount = 0;
    m_pdwBlockList = (uint32_t*)( m_pBuf + BLOCKID_OFFSET );	// point to block info

	// block number
	m_nBlockNumber = (uint32_t)( ( llBufSize - llHeadSize ) / llBlockSize );
	m_pFirstBlock = pBuf + llHeadSize;	// first block address

#ifdef _DEBUG
	assert( (int64_t)m_nBlockNumber*4 + 64 < llHeadSize );
#endif // _DEBUG

	if( (int64_t)m_nBlockNumber*4 + 64 >= llHeadSize )
		return EINVAL;

#ifdef _DEBUG
	fprintf( stderr, "BufSize: %lli, HeadSize:%lli, BlockSize:%lli\n", llBufSize, llHeadSize, llBlockSize );
	fprintf( stderr, "BlockNumber: %d\n", m_nBlockNumber );
#endif // _DEBUG

	return LoadWorkingDataFromHeadArea();
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Head area is using for save the memory block allocation
 *	When Invalidate is call, save the allocation data to the head area
 *
 * @note
 *		not destory data of the memory
 */
void CMyMemoryBlockManager::Invalidate()
{
	if( NULL == m_pBuf )
		return;

	SaveWorkingDataToHeadArea( true );

    m_dwFirstFreeBlock = 0;
    m_dwUsedBlockCount = 0;
    m_dwFreeBlockCount = 0;
    m_pdwBlockList = NULL;

    m_pBuf = NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Save working data to head area
 */
void CMyMemoryBlockManager::SaveWorkingDataToHeadArea( bool bDoLock )
{
	CSingleLock SyncObj( &m_SyncObj, bDoLock );

#ifdef _DEBUG
	assert( m_dwUsedBlockCount+m_dwFreeBlockCount <= m_nBlockNumber );
	DebugCheckUsedFreeItems( false );
#endif //_DEBUG

	uint32_t * pdwCounter = (uint32_t *)( m_pBuf + COUNTER_OFFSET );

	pdwCounter[0] = m_dwFirstFreeBlock;
	pdwCounter[1] = m_dwUsedBlockCount;
	pdwCounter[2] = m_dwFreeBlockCount;
	pdwCounter[3] = 0;	// reserved
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Load working data from Head area
 *
 * @return	0				succ
 *			other			error code
 * @note
 *		head area definition
 *	   0:		TagID					16 bytes
 *	0x10:		UsedItemCount			4
 *	0x14:		FreeItemCount			4
 *	0x18:		Reserved				8
 *	0x20:		aUsedItemID				UsedItemCount * 8
 *	...:		aFreeItemID				FreeItemCount * 8
 */
int CMyMemoryBlockManager::LoadWorkingDataFromHeadArea()
{	// check TagID
	CSingleLock SyncObj( &m_SyncObj, true );

	if( memcmp( s_aMMBMTagID, m_pBuf, 16 ) )
	{
		PresetWorkingData();
		return 0;
	}

	uint32_t * pdwCounter = (uint32_t *)( m_pBuf + COUNTER_OFFSET );

	m_dwFirstFreeBlock = pdwCounter[0];
	m_dwUsedBlockCount = pdwCounter[1];
	m_dwFreeBlockCount = pdwCounter[2];

#ifdef _DEBUG
	assert( pdwCounter[3] == 0 );
	assert( m_dwFirstFreeBlock );
	assert( m_dwUsedBlockCount+m_dwFreeBlockCount <= m_nBlockNumber );
#endif // _DEBUG

	if( MMBM_INVALID_BLOCK_ID == m_dwFirstFreeBlock )
	{	// error data
		PresetWorkingData();
		return 0;
	}
	if( m_dwUsedBlockCount+m_dwFreeBlockCount > m_nBlockNumber )
	{
		PresetWorkingData();
		return 0;
	}

	// buffer is enlarged
	if( m_dwUsedBlockCount+m_dwFreeBlockCount < m_nBlockNumber )
	{
		// 0 is reserved for NULL
		for( uint32_t i=m_dwUsedBlockCount+m_dwFreeBlockCount+1; i<m_nBlockNumber; i++ )
		{
			m_pdwBlockList[i] = i+1;	// m_pdwBlockList[0] is reserved
		}
		m_pdwBlockList[m_nBlockNumber] = m_dwFirstFreeBlock;		// the last one
		m_dwFirstFreeBlock = m_dwUsedBlockCount+m_dwFreeBlockCount+1;
	}

#ifdef _DEBUG
	DebugCheckUsedFreeItems( false );
#endif //_DEBUG

	return 0;
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Preset working data
 */
void CMyMemoryBlockManager::PresetWorkingData()
{
	memcpy( m_pBuf, s_aMMBMTagID, sizeof(s_aMMBMTagID) );

    m_dwFirstFreeBlock = 1;		// first block
    m_dwUsedBlockCount = 0;
    m_dwFreeBlockCount = 0;

	// 0 is reserved for NULL
	m_pdwBlockList[0] = MMBM_INVALID_BLOCK_ID;					// m_pdwBlockList[0] is reserved
	for( uint32_t i=1; i<m_nBlockNumber; i++ )
	{
		m_pdwBlockList[i] = i+1;	// point to next
	}
	m_pdwBlockList[m_nBlockNumber] = MMBM_INVALID_BLOCK_ID;		// the last one
	m_dwFreeBlockCount = m_nBlockNumber;

	SaveWorkingDataToHeadArea( false );
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Allocate one block
 *
 * @return		0						failed, no enough memory
 *				other					memory block
 */
uint32_t CMyMemoryBlockManager::Allocate()
{
	CSingleLock SyncObj( &m_SyncObj, true );

#ifdef _DEBUG
	assert( m_dwUsedBlockCount + m_dwFreeBlockCount == m_nBlockNumber );
#endif //_DEBUG

	if( MMBM_INVALID_BLOCK_ID == m_dwFirstFreeBlock )
	{
	#ifdef _DEBUG
		assert( 0 == m_dwFreeBlockCount );
	#endif // _DEBUG
		return MMBM_INVALID_BLOCK_ID;
	}

	uint32_t dwRetVal = m_dwFirstFreeBlock;		// set return value

	// get next block
	m_dwFirstFreeBlock = m_pdwBlockList[ m_dwFirstFreeBlock ];
	m_dwFreeBlockCount --;
	m_dwUsedBlockCount ++;

    // set in using
	m_pdwBlockList[ dwRetVal ] = MMBM_IN_USING_BLOCK_ID;

	SaveWorkingDataToHeadArea( false );

#ifdef _DEBUG
	assert( dwRetVal );
	assert( dwRetVal <= m_nBlockNumber );
	assert( m_dwFirstFreeBlock <= m_nBlockNumber );
#endif // _DEBUG

	return dwRetVal;
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Free one block
 *
 * @param [in]	dwBlockIndex			memory block to be free
 */
void CMyMemoryBlockManager::Free( uint32_t dwBlockIndex )
{
	CSingleLock SyncObj( &m_SyncObj, true );

#ifdef _DEBUG
	assert( MMBM_INVALID_BLOCK_ID != dwBlockIndex );
	assert( dwBlockIndex <= m_nBlockNumber );
	assert( m_dwUsedBlockCount + m_dwFreeBlockCount == m_nBlockNumber );
	assert( m_pdwBlockList[ dwBlockIndex ] == MMBM_IN_USING_BLOCK_ID );
	if( MMBM_INVALID_BLOCK_ID == m_dwFirstFreeBlock )
		assert( 0 == m_dwFreeBlockCount );
#endif //_DEBUG

	if( MMBM_INVALID_BLOCK_ID == dwBlockIndex || dwBlockIndex > m_nBlockNumber )
		return;

	m_pdwBlockList[ dwBlockIndex ] = m_dwFirstFreeBlock;
	m_dwFirstFreeBlock = dwBlockIndex;

	m_dwUsedBlockCount --;
	m_dwFreeBlockCount ++;

	SaveWorkingDataToHeadArea( false );
}

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
uint8_t * CMyMemoryBlockManager::GetBuffer( uint32_t dwBlockIndex )
{
#ifdef _DEBUG
	assert( MMBM_INVALID_BLOCK_ID != dwBlockIndex );
	assert( dwBlockIndex <= m_nBlockNumber );
	assert( m_dwUsedBlockCount + m_dwFreeBlockCount == m_nBlockNumber );
#endif //_DEBUG

	if( MMBM_INVALID_BLOCK_ID == dwBlockIndex || dwBlockIndex > m_nBlockNumber )
		return NULL;

	dwBlockIndex --;
    uint64_t llOffset = dwBlockIndex;
    llOffset *= m_llBlockSize;

    return m_pFirstBlock + llOffset;
}

//--------------------------------------------------------------
/** CYJ 2015-04-08
 *
 *	Debug, test used and free items
 */
void CMyMemoryBlockManager::DebugCheckUsedFreeItems( bool bDump )
{
#ifdef _DEBUG

	assert( m_dwUsedBlockCount + m_dwFreeBlockCount == m_nBlockNumber );
	assert( m_nBlockNumber );

	uint8_t * pBuf = new uint8_t[ m_nBlockNumber + 1 ];
	memset( pBuf, 0, m_nBlockNumber + 1 );

	uint32_t i;
	uint32_t dwBlockID;

	// save used item block ID
	int nUsedItemCount = 0;
	for( i=1; i<=m_nBlockNumber; i++ )
	{
		if( m_pdwBlockList[i] != MMBM_IN_USING_BLOCK_ID )
			continue;
		pBuf[ i ] = 1;
		nUsedItemCount ++;
	}
	assert( nUsedItemCount == m_dwUsedBlockCount );

	// save free item block ID
	dwBlockID = m_dwFirstFreeBlock;
	if( m_dwFreeBlockCount )
		assert( dwBlockID );
	int nFreeCount = 0;
	while( dwBlockID != MMBM_INVALID_BLOCK_ID )
	{
		assert( 0 == pBuf[ dwBlockID ] );
		assert( dwBlockID <= m_nBlockNumber );
		pBuf[ dwBlockID ] = 1;
		dwBlockID = m_pdwBlockList[ dwBlockID ];
		nFreeCount ++;
	}
	assert( nFreeCount == m_dwFreeBlockCount );
	assert( MMBM_INVALID_BLOCK_ID == dwBlockID );

	assert( 0 == pBuf[0] );
	for( uint32_t i=1; i<m_nBlockNumber; i++ )
	{
		assert( pBuf[i] );
	}

	if( bDump )
	{
		fprintf( stderr, "\n=================< BlockMgr >======================\n" );
		fprintf( stderr, "\n\nFirst Free Item: 0x%08x, %d in used, %d free\n", m_dwFirstFreeBlock, m_dwUsedBlockCount, m_dwFreeBlockCount );

		int j = 0;
		for(i=0; i<=m_nBlockNumber; i++ )
		{
			fprintf( stderr, "0x%08x, ", m_pdwBlockList[i] );
			j ++;
			if( j >= 16 )
			{
				fprintf( stderr, "\n" );
				j=0;
			}
		}
		fprintf( stderr, "\nFree List:\n" );
		dwBlockID = m_dwFirstFreeBlock;
		j=0;
		while( dwBlockID != MMBM_INVALID_BLOCK_ID )
		{
			fprintf( stderr, "0x%08x, ", dwBlockID );
			dwBlockID = m_pdwBlockList[ dwBlockID ];
			j ++;
			if( j >= 16 )
			{
				fprintf( stderr, "\n" );
				j=0;
			}
		}
		fprintf( stderr, "\n" );
	}

	delete pBuf;
#endif // _DEBUG
}

//--------------------------------------------------------------
/** CYJ 2015-04-09
 *
 *	Sync working data to head area
 */
void CMyMemoryBlockManager::Flush()
{
	SaveWorkingDataToHeadArea( true );
}

