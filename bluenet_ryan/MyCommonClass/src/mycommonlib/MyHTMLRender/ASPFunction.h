///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-5-12
///
///		用途：
///			ASP 外部函数
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __MYHTML_ASP_EXT_FUNCTION_20050512__
#define __MYHTML_ASP_EXT_FUNCTION_20050512__

#include <BScript/BScriptEngine.h>
class CHTMLParser;

typedef void (*PFN_OnModuleCreateDelete)( BSModule * pModule, bool bIsCreate, DWORD dwUserData );

extern "C" void MyHTMLRegisterModuleCallBack( CHTMLParser * pParser, PFN_OnModuleCreateDelete pFn, DWORD dwUserData );

#endif	// __MYHTML_ASP_EXT_FUNCTION_20050512__
