/**************************************************************************************
 *
 *	Mapped file
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.7 @ Xi'an
 *
 **************************************************************************************/

#ifndef __MY_MAPPED_FILE_H_20150407__
#define __MY_MAPPED_FILE_H_20150407__

#include <stdint.h>

#pragma pack(push,8)

class CMyMappedFile
{
public:
	CMyMappedFile();
	virtual ~CMyMappedFile();

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Open file name and map it
	 *
	 * @param [in]	pszFileName			data file name
	 * @param [in]	llFileSizeNeed		file size needed
	 * @param [in]	bReadOnly			read only
	 * @param [in]	dwCreateMode		if file not exist, create mode, default is S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH
	 *
	 * @return		0					succ
	 *				other				error code
	 * @note
	 *	1. If file not exist, create it
	 *	2. If file size if small than llFileSizeNeed, file it with 0
	 *	3. If file size if large then llFileSizeNeed, only [ 0, llFileSizeNeed) is mapped
	 */
	int Open( const char *pszFileName, int64_t llFileSizeNeed, bool bReadOnly=false, uint32_t dwCreateMode = 0 );

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Unmap and close the file
	 */
	void Close();

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Sync data from memory to file.
	 *
	 * @param [in]	llOffset			offset
	 * @param [in]	llLength			length to be commit
	 *
	 * @return		0					succ
	 *				other				error code
	 */
	int SyncToFile( int64_t llOffset, int64_t llLength );

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Get Mapped memory address
	 *
	 * @param [in]	llOffset			offset
	 *
	 * @return		NULL				failed
	 *				other				mapped address
	 */
	uint8_t * GetBuffer( int64_t llOffset );

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Is Read Only
	 */
	bool IsReadOnly()const{ return m_bIsReadOnly; }

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Get file handle
	 */
	int	GetFileHandle()const { return m_hFile; }

	//--------------------------------------------------------------
	/** CYJ 2015-04-07
	 *
	 *	Get file length
	 */
	int64_t GetFileLength() const { return m_llFileLength; }

protected:
	int PrepareFile( const char *pszFileName, int64_t llFileSizeNeed, uint32_t dwCreateMode );

protected:
	int 		m_hFile;
	bool		m_bIsReadOnly;
	int64_t		m_llFileLength;
	uint8_t * 	m_pBufferMapped;
};


#pragma pack(pop)

#endif // __MY_MAPPED_FILE_H_20150407__

