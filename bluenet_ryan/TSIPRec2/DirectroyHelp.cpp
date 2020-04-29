// DirectroyHelp.cpp: implementation of the CDirectroyHelp class.
//
//////////////////////////////////////////////////////////////////////
//  CYJ,2005-8-1 修改Mkdir，增加缓冲区长度

#include "stdafx.h"
#include "DirectroyHelp.h"

#ifdef _WIN32
    #ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[]=__FILE__;
    #define new DEBUG_NEW
    #endif
#else
	#include <sys/stat.h>
    #include <sys/types.h>
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//	创建子目录
//	入口参数
//		pszDirectory						目录名
//  CYJ,2005-8-1 修改，增加缓冲区长度
void CDirectroyHelp::Mkdir(LPCSTR pszDirectroy)
{
	char szTmp[ _MAX_PATH*4 ];		//  CYJ,2005-8-1 修改，增加缓冲区长度
	strncpy( szTmp, pszDirectroy, sizeof(szTmp)-1 );	//  CYJ,2005-8-1 防止拷贝溢出
	szTmp[ sizeof(szTmp)-1 ] = 0;
	int nLen = strlen(szTmp);
	int nGrayNum = 0;						//	目录级数
	for(int i=0; i<nLen; i++)
	{
		if( szTmp[i] == '\\' || szTmp[i] == '/' )
		{									//	需要创建目录
			nGrayNum  ++;
			if( i == 0 )					//	如 "\TS\TMP"
				continue;
			if( nGrayNum  == 1 && szTmp[1] == ':' )
				continue;					//	如 "C:\TS\TMP"
			szTmp[i] = 0;
		#ifdef _WIN32
			::CreateDirectory( szTmp, NULL );
            szTmp[i] = '\\';
		#else
			mkdir( szTmp, S_IREAD|S_IWRITE|S_IEXEC|S_IRGRP|S_IWGRP|S_IROTH|S_IXGRP|S_IXOTH );
            szTmp[i] = '/';
		#endif //_WIN32
		}
	}
}
