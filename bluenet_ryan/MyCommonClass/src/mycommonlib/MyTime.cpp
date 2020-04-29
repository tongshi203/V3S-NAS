#include "stdafx.h"
#include "MyTime.h"

CTime::CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
	int nDST)
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	ASSERT(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	ASSERT(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	ASSERT(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	ASSERT(m_time != -1);       // indicates an illegal input time
	m_TimeWhenCallLocalTime = -1;
}

CTime::CTime()
{
	m_TimeWhenCallLocalTime = -1;
}

CTime::CTime(const CTime& timeSrc)
{
	m_time = timeSrc.m_time;
	m_TimeWhenCallLocalTime = -1;
}

CTime::CTime(time_t time)
{
	m_time = time;
	m_TimeWhenCallLocalTime = -1;
}

CTime CTime::GetCurrentTime()
// return the current system time
{
	return CTime(::time(NULL));
}

CMyString CTime::Format(const char * pFormat)
{
	char szBuffer[100];
	const struct tm* ptmTemp = GetLocalTm();

#ifdef _WIN32
	if (ptmTemp == NULL ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
    {
    	szBuffer[0] = '\0';
    }
#else
	if (ptmTemp == NULL || !strftime(szBuffer, sizeof(szBuffer)-1, pFormat, ptmTemp))
    {
    	szBuffer[0] = '\0';
    }
#endif //_WIN32

	return szBuffer;
}

const CTime& CTime::operator=(const CTime& timeSrc)
	{ m_time = timeSrc.m_time; return *this; }
const CTime& CTime::operator=(time_t t)
	{ m_time = t; return *this; }
time_t CTime::GetTime()
	{ return m_time; }
int CTime::GetYear()
	{ return (GetLocalTm()->tm_year) + 1900; }
int CTime::GetMonth()
	{ return GetLocalTm()->tm_mon + 1; }
int CTime::GetDay()
	{ return GetLocalTm()->tm_mday; }
int CTime::GetHour()
	{ return GetLocalTm()->tm_hour; }
int CTime::GetMinute()
	{ return GetLocalTm()->tm_min; }
int CTime::GetSecond()
	{ return GetLocalTm()->tm_sec; }
int CTime::GetDayOfWeek()
	{ return GetLocalTm()->tm_wday + 1; }
BOOL CTime::operator==(CTime time) const
	{ return m_time == time.m_time; }
BOOL CTime::operator!=(CTime time) const
	{ return m_time != time.m_time; }
BOOL CTime::operator<(CTime time) const
	{ return m_time < time.m_time; }
BOOL CTime::operator>(CTime time) const
	{ return m_time > time.m_time; }
BOOL CTime::operator<=(CTime time) const
	{ return m_time <= time.m_time; }
BOOL CTime::operator>=(CTime time) const
	{ return m_time >= time.m_time; }
const struct tm* CTime::GetLocalTm()
{
	// 2011.11.15 CYJ Add, using localtime_r
	if( m_TimeWhenCallLocalTime != m_time )
	{
		m_TimeWhenCallLocalTime = m_time;
		return localtime_r( &m_time, &m_LocalTm );
	}
	else
		return &m_LocalTm;
}
