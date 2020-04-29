// PYH2HZ.h : Declaration of the CPYH2HZ

#ifndef __PYH2HZ_H_
#define __PYH2HZ_H_

/////////////////////////////////////////////////////////////////////////////
// CPYH2HZ
class CPYH2HZ
{
public:
	CPYH2HZ();	
	static char HZTOPY( unsigned short wHZ);
private:
	static char GetPyFormHZ1( unsigned short wHZ );
};

#endif //__PYH2HZ_H_
