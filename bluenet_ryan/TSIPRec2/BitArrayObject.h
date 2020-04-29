// BitArrayObject.h: interface for the CBitArrayObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITARRAYOBJECT_H__2FA9A446_7B28_42DE_9105_6CCA129100B2__INCLUDED_)
#define AFX_BITARRAYOBJECT_H__2FA9A446_7B28_42DE_9105_6CCA129100B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBitArrayObject  
{
public:
	void CleanDataOnly();
	CDWordArray & GetUserDefData();
	void Reset();
	CBitArrayObject();
	virtual ~CBitArrayObject();

	long GetSubFileHasReceived();
	long GetTotalSubFileCount();
	BOOL IsSubFileOK(long nIndex);
	PBYTE GetDataBuffer();
	BOOL SetTotalSubFileCount(long nNewValue);
	long SetBitValue(int nIndex, int nBitValue );
	void Prepare();
	long GetNextFileID(int nBitValue );
	long ReStat();
	long Combine(CBitArrayObject * pSrcObj);
	long LoadFromFile( LPCSTR lpszFileName );
	BOOL SaveToFile( LPCSTR lpszFileName );
	CBitArrayObject & operator =( CBitArrayObject & src );

protected:
	CByteArray	m_BitmapArray;
	int			m_nBit_1_Count;			//	比特为 1 的个数
	int			m_nTotalBitCount;		//	比特总数
	int			m_nNextFileIDPtr;		//  下一个文件的比特数
	CDWordArray	m_adwUserDefData;		//	用户自定义数据
};

#endif // !defined(AFX_BITARRAYOBJECT_H__2FA9A446_7B28_42DE_9105_6CCA129100B2__INCLUDED_)
