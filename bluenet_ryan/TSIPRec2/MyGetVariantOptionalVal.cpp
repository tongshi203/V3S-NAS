// MyGetVariantOptionalVal.cpp: implementation of the CMyGetVariantOptionalVal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyGetVariantOptionalVal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//	获取缓冲区地址信息
//	入口参数
//		pBuf				输入地址，可能为 BYTE * 数组或 一维SafeArray类型
//		pdwBufLen			输出缓冲区长度，若是 SafeArray 类型
PBYTE CMyGetVariantOptionalVal::GetBufPtrFromVariant(VARIANT *pBuf, PDWORD pdwBufLen/*=NULL*/)
{
	ASSERT( pBuf );
	if( pdwBufLen )
		*pdwBufLen = 0;

	if( V_VT( pBuf) == VT_ERROR || V_ERROR(pBuf) == DISP_E_PARAMNOTFOUND || V_VT(pBuf)==VT_EMPTY )
		return NULL;			//	错误

	if( pBuf->vt == (VT_UI1|VT_ARRAY|VT_BYREF) )
		return pBuf->pbVal;

	ASSERT( FALSE );
	AfxMessageBox("没有实现");

	return NULL;
}

//	获取数据
int CMyGetVariantOptionalVal::GetVarValue(VARIANT &varData, int nDefValue)
{
	CComVariant varTmp( varData );
	if( V_VT( &varTmp) == VT_ERROR || V_ERROR( &varTmp) == DISP_E_PARAMNOTFOUND || V_VT(&varTmp)==VT_EMPTY )
		return nDefValue;

	::VariantCopyInd( &varTmp, &varTmp );
	varTmp.ChangeType( VT_I4 );
	return varTmp.lVal;
}

//	获取字符串
CString CMyGetVariantOptionalVal::GetStringVarValue(VARIANT &varData, LPCSTR lpszDefValue)
{
	CString strRetVal;	

	CComVariant varTmp( varData );
	if( V_VT( &varTmp) == VT_ERROR || V_ERROR( &varTmp) == DISP_E_PARAMNOTFOUND || V_VT(&varTmp)==VT_EMPTY )
	{
		if( lpszDefValue )
			strRetVal = lpszDefValue;
		return strRetVal;
	}

	::VariantCopyInd( &varTmp, &varTmp );
	varTmp.ChangeType( VT_BSTR );
	strRetVal = varTmp.bstrVal;
	return strRetVal;
}
