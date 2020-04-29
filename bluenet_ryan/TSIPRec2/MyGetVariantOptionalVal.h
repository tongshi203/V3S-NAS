// MyGetVariantOptionalVal.h: interface for the CMyGetVariantOptionalVal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYGETVARIANTOPTIONALVAL_H__D9B31F8E_249D_4C66_93B5_50D52BF3B83F__INCLUDED_)
#define AFX_MYGETVARIANTOPTIONALVAL_H__D9B31F8E_249D_4C66_93B5_50D52BF3B83F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyGetVariantOptionalVal  
{
public:
	static CString GetStringVarValue( VARIANT & varData, LPCSTR lpszDefValue=NULL );
	static PBYTE GetBufPtrFromVariant( IN VARIANT * pBuf, OUT PDWORD pdwBufLen = NULL );
	static int GetVarValue(VARIANT &varData, int nDefValue);
};

#endif // !defined(AFX_MYGETVARIANTOPTIONALVAL_H__D9B31F8E_249D_4C66_93B5_50D52BF3B83F__INCLUDED_)
