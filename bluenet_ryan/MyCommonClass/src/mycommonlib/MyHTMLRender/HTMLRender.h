///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-23
///
///		用途：
///			HTML 显示
///=======================================================

#if !defined(AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_)
#define AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MyString.h>

class CHTMLParser;
class CHTMLElementsBase;

#if defined(__FOR_MICROWIN_EMBED__)||defined(__FOR_MY_OSD_EMBED__)
   #include <mygdiobjs.h>
#endif //__FOR_MICROWIN_EMBED__

class CHTMLRender  
{
public:
	int SetFontSize(int nNewSize=5);
	int GetFontSize(){ return m_nFontSize; }
	CDC * GetDC(){ return m_pDC; }
	void SetDC( CDC * pDC ){m_pDC = pDC;}
	CHTMLRender(CDC * pDC);
	virtual ~CHTMLRender();

	void Render( CHTMLParser * pParser, int nWidth, int nHeight );

	void SetBold( bool bBold ){ m_bIsBold=bBold; }
	bool IsBold(){ return m_bIsBold; }

	void SetUnderLine( bool bUnderLine ){ m_bIsUnderLine = bUnderLine; }
	bool IsUnderLine(){ return m_bIsUnderLine; }

	COLORREF	GetTextColor(){ return m_colorText; }
	void		SetTextColor( COLORREF newColor );
	COLORREF	GetLinkTextColor(){ return m_colorLinkText; }
	void		SetLinkTextColor( COLORREF newColor ){ m_colorLinkText=newColor; }
	COLORREF	GetActiveLinkTextColor(){ return m_colorActiveLink; }
	void		SetActiveLinkTextColor( COLORREF newColor ){ m_colorActiveLink=newColor; }
	int			GetWidth(){return m_nWinWidth;}
	int			GetHeight(){return m_nWinHeight;}

private:
	void RenderOneItem( CHTMLElementsBase * pElement, int & x, int & y );

private:
	CFont		m_Font_x_12;		//	小字体，12x12
	CFont		m_Font_x_16;		//	大字体，16x16
	int			m_nFontSize;		//	当前使用的字体大小
	CFont *		m_pFont;			//	当前使用的字体指针
	CPen		m_UnderLinePen;		//	用于画下划线，其颜色与当前Text相同，且在改变TextColor时改变

	CDC *		m_pDC;				//	画图对象
	CHTMLParser *	m_pParser;
	
	COLORREF	m_colorText;		//	普通字体颜色
	COLORREF	m_colorLinkText;	//	连接到字体颜色
	COLORREF	m_colorActiveLink;	//	当前连接字体

	CMyString	m_strTitle;			//	标题	

	bool		m_bIsUnderLine;		//	是否有下划线
	bool		m_bIsBold;			//	是否粗体

	int			m_nWinWidth;
	int			m_nWinHeight;	
};

#endif // !defined(AFX_HTMLRENDER_H__2E7426CC_9FE3_409F_A222_3BEEB4EF81D8__INCLUDED_)
