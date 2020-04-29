// MyAspSite.cpp: implementation of the CMyResponse class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyAspSite.h"

#include <BasicEngine/basic_interface.h>
#include <MyArray.h>
#include <MyString.h>
#include "MyRequest.h"

#if defined(_DEBUG) && defined(_WIN32)
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

class CMyResponse : public cobject
{
	friend class CMyAspSite;
public:
	CMyResponse(CASPInOutObject * pOutputObject);
	virtual ~CMyResponse();

	void Reset();

	// overridables -----------------------

	virtual LPCSTR get_name();
	virtual void	you_are(int ndimensions = 0, variable* indexes = NULL);
	virtual int		get_dimensions();

private:
// properties/subs/functions ----------

//	void		on_set_some_prop(variable val);
//	variable	on_get_some_prop();

//	void		on_call_some_function(int nargs, variable* pargs, variable& result);

	void		on_call_Write_sub(int nargs, variable* pargs);
	void		on_call_WriteBlock_sub(int nargs, variable* pargs);

// macros -----------------------------

	DECLARE_PROP_MAP()		// if your class has properties
	DECLARE_FUNCTION_MAP()	// if your calss has functions
	DECLARE_SUB_MAP()		// if your calss has subs

	virtual void SafeDelete();

private:
	CMyArray< CMyString >	m_astrBlock;
	CASPInOutObject * m_pOutputObject;
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyResponse::CMyResponse(CASPInOutObject * pOutputObject)
{
	ASSERT( pOutputObject );
	m_pOutputObject = pOutputObject;
}

CMyResponse::~CMyResponse()
{

}

void CMyResponse::SafeDelete()
{
	delete this;
}

void CMyResponse::Reset()
{
	ASSERT( m_pOutputObject );
	m_astrBlock.RemoveAll();
}

// maps -----------------------------------------------------------------------

BEGIN_PROP_MAP(CMyResponse)
//	Usage :
//	PROPERTY(property_name, get_function, set_function)

//	Sample:
//	PROPERTY("SomeProperty", on_get_some_property, on_set_some_property)
END_PROP_MAP

BEGIN_FUNCTION_MAP(CMyResponse)
//	Usage :
//	FUNCTION(function_name, argument_count, implementation_function)

//	Sample :
//	FUNCTION("SomeFunction", 0, on_call_some_function)
END_FUNCTION_MAP

BEGIN_SUB_MAP(CMyResponse)
//	Usage :
//	FUNCTION(sub_name, argument_count, implementation_function)
	SUB( "Write", 1, &CMyResponse::on_call_Write_sub )
	SUB( "WriteBlock", 1, &CMyResponse::on_call_WriteBlock_sub )
END_SUB_MAP

// implementation -------------------------------------------------------------

void CMyResponse::you_are(int ndimensions, variable* indexes)
{
}

LPCSTR CMyResponse::get_name()
{
	return "Response";
}

int CMyResponse::get_dimensions()
{
	return 0;
}

///-------------------------------------------------------
/// CYJ,2004-7-8
/// Function:
///		Write
/// Input parameter:
///		None
/// Output parameter:
///		None
void CMyResponse::on_call_Write_sub(int nargs, variable* pargs)
{
	ASSERT( nargs == 1 && pargs && m_pOutputObject );
	if( nargs != 1 || NULL == pargs || NULL == m_pOutputObject )
		return;
	LPCSTR lpszText = (LPCSTR)pargs[0];
	m_pOutputObject->Write( (void*)lpszText, strlen(lpszText) );
}

///-------------------------------------------------------
/// CYJ,2004-7-8
/// Function:
///		WriteBlock
/// Input parameter:
///		nBlockNo
/// Output parameter:
///		None
void CMyResponse::on_call_WriteBlock_sub(int nargs, variable* pargs)
{
	ASSERT( nargs == 1 && pargs && m_pOutputObject );
	if( nargs != 1 || NULL == pargs || NULL == m_pOutputObject )
		return;
	int nBlock = (int)pargs[0];
	ASSERT( nBlock >= 0 && nBlock < m_astrBlock.GetSize() );
	if( nBlock < 0 || nBlock >= m_astrBlock.GetSize() )
		return;
	CMyString & strBlock = m_astrBlock[nBlock];
	m_pOutputObject->Write( (void*)(LPCSTR)strBlock, strBlock.GetLength() );
}

//////////////////////////////////////////////////////////////////
///		CAspSite
//////////////////////////////////////////////////////////////////
CMyAspSite::CMyAspSite(CASPInOutObject * pOutObject)
{
	ASSERT( pOutObject );
	m_pResponseObj = new CMyResponse( pOutObject );
	m_pEngine	= CreateBasicEngine();
	m_pOutputObject = pOutObject;
	m_pstrLastError = new CMyString;
	m_pSplitedAspPage = new CMyString;
	m_pRequest = new CMyRequest;

	if( m_pEngine )
	{
		m_pEngine->get_root()->add_child( (cobject*)m_pResponseObj );
		if( m_pRequest )
		{
			PEXTENSION_FUNCTION_FUNCTION pfnTmp = m_pRequest->GetRequestExportFunction();
			if( pfnTmp )
				m_pEngine->add_extension_function( "Request", 1, pfnTmp );
		}
	}
}

CMyAspSite::~CMyAspSite()
{
	if( m_pEngine )
		m_pEngine->Release();
	if( m_pstrLastError )
		delete m_pstrLastError;
	if( m_pSplitedAspPage )
		delete m_pSplitedAspPage;
	if( m_pRequest )
		delete m_pRequest;
}

///-------------------------------------------------------
/// CYJ,2004-7-8
/// Function:
///		run the web page in memory
/// Input parameter:
///		pszContent				web page
///		lpszURL					relative URL, default is NULL
/// Output parameter:
///		true					succ
///		false					failed
bool CMyAspSite::Run( LPCSTR pszContent, LPCSTR lpszURL )
{
	ASSERT( IsValid() );
	if( false == IsValid() )
		return false;
	if( NULL == lpszURL )
		lpszURL = "";
	if( m_pRequest )
		m_pRequest->SetURL( lpszURL );

	m_pOutputObject->ResetBuffer();

	*m_pstrLastError = "";
	ASSERT( m_pResponseObj && m_pEngine );
	if( NULL == m_pResponseObj || NULL == m_pEngine )
	{
		*m_pstrLastError = "Failed to initialize objects!";
		return false;
	}
	ASSERT( pszContent );
	if( NULL == pszContent || 0 == *pszContent )
	{
		*m_pstrLastError = "Empty Content!";
		return false;
	}

	m_pResponseObj->Reset();

	CMyString * pstrScript = SplitASPPage( pszContent );
	if( pstrScript->IsEmpty() )
	{
		*m_pstrLastError = "Parse Web page failed.";
		return false;
	}

	m_pEngine->SetSourceCode( *pstrScript );

	if( !m_pEngine->run() )
	{
		*m_pstrLastError = m_pEngine->GetLastError( NULL );
		return false;
	}

	return true;
}

///-------------------------------------------------------
/// CYJ,2004-7-8
/// Function:
///		get error description and line no
/// Input parameter:
///		None
/// Output parameter:
///		None
LPCSTR CMyAspSite::GetErrorDescription()
{
	return *m_pstrLastError;
}

//-----------------------------------------------------
//	分离 ASP 网页内容
//	入口参数
//		strSrcASP				待分解的 ASP 网页，将被破坏
//		strScriptBuf			输出缓冲区
//	返回参数
//		若为空字符串，则失败
//		其他				成功
CMyString * CMyAspSite::SplitASPPage(LPCSTR pszContent)
{
	CMyString & strRetVal = *m_pSplitedAspPage;
	strRetVal = "";
	CMyResponse* pResponseObj = m_pResponseObj;
	int nLen = strlen( pszContent );
	char * pszNewBuffer = new char[nLen+1];
	if( NULL == pszNewBuffer )
		return NULL;
	memcpy( pszNewBuffer, pszContent, nLen );
	pszNewBuffer[nLen] = 0;

	CMyString strTmp;
	int nBlockNo = 0;
	char * pszHead = pszNewBuffer;
	while( pszContent )
	{
		char * pszScriptStart = strstr( pszHead, "<%" );
		if( pszScriptStart == NULL )		//	没有发现脚本
		{
			if( *pszHead )					//	有数据
			{
				strTmp = pszHead;
				pResponseObj->m_astrBlock.Add( strTmp );
				strTmp.Format("\n\nResponse.WriteBlock(%d)\n\n",nBlockNo);
				strRetVal += strTmp;
			}
			break;							//	结束
		}
		*pszScriptStart ++ = 0;
		pszScriptStart ++;					//	跳过 <%
		char * pszScriptEnd = strstr( pszScriptStart, "%>" );
		if( pszScriptEnd == NULL )
		{
			delete pszNewBuffer;
			return m_pSplitedAspPage;
		}
		* pszScriptEnd ++ = '\n';
		*pszScriptEnd ++= 0;

		if( *pszHead )					//	有数据
		{
			strTmp = pszHead;
			pResponseObj->m_astrBlock.Add( strTmp );
			strTmp.Format("\n\nResponse.WriteBlock(%d)\n\n",nBlockNo);
			strRetVal += strTmp;
			nBlockNo ++;
		}
		strRetVal += pszScriptStart;
		pszHead = pszScriptEnd;
	}
	ASSERT( nBlockNo == pResponseObj->m_astrBlock.GetSize()-1 );

	delete pszNewBuffer;

	return m_pSplitedAspPage;
}

///-------------------------------------------------------
/// CYJ,004-7-8
/// Function:
///		Is Site object valid
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CMyAspSite::IsValid()
{
	return ( m_pResponseObj && m_pEngine && m_pstrLastError );
}

///-------------------------------------------------------
/// CYJ,2004-7-9
/// Function:
///		Get asp engine
/// Input parameter:
///		None
/// Output parameter:
///		None
CBasicEngine*	CMyAspSite::GetBasicEngine()
{
	return m_pEngine;
}

///-------------------------------------------------------
/// CYJ,2004-7-12
/// Function:
///		Register object
/// Input parameter:
///		pObject			the object to be register
/// Output parameter:
///		true			succ
///		false			failed
bool CMyAspSite::RegisterObject( cobject* pObject )
{
	if( NULL == m_pEngine )
		return false;
	return (m_pEngine->get_root()->add_child( pObject ) == TRUE);
}
