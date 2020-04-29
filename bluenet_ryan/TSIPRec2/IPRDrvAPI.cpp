///---------------------------------------------------------
///
///      Chen Yongjian @ Xi'an Tongshi Technology Limited
///				2004.4.8
///      This file is implemented:
///				IPR Driver API implement
///-----------------------------------------------------------

#include "stdafx.h"
#include "DVBFileReceiver.h"
#include "IPRecSvr.h"
#include <stdio.h>

#include "DecoderThread.h"
#include "UDPRecThread.h"
#include "FileWriterThread.h"

#include "DirectroyHelp.h"
#include <time.h>

extern CDecoderThread		g_DecoderThread;
extern CUDPRecThread 		g_UDPRecThread;
extern CFileWriterThread	g_FileWriterThread;
static long g_nIPRD_InitTimes = 0;


///------------------------------------------
/// Function:
///		Initialize the IPR driver, must call before any operate on the IPRD
/// Input Parameter:
///		None
/// Output Parameter:
///		true			succ
///		false			failed
#ifdef _WIN32
	extern "C" bool WINAPI IPRD_Init(void)
#else
	bool IPRD_Init(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( ::InterlockedIncrement( &g_nIPRD_InitTimes ) > 1 )
		return true;			//  2004-5-18 has initialized

#ifdef _DEBUG
    TRACE("IPRD_Init is called.\n");
#endif //_DEBUG

	if( FALSE == g_DecoderThread.CreateThread() )
    	return false;
    if( FALSE == g_UDPRecThread.CreateThread() )
    	return false;
	if( FALSE == g_FileWriterThread.CreateThread() )
		return false;

#ifdef _DEBUG
   TRACE("Create thread succ.\n");
#endif //_DEBUG
    return true;
}

///------------------------------------------
/// Function:
///		Notify the IPR driver that the application will exit
///		the driver should free all resources
/// Input Parameter:
///		None
/// Output Parameter:
///		None
#ifdef _WIN32
	extern "C" void WINAPI IPRD_Close(void)
#else
	void IPRD_Close(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( InterlockedDecrement( &g_nIPRD_InitTimes) )
		return;

	g_DecoderThread.StopThread();
    g_UDPRecThread.StopThread();
	g_FileWriterThread.StopThread();

}

