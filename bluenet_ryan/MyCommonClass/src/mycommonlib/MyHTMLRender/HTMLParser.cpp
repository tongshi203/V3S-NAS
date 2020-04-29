///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-22
///
///		用途：
///			HTML 分析与显示器
///=======================================================

#include "stdafx.h"
#include "HTMLParser.h"
#include "HTMLTokenizer.h"
#include "MyCompoundFileObj.h"
#include <mylibhttp/myhttpfile.h>
#include <stdlib.h>

#ifndef _WIN32
	#include <MyFile.h>
#endif //_WIN32

#if defined(_DEBUG) && defined(_WIN32)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern "C" char *b_strupper( char* );

class CMyMemBufferManager
{
public:
	CMyMemBufferManager()
	{
		m_nBufLen = 0;
		m_pBuffer = NULL;
		m_bAutoFreeBuf = false;
	}
	~CMyMemBufferManager()
	{
		Invalid();		
	}
	void Invalid()
	{
		if( m_bAutoFreeBuf )
			delete m_pBuffer;
		m_nBufLen = 0;
		m_pBuffer = NULL;
		m_bAutoFreeBuf = false;
	}
	operator PBYTE() const { return m_pBuffer; }
	operator int() const { return m_nBufLen; }
public:
	int		m_nBufLen;
	PBYTE	m_pBuffer;
	bool	m_bAutoFreeBuf;
};

static CMyString DecodeURL( LPCSTR lpszURL )
{
	CMyString strDecoded = lpszURL;

	strDecoded.Replace( '+', ' ' );

	// first see if there are any %s to decode....
	if( strDecoded.Find( '%' ) < 0 )
		return strDecoded;
	
	char * pszSrc;
	char * pszDst;
	int nLen = strDecoded.GetLength();
	pszSrc = strDecoded.GetBuffer( nLen );
	pszDst = pszSrc;
	// iterate through the string, changing %dd to special char....
	while( *pszSrc )
	{
		char ch = *pszSrc ++;
		if ( ch == '%' )
		{
			if ( *pszSrc == '%' )
			{								//	就是为了编码 %
				*pszDst = '%';				//	%
				pszSrc ++;
				pszDst ++;
				nLen --;					//	减少 1 个字符的编码	, %% ==> %
				continue;
			}
			else
			{
				WORD wWideChar = 0;
				BYTE ch1 = *pszSrc++;
				BYTE ch2 = *pszSrc++;
				ch1 = (ch1 >= 'A') ? ((ch1&0xdf)-'A'+0xa) : (ch1-'0');
				ch2 = (ch2 >= 'A') ? ((ch2&0xdf)-'A'+0xa) : (ch2-'0');	
				*pszDst ++ = ch1*16 + ch2;
				nLen -= 2;					
			}
		}
		else
		{
			*pszDst = ch;				//	普通的数据
			pszDst ++;
		}
	}
	strDecoded.ReleaseBuffer( nLen );	//	释放缓冲区

	return strDecoded;
}

///////////////////////////////////////////////////////////////////////
// CMyRequest
CMyRequest::CMyRequest( BSModule * pModule, const char * lpszFunctionName )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pszSrcRequest = NULL;
}

CMyRequest::~CMyRequest()
{
	if( m_pszSrcRequest )
		free( (void*)m_pszSrcRequest );
}

void CMyRequest::OnFunctionCall()
{
	if( GetArgCount() < 1 )
		return;
	if( IsArgTypeString(0) )
	{	
		CBSAutoReleaseString strName = GetArgAsString( 0 );
		if( NULL == strName.m_pString || 0 == *strName.m_pString )
			SetRetValue( "" );
		else
			SetRetValue( m_mapRequest[strName.m_pString] );
	}
	else
	{					// as number
		int nIndex = GetArgAsInt( 0 );
		if( nIndex < 0 || nIndex >= m_astrRequest.GetSize() )	
			SetRetValue( "" );
		else
			SetRetValue( m_astrRequest[nIndex] );
	}
}

void CMyRequest::SafeDelete()
{
	delete this;
}

