// MyHTMLRender.h: interface for the CMyHTMLRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYHTMLRENDER_H__84A58C94_18C3_4C1F_8B23_4250508D6143__INCLUDED_)
#define AFX_MYHTMLRENDER_H__84A58C94_18C3_4C1F_8B23_4250508D6143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyCompoundFileObj;
class CHTMLParser;
class CHTMLRender;
class CMyString;

#ifndef _WIN32
	class CDC;
#endif //_WIN32

#ifndef __FOR_MICROWIN_EMBED__
#ifdef _MSC_VER
  #ifndef _LIB
	#ifdef _AFXDLL
		#ifdef _DEBUG
			#pragma comment(lib, "LibMyHTMLRenderD.lib")
		#else
			#pragma comment(lib, "LibMyHTMLRender.lib")
		#endif //_DEBUG
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "LibMyHTMLRenderSD.lib")
		#else
			#pragma comment(lib, "LibMyHTMLRenderS.lib")
		#endif //_DEBUG
	#endif //_AFXDLL
  #endif //_LIB
#endif //_DEBUG
#endif // __FOR_MICROWIN_EMBED__

class CMyHTMLRender  
{
public:
	bool IsValid();
	bool Parse( PBYTE pBuf, int nLen, LPCSTR lpszStartFileName = NULL );
	bool Parse( PBYTE pBuf, int nLen, LPCSTR lpszContent, int nContentBufSize, LPCSTR lpszRequest=NULL );
	bool Parse( LPCSTR lpszStartFileName );
	void Render( int nWidth, int nHeight, CDC * pDC = NULL );
	LPCSTR GetActiveLinkURL( bool bHaseBaseURL = true );
	void MoveBackActiveLink();
	void MoveToNextActiveLink();
	CHTMLParser	* GetParser() const{ return	m_pParser; }

	CMyString GetLinkURL( int nIndex, CMyString * pstrID = NULL );
	int SetActiveLinkIndex( int nNewIndex );
	int GetActiveLinkIndex();
	int GetLinkUrlCount();


	CMyHTMLRender(CDC * pDC);
	virtual ~CMyHTMLRender();

private:
	CMyString			*	m_pActiveLinkURL;
	CMyCompoundFileObj	*	m_pFileObj;
	CHTMLParser			*	m_pParser;
	CHTMLRender			*	m_pRender; 
	CDC *	m_pDC;
};

extern void EnableGlobalImageCache( bool bValue );


#endif // !defined(AFX_MYHTMLRENDER_H__84A58C94_18C3_4C1F_8B23_4250508D6143__INCLUDED_)
