// StatBpsHelper.cpp: implementation of the CStatBpsHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include "StatBpsHelper.h"

#ifndef _WIN32
	#include <MySyncObj.h>
    #include <sys/time.h>
#endif //_WIN32

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStatBpsHelper::CStatBpsHelper()
{
	Reset();
}

CStatBpsHelper::~CStatBpsHelper()
{

}

///-------------------------------------------------------
/// 2003-2-22
/// 功能：
///		重新初始化
/// 入口参数：
///		无
/// 返回参数：
///		无
void CStatBpsHelper::Reset()
{	
	m_dwActureTickCount = GetSysTickCount();
	m_dwTickCount = m_dwActureTickCount;
	m_dwTotalBytes = 0;
	m_nBPSValue = 0;
	m_dwPeroidTickCount = 4000;
	m_bUseAsAverage = TRUE;
	m_nPrecisionUnit = 1;			//	缺省为 1 MS
}

///-------------------------------------------------------
/// 2003-2-22
/// 功能：
///		获取返回值
/// 入口参数：
///		无
/// 返回参数：
///		当前的BPS，单位 bps
int CStatBpsHelper::GetBPS()
{
	return ::InterlockedExchangeAdd( &m_nBPSValue, 0 );
}

///-------------------------------------------------------
/// 2003-2-22
/// 功能：
///		设置检测周期
/// 入口参数：
///		dwNewValue			新的检测周期
/// 返回参数：
///		原来的检测周期
DWORD CStatBpsHelper::SetPeroid(DWORD dwNewValue)
{
#ifdef _DEBUG
	ASSERT( 0 == (m_dwPeroidTickCount%m_nPrecisionUnit) );
	if( (m_dwPeroidTickCount%m_nPrecisionUnit) )
		TRACE("It's recommanded the PeroidTickCount should be multiple of Precision.\n");
#endif // _DEBUG
	DWORD dwOldValue = m_dwPeroidTickCount;
	if( dwNewValue < 50 )				// 不能太小
		dwNewValue = 2000;
	m_dwPeroidTickCount = dwNewValue;
	return dwOldValue;
}

///-------------------------------------------------------
/// 2003-2-23
/// 功能：
///		预制速率，单位 kbps
/// 入口参数：
///		nNewValueKB		预制的速率
/// 返回参数：
///		无
void CStatBpsHelper::PresetBPS(int nNewValueKB)
{
	m_nBPSValue = nNewValueKB;
	m_dwTickCount = GetSysTickCount() - 2000;	//	想到于2秒前的时间
	m_dwTotalBytes = nNewValueKB/4;
}

///-------------------------------------------------------
/// 2003-2-28
/// 功能：
///		添加速率
/// 入口参数：
///		nNewValue				字节数
///		bEnableCalucateBPS		允许计算速率，缺省为 TRUE
/// 返回参数：
///
void CStatBpsHelper::AddBytes( long nNewValue, BOOL bEnableCalucateBPS )
{
	m_dwTotalBytes += nNewValue;
	if( bEnableCalucateBPS )
		CalculateBPS();
}

///-------------------------------------------------------
/// 2003-2-23
/// 功能：
///		计算速率，并返回当前速率
/// 入口参数：
///		无
/// 返回参数：
///		当前速率
long CStatBpsHelper::CalculateBPS()
{
	DWORD dwActualTickCount = GetSysTickCount();
	DWORD dwTickCount = dwActualTickCount;
	if( m_nPrecisionUnit > 1 )
		dwTickCount = dwTickCount - (dwTickCount%m_nPrecisionUnit);	//	取整
	ASSERT( m_dwPeroidTickCount >= 8 );
	if( DWORD(dwTickCount - m_dwTickCount) < m_dwPeroidTickCount ||\
		DWORD(dwActualTickCount-m_dwActureTickCount) < m_dwPeroidTickCount*4/5 )
	{
		return m_nBPSValue;
	}
	m_dwActureTickCount = dwActualTickCount;
	DWORD dwDelta = dwTickCount-m_dwTickCount;
	float fBPS = float(m_dwTotalBytes) * 8000.0f;
	long nBPS = long( fBPS/dwDelta + 0.5f);		//	四舍五入
	::InterlockedExchange( &m_nBPSValue, nBPS );

//	TRACE("This=%08X, %d Bytes / %d ms = %d bps.\n", this, m_dwTotalBytes, dwDelta, nBPS );

	if( m_bUseAsAverage )
	{
		m_dwTickCount = m_dwTickCount/2 + dwTickCount/2;
		m_dwTotalBytes /= 2;
	}
	else
	{
		m_dwTickCount = dwTickCount;
		m_dwTotalBytes = 0;
	}

	return nBPS;
}

///-------------------------------------------------------
/// 2003-3-1
/// 功能：
///		使用平均得方式进行计算
/// 入口参数：
///		bUseAverage				= TRUE 前后两次平均
/// 返回参数：
///		原来的设置
BOOL CStatBpsHelper::SetStatMethod(BOOL bUseAverage)
{
	BOOL bRetVal = m_bUseAsAverage;
	m_bUseAsAverage = bUseAverage;
	return bRetVal;
}

///-------------------------------------------------------
/// 2003-3-1
/// 功能：
///		设置统计时间精度
/// 入口参数：
///		nNewValue		时间精度，单位 MS
/// 返回参数：
///		原来的设置值
int CStatBpsHelper::SetStatTimePrecision(int nNewValue)
{
	ASSERT( nNewValue >= 1 );
	if( nNewValue < 1 )
		nNewValue = 1;
	int nRetVal = m_nPrecisionUnit;
	m_nPrecisionUnit = nNewValue;
	return nRetVal;
}

///-----------------------------------------------
///	Get System tick count
DWORD CStatBpsHelper::GetSysTickCount()
{
#ifdef _WIN32
	return ::GetTickCount();
#else  // Linux
	struct timeval now;
    gettimeofday( &now, NULL );
    DWORD dwRetVal = now.tv_sec * 1000 ;
    dwRetVal += (now.tv_usec/1000);
    return dwRetVal;
#endif //	_WIN32
}
