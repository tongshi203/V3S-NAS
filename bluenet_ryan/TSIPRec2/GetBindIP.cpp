// GetBindIP.cpp : implementation file
//
////////////////////////////////////////////////////
//  2003-9-3	修改 OnInitDialog，只有一个网卡也执行绑定功能
//	2002.6.29	添加函数 OnDblclkListAdapter，双击表示选中该卡


#include "stdafx.h"
#include "GetBindIP.h"
#include "IPHlpAPI.H"
#include "Winsock2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGetBindIP dialog

//	执行绑定 IP 的对话框
//	入口参数
//		lpszBuf				缓冲区地址
//		nBufLen				缓冲区长度
//		pnIPAddressCount	网卡适配器个数
//		pszAdapterName		输出适配器名称，可以为0，表示不输出适配器名称
//		nAdapterBufSize		适配器缓冲区大小
//	注：
//		当返回为 ERROR_CANCELLED 时，pszAdapterName 不输出适配器名称
extern "C" HRESULT WINAPI DoBindIPDlgExA( LPSTR lpszBuf, int nBufLen, int * pnIPAddressCount, char * pszAdapterName, int nAdapterBufSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ASSERT( lpszBuf && nBufLen );
	if( NULL == lpszBuf || 0 == nBufLen )
		return ERROR_INVALID_DATA;

	if( pnIPAddressCount )
		*pnIPAddressCount = 0;
	if( pszAdapterName )
		*pszAdapterName = 0;

	CGetBindIP dlg;
	dlg.m_strIP = lpszBuf;
	if( IDOK != dlg.DoModal() )
		return ERROR_CANCELLED;			//	cancel

	if( nBufLen < dlg.m_strIP.GetLength() )
		return ERROR_BUFFER_OVERFLOW;

	strcpy( lpszBuf, dlg.m_strIP );
	if( pnIPAddressCount )
		*pnIPAddressCount = dlg.m_nIPAddressCount;

	if( pszAdapterName )
	{
		if( dlg.m_strAdapterName.GetLength() > nAdapterBufSize )
			return ERROR_BUFFER_OVERFLOW;
		strcpy( pszAdapterName, dlg.m_strAdapterName );
	}

	return ERROR_SUCCESS;
}

//	执行绑定 IP 的对话框
//	入口参数
//		lpszBuf				缓冲区地址
//		nBufLen				缓冲区长度
//		pnIPAddressCount	网卡适配器个数
extern "C" HRESULT WINAPI DoBindIPDlgA( LPSTR lpszBuf, int nBufLen, int * pnIPAddressCount )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return DoBindIPDlgExA( lpszBuf, nBufLen, pnIPAddressCount, NULL, 0 );
}

