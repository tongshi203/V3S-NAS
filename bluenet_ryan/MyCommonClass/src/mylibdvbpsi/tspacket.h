///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2005-1-11
///
///		用途：
///			TS 分组解释
///
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

#ifndef __TS_PACKET_H_20050111__
#define __TS_PACKET_H_20050111__

#pragma pack(push,1)

#define DVB_TS_PACKET_SIZE	188
#define INVALID_PID			0x1FFF

#ifndef __MY_PACKTED__
	#ifdef _WIN32
		#define __MY_PACKTED__
	#else
		#define	__MY_PACKTED__ __attribute__ ((packed)) 
	#endif //_WIN32 
#endif // __MY_PACKTED__

typedef struct tagDVB_TS_PACKET
{
	BYTE	m_abyData[DVB_TS_PACKET_SIZE];

public:
	//	Is sychronized, a TS Packet should be leaded by 0x47
	inline bool IsTSPaket() const { return 0x47 == m_abyData[0]; }

	//	Get PID of this TS Packet
	inline WORD	GetPID()const { return ((m_abyData[1]&0x1F)<<8) | m_abyData[2]; }

	// Is this TS Packet has error occured
	inline bool IsError()const { return (m_abyData[1] & 0x80) != 0; }

	// Is payload unit start
	inline bool IsPayloadUnitStart()const { return (m_abyData[1] & 0x40) != 0; }

	// Is priority
	inline bool IsTransportPriority()const { return (m_abyData[1] & 0x20) != 0; }

	// Get Transport scrambling control
	inline BYTE GetScramblingControl()const { return (m_abyData[3]>>6)&3; }

	// Get Adaptation field control
	inline BYTE GetAdaptationControl()const { return (m_abyData[3]>>4)&3; }

	// Get Continuity counter
	inline BYTE GetContinuityCount()const { return m_abyData[3] & 0xF; }

	//	get payload data buffer and its length
	// Input Parameter:
	//		nDataLen			output payload data length
	// Output Parameter:
	//		Payload data buffer address, NULL no payload
	inline PBYTE GetPayloadData( int & nDataLen ) 
	{
		if( GetPID() == 0x1FFF || !(m_abyData[3] & 0x10) )
		{
			nDataLen = 0;
			return NULL;			//	no payload data
		}
		if( m_abyData[3] & 0x20 )
		{
			nDataLen = DVB_TS_PACKET_SIZE - 5 - m_abyData[4];			
			if( nDataLen <= 0 )
			{
				nDataLen = 0;
				return NULL;
			}
			return m_abyData + 5 + m_abyData[4];	// 跳过包头&Adaptation field
		}
		else
		{
			nDataLen = DVB_TS_PACKET_SIZE - 4;
			return m_abyData + 4;
		}
	}
}__MY_PACKTED__ DVB_TS_PACKET,*PDVB_TS_PACKET;


class CTSPacketResponser
{
public:
	virtual void PushOneTSPacket( PDVB_TS_PACKET pPacket ) = 0;	//	接收到一个TS分组
};

#pragma pack(pop)

#endif // __TS_PACKET_H_20050111__
