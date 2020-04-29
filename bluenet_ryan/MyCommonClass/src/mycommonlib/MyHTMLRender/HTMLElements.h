///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2004-11-22
///
///		用途：
///			HTML 语言元素
///=======================================================

#if !defined(AFX_HTMLELEMENTS_H__82D17758_5835_4C9E_B22B_B51F4A88F942__INCLUDED_)
#define AFX_HTMLELEMENTS_H__82D17758_5835_4C9E_B22B_B51F4A88F942__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <MyString.h>
class CHTMLRender;
#ifdef _DEBUG
  #include <stdio.h>
#endif //_DEBUG

class CDC;
class CMyImageObject;

#define DEFAULT_ONE_LINE_HEIGHT		30
#define DEFAULT_WINDOW_WIDTH		800
#define DEFAULT_WINDOW_HEIGHT		600

#define	INVALID_COLOR_VALUE			0xFFFFFFFF

class CHTMLElementsBase
{
public:
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual bool SetParameter( CMyString & strParam );
	virtual void SetCurrentLineHeight( int nNewValue );
	virtual int OnNewLine();
	virtual int GetLineHeight();
	virtual CMyString GetLinkURL();
	virtual CHTMLElementsBase * GetElementOfLinkURL();
	virtual CMyString GetID(){ return CMyString(""); };
	CHTMLElementsBase( CHTMLElementsBase * pParent );
	virtual ~CHTMLElementsBase();

	CHTMLElementsBase * GetChild(){ return m_pChild; }
	CHTMLElementsBase * GetNext(){ return m_pNext; }
	CHTMLElementsBase * GetParent(){ return m_pParent; }
	void SetNext( CHTMLElementsBase * pNext );
	void AppendChild( CHTMLElementsBase * pChild );
	int GetX(){ return m_x; }
	int GetY(){ return m_y; }
	int GetWidth(){ return m_cx;}
	int GetHeight(){ return m_cy; }
	int Get_Z_Index(){ return m_z_index; }
	void Set_Z_Index(int nNewValue){ m_z_index = nNewValue; }	

	COLORREF GetValueAsColor( CMyString & strValue );
	CMyString GetOneParameterItem( CMyString & strParam, LPCSTR lpszItemName, char cSeparator );

	virtual void SetCoordinate( int x, int y, int cx, int cy );
	virtual LPCSTR GetTagName(){ return ""; };
	
	void SetCDC( CDC * pDC ){ m_pDC = pDC; }

#ifdef _DEBUG
public:	
	virtual void Dump();	
	CMyString FormatColor( COLORREF colorValue );
	CMyString m_strParam;
#endif //_DEBUG

public:
	void Preset();

protected:
	CHTMLElementsBase * m_pParent;
	CHTMLElementsBase * m_pChild;
	CHTMLElementsBase * m_pNext;
	int m_x;
	int m_y;
	int m_cx;
	int m_cy;
	int m_z_index;
	CDC *	m_pDC;
};

class CHTMLContainerBase : public CHTMLElementsBase
{
public:
	CHTMLContainerBase(CHTMLElementsBase * pParent);
	virtual ~CHTMLContainerBase();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual void SetCurrentLineHeight( int nNewValue );
	virtual int OnNewLine();
	virtual int GetLineHeight();
	void SetDefLineHeight( int nNewValue );

private:
	int	m_nCurrentLineHeight;
	int m_nDefLineHeight;
};

class CHTML_BODY_Element : public CHTMLContainerBase
{
public:
	CHTML_BODY_Element( CHTMLElementsBase * pParent );
	virtual ~CHTML_BODY_Element();

	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual bool SetParameter( CMyString & strParam );

	CMyString GetFileName();
	void SetImageData( PBYTE pBuf, int nLen );	
	virtual LPCSTR GetTagName(){ return "BODY"; };

#ifdef _DEBUG
public:
	virtual void Dump();	
#endif //_DEBUG

private:
	COLORREF	m_colorBg;
	COLORREF	m_colorText;
	COLORREF	m_colorALink;
	COLORREF	m_colorVLink;			//	current active link color
	CMyString	m_strBgImgFileName;

	CMyImageObject * m_pImageObj;
};

class CHTML_Dummy_Element : public CHTMLContainerBase
{
public:
	CHTML_Dummy_Element(CHTMLElementsBase * pParent, LPCSTR lpszTagName );
	virtual ~CHTML_Dummy_Element();
	virtual LPCSTR GetTagName(){ return (char*)m_strTagName; }

#ifdef _DEBUG
public:
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("<%s>  %s\n", (LPCSTR)m_strTagName, (LPCSTR)m_strParam); }	
#endif //_DEBUG