void CMyRequest::SetRequst( LPCSTR lpszRequest )
{
	m_mapRequest.RemoveAll();
	m_astrRequest.RemoveAll();
	if( NULL == lpszRequest || 0 == *lpszRequest )
		return;	
	if( m_pszSrcRequest )
		free( (void*)m_pszSrcRequest );
	CMyString strTmp = DecodeURL( lpszRequest );
	m_pszSrcRequest = strdup( (char*)strTmp );
	if( NULL == m_pszSrcRequest )
		return;
	int nLen = strlen( m_pszSrcRequest);

	char * pszEnd;
	char * pszHead = m_pszSrcRequest;
	while( pszHead && nLen )
	{
		pszEnd = NULL;
		for(int i=0; i<nLen; i++ )
		{
			if( pszHead[i] == '&' )
			{
				pszEnd = pszHead + i;
				break;					 
			}
		}
		if( pszEnd )
			*pszEnd ++ = 0;
		nLen -= (pszEnd - pszHead);
		char * pszSplit = strchr( pszHead, '=' );
		if( pszSplit == NULL )
		{
			pszHead = pszEnd;
			continue;							//	错误的格式
		}
		*pszSplit ++ = 0;
		b_strupper( pszHead );
		m_mapRequest[pszHead] = pszSplit;
		LPCSTR lpszTTT = m_mapRequest[pszHead];
		m_astrRequest.Add( pszSplit );
		pszHead = pszEnd;
	}
}
	
///////////////////////////////////////////////////////////////////////
// CMyResponseWrite 
CMyResponseWrite::CMyResponseWrite( BSModule * pModule, const char * lpszFunctionName, CMyString * pOutString )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pOutString = pOutString;
}

// ResponseWrite
void CMyResponseWrite::OnFunctionCall()
{
	if( NULL == m_pOutString )
		return;
	if( GetArgCount() < 1  )
		return;
	CBSAutoReleaseString strText = GetArgAsString( 0 );
	if( NULL == strText.m_pString || 0 == strText.m_pString )
		return;
	*m_pOutString += strText.m_pString;
}

void CMyResponseWrite::SafeDelete()
{
	delete this;
}

///////////////////////////////////////////////////////////////////////
// CMyResponseWriteBlock
CMyResponseWriteBlock::CMyResponseWriteBlock( BSModule * pModule, const char * lpszFunctionName, CMyStringArray * pArray, CMyString * pOutString )
	:CBSExternFunctionTemplete( pModule, lpszFunctionName )
{
	m_pBlockString = pArray;
	m_pOutString = pOutString;
}

// ResponseWriteBlock
void CMyResponseWriteBlock::OnFunctionCall()
{
	if( NULL == m_pBlockString )
		return;
	if( GetArgCount() < 1  )
		return;
	int nIndex = GetArgAsInt( 0 );
	if( nIndex < 0 || nIndex >= m_pBlockString->GetSize() )
		return;
	*m_pOutString += m_pBlockString->ElementAt( nIndex );
}

void CMyResponseWriteBlock::SafeDelete()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHTMLParser::CHTMLParser(CDC * pDC)
	: m_HTMLRootElement( NULL )
{
	m_pCurrentParent = &m_HTMLRootElement;
	m_nActiveLinkIndex = 0;
	m_bIsLocalFile = TRUE;
	m_pDC = pDC;
	
	m_pRequestObj = NULL;
	m_pWriteObj = NULL;
	m_pWriteBlockObj = NULL;
	m_pfnOnModuleCreateDelete = NULL;
}

