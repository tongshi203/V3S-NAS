///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-2-7
///
///		用途：
///			封装ＴＳ分组
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __TSPACKET_ENCAPSULATE_H_20050207__
#define __TSPACKET_ENCAPSULATE_H_20050207__

#include "tspacket.h"

#pragma pack(push,4)

class CTSAdaptionFieldEncapsulate
{
public:
	CTSAdaptionFieldEncapsulate();
	void SetPCR( LONGLONG llPCR, WORD wExtension );
	void SetOPCR( LONGLONG llPCR, WORD wExtension );
	void SetSplicingPoint( BYTE byValue );
	void SetDiscontinuity( bool bValue=true ){ m_bDiscontinuity = bValue; }
	void SetRandomAccess( bool bValue=true ){ m_bRandomAccess = bValue; }	
	void SetElementStreamPrority( bool bValue=true){ m_bElementStreamPrority = bValue; }
	
	void Preset();
	PBYTE Build( int & nLen );

private:
	BYTE	m_abyDataBuf[100];	
	bool	m_bPCR_Flag;
	LONGLONG m_llPCR;
	WORD	m_wPCR_Extension;

	bool	m_bOPCR_Flag;
	LONGLONG m_llOPCR;
	WORD	m_wOPCR_Extension;

	bool	m_bSplicingPointFlag;
	BYTE	m_bySplicingPoint;

	bool	m_bDiscontinuity;
	bool	m_bRandomAccess;
	bool	m_bElementStreamPrority;
};

class CTSPacketEncapsulator : public DVB_TS_PACKET
{
public:
	CTSPacketEncapsulator();
	void Preset();
	void SetPID( WORD wPID );
	void SetPayloadStartIndicator( BOOL bStart = true );
	void SetAdaptionField( BYTE byValue, BYTE byLen, PBYTE pBuf = NULL );
	void SetContinuity( BYTE byValue );
};

class CDVBPacketEncapsulatorBase
{
public:
	CDVBPacketEncapsulatorBase();
	WORD SetPID(WORD wPID);
	//	分配一个TS分组，返回 NULL 失败
	virtual PDVB_TS_PACKET AllocateTSPacketBuffer() = 0;
	//	一个TS分组完成
	virtual void OnTSPacketReady( PDVB_TS_PACKET pTSPacket ) = 0;
	CTSPacketEncapsulator * GetTSPacket();

protected:
	WORD	m_wPID;
	BYTE	m_byTSContinuity;
};

#pragma pack(pop)

#endif // __TSPACKET_ENCAPSULATE_H_20050207__
