#ifndef __MY_TIME_H_20040401__
#define __MY_TIME_H_20040401__

#pragma pack(push,4)

#include <time.h>
#include "MyString.h"

class CTime
{
public:

// Constructors
	static CTime GetCurrentTime();

	CTime();
	CTime(time_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1);
	CTime(const CTime& timeSrc);

	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

// Attributes
//	struct tm* GetGmtTm() const;
	const struct tm* GetLocalTm();

	time_t GetTime();
	int GetYear();
	int GetMonth();       // month of year (1 = Jan)
	int GetDay();         // day of month
	int GetHour();
	int GetMinute();
	int GetSecond();
	int GetDayOfWeek();   // 1=Sun, 2=Mon, ..., 7=Sat

// Operations
	// time math
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	// formatting using "C" strftime
	CMyString Format(const char * pFormat);

private:
	time_t m_time;
	struct tm	m_LocalTm;
	time_t 	m_TimeWhenCallLocalTime;
};

#pragma pack(pop)

#endif // __MY_TIME_H_2004001__
