// BScriptEngine.cpp: implementation of the CBScriptEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "bscript.h"
#include <stdio.h>
#include <string.h>
#include "blib.h"
#include <stdlib.h> 
#include "BScriptEngine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static void DeleteOneFunction( bpointer pExtFuncObj )
{
	if( NULL == pExtFuncObj )
		return;
	((CBSExternFunctionTemplete *)pExtFuncObj)->Release();
}


class CMyHashTable
{
public:
	CMyHashTable()
	{
		m_hTable = bhash_new();
	}
	~CMyHashTable()
	{
		if( m_hTable )
			bhash_free_destruct( m_hTable, DeleteOneFunction );
	}
	void Register( char * pszFunctionName, CBSExternFunctionTemplete * pObj )
	{
		bhash_insert( m_hTable, pszFunctionName, pObj );
		pObj->AddRef();
	}
	void Deregister( char * pszFunctionName )
	{
		bhash_remove( m_hTable, pszFunctionName, DeleteOneFunction );
	}
	CBSExternFunctionTemplete * operator[](char * pszFunctionName)
	{
		// assume pszFunctionName is upper case
		return (CBSExternFunctionTemplete * )bhash_lookup( m_hTable, pszFunctionName );
	}
private:
	BHash *	m_hTable;
};

static CMyHashTable		s_ExternFuncTbl;


CBSAutoReleaseString::CBSAutoReleaseString()
{
	m_pString = 0;
}

CBSAutoReleaseString::~CBSAutoReleaseString()
{
	if( m_pString )
		free( (void*)m_pString );
	m_pString = 0;
}

CBSAutoReleaseString::CBSAutoReleaseString(const CBSAutoReleaseString & strText)
{
	if( strText.m_pString )
		m_pString = strdup( strText.m_pString );
	else
		m_pString = 0;
}

CBSAutoReleaseString::CBSAutoReleaseString(const char * pszText )
{
	if( pszText )
		m_pString = strdup( pszText );
	else
		m_pString = 0;
}

CBSAutoReleaseString & CBSAutoReleaseString::operator = (const char * pszText )
{
	if( m_pString )
		free( (void*)m_pString );
	m_pString = 0;

	if( pszText )
		m_pString = strdup( pszText );
	return *this;
}

CBSAutoReleaseString & CBSAutoReleaseString::operator = (const CBSAutoReleaseString & strText )
{
	return operator=( strText.m_pString );	
}


CBSExternFunctionTemplete::CBSExternFunctionTemplete( BSModule * pModule, const char * lpszFunctionName )
{
	m_pModule = pModule;
	m_pszFunctionName = strdup( lpszFunctionName );
	b_strupper( m_pszFunctionName );

	m_nRef = 0;
	s_ExternFuncTbl.Register( (char*)m_pszFunctionName, this );
	m_pArgList = 0;
	m_pContext = 0;
	m_pRetVal = 0;

	bhash_insert( pModule->builtin_functions, m_pszFunctionName, FunctionCallBackLink );
}

CBSExternFunctionTemplete::~CBSExternFunctionTemplete()
{
	if( m_pszFunctionName )
	{
		s_ExternFuncTbl.Deregister( m_pszFunctionName );
		free( (void*)m_pszFunctionName );
	}
}

long CBSExternFunctionTemplete::AddRef()
{
	m_nRef ++;
	return m_nRef;
}

long CBSExternFunctionTemplete::Release()
{
	m_nRef --;
	if( m_nRef )
		return m_nRef;
	delete this;
	return 0;
}

BVariant* CBSExternFunctionTemplete::FunctionCallBackLink( BSContext *context, BList *args )
{
	CBSExternFunctionTemplete * pObj = s_ExternFuncTbl[context->m_pBuildinFunctionName];
	if( NULL == pObj )
		return 0;
	pObj->m_pRetVal = (BVariant*)0;
	pObj->m_pContext = context;
	pObj->m_pArgList = args;

	pObj->OnFunctionCall();

	BVariant* pRetVal = pObj->m_pRetVal;
	pObj->m_pRetVal = (BVariant*)0;
	return pRetVal;
}

int CBSExternFunctionTemplete::GetArgCount()
{
	if( NULL == m_pArgList )
		return 0;
	return blist_length( m_pArgList );
}

bool CBSExternFunctionTemplete::IsArgTypeString( int nIndex )
{	
	BVariant* pArg = (BVariant*) blist_nth_data( m_pArgList, nIndex );
	if( NULL == pArg )
		return false;
	return ( pArg->type == BV_STRING );
}

bool CBSExternFunctionTemplete::GetArgAsBool( int nIndex )
{
	/* get the arguments */
	BVariant * pArg = bvariant_copy( (BVariant*) blist_nth_data( m_pArgList, nIndex ) );
	bvariant_cast( pArg, BV_BOOLEAN );
	bool bRetVal = bvariant_as_boolean( pArg ) ? true : false;
	bvariant_free( pArg );
	return bRetVal;
}