CHTMLParser::~CHTMLParser()
{
	DeleteAllElements();

	ASSERT( m_pRequestObj == NULL );
	ASSERT( m_pWriteBlockObj == NULL );
	ASSERT( m_pWriteObj == NULL );
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		解析文件
/// Input parameter:
///		pSrcFile				源文件
///		lpszStartFileName		原始文件名
/// Output parameter:
///		None
bool CHTMLParser::Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszStartFileName )
{	
	CMyString strNewStartFileName = SetRequestString( lpszStartFileName );
	lpszStartFileName = strNewStartFileName;
	
	if( NULL == lpszStartFileName )
		lpszStartFileName = "INDEX.HTM";
	m_pSrcFile = pSrcFile;
	m_strBaseURL = "";

	PONE_COMPOUND_FILE pFileData = pSrcFile->Find( lpszStartFileName );
	if( NULL == pFileData )
		return false;

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;

	CMyString strTmp = lpszStartFileName;
	int nPos = strTmp.Find( '.' );
	if( nPos > 0 )
		strTmp = strTmp.Mid( nPos+1 );
	else
		strTmp = "";
	strTmp.MakeUpper();
	if( strTmp.Find("HTM") >= 0 || strTmp.Find("TXT") >= 0 )
		return Parse( (LPCSTR)pFileData->m_pDataBuf, pFileData->m_nFileLen );
	else
	{								//	图形文件
		strTmp.Format("src=%s", lpszStartFileName );
		CreateInstance( HTML_TAG_IMAGE, strTmp );
		DoSpecialSetting( HTML_TAG_IMAGE );		
		return true;
	}		
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		解析一个缓冲区，同时设置复合文档对象
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTMLParser::Parse( CMyCompoundFileObj * pSrcFile, LPCSTR lpszHTMLBuf, int nLen, LPCSTR lpszRequest )
{
	SetRequestString( lpszRequest );

	m_pSrcFile = pSrcFile;
	m_strBaseURL = "";

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;

	return Parse( lpszHTMLBuf, nLen );
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		解析实际网页，即非复合文档
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CHTMLParser::Parse( LPCSTR lpszStartFileName )
{
	CMyString strNewStartFileName = SetRequestString( lpszStartFileName );
	lpszStartFileName = strNewStartFileName;

	m_pSrcFile = NULL;

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();
	m_nActiveLinkIndex = 0;
	
	m_strBaseURL = lpszStartFileName;
	CMyString strRefFileName = m_strBaseURL;
	int nPos = m_strBaseURL.ReverseFind( '\\' );
	if( nPos < 0 )
		nPos = m_strBaseURL.ReverseFind( '/' );
	if( nPos >= 0 )
	{
		strRefFileName = m_strBaseURL.Mid( nPos+1 );
		m_strBaseURL.ReleaseBuffer( nPos+1 );
	}
	else
	{
		strRefFileName = m_strBaseURL;
		m_strBaseURL = "";
	}
	
	// http://
	if( 0 == m_strBaseURL.Left(7).CompareNoCase( "http://" ) )
		m_bIsLocalFile = FALSE;
	else
	{
		if( 0 == m_strBaseURL.Left(7).CompareNoCase( "file://" ) )
			m_strBaseURL.Delete( 0, 7 );
		m_bIsLocalFile = TRUE;
	}

	CMyString strTmp = strRefFileName;
	nPos = strTmp.Find( '.' );
	if( nPos > 0 )
		strTmp = strTmp.Mid( nPos+1 );
	else
		strTmp = "";
	strTmp.MakeUpper();

	if( strTmp.Find("HTM") < 0 && strTmp.Find("TXT") < 0 )
	{						//	图形、图象
		strTmp.Format("src=%s", (char*)strRefFileName );
		CreateInstance( HTML_TAG_IMAGE, strTmp );
		DoSpecialSetting( HTML_TAG_IMAGE );		
		return true;
	}

	CMyMemBufferManager MemHelper;
	if( false == ReadFile( strRefFileName, MemHelper ) )
		return false;
	
	return Parse( (LPCSTR)MemHelper.m_pBuffer, MemHelper.m_nBufLen );
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		解析 HTML 网页
/// Input parameter:
///		lpszHTMLContent			待显示的网页内容
///		nLen					文本长度
/// Output parameter:
///		true					成功
///		false					失败
/// Note:
///		可能 nLen < strlen(lpszHTMLContent)，此时，只分析到 nLen 为止
bool CHTMLParser::Parse(LPCSTR lpszHTMLContent, int nLen)
{
	CMyString strTmpSrcPage;
	char * pszTmpBuf = strTmpSrcPage.GetBuffer( nLen+1 );
	if( NULL == pszTmpBuf )
		return false;
	strncpy( pszTmpBuf, lpszHTMLContent, nLen );
	strTmpSrcPage.ReleaseBuffer( nLen );
	CMyString strResultPage;

	if( SplitASPPage( strTmpSrcPage, strResultPage ) )
	{
		m_strAspResponse.ReleaseBuffer( 0 );
		strTmpSrcPage.ReleaseBuffer( 0 );
		Run( (char*)strResultPage );
		strResultPage.ReleaseBuffer( 0 );
		lpszHTMLContent = (char*)m_strAspResponse;
		nLen = m_strAspResponse.GetLength();
	}

	m_pCurrentElement = &m_HTMLRootElement;	
	m_aActionElements.RemoveAll();
	DeleteAllElements();
	m_HTMLRootElement.Preset();

	CHTMLTokenizer tokenizer;
	tokenizer.SetText( lpszHTMLContent, nLen );

	HTML_ELEMENT_ENUM nTagType;
	CMyString strParam;	

	while( (nTagType=tokenizer.NextToken( strParam )) < HTML_TAG_ERROR )
	{
		if( HTML_TAG_RIGHT_SEPARATOR == nTagType )
		{
			strParam.MakeUpper();

#ifdef _DEBUG_
			m_pCurrentParent->CHTMLElementsBase::Dump();
			TRACE("</%s>\n", strParam);
#endif // _DEBUG

			PopElement();
			continue;
		}

		CreateInstance( nTagType, strParam );
		ASSERT( m_pCurrentElement );		

		if( HTML_TAG_TITLE == nTagType )
		{
			tokenizer.NextToken( m_strTitle );
			m_pCurrentElement->SetParameter( m_strTitle );
		}
		else 
			DoSpecialSetting( nTagType );		

#ifdef _DEBUG_
		m_pCurrentElement->Dump();
#endif //_DEBUG

	}

#ifdef _DEBUG
	TRACE("\n\n\n------------------------------------------------\n");
	Dump( &m_HTMLRootElement );
#endif //_DEBUG

	m_nActiveLinkIndex = 0;
	if( m_aActionElements.GetSize() )
		m_aActionElements[0]->SetActiveLink();

	return true;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		处理一些特殊处理
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::DoSpecialSetting(HTML_ELEMENT_ENUM nTagType)
{
	ASSERT( m_pCurrentElement );
	if( NULL == m_pCurrentElement )
		return;

	CMyMemBufferManager MemHelper;
	//	以下为特殊处理	
	if( HTML_TAG_IMAGE == nTagType )
	{				//	需要设置图形
		CHTML_IMAGE_Element * pImage = static_cast<CHTML_IMAGE_Element*>(m_pCurrentElement);

		if( ReadFile( pImage->GetFileName(), MemHelper ) )
			pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen );

		//  CYJ,2005-4-11 set active link image data
		CMyString strActiveURL = pImage->GetActiveImgFileName();
		if( strActiveURL.GetLength() && ReadFile( strActiveURL, MemHelper ) )
			pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen, false );
	}
	else if( HTML_TAG_BODY == nTagType )
	{
		CHTML_BODY_Element * pImage = static_cast<CHTML_BODY_Element*>(m_pCurrentElement);
		CMyString strFileName = pImage->GetFileName();
		if( false == strFileName.IsEmpty() )
		{
			if( ReadFile( (char*)strFileName, MemHelper ) )
				pImage->SetImageData( MemHelper.m_pBuffer, MemHelper.m_nBufLen );			
		}
	}
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		跳到上一级
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::PopElement()
{
	CHTMLElementsBase * pParent = m_pCurrentParent->GetParent();
	if( pParent )
		m_pCurrentParent = pParent;
}

///-------------------------------------------------------
/// CYJ,2004-11-22
/// Function:
///		创建一个新的元素
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note:
///		创建新的实例，并给m_pCurrentElement赋值
void CHTMLParser::CreateInstance(HTML_ELEMENT_ENUM nTagType, CMyString & strParam)
{
	CHTMLElementsBase * pChild = NULL;
	bool bNewAsParent = true;
	switch( nTagType )
	{
	case HTML_TAG_TEXT:
		{
			CHTMLTextElement * pTextChild = new CHTMLTextElement( m_pCurrentParent );
			pChild = static_cast<CHTMLElementsBase*>(pTextChild);
			bNewAsParent = false;
			if( FALSE == m_pCurrentParent->GetLinkURL().IsEmpty() )
			{
				CHTMLLinkedElementBase * pLinkedElement = static_cast<CHTMLLinkedElementBase*>( pTextChild );
				m_aActionElements.Add( pLinkedElement );	//	存在连接
			}
		}
		break;
	case HTML_TAG_IMAGE:
		{
			CHTML_IMAGE_Element * pImageChild = new CHTML_IMAGE_Element( m_pCurrentParent );
			pChild = static_cast<CHTMLElementsBase*>( pImageChild );
			bNewAsParent = false;
			if( FALSE == m_pCurrentParent->GetLinkURL().IsEmpty() )
			{
				CHTMLLinkedElementBase * pLinkedElement = static_cast<CHTMLLinkedElementBase*>( pImageChild );
				m_aActionElements.Add( pLinkedElement );	//	存在连接
			}
		}
		break;

	case HTML_TAG_A:
		pChild = new CHTML_A_Element( m_pCurrentParent );
		break;
	case HTML_TAG_DIV:
		pChild = new CHTML_DIV_Element( m_pCurrentParent );
		break;
	case HTML_TAG_BR:
		pChild = new CHTML_BR_Element( m_pCurrentParent );
		bNewAsParent = false;
		break;	
	case HTML_TAG_FONT:
		pChild = new CHTML_FONT_Element( m_pCurrentParent );
		break;
	case HTML_TAG_U:
		pChild = new CHTML_U_Element( m_pCurrentParent );
		break;
	case HTML_TAG_B:
		pChild = new CHTML_B_Element( m_pCurrentParent );		
		break;
	case HTML_TAG_HTML:		
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "HTML" );	// 空的		
		break;
	case HTML_TAG_BODY:
		pChild = new CHTML_BODY_Element( m_pCurrentParent );
		break;
	case HTML_TAG_TITLE:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "TITLE" );	// 空的		
		break;
	case HTML_TAG_PRE:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "PRE" );	// 空的,PRE
		break;
	case HTML_TAG_UNKNOWN:
		pChild = new CHTML_Dummy_Element( m_pCurrentParent, "UNKNOWN" );	// 空的		
		break;
	case HTML_TAG_LINE:
		bNewAsParent = false;
		pChild = new CHTML_LINE_Element( m_pCurrentParent );
		break;
	case HTML_TAG_RECT:
		bNewAsParent = false;
		pChild = new CHTML_RECT_Element( m_pCurrentParent );	
		break;
	case HTML_TAG_P:			//  CYJ,2005-8-2 add
		pChild = new CHTML_P_Element( m_pCurrentParent );
		break;
	case HTML_TAG_COMMENT:
		return;
	default:
		return;
	}

	pChild->SetParameter( strParam );	

	pChild->SetCoordinate( m_pCurrentParent->GetX(), m_pCurrentParent->GetY(),
		m_pCurrentParent->GetWidth(), m_pCurrentParent->GetHeight() );
	if( HTML_TAG_DIV != nTagType )		// DIV 会重新指定 z-index
		pChild->Set_Z_Index( m_pCurrentParent->Get_Z_Index() );

	m_pCurrentParent->AppendChild( pChild );

	if( bNewAsParent )	
		m_pCurrentParent = pChild;		

	m_pCurrentElement = pChild;
	pChild->SetCDC( m_pDC );
	m_aAllElements.Add( pChild );
}


