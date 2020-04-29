// DVBEPGReceiver.cpp: implementation of the CDVBEPGReceiver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPRecSvr.h"
#include "DVBEPGReceiver.h"
#ifndef _WIN32
	#include <MyArchive.h>
	#include <MyMemFile.h>
#endif //_WIN32

#ifdef _WIN32
	#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
	#endif
#endif //


#ifdef _WIN32
	extern "C" IDVB_EPG_Receiver * WINAPI Create_DVB_EPG_Receiver(void)
#else
	extern "C" IDVB_EPG_Receiver * Create_DVB_EPG_Receiver(void)
#endif //_WIN32
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	CDVBEPGReceiver * pInstance = new CDVBEPGReceiver;
	if( NULL == pInstance )
		return NULL;
	pInstance->AddRef();

	return static_cast<IDVB_EPG_Receiver*>(pInstance);
}



class COneChannelDefSetting
{
public:
	COneChannelDefSetting();
	virtual void Serialize( CArchive& ar );
	COneChannelDefSetting & operator =(const COneChannelDefSetting & src);
	void ToXML( CString & strOutput );

public:
	CString	m_strName;							//	名称
	CString	m_strChineseName;					//	中文名称
	CString	m_strIDFile;						//	特征文件名
	CString	m_strDefHomePage;					//	默认主页
	CString	m_strDefSavePath;					//	保存路径
	CString	m_strHostIP;						//	多播 IP
	int		m_nHostSocketPort;						//	多播端口
	DWORD	m_dwPID;								//	PID
	CString	m_strPostCode;						//  2003-10-14 适用地区邮编
};

COneChannelDefSetting::COneChannelDefSetting()
{

}

void COneChannelDefSetting::Serialize( CArchive& ar )
{
	CString strTmp;
	int nCount = 1;								//  2003-10-14 升级了，添加 PostCode 属性
	if( ar.IsStoring() )
	{
		ar << m_strName;						//	名称
		ar << m_strChineseName;					//	中文名称
		ar << m_strIDFile;						//	特征文件名
		ar << m_strDefHomePage;					//	默认主页
		ar << m_strDefSavePath;					//	保存路径
		ar << m_strHostIP;						//	多播 IP
		ar << m_nHostSocketPort;				//	多播端口
		ar << m_dwPID;							//	PID
		ar << nCount;							//	没有其他数据
		//	扩充部分，必须按字符串方式
		ar << m_strPostCode;					//  2003-10-14 升级添加一个邮编
	}
	else
	{
		ar >> m_strName;						//	名称
		ar >> m_strChineseName;					//	中文名称
		ar >> m_strIDFile;						//	特征文件名
		ar >> m_strDefHomePage;					//	默认主页
		ar >> m_strDefSavePath;					//	保存路径
		ar >> m_strHostIP;						//	多播 IP
		ar >> m_nHostSocketPort;				//	多播端口
		ar >> m_dwPID;							//	PID
		ar >> nCount;							//	没有其他数据
		//	扩充部分，必须按字符串方式
		if( nCount >= 1 )
			ar >> m_strPostCode;				//  2003-10-14 升级了，所以需要判断是否超过 1
		//	括从部分，必须按字符串方式
		for(int i=1; i<nCount; i++)
		{										//	为了兼容，必须空读
			ar >> strTmp;
		}

#ifdef _DEBUG
	printf("Name=%s, HomePage=%s, IP=%s, Port=%d\n", (LPCSTR)m_strName,
		(LPCSTR)m_strDefHomePage, (LPCSTR)m_strHostIP, m_nHostSocketPort );
#endif //_DEBUG

	}
}