///-------------------------------------------------------
/// 2003-1-15
/// 功能：
///		获取第一个网卡地址
/// 入口参数：
///		pszIPBuffer			输出IP地址
///		nBufLen				缓冲区长度
/// 返回参数：
///		ERROR_SUCCESS			成功
///		ERROR_DEV_NOT_EXIST		没有网卡
///		ERROR_BUFFER_OVERFLOW	缓冲区溢出
extern "C" HRESULT WINAPI GetFirstIPAddressA( char * pszIPBuffer, int nBufLen )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	ASSERT( pszIPBuffer && nBufLen );
	if(NULL == pszIPBuffer || 0 == nBufLen )
		return ERROR_INVALID_DATA;

	CGetBindIP helper;
	DWORD adwIPAddressList[20];
	int nCount = helper.GetIPAddressList( adwIPAddressList, sizeof(adwIPAddressList)/sizeof(DWORD) );
	if( 0 == nCount )
		return ERROR_DEV_NOT_EXIST;		//	不存在

	CString strTmp;
	in_addr address;
	address.S_un.S_addr = adwIPAddressList[0];
	strTmp.Format("%d.%d.%d.%d",address.S_un.S_un_b.s_b1,address.S_un.S_un_b.s_b2,\
		address.S_un.S_un_b.s_b3,address.S_un.S_un_b.s_b4);

	if( nBufLen < strTmp.GetLength()+1 )
		return ERROR_BUFFER_OVERFLOW;

	strcpy( pszIPBuffer, strTmp );
	
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////
//	2002.5.23	添加
//	功能:
//		获取所有的适配器名称及其 IP 地址
//	入口参数:
//		pBuf				输入，缓冲地址
//		nBufSize			输入，缓冲大小
//	返回参数:
//		NULL				失败，可根据 GetLastError 失败错误原因
//		其他				成功
//	注：
//		适配器个数由 hostent.h_name 反映的名称为 NULL 时决定
//		只有 hostent.h_name, .h_addr_list 有效
extern "C" struct hostent * WINAPI GetAllAdaptersAndIPsA( PBYTE pBuf, int nBufSize )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	ASSERT( pBuf && nBufSize );
	if( NULL == pBuf || 0 == nBufSize )
	{
		::SetLastError( ERROR_INVALID_PARAMETER);		//	缓冲区太小
		return NULL;
	}

	ZeroMemory( pBuf, nBufSize );

	HMODULE hDll = ::LoadLibrary( "IPHLPAPI.DLL" );
	if( NULL == hDll )
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);			//	驱动没有找到
		return NULL;
	}
	
	BYTE abyBuf[20*1024];								//	20K
	DWORD dwBufSize = sizeof(abyBuf);
	ZeroMemory( abyBuf, dwBufSize );

	DWORD (WINAPI*pfnGetAdaptersInfo)( PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );
	pfnGetAdaptersInfo = \
		(DWORD(WINAPI*)(PIP_ADAPTER_INFO,PULONG))GetProcAddress( hDll, "GetAdaptersInfo" );
	if( NULL == pfnGetAdaptersInfo )
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);			//	驱动没有找到
		FreeLibrary( hDll );
		return NULL;					//	没有该函数
	}
		
	PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)abyBuf;
	DWORD dwRetVal = pfnGetAdaptersInfo( pInfo, &dwBufSize );
	if( ERROR_SUCCESS != dwRetVal )
	{
		::SetLastError(ERROR_DEV_NOT_EXIST);			//	没有找到适配器
		FreeLibrary( hDll );
		return NULL;
	}

	BOOL bError = FALSE;								//	表示发生错误	
	DWORD dwByteLeft = nBufSize;
	struct hostent * pRetVal = (struct hostent * )( pBuf + 1 );
	char * pszEmptyString = (char*)pBuf;
	*pszEmptyString = 0;								//	空字符
	pBuf ++;
	dwByteLeft --;
	pRetVal[0].h_name = NULL;							//	先表示结束，以后再修改	

	PIP_ADAPTER_INFO pTmpInfo = pInfo;
	int nCount = 0;
	while( pTmpInfo )
	{
		pTmpInfo = pTmpInfo->Next;
		nCount ++;
	}
	nCount ++;											//	一个隔离空间
	if( dwByteLeft < nCount*sizeof(struct hostent) )
	{
		::SetLastError(ERROR_INSUFFICIENT_BUFFER);		//	缓冲区不够
		FreeLibrary( hDll );
		return NULL;
	}
	pBuf += nCount*sizeof(struct hostent);
	dwByteLeft -= nCount*sizeof(struct hostent);

	nCount = 0;
	while( pInfo )
	{			
		PIP_ADDR_STRING pIPList = &pInfo->IpAddressList;
		if( NULL == pIPList )
		{
			pInfo = pInfo->Next;
			continue;
		}
		pRetVal[nCount].h_name = NULL;		//	先表示结束，以后再修改
		pRetVal[nCount].h_addrtype = 0;
		pRetVal[nCount].h_aliases = 0;
		pRetVal[nCount].h_length = 0;
		pRetVal[nCount].h_addr_list = NULL;
		LPCSTR pszAdapters = pInfo->Description;
		int nAdapterLen = strlen(pszAdapters);
		if( 0 == nAdapterLen )
		{
			pszAdapters = pInfo->AdapterName;			//	没有名称，用 UUID 代替
			nAdapterLen = strlen( pszAdapters );
		}
		nAdapterLen ++;									//	包含结尾字符
		if( dwByteLeft < (DWORD)nAdapterLen )
		{
			bError = TRUE;
			break;
		}
		strcpy( (char*)pBuf, pszAdapters );
		pRetVal[nCount].h_name = (char*)pBuf;
		pBuf += nAdapterLen;
		dwByteLeft -= nAdapterLen;		

		int nIPCount = 0;								//	计算 IP 的个数，以确定 pszIP 数组个数
		int nTotalIPStringLen = 0;
		while( pIPList )
		{			
			nIPCount ++;			
			nTotalIPStringLen += strlen(pIPList->IpAddress.String) + 1;
			pIPList = pIPList->Next;
		}
		if( dwByteLeft < nTotalIPStringLen + sizeof(char*)*nIPCount )
		{
			bError = TRUE;							//	发生错误
			break;
		}
		pRetVal[nCount].h_addr_list = (char* *)pBuf;
		int nPtrBuf = sizeof(char*) * ( nIPCount + 1 );
		pBuf += nPtrBuf;
		dwByteLeft -= nPtrBuf;
	
		pIPList = &pInfo->IpAddressList;
		nIPCount = 0;
		while( pIPList )
		{
			pRetVal[nCount].h_addr_list[nIPCount] = (char*)pBuf;
			strcpy( (char*)pBuf, pIPList->IpAddress.String );
			int nLen = strlen( pIPList->IpAddress.String ) + 1;
			pBuf += nLen;
			dwByteLeft -= nLen;
			nIPCount ++;
			pIPList = pIPList->Next;
		}

		pInfo = pInfo->Next;
		nCount ++;
	}

	FreeLibrary( hDll );
	if( bError )
	{
		::SetLastError( ERROR_INSUFFICIENT_BUFFER );
			return NULL;
	}
	return pRetVal;
}