#ifdef _DEBUG
void CHTMLParser::Dump( CHTMLElementsBase * pElement )
{
	pElement->Dump();
	CHTMLElementsBase * pChild = pElement->GetChild();
	if( pChild )
		Dump( pChild );
	pElement->CHTMLElementsBase::Dump();
	if( pChild )
		TRACE("</%s>\n", pElement->GetTagName() );
	else
		TRACE("\n");

	CHTMLElementsBase * pNext = pElement->GetNext();
	if( pNext )
		Dump( pNext );
}
#endif //_DEBUG

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		删除所有HTML元素
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::DeleteAllElements()
{
	int nCount = m_aAllElements.GetSize();
	for(int i=0; i<nCount; i++)
	{
		delete m_aAllElements[i];
	}
	m_aAllElements.RemoveAll();
	m_HTMLRootElement.Preset();
	m_pCurrentParent = &m_HTMLRootElement;
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		移动到下一个有效连接
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::MoveToNextActiveLink()
{
	if( 0 == m_aActionElements.GetSize() )
		return;

	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

	if( ++m_nActiveLinkIndex >= m_aActionElements.GetSize() )
		m_nActiveLinkIndex = 0;
	
	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-25
/// Function:
///		移回到上一个有效连接
/// Input parameter:
///		None
/// Output parameter:
///		None
void CHTMLParser::MoveBackActiveLink()
{
	if( 0 == m_aActionElements.GetSize() )
		return;
	
	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

	if( --m_nActiveLinkIndex < 0 )
		m_nActiveLinkIndex = m_aActionElements.GetSize()-1;

	m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();
}

///-------------------------------------------------------
/// CYJ,2004-11-26
/// Function:
///		获取当前选中的连接URL
/// Input parameter:
///		None
/// Output parameter:
///		长度为0的字符串，表示没有有效连接
CMyString CHTMLParser::GetActiveLinkURL( bool bHaseBaseURL )
{
	CMyString strRetVal;
	if( 0 == m_aActionElements.GetSize() )
		return strRetVal;
	if( m_nActiveLinkIndex < 0 || m_nActiveLinkIndex>= m_aActionElements.GetSize() )
		return strRetVal;

	if( m_pSrcFile )
		return m_aActionElements[m_nActiveLinkIndex]->GetLinkURL();
	else
	{
		if( bHaseBaseURL )
			strRetVal = m_strBaseURL;
		strRetVal += m_aActionElements[m_nActiveLinkIndex]->GetLinkURL();
	}
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2004-12-6
/// Function:
///		读取一个文件
/// Input parameter:
///		lpszRefFileName		相对文件名
///		MemMgr				内存管理
/// Output parameter:
///		NULL				失败
///		其他				成功
///	Note:
///		调用者需要释放内存
bool CHTMLParser::ReadFile(LPCSTR lpszRefFileName, CMyMemBufferManager & MemMgr )
{
	MemMgr.Invalid();			//	释放内存

	if( m_pSrcFile )
	{				//	复合文档方式，不应该调用这个方法
		PONE_COMPOUND_FILE pImgData = m_pSrcFile->Find( lpszRefFileName );
		if( NULL == pImgData )
			return false;
		MemMgr.m_bAutoFreeBuf = false;
		MemMgr.m_nBufLen = pImgData->m_nFileLen;
		MemMgr.m_pBuffer = pImgData->m_pDataBuf;
		return true;
	}

	CMyString strFileName;
	strFileName.Format("%s%s", (char*)m_strBaseURL , lpszRefFileName );
#ifdef _DEBUG	
	TRACE("CHTMLParse::ReadFile, URL=%s\n", (char*)strFileName );
#endif //_DEBUG	
	if( m_bIsLocalFile )
	{								//	本地文件方式
		CFile fSrc;
		if( FALSE == fSrc.Open( (LPCSTR)strFileName, CFile::modeRead|CFile::typeBinary ) )
			return false;
		int nLen = fSrc.GetLength();
		PBYTE pBuf = new BYTE[nLen];
		if( NULL == pBuf )
			return false;
		try
		{
			fSrc.Read( pBuf, nLen );
			fSrc.Close();
		}
		catch( ... )
		{
			delete pBuf;
			fSrc.Abort();
		}
		MemMgr.m_pBuffer = pBuf;
		MemMgr.m_nBufLen = nLen;
		MemMgr.m_bAutoFreeBuf = true;
		return true;
	}
	else
	{								// http
		CMyHTTPFile	HttpFile;
		if( FALSE == HttpFile.Open( (LPCSTR)strFileName ) )
			return false;
		PBYTE pBuf = (PBYTE) malloc( 4096 );
		if( NULL == pBuf )
			return false;
		int nBytesRead = 0;
		int nRetVal;
		while( (nRetVal = HttpFile.Read( pBuf+nBytesRead, 4096 ) ) )
		{
			nBytesRead += nRetVal;
			pBuf = (PBYTE)realloc( pBuf, ((nBytesRead+4096)+1023)&(0xFFFFFC00) );
			if( NULL == pBuf )
				return false;
		}
		MemMgr.m_pBuffer = pBuf;
		MemMgr.m_nBufLen = nBytesRead;
		MemMgr.m_bAutoFreeBuf = true;
		return true;					
	}

	return false;
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		On Basic module created
/// 输入参数:
///		无
/// 返回参数:
///		无
void CHTMLParser::OnModuleCreated( BSModule * pModule )
{
	ASSERT( m_pRequestObj == NULL );
	ASSERT( m_pWriteBlockObj == NULL );
	ASSERT( m_pWriteObj == NULL );
	
	m_pRequestObj = new CMyRequest( pModule, "Request" );
	m_pWriteObj = new CMyResponseWrite( pModule, "ResponseWrite", &m_strAspResponse );
	m_pWriteBlockObj = new CMyResponseWriteBlock( pModule, "ResponseWriteBlock", &m_astrASPBlock, &m_strAspResponse );

	if( m_pRequestObj && false == m_strRequestString.IsEmpty() )
	{
		m_pRequestObj->SetRequst( (char*)m_strRequestString );
		m_strRequestString = "";
	}
	if( m_pfnOnModuleCreateDelete )
		m_pfnOnModuleCreateDelete( pModule, true, m_dwOnModuleCallBackUserData );
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		print
/// 输入参数:
///		无
/// 返回参数:
///		无
/// 修改记录：
///		2006.6.30, 只支持打印整数
void CHTMLParser::Print( void * pData, bool bIsString )
{	
	if( bIsString )
	{
		if( pData && (char*)pData )
			m_strAspResponse += (char*)pData;
	}
	else
	{
		CMyString strTmp;
		strTmp.Format( "%d", (int)( *(double*)pData+0.05 ) );	// 2006.6.30 CYJ Modify, support integer only
		m_strAspResponse += strTmp;
	}
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		模块即将被删除
/// 输入参数:
///		无
/// 返回参数:
///		无
void CHTMLParser::OnDeleteModule( BSModule * pModule )
{
	if( m_pRequestObj )
		m_pRequestObj->Release();
	m_pRequestObj = NULL;

	if( m_pWriteObj )
		m_pWriteObj->Release();
	m_pWriteObj = NULL;

	if( m_pWriteBlockObj )
		m_pWriteBlockObj->Release();
	m_pWriteBlockObj = NULL;

	if( m_pfnOnModuleCreateDelete )
		m_pfnOnModuleCreateDelete( pModule, false, m_dwOnModuleCallBackUserData );
}

//	分离 ASP 网页内容
//	入口参数
//		strSrcASP				待分解的 ASP 网页，将被破坏
//		strScriptBuf			输出缓冲区
//	返回参数
//		true					是一个ASP
//		false 					正常的网页
bool CHTMLParser::SplitASPPage(CMyString & strSrcASP, CMyString &strScriptBuf)
{
	int nLen = strSrcASP.GetLength();
	char * pszHead = strSrcASP.GetBuffer( nLen+1 );
	pszHead[ nLen ] = 0;					//	清零
	m_astrASPBlock.RemoveAll();
	m_strAspResponse = "";
	
	CMyString strTmp;
	int nBlockNo = 0;
	while( pszHead )
	{
		char * pszScriptStart = strstr( pszHead, "<%" );
		if( pszScriptStart == NULL )		//	没有发现脚本 
		{
			if( *pszHead )					//	有数据
			{
				strTmp = pszHead;
				m_astrASPBlock.Add( strTmp );
				strTmp.Format("\n\nResponseWriteBlock(%d)\n\n",nBlockNo);
				strScriptBuf += strTmp;
			}
			break;							//	结束
		}
		*pszScriptStart ++ = 0;
		pszScriptStart ++;					//	跳过 <%
		char * pszScriptEnd = strstr( pszScriptStart, "%>" );
		if( pszScriptEnd == NULL )
			return false;
		* pszScriptEnd = 0;
		pszScriptEnd += 2;

		if( *pszHead )					//	有数据
		{
			strTmp = pszHead;
			m_astrASPBlock.Add( strTmp );
			strTmp.Format("\n\nResponseWriteBlock(%d)\n\n",nBlockNo);
			strScriptBuf += strTmp;
			nBlockNo ++;
		}			
		strScriptBuf += ( pszScriptStart );					       
		pszHead = pszScriptEnd;
	}
	ASSERT( nBlockNo == m_astrASPBlock.GetSize()-1 );
	strSrcASP.ReleaseBuffer( 0 );
	return (m_astrASPBlock.GetSize()>0);
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		Set request
/// 输入参数:
///		lpszRequest			文件名 & 请求
/// 返回参数:
///		新的文件名
CMyString CHTMLParser::SetRequestString(LPCSTR lpszRequest)
{
	CMyString strRetVal = lpszRequest;
	strRetVal.TrimLeft();
	m_strRequestString = strRetVal;
	int nPos = m_strRequestString.Find( '?' );
	if( nPos >= 0 )
	{
		m_strRequestString.Delete( 0, nPos+1 );		// 从文件名中提取参数
		strRetVal.ReleaseBuffer( nPos );
		strRetVal.TrimRight();
	}
	return strRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-4-29
/// 函数功能:
///		Register on Module call back
/// 输入参数:
///		无
/// 返回参数:
///		无
PFN_OnModuleCreateDelete CHTMLParser::SetOnModuleCreateDeleteCallBack( PFN_OnModuleCreateDelete pfn, DWORD dwUserData )
{
	PFN_OnModuleCreateDelete pRetVal = m_pfnOnModuleCreateDelete;
	m_pfnOnModuleCreateDelete = pfn;
	m_dwOnModuleCallBackUserData = dwUserData;
	return pRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-5-12
/// 函数功能:
///		register
/// 输入参数:
///		无
/// 返回参数:
///		无
extern "C" void MyHTMLRegisterModuleCallBack( CHTMLParser * pParser, PFN_OnModuleCreateDelete pFn, DWORD dwUserData )
{	
	ASSERT( pParser );
	if( pParser )
		pParser->SetOnModuleCreateDeleteCallBack( pFn, dwUserData );
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取所有链接个数
/// 输入参数:
///		无
/// 返回参数:
///		无
int CHTMLParser::GetLinkUrlCount()
{
	return m_aActionElements.GetSize();
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取当前有效链接到序号
/// 输入参数:
///		无
/// 返回参数:
///		无
int CHTMLParser::GetActiveLinkIndex()
{
	return m_nActiveLinkIndex;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		设置新的有效链接
/// 输入参数:
///		nNewIndex			新的链接数组下标
/// 返回参数:
///		原来的序号
int CHTMLParser::SetActiveLinkIndex(int nNewIndex)
{
	int nRetVal = m_nActiveLinkIndex;
	if( m_aActionElements.GetSize() )
	{	
		m_aActionElements[m_nActiveLinkIndex]->SetActiveLink( false );

		if( nNewIndex < 0 )
			m_nActiveLinkIndex = m_aActionElements.GetSize()-1;
		else if( nNewIndex >= m_aActionElements.GetSize() )
			nNewIndex = 0;

		m_nActiveLinkIndex = nNewIndex;

		m_aActionElements[m_nActiveLinkIndex]->SetActiveLink();		
	}

	return nRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-6-3
/// 函数功能:
///		获取指定序号的链接与ID
/// 输入参数:
///		nIndex		数组下标
///		pstrID		输出ID，缺省为 NULL
/// 返回参数:
///		指定序号的链接
CMyString CHTMLParser::GetLinkURL(int nIndex, CMyString *pstrID)
{
	CMyString strRetVal;
	if( nIndex < 0 || nIndex >= m_aActionElements.GetSize() )
		return strRetVal;

	if( pstrID )
	{	
		CHTMLElementsBase * pElement = m_aActionElements[nIndex]->GetElementOfLinkURL();
		if( pElement )
		{
			CHTML_A_Element * pA = (CHTML_A_Element *)pElement;
			*pstrID = pA->GetID();
		}
		else
			*pstrID = "";
	}

	return strRetVal;
}
