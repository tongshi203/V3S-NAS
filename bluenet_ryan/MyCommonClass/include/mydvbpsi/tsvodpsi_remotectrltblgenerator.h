///=======================================================
///
///     作者：陈永健
///     西安通视
///     日期：2007-1-17
///
///		用途：
///			通视 VOD 点播远程控制表
///
///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///!	该文件仅限在“通视”内部使用!	  			     !
///!													 !
///!	我不保证该文件的百分之百正确!					 !
///!	若要使用该文件，您需要承担这方面的风险!			 !
///=======================================================

// TSVODPSI_RemoteCtrlTblGenerator.h: interface for the CTSVODPSI_RemoteCtrlTblGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_)
#define AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_

#include "tsvodpsi_tablegenerator.h"

#ifndef MY_LONG64
	#ifdef _WIN32
		typedef __int64	MY_LONG64;
	#else
		typedef long long MY_LONG64;
	#endif //_WIN32
#endif // MY_LONG64

#pragma pack(push,4)

class CTSVODPSI_RemoteCtrlTblGenerator : public CTSVODPSI_TableGeneratorBase
{
public:
	CTSVODPSI_RemoteCtrlTblGenerator(BYTE byTableID, bool bDoCompress = false);
	virtual ~CTSVODPSI_RemoteCtrlTblGenerator();

	virtual PBYTE GetPrivateData( int & nOutLen );

	void Initialize();
	void SetSID( WORD wSID );
	void SetSTBID( MY_LONG64 llSTBID, MY_LONG64 llEndSTBID = 0);
	void SetSTBID( BYTE abySTBID[8] );
	void SetEncryptParameter( WORD wParameter );
	void CleanInstructions();
	int AddIns_SwitchChannel( BYTE byChNo );					// 切换频道
	int AddIns_ReceiveProgram( BYTE byPhysNo, WORD wPMT_PID );	// 收看节目
	int AddIns_VODOperatorResponse( PBYTE pBuf, BYTE byDataLen );	// VOD 点播返回值
	int AddIns_PrecompiledInstructions( WORD wCommand, PBYTE pBuf, int nLen );

	enum{
		BUFSIZE_FOR_TBL_HEADER = 100,	// 保留 100 字节用于表头
	};

	enum
	{
		RCMDID_SWITCH_CHANNEL = 1,				// 切换频道
		RCMDID_OSD_SHOW_TEXT,					// 显示文字
		RCMDID_RECEIVE_PROGRAM,					// 接收节目
		RCMDID_RECOMMEND_PROGRAM,				// 推荐节目
		RCMDID_VOD_OPERATION_RESPONSE,			// 点播返回值
	};
	
private:
	WORD	m_wSID;					// 如果 != 0 ，则表示该表是针对一个SID的，反之，针对一个STB ID
	
	MY_LONG64	m_llSTBID;
	MY_LONG64  m_llEndSTBID;
	
	PBYTE	m_pDataBuf;
	int		m_nBufSize;
	int		m_nDataLen;

	int		m_nInstructionCount;
	DWORD	m_dwEncryptParameter;

private:
	bool	AcquireMem( int nIncBytes );
};

#pragma pack(pop)

#endif // !defined(AFX_TSVODPSI_REMOTECTRLTBLGENERATOR_H__5D60638D_17A7_4AAB_8110_3E43A2C55975__INCLUDED_)