private:
	CMyString m_strTagName;
};

//------------------------------------------------
//	Linked Element Base
class CHTMLLinkedElementBase : public CHTMLElementsBase
{
public:
	CHTMLLinkedElementBase(CHTMLElementsBase * pParent) : CHTMLElementsBase(pParent)
	{ m_bIsActiveLink = false; }
	virtual ~CHTMLLinkedElementBase(){};
	bool	IsActiveLink(){ return m_bIsActiveLink; }
	void	SetActiveLink( bool bActive=true ){ m_bIsActiveLink=bActive; }
protected:
	bool		m_bIsActiveLink;		//	当前有效的连接
};


//------------------------------------------------
//	Text element
class CHTMLTextElement : public CHTMLLinkedElementBase
{
public:
	CHTMLTextElement(CHTMLElementsBase * pParent);
	virtual ~CHTMLTextElement();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual bool SetParameter( CMyString & strParam );
	

#ifdef _DEBUG
public:
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("\"%s\"\n", (char*)m_strText.Left(20)); }
#endif //_DEBUG

private:
	CMyString m_strText;	
	bool		m_bIsPreparedFormat;	//	预定格式
};

//------------------------------------------------
//	A element
class CHTML_A_Element : public CHTMLElementsBase
{
public:
	CHTML_A_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_A_Element();

	virtual bool SetParameter( CMyString & strParam );
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual CMyString GetLinkURL(){ return m_strHRef; }
	virtual CHTMLElementsBase * GetElementOfLinkURL() { return this; }
	virtual LPCSTR GetTagName(){ return "A"; }
	virtual CMyString GetID(){ return m_strID; }

#ifdef _DEBUG
public:	
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("<A  href=%s>\n", (LPCSTR)m_strHRef); }	
#endif //_DEBUG

private:
	CMyString	m_strHRef;
	CMyString	m_strID;
	COLORREF	m_OldTextColor;
	bool		m_bOldUnderLine;
	bool		m_bShowUnderLine;		//  CYJ,2005-7-20 是否显示下划线
};

//------------------------------------------------
// DIV element
class CHTML_DIV_Element : public CHTMLContainerBase
{
public:
	CHTML_DIV_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_DIV_Element();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual bool SetParameter( CMyString & strParam );
	virtual void SetCoordinate( int x, int y, int cx, int cy ){}	//	指定位置，所以不需要也不能再设置
	virtual LPCSTR GetTagName(){ return "DIV"; };

#ifdef _DEBUG
public:
	virtual void Dump()
	{
		CHTMLElementsBase::Dump(); 
		TRACE("<DIV left:%d; top:%d; width:%d; height:%d; z-index:%d>\n", m_x, m_y, m_cx, m_cy, m_z_index ); 
	}	
#endif //_DEBUG

private:	
	int m_nOld_x;
	int m_nOld_y;
};

//------------------------------------------------
// BR element
class CHTML_BR_Element : public CHTMLElementsBase
{
public:
	CHTML_BR_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_BR_Element();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);

#ifdef _DEBUG
public:
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("<BR>\n"); }
#endif //_DEBUG


};

//------------------------------------------------
// IMAGE element
class CHTML_IMAGE_Element : public CHTMLLinkedElementBase
{
public:
	CHTML_IMAGE_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_IMAGE_Element();
	virtual bool SetParameter( CMyString & strParam );
	virtual void SetCoordinate( int x, int y, int cx, int cy )
	{	m_x = x; m_y = y; }		//	图形的大小，有图形本身决定，不能修改

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	CMyString GetFileName();
	CMyString GetActiveImgFileName();
	void SetImageData( PBYTE pBuf, int nLen, bool bNormal = true );	
	virtual LPCSTR GetTagName(){ return "IMAGE"; };

#ifdef _DEBUG
public:
	virtual void Dump()
	{
		CHTMLElementsBase::Dump(); 
		if( m_strActiveURLFileName.IsEmpty() )
			TRACE("<IMAGE  src=%s>\n", (char*)m_strFileName ); 
		else
			TRACE("<IMAGE  src=%s activesrc=%s>\n", (char*)m_strFileName, (char*)m_strActiveURLFileName ); 
	}	
#endif //_DEBUG

private:
	CMyString	m_strFileName;			// image file name
	CMyString	m_strActiveURLFileName;
	CMyImageObject	*	m_pImageObj;
	CMyImageObject	*	m_pImageObjActive;
};

