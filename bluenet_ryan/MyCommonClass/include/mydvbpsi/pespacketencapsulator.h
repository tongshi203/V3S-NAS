///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-2-7
///
///		用途：
///			PES 封装
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// PESPacketEncapsulator.h: interface for the PESPacketEncapsulator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PESPACKETENCAPSULATOR_H__9C4A5894_B9AA_498D_9C5C_2D46E0995157__INCLUDED_)
#define AFX_PESPACKETENCAPSULATOR_H__9C4A5894_B9AA_498D_9C5C_2D46E0995157__INCLUDED_

#include "tspacket.h"	// Added by ClassView
#include "tspacketencapsulator.h"

#pragma pack(push,4)

class CMyBitStream;

///////////////////////////////////////////////////////////////////////
// PES 标志位编码，不包括PES头部分6个字节
class CPESPacketFlagsEncapsulator
{
public:
	CPESPacketFlagsEncapsulator();
	void Preset();
	PBYTE Build( int & nLen );
	void SetPTS( LONGLONG llPTS);
	void SetPTSDTS( LONGLONG llPTS, LONGLONG llDTS );
	void SetESCR( LONGLONG llESCR, WORD wExtension );
	void SetESRate(DWORD dwESRate);
	void SetCRC( WORD wLastPESCRC );

public:
	BYTE m_byPESScramblingControl;		// 2 比特有效，缺省为0
	bool m_bPES_Priority;				// 缺省为 0
	bool m_bDataAlignmentIndicator;		// 缺省为 0
	bool m_bCopyRight;					// 缺省为 0
	bool m_bOriginalOrCopy;				// 缺省为 1

private:
	BYTE	m_abyData[128];
	
	BYTE m_byPTS_DTS_Flag;				// 缺省为 0
	bool m_bESCRFlag;					// 缺省为 0
	bool m_bESRateFlag;					// 缺省为 0

	LONGLONG	m_llPTS;
	LONGLONG	m_llDTS;
	LONGLONG	m_llESCR;
	WORD		m_wESCRExtension;
	DWORD		m_dwESRate;

	bool		m_bHasCRC;				// 是否有CRC
	WORD		m_wLastPESCRC;			// 上一帧PES数据的CRC16

private:
	void OutputPTS_DTS( CMyBitStream * pOutBs );
	void OutputESCR( CMyBitStream * pOutBs );
};

class CPESPacketEncapsulator : public CDVBPacketEncapsulatorBase
{
public:
	int Encapsulate( PBYTE pESData, int nLen, BYTE byStreamID );
	void SetPCR( LONGLONG llPCR, WORD wExtension );
	void SetPTS( LONGLONG llPTS );
	void SetPTSDTS(LONGLONG llPTS, LONGLONG llDTS);
	void SetESCR( LONGLONG llESCR, WORD wExtension );
	void SetRandomAccess( bool bValue = true );
	CPESPacketEncapsulator();
	virtual ~CPESPacketEncapsulator();
	void DoCalculateCRC( bool bCalculate=true ){ m_bCalculateCRC = bCalculate; }
	bool IsDoCalculateCRC()const{return m_bCalculateCRC;}

private:	
	int EncapsulateFirstPESPacket(PBYTE pESData, int nLen, BYTE byStreamID);
	int EncapsulateLastPESPacket(PBYTE pESData, int nLen);

private:
	bool	m_bHasAdationField;
	CTSAdaptionFieldEncapsulate	m_AdaptionEncapsulator;
	CPESPacketFlagsEncapsulator	m_PESPacketFlagsEncapsulator;
	bool	m_bCalculateCRC;
	WORD	m_wLastPESCRC;
};

#pragma pack(pop)

#endif // !defined(AFX_PESPACKETENCAPSULATOR_H__9C4A5894_B9AA_498D_9C5C_2D46E0995157__INCLUDED_)