//-------------------------------------------
//	修改记录：
//  2003-10-14 添加 m_strPostCode
COneChannelDefSetting & COneChannelDefSetting::operator =(const COneChannelDefSetting & src)
{
	m_strName = src.m_strName;							//	名称
	m_strChineseName = src.m_strChineseName;			//	中文名称
	m_strIDFile = src.m_strIDFile;						//	特征文件名
	m_strDefHomePage = src.m_strDefHomePage;			//	默认主页
	m_strDefSavePath = src.m_strDefSavePath;			//	保存路径
	m_strHostIP = src.m_strHostIP;						//	多播 IP
	m_nHostSocketPort = src.m_nHostSocketPort;			//	多播端口
	m_dwPID = src.m_dwPID;								//	PID
	m_strPostCode = src.m_strPostCode;					//  2003-10-14 PostCode
	return *this;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		output the paramter to EPG in XML format
/// Input parameter:
///		None
/// Output parameter:
///		None
///	Note:
///		The channel No is < 0 to indicate this is a older channel
///		VendorID=0		-> tongshi
void COneChannelDefSetting::ToXML(CString & strOutput)
{
	strOutput.Format(
"  <CHANNEL>\n<CHANNEL_ID>%d</CHANNEL_ID>\n\
        <NAME>%s</NAME>\n\
        <MULTICAST_IP>%s</MULTICAST_IP>\n\
        <MULTICAST_PORT>%d</MULTICAST_PORT>\n\
        <BRO_VENDOR_ID>0</BRO_VENDOR_ID>\n\
        <POSTCODES>%s</POSTCODES>\n\
        <ENCRYPT_PARAM>0</ENCRYPT_PARAM>\n\
        <PID>0x%X</PID>\n\
        <DEFAULT_HOME_PAGE>%s</DEFAULT_HOME_PAGE>\n\
  </CHANNEL>\n",\
		-m_nHostSocketPort,	(LPCSTR)m_strName, (LPCSTR)m_strHostIP, m_nHostSocketPort,\
		(LPCSTR)m_strPostCode, m_dwPID, (LPCSTR)m_strDefHomePage );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVBEPGReceiver::CDVBEPGReceiver()
{
	m_bEnableOldEPG = true;
}

CDVBEPGReceiver::~CDVBEPGReceiver()
{

}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		Initialize receiver
/// Input parameter:
///		None
/// Output parameter:
///		None
BOOL CDVBEPGReceiver::Init(bool bSaveFileInBackground, int varMaxPacketSize)
{
fprintf( stderr, "CDVBEPGReceiver::Init\n");

	m_bEnableOldEPG = true;
	m_mapFileEPG.RemoveAll();

	return CDVBFileReceiver::Init( bSaveFileInBackground, varMaxPacketSize );
}

DWORD CDVBEPGReceiver::QueryInterface( REFIID iid,void ** ppvObject)
{
#ifdef _WIN32
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

	if( IID_IDVBEPGReceiver == iid )
	{
		AddRef();
		*ppvObject = static_cast<IDVB_EPG_Receiver*>(this);
		return 0;	//	S_OK;
	}

	return CDVBFileReceiver::QueryInterface( iid, ppvObject );
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		Get if enable old EPG format
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CDVBEPGReceiver::GetEnableOldEPG()
{
	return m_bEnableOldEPG;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		set enable older format
/// Input parameter:
///		bNewValue			enable or not
/// Output parameter:
///		None
void CDVBEPGReceiver::SetEnableOldEPG( bool bNewValue )
{
	m_bEnableOldEPG = bNewValue;
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		on one EPG file ok
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBEPGReceiver::Fire_OnFileOK( IFileObject * pObject, HDATAPORT hDataPort )
{
	if( NULL == m_pFileEventObject )		//	the File OK Event is not registerd
		return;
	ASSERT( pObject );
	if( NULL == pObject )
		return;
	if( pObject->GetDataLen() < 10 )		//	错误的文件长度，肯定不是EPG
		return;
	PBYTE pBuf = pObject->GetBuffer();
	DWORD dwTagID = *(PDWORD)pBuf;
	if( dwTagID == 0x12345678 )
	{										// Older EPG tag id
		if( false == m_bEnableOldEPG )
			return;							//  the older EPG is disabled
		if( false == TransferToXMLFormat( pObject ) )
			return;
	}
	CDVBFileReceiver::Fire_OnFileOK( pObject, hDataPort );
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///		On sub file OK, do nothing here
/// Input parameter:
///		None
/// Output parameter:
///		None
void CDVBEPGReceiver::Fire_OnSubFileOK( IFileObject * pObject, HDATAPORT hDataPort  )
{
	//	EPG 不运行大文件方式
}

///-------------------------------------------------------
/// CYJ,2004-5-17
/// Function:
///
/// Input parameter:
///		None
/// Output parameter:
///		None
bool CDVBEPGReceiver::TransferToXMLFormat(IFileObject *pObject)
{
	CMemFile fSrc;
	CString strTmpXML;
	CString strXML = "<?xml version=\"1.0\" encoding=\"gb2312\"?>\n<EPG>\n";
	try
	{
		fSrc.Attach( pObject->GetBuffer(), pObject->GetDataLen() );

		CArchive loadArchive(&fSrc, CArchive::load|CArchive::bNoFlushOnDelete  );
#ifdef _WIN32
		loadArchive.m_bForceFlat = FALSE;
#endif //_WIN32

		DWORD dwIDFlags;
		loadArchive >> dwIDFlags;
		if( dwIDFlags != 0x12345678 )
		{
			fSrc.Detach();
			return false;
		}
		DWORD dwBroID;
		time_t	SectionBuildTime;
		loadArchive >> dwBroID;				//	标识播出机，一般是IP地址
		loadArchive >> SectionBuildTime;	//	变化时间

		time_t & LastTime = m_mapFileEPG[ dwBroID ];
		if( LastTime == SectionBuildTime )
		{
			fSrc.Detach();					//	没有变化
			return false;
		}
		LastTime = SectionBuildTime;		//	记录变化

		WORD	wExtLen;					//	继续读取其他数据
		loadArchive >> wExtLen;				//	过程字节数
		int i;
		for(	i=0; i<wExtLen; i++)
		{
			BYTE byTmp;
			loadArchive >> byTmp;			//	兼容
		}

		int nCount;
		loadArchive >> nCount;				//	保存个数
		for(i=0; i<nCount; i++)
		{
			COneChannelDefSetting one;
			one.Serialize( loadArchive );
			one.ToXML( strTmpXML );
			strXML += strTmpXML;
		}
		loadArchive.Close();

		strXML += "</EPG>";
		pObject->SetBufSize( strXML.GetLength() + 1 );
		pObject->PutDataLen( strXML.GetLength() );
		memcpy( pObject->GetBuffer(), strXML, strXML.GetLength()+1 );
	}
	catch( ... )
	{
		return false;
	}
	return true;
}
