///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-12
///
///		用途：
///			TS 流解复用
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================


#if !defined(AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_)
#define AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_

#include "tspacket.h"

#pragma pack(push,4)

class CPID_ResponserMapper;

class CTSPacketDemux
{
public:
	void DeregisterResponser( WORD wPID );
	void RegisterResponser( WORD wPID, CTSPacketResponser * pResponser );	
	void PushOneTSPacket( PDVB_TS_PACKET pPacket );
	bool IsValid(){ return m_pMapper != NULL; }
	void Reset();

	CTSPacketDemux();
	virtual ~CTSPacketDemux();

private:
	CPID_ResponserMapper * m_pMapper;
};

#pragma pack(pop)

#endif // !defined(AFX_TSPACKETDEMUX_H__4496255B_82FA_4F32_A767_D45D6AD82BFD__INCLUDED_)
