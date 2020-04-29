///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2006-9-21
///
///		用途：
///			IP Over DVB MPE decoder
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// DVBPSITable_IP_MPE.h: interface for the CDVBPSITable_IP_MPE class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_)
#define AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_

#include "dvbpsitables.h"

#pragma pack(push,4)

class CDVBPSITable_IP_MPE : public CDVBPSITablesBase
{
public:
	CDVBPSITable_IP_MPE();
	virtual ~CDVBPSITable_IP_MPE();

	enum{
		MPE_TABLE_MAX_SIZE = 2*1024,		// 2K, 目前只考虑 IPv4 的以太帧，以后再考虑其他的
	};

	virtual void OnTableReceived();			//	接收到表了
	virtual PDVB_PSI_TABLE_BASE GetTableLastReceived();		//	读取上次接收到的表
	virtual void Reset(bool bForce=false);
	virtual bool IsValid();
	virtual bool IsPSIPacketIntegral();

	virtual void OnEthernetFrame( PBYTE pEthernetAddr, PBYTE pIPPacket, int nIPLen );

	virtual void Dump(FILE*fOutput=NULL);

private:
	CMyHeap	m_EthFrame;
	int m_nDataHeaderLen;		// IP 包头长度
	BYTE	m_abyEthAddr[6];	// 6 bytes ethernet addr
};


#pragma pack(pop)

#endif // !defined(AFX_DVBPSITABLE_IP_MPE_H__284625B0_07C2_411A_A11D_7AEAA33CC8B4__INCLUDED_)
