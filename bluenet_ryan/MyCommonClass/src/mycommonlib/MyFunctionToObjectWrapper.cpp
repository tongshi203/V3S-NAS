// MyBasicFuncObjWrappe.cpp: implementation of the CMyBasicFuncObjWrappe1 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyFunctionToObjectWrapper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma pack( push, 1 )
struct tagPushBPS
{
	BYTE	m_byPushEbp;			//	55			 push	 ebp
	WORD	m_wMovEbpEsp;			//	8b ec		 mov	 ebp, esp
};	
										//	50			push eax											
struct tagCallAndRet
{
	BYTE	m_byPushUseParam;		//	0x68		 push dword ptr of UserDefData
	DWORD	m_dwUserParam;

	BYTE	m_byCall;				//	e8			call Static function
	DWORD	m_dwFuncOff;			
	WORD	m_wMov_ESP_EBP;			//	8bh 0e5h	mov esp,ebp
	DWORD	m_dwPopEbpAndRet;		//	5d			pop	 ebp
									//	c2 0c 00	ret	 0ch										
};
#pragma pack(pop)

CMyFunctionToObjectWrapper::CMyFunctionToObjectWrapper(int nArgCount,DWORD dwUserData,void * pFunction, BOOL bIsPascalCall)
{
	int nByteNeed = sizeof(struct tagPushBPS) + sizeof(struct tagCallAndRet) + nArgCount*4;
	m_pThunkBuffer = new BYTE[nByteNeed];

	struct tagPushBPS *		pPushBPS = (struct tagPushBPS *	)m_pThunkBuffer;
	DWORD * pdwMovAndPush = (DWORD * )(m_pThunkBuffer + sizeof(struct tagPushBPS) );
	struct tagCallAndRet *	pCallAndRet = \
		(struct tagCallAndRet *)( m_pThunkBuffer + sizeof(struct tagPushBPS) + nArgCount*4 );

	pPushBPS->m_byPushEbp = 0x55;			//	55			 push	 ebp
	pPushBPS->m_wMovEbpEsp = 0xec8b;			//	8b ec		 mov	 ebp, esp

	for(int i=0; i<nArgCount; i++)
	{
		pdwMovAndPush[i] = 0x5000458b + (((nArgCount-i)*4+4)<<16);
	}

	pCallAndRet->m_byPushUseParam = 0x68;		//	0x68		 push dword ptr of UserDefData
	pCallAndRet->m_dwUserParam = dwUserData;

	pCallAndRet->m_byCall = 0xe8;				//	e8			call Static function

	pCallAndRet->m_dwFuncOff = (DWORD)pFunction - DWORD(&pCallAndRet->m_dwFuncOff) - 4;;
	pCallAndRet->m_wMov_ESP_EBP = 0xe58b;			//	8bh 0e5h	mov esp,ebp
	if( bIsPascalCall )
	{
		pCallAndRet->m_dwPopEbpAndRet = 0xc25d + nArgCount*0x40000;		//	5d			pop	 ebp
													//	c2 0c 00	ret	 0ch
	}
	else
	{
		pCallAndRet->m_dwPopEbpAndRet = 0xc35d;		//	5d			pop	 ebp
													//	c3	ret
	}
}

CMyFunctionToObjectWrapper::~CMyFunctionToObjectWrapper()
{
	if( m_pThunkBuffer )
		delete m_pThunkBuffer;
}

void * CMyFunctionToObjectWrapper::GetFucntionEntry()
{
	return (void*)m_pThunkBuffer;
}
