///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-22
///
///		用途：
///			HTML 分析与显示器
///=======================================================

#if !defined(AFX_HTMLPARSER_H__286AEA33_83E7_4E2E_827C_8EF6AB8B2AC4__INCLUDED_)
#define AFX_HTMLPARSER_H__286AEA33_83E7_4E2E_827C_8EF6AB8B2AC4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HTMLElements.h"
#include "HTMLTokenizer.h"
#include <MyArray.h>
#include <MyMap.h>
#include "ASPFunction.h"

class CMyCompoundFileObj;
class CMyMemBufferManager;
class CDC;

typedef CMyArray<CMyString> CMyStringArray;

class CMyRequest : public CBSExternFunctionTemplete
{
public:
	CMyRequest( BSModule * pModule, const char * lpszFunctionName );
	virtual ~CMyRequest();
	virtual void OnFunctionCall();
	virtual void SafeDelete();
	void SetRequst( LPCSTR lpszRequest );
private:
	CMyMap< CMyString, LPCSTR, LPCSTR, LPCSTR>	m_mapRequest;
	CMyArray<char*>	m_astrRequest;
	char *	m_pszSrcRequest;
};

class CMyResponseWrite : public CBSExternFunctionTemplete
{
public:
	CMyResponseWrite( BSModule * pModule, const char * lpszFunctionName, CMyString * pOutString );
	virtual void OnFunctionCall();
	virtual void SafeDelete();
private:
	CMyString *			m_pOutString;
};

class CMyResponseWriteBlock : public CBSExternFunctionTemplete
{
public:
	CMyResponseWriteBlock( BSModule * pModule, const char * lpszFunctionName, CMyStringArray * pArray, CMyString * pOutString );
	virtual void OnFunctionCall();
	virtual void SafeDelete();
private:
	CMyStringArray *	m_pBlockString;
	CMyString *			m_pOutString;
};

class CHTMLParser : public CBScriptEngine
{
public:		
	CMyString GetLinkURL( int nIndex, CMyString * pstrID = NULL );
	int SetActiveLinkIndex( int nNewIndex );
	int GetActiveLinkIndex();
	int GetLinkUrlCount();
	CMyString GetActiveLinkURL(bool bHaseBaseURL = true);
	void MoveBackActiveLink();
	void MoveToNextActiveLink();
	bool Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszStartFileName = NULL );
	bool Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszHTMLBuf, int nLen, LPCSTR lpszRequest=NULL );
	bool Parse( LPCSTR lpszStartFileName );
	CHTMLParser(CDC * pDC);
	virtual ~CHTMLParser();
	virtual void OnModuleCreated( BSModule * pModule );
	virtual void OnDeleteModule( BSModule * pModule );
	virtual void Print( void * pData, bool bIsString );
	PFN_OnModuleCreateDelete SetOnModuleCreateDeleteCallBack( PFN_OnModuleCreateDelete pfn, DWORD dwUserData );

	CHTMLContainerBase	m_HTMLRootElement;
	CHTMLElementsBase * m_pCurrentParent;
	CHTMLElementsBase * m_pCurrentElement;

	CMyArray<CHTMLLinkedElementBase *> m_aActionElements;
	CMyArray<CHTMLElementsBase *> m_aAllElements;

	CMyStringArray m_astrASPBlock;		//	存放ASP脚本的字符块
	CMyStringArray m_astrRequest;			//	存放ASP脚本的字符块
	CMyString	m_strAspResponse;				//	ASP运行结果
	
	CMyString			m_strTitle;	

#ifdef _DEBUG
	void Dump( CHTMLElementsBase * pElement );
#endif //_DEBUG

private:
	void DeleteAllElements();
	void CreateInstance( HTML_ELEMENT_ENUM nTagType, CMyString & strParam );
	void PopElement();
	bool Parse( LPCSTR lpszHTMLContent, int nLen );
	void DoSpecialSetting( HTML_ELEMENT_ENUM nTagType );
	bool ReadFile( LPCSTR lpszRefFileName, CMyMemBufferManager & MemMgr );
	bool SplitASPPage(CMyString &strSrcASP, CMyString &strScriptBuf);
	CMyString SetRequestString( LPCSTR lpszRequest );

private:	
	CMyCompoundFileObj *	m_pSrcFile;
	int m_nActiveLinkIndex;
	CMyString				m_strBaseURL;		// 基本路径，可能 file:// 或 http 协议，缺省为 file 协议
	bool					m_bIsLocalFile;
	CDC *	m_pDC;

	CMyRequest *	m_pRequestObj;
	CMyResponseWrite *	m_pWriteObj;
	CMyResponseWriteBlock *	m_pWriteBlockObj;
	CMyString			m_strRequestString;
	PFN_OnModuleCreateDelete	m_pfnOnModuleCreateDelete;
	DWORD	m_dwOnModuleCallBackUserData;
};

#endif // !defined(AFX_HTMLPARSER_H__286AEA33_83E7_4E2E_827C_8EF6AB8B2AC4__INCLUDED_)