int CBSExternFunctionTemplete::GetArgAsInt( int nIndex )
{
	/* get the arguments */
	BVariant * pArg = bvariant_copy( (BVariant*) blist_nth_data( m_pArgList, nIndex ) );
	bvariant_cast( pArg, BV_INTEGER );
	int nRetVal = bvariant_as_integer( pArg );
	bvariant_free( pArg );
	return nRetVal;
}

double CBSExternFunctionTemplete::GetArgAsDouble( int nIndex )
{
	/* get the arguments */
	BVariant * pArg = bvariant_copy( (BVariant*) blist_nth_data( m_pArgList, nIndex ) );
	bvariant_cast( pArg, BV_DOUBLE );
	double fRetVal = bvariant_as_double( pArg );
	bvariant_free( pArg );
	return fRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-4-28
/// 函数功能:
///		
/// 输入参数:
///		无
/// 返回参数:
///		无
/// 说明：
///		调用者需要释放对应的字符串
CBSAutoReleaseString CBSExternFunctionTemplete::GetArgAsString( int nIndex )
{
	/* get the arguments */
	BVariant * pArg = bvariant_copy( (BVariant*) blist_nth_data( m_pArgList, nIndex ) );
	bvariant_cast( pArg, BV_STRING );
	CBSAutoReleaseString strRetVal; 
	strRetVal = bvariant_as_string( pArg );
	bvariant_free( pArg );
	return strRetVal;
}

void * CBSExternFunctionTemplete::GetArgAsPointer( int nIndex )
{
	/* get the arguments */
	BVariant * pArg = bvariant_copy( (BVariant*) blist_nth_data( m_pArgList, nIndex ) );
	bvariant_cast( pArg, BV_POINTER );
	bpointer pRetVal = bvariant_as_pointer( pArg );
	bvariant_free( pArg );
	return pRetVal;
}

void CBSExternFunctionTemplete::SetRetValue( int nValue )
{
	if( m_pRetVal )
		bvariant_free( m_pRetVal );
	m_pRetVal = bvariant_new();
// 目前不支持整数，所以只有设置为浮点数
//	bvariant_set_integer( m_pRetVal, nValue );
	bvariant_set_double( m_pRetVal, (double)nValue );
}

void CBSExternFunctionTemplete::SetRetValue( double fValue )
{
	if( m_pRetVal )
		bvariant_free( m_pRetVal );
	m_pRetVal = bvariant_new();
	bvariant_set_double( m_pRetVal, fValue );
}

void CBSExternFunctionTemplete::SetRetValue( const char * pszValue )
{
	if( m_pRetVal )
		bvariant_free( m_pRetVal );
	m_pRetVal = bvariant_new();
	bvariant_set_string( m_pRetVal, (char*)pszValue );
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBScriptEngine::CBScriptEngine()
{
	m_pEngine = bs_interpreter_create();
}

CBScriptEngine::~CBScriptEngine()
{
	if( m_pEngine )
		bs_interpreter_destroy( m_pEngine );
}

bool CBScriptEngine::IsValid()
{
	return ( m_pEngine != NULL );
}

static void my_printf_link( void * pData, int bIsString, void * pContext )
{
	if( NULL == pContext )
		return;
	CBScriptEngine * pEngine = (CBScriptEngine*)pContext;
	pEngine->Print( pData, bIsString==1 );
}

///-------------------------------------------------------
/// CYJ,2005-4-27
/// 函数功能:
///		run
/// 输入参数:
///		pszCode			script code
/// 返回参数:
///		无
void CBScriptEngine::Run( char * pszCode )
{
	if( !IsValid() || NULL == pszCode || 0 == *pszCode )
		return;
	BSModule *module;
	module = bs_module_create( "bscript" );
	module->m_pfnPrint = my_printf_link;
	module->m_pPrintContext = this;
	OnModuleCreated( module );
	bs_module_set_code( module, pszCode );
	bs_module_execute( module );
	OnDeleteModule( module );
	bs_module_destroy( module );
}

///-------------------------------------------------------
/// CYJ,2005-4-27
/// 函数功能:
///		run from file
/// 输入参数:
///		pszFileName		run file
/// 返回参数:
///		无
void CBScriptEngine::RunFile( char * pszFileName )
{
	if( !IsValid() || NULL == pszFileName || 0 == *pszFileName )
		return;
	BSModule *module;
	module = bs_module_create( "bscript" );
	module->m_pfnPrint = my_printf_link;
	module->m_pPrintContext = this;
	OnModuleCreated( module );
	bs_module_load_file( module, pszFileName );
	bs_module_execute( module );
	OnDeleteModule( module );
	bs_module_destroy( module );
}

void CBScriptEngine::Print( void * pData, bool bIsString )
{
	if( bIsString )
		printf( (char*) pData );
	else
		printf( "%g", *(double*)pData );
}

///-------------------------------------------------------
/// CYJ,2005-4-28
/// 函数功能:
///		On module created
/// 输入参数:
///		无
/// 返回参数:
///		无
void CBScriptEngine::OnModuleCreated(BSModule *pModule)
{
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		Module 即将被删除
/// 输入参数:
///		无
/// 返回参数:
///		无
void CBScriptEngine::OnDeleteModule( BSModule * pModule )
{
}
