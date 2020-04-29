// UNLZSS32.h: interface for the CUNLZSS32 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNLZSS32_H__D13A7585_F633_11D2_B30C_00C04FCCA334__INCLUDED_)
#define AFX_UNLZSS32_H__D13A7585_F633_11D2_B30C_00C04FCCA334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UnCompressObj.h"

class CUNLZSS32 : public CUnCompressObj
{
public:
	CUNLZSS32();
	virtual ~CUNLZSS32();

	virtual PBYTE DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf=NULL);
	virtual int GetFileInfo(int nFileNo,CFileStatus & outfStatus);
	virtual int GetFileNum();
	virtual int Attach(int nFileLen,PBYTE pBuf);
	virtual DWORD GetCompressMethod();
	virtual int GetDecoderVersion();
	virtual void FreeMemory();

	enum {	CURRENT_UNLZSS_VERSION = 0x100,
			UNLZSS_MINOR_VER = 1,
	};
private:
	enum {	N	= 4096,			// size of ring buffer 
			F	=  18,			// upper limit for match_length
		THRESHOLD =	2,			// encode string into position and length
								// if match_length is greater than this 
			NIL	=	N,			// index for root of binary search trees 
	};
	enum { OUT_TEXT_BUF_SIZE = F*6 };
private:
	void Decode();
	PBYTE m_pOutBufAutoAlloc;					//	自动分配的内存
};

#endif // !defined(AFX_UNLZSS32_H__D13A7585_F633_11D2_B30C_00C04FCCA334__INCLUDED_)
