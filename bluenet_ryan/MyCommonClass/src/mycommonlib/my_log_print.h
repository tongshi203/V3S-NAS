// DebugLog.h: interface for the CDebugLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGLOG_H__8DE6BE38_CCFA_4AB8_B969_8E992A4A53B6__INCLUDED_)
#define AFX_DEBUGLOG_H__8DE6BE38_CCFA_4AB8_B969_8E992A4A53B6__INCLUDED_

#include <stdio.h>
#include <stdarg.h>

// 2016.12.17 CYJ Add
#pragma pack(push,8)

//------------------------------------------------
// Log level, using bit 0 ~ 7
#define MY_LOG_LEVEL_MESSAGE		4
#define MY_LOG_LEVEL_WARNING		3
#define MY_LOG_LEVEL_ERROR			2
#define MY_LOG_LEVEL_FATAL_ERROR	1

#define MY_LOG_LEVEL_VALUE_MASK		0xFF
//--------------------------------------------
// log flag, indicate this is a debug information
#define MY_LOG_LEVEL_DEBUG			0x100
#define MY_LOG_LEVEL_DEBUG_MSG		(MY_LOG_LEVEL_DEBUG|MY_LOG_LEVEL_MESSAGE)
// for ERROR and FATAL_ERROR, message will save to file automatic
// for WARNING and MESSAGE, will not save to file, unless WRITE_TO_FILE flag is set
#define MY_LOG_LEVEL_WRITE_TO_FILE	0x200

//////////////////////////////////////////////////////////////////////////////
typedef void (* PFN_MYLOG_PRINT_LOG)( int nLevel, const char * lpszLogText );

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * Initialize MyLog env
 *
 * @param [in]	lpszLogFilePath		log save folder path
 *
 * @return
 *		0		succ
 *		other	error code
 * @note
 *		the folder must be created before calling
 */
int  MyLog_Init( const char * lpszLogFilePath, PFN_MYLOG_PRINT_LOG pfnPrintLog = NULL, bool bSaveAllToFile=true );

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * print log
 *
 * @param [in]	nLevel			log level
 * @param [in]	lpszFmt			format
 *
 */
void MyLog_Printf(int nLevel, const char * lpszFmt, ... );

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * Close log file handle and release resource
 */
void MyLog_Exit();

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Lock the Sync Object
 */
void MyLog_Lock();

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Unlock the Sync Object
 */
void MyLog_Unlock();

/////////////////////////////////////////////////////
class CMyLogSyncHelper
{
public:
	CMyLogSyncHelper()
	{
		MyLog_Lock();
	}

	virtual ~CMyLogSyncHelper()
	{
		MyLog_Unlock();
	}
};

// 2016.12.17 CYJ Add
#pragma pack(pop)

#endif // !defined(AFX_DEBUGLOG_H__8DE6BE38_CCFA_4AB8_B969_8E992A4A53B6__INCLUDED_)











