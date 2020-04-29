// TSVODPSI_TableGenerator.h: interface for the CTSVODPSI_TableGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSVODPSI_TABLEGENERATOR_H__6FA7974C_F0E1_4887_B0ED_78650605F9EF__INCLUDED_)
#define AFX_TSVODPSI_TABLEGENERATOR_H__6FA7974C_F0E1_4887_B0ED_78650605F9EF__INCLUDED_

#include "psitablegenerator.h"

#pragma pack(push,4)

class CTSVODPSI_TableGeneratorBase : public CDVBPSI_TableGeneratorBase
{
public:
	CTSVODPSI_TableGeneratorBase( BYTE byTableID, bool bDoCompress = false );
	virtual ~CTSVODPSI_TableGeneratorBase();

	bool Build();
	virtual PBYTE GetPrivateData( int & nOutLen ) = 0;
	virtual void Preset();
	virtual bool SetModifyFlag( bool bModified );

protected:
	BYTE	m_byBuildCounter;
	bool	m_bDoCompressed;
	BYTE	m_byTableID;
	PBYTE	m_pbyOutBuf;
	int		m_nOutBufLen;			// 实际输出的字节数
	DWORD	m_dwOutBufSize;			// 实际输出地缓冲区大小
	bool	m_bIsModified;
};

#pragma pack( push, 1 )
typedef struct tagTONGSHI_VOD_IDENTITY_TBL
{
	char	m_acIdentity[10];
	DWORD	m_dwRootChFreq_kHZ;
	DWORD	m_dwReserved_1;
	BYTE	m_abyData[514];			// 接入 IP 和 电话
}TONGSHI_VOD_IDENTITY_TBL,*PTONGSHI_VOD_IDENTITY_TBL;
#pragma pack( pop )

///////////////////////////////////////////////////////////////////////
// CTSVODPSI_VOD_IdentityTableGenerator
class CTSVODPSI_VOD_IdentityTableGenerator : public CTSVODPSI_TableGeneratorBase
{
public:
	CTSVODPSI_VOD_IdentityTableGenerator();
	virtual PBYTE GetPrivateData( int & nOutLen );
	DWORD	SetRootChFreq( DWORD dwFreqInKHz );	
	bool SetServiceIPAndPort( LPCSTR lpszIP, int nPort );
	void SetServiceTelephoneNo( LPCSTR lpszTelephoneNo );
	void Preset();

private:
	TONGSHI_VOD_IDENTITY_TBL	m_PrivateData;
	int m_nPrivateDataLen;
	CMyString m_strIPAndPort;
	CMyString m_strTelephoneNo;
};

#pragma pack(pop)

#endif // !defined(AFX_TSVODPSI_TABLEGENERATOR_H__6FA7974C_F0E1_4887_B0ED_78650605F9EF__INCLUDED_)
