// StatBpsHelper.h: interface for the CStatBpsHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_)
#define AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_

#pragma pack(push,4)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
	#include <MyComDef.h>
#endif // _WIN32

class CStatBpsHelper  
{
public:
	int SetStatTimePrecision( int nNewValue );
	BOOL SetStatMethod( BOOL bUseAverage = TRUE );
	void PresetBPS( int nNewValueKB );
	DWORD SetPeroid( DWORD dwNewValue );
	int GetBPS();
	void Reset();
	CStatBpsHelper();
	virtual ~CStatBpsHelper();

	void AddBytes( long nNewValue, BOOL bEnableCalucateBPS = TRUE );
	long CalculateBPS();

	static DWORD GetSysTickCount();

private:
	int		m_nPrecisionUnit;				//	计算精度，单位毫秒，缺省为 1 MS
	BOOL	m_bUseAsAverage;				//	平均得方式进行计算速率，缺省为 TRUE
	DWORD	m_dwTickCount;					//	当前时间
	DWORD	m_dwTotalBytes;					//	当前已经接收到的字节数
	long	m_nBPSValue;					//	bps
	DWORD	m_dwPeroidTickCount;			//	统计间隔
	DWORD	m_dwActureTickCount;			//	实际的时间间隔
};

#pragma pack(pop)

#endif // !defined(AFX_STATBPSHELPER_H__5E29B9E3_3100_4991_9C18_07BB5478CB89__INCLUDED_)
