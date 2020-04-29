// MyBasicFuncObjWrappe.h: interface for the CMyBasicFuncObjWrappe1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYBASICFUNCOBJWRAPPE_H__3AD50391_13C1_4CC3_84F4_702A5B86ECC9__INCLUDED_)
#define AFX_MYBASICFUNCOBJWRAPPE_H__3AD50391_13C1_4CC3_84F4_702A5B86ECC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma pack(push,4)

class CMyFunctionToObjectWrapper
{
public:
	CMyFunctionToObjectWrapper(int nArgCount,DWORD dwUserData,void * pFunction, BOOL bIsPascalCall=false);
	virtual ~CMyFunctionToObjectWrapper();

	void * GetFucntionEntry();

private:
	BYTE * m_pThunkBuffer;
};

#pragma pack(pop)

#endif // !defined(AFX_MYBASICFUNCOBJWRAPPE_H__3AD50391_13C1_4CC3_84F4_702A5B86ECC9__INCLUDED_)
