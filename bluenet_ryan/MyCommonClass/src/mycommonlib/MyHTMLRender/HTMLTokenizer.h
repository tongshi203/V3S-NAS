///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-22
///
///		用途：
///			自定义HTML字符分析器
///=======================================================

#if !defined(AFX_HTMLTOKENIZER_H__E91F85D0_5379_4D35_9A8C_FED3CE8BFA96__INCLUDED_)
#define AFX_HTMLTOKENIZER_H__E91F85D0_5379_4D35_9A8C_FED3CE8BFA96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyString.h>

typedef enum
{
	HTML_TAG_RIGHT_SEPARATOR = 0xF000,

	HTML_TAG_UNKNOWN = 0xFF00,
	HTML_TAG_ERROR,
	HTML_TAG_END,	

	HTML_TAG_TEXT = 0,
	HTML_TAG_A,
	HTML_TAG_DIV,
	HTML_TAG_BR,
	HTML_TAG_HTML,
	HTML_TAG_TITLE,
	HTML_TAG_BODY,	
	HTML_TAG_IMAGE,	
	HTML_TAG_FONT,
	HTML_TAG_U,
	HTML_TAG_B,	
	HTML_TAG_PRE,
	HTML_TAG_LINE,
	HTML_TAG_RECT,
	HTML_TAG_P,
	HTML_TAG_COMMENT,
}HTML_ELEMENT_ENUM;

class CHTMLTokenizer  
{
public:
	HTML_ELEMENT_ENUM NextToken( CMyString & strParam );
	void SetText( LPCSTR lpszHTML, int nLen );
	CHTMLTokenizer();
	virtual ~CHTMLTokenizer();


private:
	HTML_ELEMENT_ENUM GetTagType( CMyString & strTag );
	CMyString GetString( bool bModifyBeginPtr = true );
	void DoNormalText( CMyString & strParam );
	void SkipBlankChar();
	CMyString	m_strSrcHTML;
	char *		m_pszBegin;
	char *		m_pszNextBegin;
};

#endif // !defined(AFX_HTMLTOKENIZER_H__E91F85D0_5379_4D35_9A8C_FED3CE8BFA96__INCLUDED_)