//------------------------------------------------
// FONT element
class CHTML_FONT_Element : public CHTMLElementsBase
{
public:
	CHTML_FONT_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_FONT_Element();

	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual bool SetParameter( CMyString & strParam );
	virtual LPCSTR GetTagName(){ return "FONT"; };


#ifdef _DEBUG
public:
	virtual void Dump()
	{ 
		CHTMLElementsBase::Dump(); 
		CMyString strTmp = FormatColor(m_Color);
		TRACE("<FONT   size=%d, color=%s>\n", m_nSize, (LPCSTR)strTmp );
	}	
#endif //_DEBUG

private:
	int			m_nSize;
	COLORREF	m_Color;

	int			m_nOldFontSize;
	COLORREF	m_OldColor;
};

//------------------------------------------------
// U element
class CHTML_U_Element : public CHTMLElementsBase
{
public:
	CHTML_U_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_U_Element();

	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual LPCSTR GetTagName(){ return "U"; };

#ifdef _DEBUG
public:
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("<U>\n"); }	
#endif //_DEBUG

private:
	bool	m_bOldValue;
};

//------------------------------------------------
// B element
class CHTML_B_Element : public CHTMLElementsBase
{
public:
	CHTML_B_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_B_Element();

	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual LPCSTR GetTagName(){ return "B"; };

#ifdef _DEBUG
public:
	virtual void Dump(){ CHTMLElementsBase::Dump(); TRACE("<B>\n"); }	
#endif //_DEBUG

private:
	bool	m_bOldValue;
};

//------------------------------------------------
// LINE element
class CHTML_LINE_Element : public CHTMLElementsBase
{
public:
	CHTML_LINE_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_LINE_Element();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual LPCSTR GetTagName(){ return "LINE"; };
	virtual bool SetParameter( CMyString & strParam );

#ifdef _DEBUG
public:
	virtual void Dump()
	{		
		CHTMLElementsBase::Dump(); 
		CMyString strTmp = FormatColor(m_Color);
		TRACE("<LINE x1=%d y1=%d x2=%d y2=%d width=%d color=%s>\n", m_x1, m_y1, m_x2, m_y2, m_nWidth, (char*)strTmp ); 
	}	
#endif //_DEBUG

private:
	int m_x1;
	int m_y1;
	int m_x2;
	int m_y2;
	int m_nWidth;
	COLORREF m_Color;
};

//------------------------------------------------
// RECT element
class CHTML_RECT_Element : public CHTMLElementsBase
{
public:
	CHTML_RECT_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_RECT_Element();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual LPCSTR GetTagName(){ return "RECT"; };
	virtual bool SetParameter( CMyString & strParam );

#ifdef _DEBUG
public:
	virtual void Dump()
	{		
		CHTMLElementsBase::Dump(); 
		CMyString strTmp = FormatColor(m_Color);
		TRACE("<RECT x1=%d y1=%d x2=%d y2=%d color=%s>\n", m_x1, m_y1, m_x2, m_y2, (char*)strTmp ); 
	}	
#endif //_DEBUG

private:
	int m_x1;
	int m_y1;
	int m_x2;
	int m_y2;
	COLORREF m_Color;
};

class CHTML_P_Element : public CHTMLElementsBase
{
public:
	CHTML_P_Element(CHTMLElementsBase * pParent);
	virtual ~CHTML_P_Element();

	virtual void OnRender(CHTMLRender * pRender, int & x, int & y);
	virtual void OnEndRender(CHTMLRender * pRender, int & x, int & y);
	virtual LPCSTR GetTagName(){ return "P"; };
	virtual bool SetParameter( CMyString & strParam );

#ifdef _DEBUG
public:
	virtual void Dump()
	{		
		CHTMLElementsBase::Dump(); 
		CMyString strTmp = FormatColor(m_Color);
		CMyString strBkColor = FormatColor(m_BkColor);
		TRACE("<P Style=\"COLOR:%s, BACKGROUND-COLOR:%s\">\n", (char*)strTmp, (char*)strBkColor );
	}	
#endif //_DEBUG

private:
	COLORREF	m_Color;
	COLORREF	m_BkColor;
	COLORREF	m_OldColor;
	COLORREF	m_OldBkColor;
	int			m_nOldOutputBkMode;	
};

extern void EnableGlobalImageCache( bool bValue );

#endif // !defined(AFX_HTMLELEMENTS_H__82D17758_5835_4C9E_B22B_B51F4A88F942__INCLUDED_)
