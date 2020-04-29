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

#include "stdafx.h"
#include "tspacketencapsulator.h"
#include "bitstream.h"

///////////////////////////////////////////////////////////////////////
//	CTSAdaptionFieldEncapsulate
///////////////////////////////////////////////////////////////////////
CTSAdaptionFieldEncapsulate::CTSAdaptionFieldEncapsulate()
{
    Preset();
    memset(m_abyDataBuf, 0, sizeof(m_abyDataBuf) );
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// 函数功能:
///		设置PCR
/// 输入参数:
///		llPCR			PCR
///		wExtension		扩展
/// 返回参数:
///		无
void CTSAdaptionFieldEncapsulate::SetPCR( LONGLONG llPCR, WORD wExtension )
{
    m_bPCR_Flag = true;
    m_llPCR = llPCR;
    m_wPCR_Extension = wExtension;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// 函数功能:
///		设置OPCR
/// 输入参数:
///		llOPCR			OPCR
///		wExtension		扩展
/// 返回参数:
///		无
void CTSAdaptionFieldEncapsulate::SetOPCR( LONGLONG llPCR, WORD wExtension )
{
    m_bOPCR_Flag = true;
    m_llOPCR = llPCR;
    m_wOPCR_Extension = wExtension;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// 函数功能:
///		设置结合点
/// 输入参数:
///		byValue			数值
/// 返回参数:
///		无
void CTSAdaptionFieldEncapsulate::SetSplicingPoint( BYTE byValue )
{
    m_bSplicingPointFlag = true;
    m_bySplicingPoint = byValue;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// 函数功能:
///		预制参数
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSAdaptionFieldEncapsulate::Preset()
{
    m_bDiscontinuity = false;
    m_bRandomAccess = false;
    m_bElementStreamPrority = false;

    m_bPCR_Flag = false;
    m_llPCR = 0;
    m_wPCR_Extension = 0;

    m_bOPCR_Flag = false;
    m_llOPCR = 0;
    m_wOPCR_Extension = 0;

    m_bSplicingPointFlag = false;
    m_bySplicingPoint = 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-23
/// 函数功能:
///		编译输出
/// 输入参数:
///		nLen			输出长度
/// 返回参数:
///		非NULL			缓冲区
///		NULL			失败
PBYTE CTSAdaptionFieldEncapsulate::Build( int & nLen )
{
    CMyBitStream	OutStream( m_abyDataBuf, sizeof(m_abyDataBuf) );
    OutStream.PutBit( m_bDiscontinuity );
    OutStream.PutBit( m_bRandomAccess );
    OutStream.PutBit( m_bElementStreamPrority );
    OutStream.PutBit( m_bPCR_Flag );
    OutStream.PutBit( m_bOPCR_Flag );
    OutStream.PutBit( m_bSplicingPointFlag );
    OutStream.PutBits( 0, 2 );			//	其他两位为0

    if( m_bPCR_Flag )
    {
        OutStream.PutBits32( DWORD(m_llPCR >> 1) );
        OutStream.PutBit( BYTE(m_llPCR & 1) );
        OutStream.PutBits( 0, 6 );
        OutStream.PutBits( m_wPCR_Extension, 9 );
    }

    if( m_bOPCR_Flag )
    {
        OutStream.PutBits32( DWORD(m_llOPCR >> 1) );
        OutStream.PutBit( BYTE(m_llOPCR & 1) );
        OutStream.PutBits( 0, 6 );
        OutStream.PutBits( m_wOPCR_Extension, 9 );
    }

    if( m_bSplicingPointFlag )
        OutStream.PutBits8( m_bySplicingPoint );

    OutStream.FinishWrite();

    nLen = OutStream.GetTotalWriteBits()/8;
    return m_abyDataBuf;
}

///////////////////////////////////////////////////////////////////////
//	CTSPacketEncapsulator
///////////////////////////////////////////////////////////////////////
CTSPacketEncapsulator::CTSPacketEncapsulator()
{ 
    Preset();
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		初始化TS头4个字节
/// 输入参数:
///		无
/// 返回参数:
///		无
void CTSPacketEncapsulator::Preset()
{
    m_abyData[0] = 0x47;		// 同步字
    m_abyData[1] = 0x1F;		// 无效的PID
    m_abyData[2] = 0xFF;
    m_abyData[3] = 0x10;		// 序号为0，仅包含有效负载
}
///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		设置PID
/// 输入参数:
///		wPID		PID 数值
/// 返回参数:
///		无	
void CTSPacketEncapsulator::SetPID( WORD wPID )
{
    m_abyData[1] &= 0xE0;
    m_abyData[1] |= BYTE( (wPID>>8)&0x1F );
    m_abyData[2] = BYTE(wPID);
}
///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		设置负载起始标志
/// 输入参数:
///		bStart			是否起始，缺省为 true
/// 返回参数:
///		无
void CTSPacketEncapsulator::SetPayloadStartIndicator( BOOL bStart )
{
    m_abyData[1] &= (~0x40);	// 1011 1111
    if( bStart )
        m_abyData[1] |= 0x40;
}	

///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///
/// 输入参数:
///		byValue			控制域数值， 1 ～ 3
///		byLen			适配域长度， 0 ～ 183
///		pBuf			附加数据，缺省为NULL
/// 返回参数:
///		无	
void CTSPacketEncapsulator::SetAdaptionField( BYTE byValue, BYTE byLen, PBYTE pBuf )
{
    m_abyData[3] &= 0xCF;		// 1100 1111
    byValue &= 3;
    byValue <<= 4;
    m_abyData[3] |= byValue;

    if( byValue & 0x20 )
    {							//	存在适配数据
        m_abyData[4] = byLen;
        if( pBuf )
            memcpy( m_abyData+5, pBuf, byLen );
        else
            memset( m_abyData+5, 0, byLen );			//  CYJ,2006-9-6 添加
    }
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		设置连续性指针
/// 输入参数:
///		byValue			连续性指针
/// 返回参数:
///		无
void CTSPacketEncapsulator::SetContinuity( BYTE byValue )
{
    byValue &= 0xF;
    m_abyData[3] &= 0xF0;
    m_abyData[3] |= byValue;
}

///////////////////////////////////////////////////////////////
CDVBPacketEncapsulatorBase::CDVBPacketEncapsulatorBase()
{
    m_wPID = INVALID_PID;
    m_byTSContinuity = 0;
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		设置TS分组的PID
/// 输入参数:
///		wPID			分组的PID
/// 返回参数:
///		原来的PID
WORD CDVBPacketEncapsulatorBase::SetPID(WORD wPID)
{
    WORD wRetVal = m_wPID;
    m_wPID = wPID & 0x1FFF;
    return wRetVal;
}

///-------------------------------------------------------
/// CYJ,2005-2-7
/// 函数功能:
///		分配一个TS分组，并设置相应的字段
/// 输入参数:
///		无
/// 返回参数:
///		NULL			失败
///		其他			TS分组
///	说明：
///		该方法调用AllocateTSPacketBuffer方法，以获得TS分组
CTSPacketEncapsulator * CDVBPacketEncapsulatorBase::GetTSPacket()
{
    PDVB_TS_PACKET pPacket = AllocateTSPacketBuffer();
    if( NULL == pPacket )
        return NULL;
    CTSPacketEncapsulator *pRetVal = (CTSPacketEncapsulator *)pPacket;

    pRetVal->Preset();
    pRetVal->SetPID( m_wPID );
    pRetVal->SetContinuity( m_byTSContinuity & 0xF );
    m_byTSContinuity ++;

    return pRetVal;
}
