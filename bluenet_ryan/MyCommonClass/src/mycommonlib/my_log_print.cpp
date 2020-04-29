// DebugLog.cpp: implementation of the CDebugLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <unistd.h>
#include <sched.h>


#include "my_log_print.h"
#include "MyTime.h"
#include "MyString.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static FILE *		s_fpLogFile = NULL;
static CMyString	s_strLogPath;
static char			s_szLogBuffer[ 1024 ];
static PFN_MYLOG_PRINT_LOG	s_pfnMyPrintLogCb = NULL;	// if not NULL, then call this callback function to print log to screen
static pthread_mutex_t	s_LogSyncObj = PTHREAD_MUTEX_INITIALIZER;
// the day when current log openning
// if the day is changed, close the log and zip to backup folder, and then open a new log
static int			s_nDayOfCurrentLogFile = 0;
static int			s_nYYYYMMDDOfCurrentFile = 0;		// year month and day
static bool		g_bSaveAllMsgToFile = true;

#define LOG_BACKUP_SUB_FOLDER	"log_backup"

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * Open log file with date information
 *
 * @param
 *
 * @return
 */
static int MyLog_OpenLogFile()
{
	CMyString strTmp;
	CTime tNow = CTime::GetCurrentTime();

	if( s_fpLogFile )
	{
		const int nCacheDataSize = 0x10000;	// 64 KB
		PBYTE pBuf = new BYTE[ nCacheDataSize ];

		strTmp.Format( "%s%s/logbak_%08d.gz", (char*)s_strLogPath, LOG_BACKUP_SUB_FOLDER, s_nYYYYMMDDOfCurrentFile );
		gzFile hOutFile = gzopen( (char*)strTmp, "wb9" );
		if( pBuf && Z_NULL != hOutFile )
		{
			fseek( s_fpLogFile, 0, SEEK_SET );
			while( 1 )
			{
				int nRetVal = fread( pBuf, 1, nCacheDataSize, s_fpLogFile );

				if( nRetVal > 0 )
					gzwrite( hOutFile, pBuf, nRetVal );

				if( nRetVal != nCacheDataSize )
					break;		// EOF
			}

			fclose( s_fpLogFile );
			s_fpLogFile = NULL;
			// remove

			strTmp.Format( "%s%08d.txt", (char*)s_strLogPath, s_nYYYYMMDDOfCurrentFile );
			unlink( (char*)strTmp );
		}

		if( pBuf )
			delete pBuf;

		if( Z_NULL != hOutFile )
			gzclose( hOutFile );

		if( s_fpLogFile )
			fclose( s_fpLogFile );
		s_fpLogFile = NULL;
	}
	s_fpLogFile = NULL;
	s_nDayOfCurrentLogFile = tNow.GetDay();
	s_nYYYYMMDDOfCurrentFile = tNow.GetYear() * 10000 + tNow.GetMonth() * 100 + s_nDayOfCurrentLogFile;

	strTmp.Format( "%s%08d.txt", (char*)s_strLogPath, s_nYYYYMMDDOfCurrentFile );

#ifdef _DEBUG
	fprintf( stderr, "Open Logfile %s\n", (char*)strTmp );
#endif //_DEBUG

	// 2012.11.30 CYJ Modify, "w+t" => "a+t"
	s_fpLogFile = fopen( (char*)strTmp, "a+t" );

	if( s_fpLogFile )
		return 0;

	fseek( s_fpLogFile, 0, SEEK_END );

	return errno;
}

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
int  MyLog_Init( const char * lpszLogFilePath, PFN_MYLOG_PRINT_LOG pfnPrintLog, bool bSaveAllToFile )
{
	s_strLogPath = lpszLogFilePath;
	s_pfnMyPrintLogCb = pfnPrintLog;
	g_bSaveAllMsgToFile = bSaveAllToFile;

	int nLen = s_strLogPath.GetLength();
	if( s_strLogPath[ nLen-1 ] != '/' )
		s_strLogPath += '/';

#ifdef _DEBUG
	fprintf( stderr, "LogPath=%s\n", (char*)s_strLogPath );
#endif //_DEBUG

	// create log backup folder, and this folder is some compressed log file
	CMyString strTmp = s_strLogPath;
	strTmp += LOG_BACKUP_SUB_FOLDER;
	mkdir( (char*)strTmp, S_IREAD|S_IWRITE|S_IEXEC|S_IRGRP|S_IWGRP|S_IROTH|S_IXGRP|S_IXOTH );

	int nRetVal = MyLog_OpenLogFile();
	if( 0 == nRetVal && s_fpLogFile )
	{
		CTime tNow = CTime::GetCurrentTime();
		fprintf( s_fpLogFile, "\n\n\n------------------- %04d.%02d.%02d %02d:%02d:%02d --------------------\n\n",\
				tNow.GetYear(), tNow.GetMonth(), tNow.GetDay(),\
				tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
	}

	return nRetVal;
}

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * print log
 *
 * @param [in]	nLevel			log level
 * @param [in]	lpszFmt			format
 *
 */
void MyLog_Printf(int nLevel, const char * lpszFmt, ... )
{
	va_list fmt_args;
    va_start( fmt_args, lpszFmt );     /* Initialize variable arguments. */
    int nLevelValue = nLevel&MY_LOG_LEVEL_VALUE_MASK;
    CTime tNow = CTime::GetCurrentTime();
    int nDayOfMonth = tNow.GetDay();
    int nFormatIsEndOfNewLine = strlen( lpszFmt );
    if( nFormatIsEndOfNewLine )
		nFormatIsEndOfNewLine = ( lpszFmt[ nFormatIsEndOfNewLine - 1 ] == '\n' ) ? 1 : 0;

	pthread_mutex_lock( &s_LogSyncObj );

	if( s_nDayOfCurrentLogFile != nDayOfMonth )
		MyLog_OpenLogFile();			// open a new log

	// ErrorLeve, yyyy.mm.dd HH:MM:SS
	sprintf( s_szLogBuffer, "%02X %04d.%02d.%02d %02d:%02d:%02d ",\
			nLevelValue,\
			tNow.GetYear(), tNow.GetMonth(), nDayOfMonth,\
			tNow.GetHour(), tNow.GetMinute(), tNow.GetSecond() );
	char * pszLogBuf = s_szLogBuffer + 23;

	vsnprintf( pszLogBuf, sizeof(s_szLogBuffer)-24, lpszFmt, fmt_args );
	va_end( fmt_args );

	if( s_fpLogFile )
	{
		if( g_bSaveAllMsgToFile || (nLevel&MY_LOG_LEVEL_WRITE_TO_FILE) || nLevelValue <= MY_LOG_LEVEL_ERROR )
		{
			fprintf( s_fpLogFile, s_szLogBuffer );
			if( 0 == nFormatIsEndOfNewLine )
				fprintf( s_fpLogFile, "\n" );
			fflush( s_fpLogFile );
		}
	}

	if( s_pfnMyPrintLogCb )
		s_pfnMyPrintLogCb( nLevel, s_szLogBuffer );
	else
	{
		if( nFormatIsEndOfNewLine )
			fprintf( stderr, "%s", s_szLogBuffer );
		else
			fprintf( stderr, "%s\n", s_szLogBuffer );
	}

	pthread_mutex_unlock( &s_LogSyncObj );
// 2015.4.1 CYJ Modify, replace pthread_yield with sched_yield
	sched_yield();
}

//--------------------------------------------------
/** CYJ,2011-07-01
 *
 * Close log file handle and release resource
 */
void MyLog_Exit()
{
	if( s_fpLogFile )
		fclose( s_fpLogFile );
	s_fpLogFile = NULL;
}

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Lock the Sync Object
 */
void MyLog_Lock()
{
	pthread_mutex_lock( &s_LogSyncObj );
}

//--------------------------------------------------------------
/** CYJ 2015-02-17
 *
 *	Unlock the Sync Object
 */
void MyLog_Unlock()
{
	pthread_mutex_unlock( &s_LogSyncObj );
}