CGetBindIP::CGetBindIP(CWnd* pParent /*=NULL*/)
	: CDialog(CGetBindIP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetBindIP)
	m_strIP = _T("");
	//}}AFX_DATA_INIT
	m_nIPAddressCount = 0;
	WSADATA wsaData;
	m_bIsWSAStartupSucc = ( 0 == m_drv.WSAStartup(0x0202,&wsaData) );	
}

CGetBindIP::~CGetBindIP()
{
	if( m_bIsWSAStartupSucc )
		m_drv.WSACleanup();
}

void CGetBindIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetBindIP)
	DDX_Control(pDX, IDC_LIST_ADAPTER, m_list_Adapter);
	DDX_Control(pDX, IDC_CB_IP, m_cb_IP);
	DDX_CBString(pDX, IDC_CB_IP, m_strIP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetBindIP, CDialog)
	//{{AFX_MSG_MAP(CGetBindIP)
	ON_LBN_SELCHANGE(IDC_LIST_ADAPTER, OnSelchangeListAdapter)
	ON_CBN_SELCHANGE(IDC_CB_IP, OnSelchangeCbIp)
	ON_LBN_DBLCLK(IDC_LIST_ADAPTER, OnDblclkListAdapter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetBindIP message handlers

void CGetBindIP::OnOK() 
{
	UpdateData();

	int nCurSel = m_cb_IP.GetCurSel();
	if( CB_ERR == nCurSel )
		return;
	DWORD dwIP = m_cb_IP.GetItemData( nCurSel );
	int nCount = m_list_Adapter.GetCount();

	m_strAdapterName.Empty();
	for(int i=0; i<nCount; i++)
	{
		if( m_list_Adapter.GetItemData( i ) == dwIP )
		{
			m_list_Adapter.GetText( i, m_strAdapterName  );
			break;
		}
	}

	CDialog::OnOK();
}

//---------------------------------------------------------
//	修改记录：
//  2003-9-3	只有一个网卡也执行绑定功能
BOOL CGetBindIP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( m_strIP.Find( "default" ) >= 0 )		//  2003-9-3 修改，只有一个网卡，也执行显示绑定
		m_strIP.Empty();

	if( m_bIsWSAStartupSucc )
	{
		EnumIPAndFillIPCB();
		UpdateData( FALSE );		
	}	

	LoadAndFillAdapter();
	UpdateData( FALSE );

	if( m_strIP.IsEmpty() )						//  2003-9-3 缺省为第一个网卡
		m_cb_IP.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//	读取网卡配置信息
//	返回参数
//		TRUE			成功并且有多个 IP(网卡)
//		FALSE			只有一个网卡，或者没有 IPHlpAPI.DLL
BOOL CGetBindIP::LoadAndFillAdapter()
{
	HMODULE hDll = ::LoadLibrary( "IPHLPAPI.DLL" );
	if( NULL == hDll )
		return FALSE;
	
	BYTE abyBuf[20*1024];
	DWORD dwBufSize = sizeof(abyBuf);
	ZeroMemory( abyBuf, dwBufSize );

	DWORD (WINAPI*pfnGetAdaptersInfo)( PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );
	pfnGetAdaptersInfo = \
		(DWORD(WINAPI*)(PIP_ADAPTER_INFO,PULONG))GetProcAddress( hDll, "GetAdaptersInfo" );
	if( NULL == pfnGetAdaptersInfo )
	{
		FreeLibrary( hDll );
		return FALSE;					//	没有该函数
	}
	
	PIP_ADAPTER_INFO pInfo = (PIP_ADAPTER_INFO)abyBuf;
	DWORD dwRetVal = pfnGetAdaptersInfo( pInfo, &dwBufSize );
	if( ERROR_SUCCESS == dwRetVal )
	{
		while( pInfo )
		{			
			LPCSTR pszAdapters = pInfo->Description;
			if( 0 == strlen(pszAdapters) )
				pszAdapters = pInfo->AdapterName;			//	没有名称，用 UUID 代替
			int nNo = m_list_Adapter.AddString( pszAdapters );
			PIP_ADDR_STRING pIPList = &pInfo->IpAddressList;

			BOOL bFound = FALSE;
			while( pIPList )
			{				
				for(int i=0; i<m_nIPAddressCount; i++)
				{
					DWORD dwIP = m_cb_IP.GetItemData( i );
					if( m_drv.inet_addr(pIPList->IpAddress.String) == dwIP )
					{
						m_list_Adapter.SetItemData( nNo, dwIP );
						bFound = TRUE;
						break;
					}
				}		
				if( bFound )				//	匹配到 IP 地址，继续下一个 IP
					break;

				pIPList = pIPList->Next;
			}

			if( FALSE == bFound )
			{								//	没有匹配到，应该是一个新的 IP，添加
				pIPList = &pInfo->IpAddressList;
				DWORD dwIP = m_drv.inet_addr( pIPList->IpAddress.String );
				int nIP = m_cb_IP.AddString( pIPList->IpAddress.String );
				m_cb_IP.SetItemData( nIP, dwIP );		// 添加新的IP
				m_list_Adapter.SetItemData( nNo, dwIP);	
			}

			pInfo = pInfo->Next;
		}
	}

	FreeLibrary( hDll );
	return m_nIPAddressCount > 1 ;
}

void CGetBindIP::OnSelchangeListAdapter() 
{
	int nCurSel = m_list_Adapter.GetCurSel();
	if( LB_ERR == nCurSel )
		return;
	DWORD dwIP = m_list_Adapter.GetItemData( nCurSel );
	int nCount = m_cb_IP.GetCount();
	for(int i=0; i<nCount; i++)
	{
		if( m_cb_IP.GetItemData(i) == dwIP )
		{
			m_cb_IP.SetCurSel( i );
			break;
		}
	}	
}

void CGetBindIP::OnSelchangeCbIp() 
{
	int nCurSel = m_cb_IP.GetCurSel();
	if( CB_ERR == nCurSel )
		return;
	DWORD dwIP = m_cb_IP.GetItemData( nCurSel );
	int nCount = m_list_Adapter.GetCount();
	for(int i=0; i<nCount; i++)
	{
		if( m_list_Adapter.GetItemData( i ) == dwIP )
		{
			m_list_Adapter.SetCurSel( i );
			break;
		}
	}
}

//	梅举本地IP地址，并填充IP CB
//	返回参数
//		IP 地址个数
int CGetBindIP::EnumIPAndFillIPCB()
{
	DWORD adwIPAddressList[20];
	m_nIPAddressCount = GetIPAddressList(adwIPAddressList,sizeof(adwIPAddressList)/sizeof(DWORD) );

	CString strTmp;
	for(int i=0; i<m_nIPAddressCount; i++)
	{
		in_addr address;
		address.S_un.S_addr = adwIPAddressList[i];
		strTmp.Format("%d.%d.%d.%d",address.S_un.S_un_b.s_b1,address.S_un.S_un_b.s_b2,\
			address.S_un.S_un_b.s_b3,address.S_un.S_un_b.s_b4);
		int nNo = m_cb_IP.AddString( strTmp );
		m_cb_IP.SetItemData( nNo, address.S_un.S_addr );
	}
	return m_nIPAddressCount;
}

//	2002.6.29 添加，双击表示选中
void CGetBindIP::OnDblclkListAdapter() 
{
	OnSelchangeListAdapter();
	OnOK();		
}

///-------------------------------------------------------
/// 2003-1-15
/// 功能：
///		获取本地IP地址列表
/// 入口参数：
///		pdwIPList		IP 地址数组
///		nBufCount		数组个数
/// 返回参数：
///		无
int CGetBindIP::GetIPAddressList(PDWORD pdwIPList, int nBufCount)
{
	if( FALSE == m_bIsWSAStartupSucc )
		return 0;				//	失败

	char szHostname[256];
	if( m_drv.gethostname(szHostname, sizeof(szHostname) ) )
	{
		  TRACE(_T("Failed in call to gethostname, WSAGetLastError returns %d\n"), m_drv.WSAGetLastError());
		  return 0;
	}

	//get host information from the host name
	HOSTENT* pHostEnt = m_drv.gethostbyname(szHostname);
	if (pHostEnt == NULL)
	{
		TRACE(_T("Failed in call to gethostbyname, WSAGetLastError returns %d\n"), m_drv.WSAGetLastError());
		return 0;
	}

	//check the length of the IP adress
	if (pHostEnt->h_length != 4)
	{
		TRACE(_T("IP address returned is not 32 bits !!\n"));
		return 0;
	}

	//call the virtual callback function in a loop
	int nAdapter = 0;
	CString strTmp;
	while( pHostEnt->h_addr_list[nAdapter] && nBufCount )
	{
		in_addr address;
		ASSERT( pHostEnt->h_length <= sizeof(in_addr) );
		CopyMemory( &address.S_un.S_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length );

		*pdwIPList = address.S_un.S_addr;
		if( *pdwIPList )
		{						//	除去地址为 0 的IP
			pdwIPList ++;
			nBufCount --;
		}
	
		nAdapter ++;
	}
	return nAdapter;
}
