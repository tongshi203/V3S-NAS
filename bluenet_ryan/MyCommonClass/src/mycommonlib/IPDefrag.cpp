//------------------------------------------------------
//
//        Chen Yongjian @ xi'an tongshi
//           2004-04-19
//
// Purpose:
//          IP defragment
//          working in single thread mode
//----------------------------------------------------

#pragma hdrstop
#include <stdio.h>
#include "IPDefrag.h"
#include <UDPHeadHelper.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)

CIPDefrag::CIPDefrag()
{
    m_LastDeleteIPTime = time(NULL);
}

CIPDefrag::~CIPDefrag()
{
    int nCount = m_aCatchedIP.GetSize();
    for(int i=0; i<nCount; i++)
    {
        m_aCatchedIP[i]->Release();
    }
    m_aCatchedIP.RemoveAll();
}

//-----------------------------------------------------------------
// Function:
//      delete the catched IP that time out
// Input Parameter:
//      None
// Output:
//      None
void CIPDefrag::DeleteTimeOutIPs()
{
    time_t now = time(NULL);
    int nCount = m_aCatchedIP.GetSize();
    for(int i=0; i<nCount; i++)
    {
        CIP_Packet * pIP = m_aCatchedIP[i];
        if( (now - pIP->m_tLastAccessTime) < 60 )
            continue;
        m_aCatchedIP.RemoveAt( i );
        pIP->Release();
    }
}

//-----------------------------------------------------------------
// Function:
//      insert one IP packet      
// Input Parameter:
//      pIPData         the IP data to be defrag
// Output:
//      FALSE           the IP is drag and not ready
//      TRUE            ready IP
// Note:
//      the data in the buffer is in IP format
BOOL CIPDefrag::InsertIP( CIP_Packet * pIPPacket )
{
    int nDataLen = pIPPacket->GetDataLen();
    PBYTE pSrcBuf = pIPPacket->GetBuffer();
	LPIPHEADER lpIPHeader = (LPIPHEADER) pSrcBuf;
    WORD wFragment = XCHG(lpIPHeader->fragment);
    if( 0 == ( wFragment & 0x3FFF ) )
        return TRUE;           // not frag

    time_t now = time(NULL) & (~7);
    if( now != m_LastDeleteIPTime )
    {
        m_LastDeleteIPTime = now;
        DeleteTimeOutIPs();
    }

    int nNo = FindIP( pIPPacket );
    if( nNo < 0 && (wFragment & 0x1FFF) )
    {
#ifdef _DEBUG
        printf("************** Not found exist IP\n");
#endif //_DEBUG        
        return FALSE;           // The first IP is not received
    }                            // And this is used in broadcast side, I can assume fine condition

    CIP_Packet * pIPExist = NULL;
    if( nNo >= 0 )
    {
        pIPExist = m_aCatchedIP[nNo];
        LPIPHEADER lpIPHeaderExist = (LPIPHEADER)pIPExist->GetBuffer();
        if( lpIPHeaderExist->identifier != lpIPHeader->identifier )
        {
            m_aCatchedIP.RemoveAt( nNo );       // the older IP, not same IP-ID
            pIPExist->Release();
            pIPExist = NULL;
        }
    }
    if( NULL == pIPExist )
    {
        if( wFragment&0x1FFF )
            return FALSE;           // must be the first packet
        pIPExist = new  CIP_Packet;
        if( NULL == pIPExist )
            return FALSE;
        pIPExist->Preset();
        pIPExist->AddRef();
        memcpy( &pIPExist->m_IP_Param, &pIPPacket->m_IP_Param, sizeof(pIPPacket->m_IP_Param) );
        m_aCatchedIP.Add( pIPExist );
        if( FALSE == pIPExist->SetBufSize( pIPPacket->GetDataLen() ) )
            return FALSE;
        memcpy( pIPExist->GetBuffer(), pIPPacket->GetBuffer(), pIPPacket->GetDataLen() );
        pIPExist->PutDataLen( pIPPacket->GetDataLen() );
#ifdef _DEBUG
        printf("***** copy the first packet, Len=%d.\n", pIPPacket->GetDataLen() );
#endif //_DEBUG
        return FALSE;
    }

    pIPExist->m_tLastAccessTime = time(NULL);

    ASSERT(wFragment&0x1FFF);
#ifdef _DEBUG
    printf("************** defrag one ip at %d, len=%d\n",wFragment<<3, XCHG(lpIPHeader->length));
#endif //_DEBUG    

    int nIPHeaderLen = ( lpIPHeader->x & 0xF) * sizeof(DWORD);
    WORD wOffset = (wFragment&0x1FFF) * 8 + nIPHeaderLen;
    DWORD dwNewLen = wOffset + XCHG(lpIPHeader->length) - nIPHeaderLen;
    if( FALSE == pIPExist->SetBufSize( dwNewLen ) )
        return FALSE;
    memcpy( pIPExist->GetBuffer()+wOffset,
        pIPPacket->GetBuffer() + nIPHeaderLen, pIPPacket->GetDataLen()-nIPHeaderLen );
    pIPExist->PutDataLen( dwNewLen );

    if( wFragment & IP_MF )
        return FALSE;           // need more packet
               // the Last packet
    if( FALSE == pIPPacket->SetBufSize( pIPExist->GetDataLen() ) )
        return FALSE;
    memcpy( pIPPacket->GetBuffer(), pIPExist->GetBuffer(), pIPExist->GetDataLen() );
    pIPPacket->PutDataLen( pIPExist->GetDataLen() );
    memcpy( &pIPPacket->m_IP_Param, &pIPExist->m_IP_Param, sizeof(pIPExist->m_IP_Param) );
    DeleteIP( pIPExist );
    
#ifdef _DEBUG
    printf("******* %d IP in the buffer.\n", m_aCatchedIP.GetSize());
#endif //_DEBUG

    lpIPHeader = (LPIPHEADER)pIPPacket->GetBuffer();
    lpIPHeader->length = XCHG( (WORD)pIPPacket->GetDataLen() );
    lpIPHeader->fragment = 0;

#ifdef _DEBUG
    UDP_HEADER * pUDPHeader = (UDP_HEADER *)( pIPPacket->GetBuffer() + 20 );
        printf("************** defrag one ip len=%d, UDP len=%d, IP=%08X, Port=%d\n",
        pIPPacket->GetDataLen(), XCHG(pUDPHeader->length),
        lpIPHeader->dest, XCHG( pUDPHeader->dest_port) );
#endif //_DEBUG

    return TRUE;
}

void CIPDefrag::DeleteIP(CIP_Packet*pIPData)
{
    int nNo = FindIP( pIPData );
    if( nNo < 0 )
        return;
    m_aCatchedIP[nNo]->Release();
    m_aCatchedIP.RemoveAt( nNo );
}

//-----------------------------------------------------------------
// Function:
//      find one IP
// Input Parameter:
//      pIPData         IP
// Output:
//      -1              not found
int CIPDefrag::FindIP(CIP_Packet* pIPData)
{
    int nCount = m_aCatchedIP.GetSize();
    for(int i=0; i<nCount; i++)
    {
        CIP_Packet * pIP = m_aCatchedIP[i];
        if( pIP->m_IP_Param.sin_port != pIP->m_IP_Param.sin_port )
            continue;
        if( pIP->m_IP_Param.sin_addr.s_addr == pIP->m_IP_Param.sin_addr.s_addr )
            return i;
    }

    return -1;
}

