// CommParamParser.h: interface for the CCommParamParser class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ICOMMPARAMPARSER_INTERFACE_H_20030816__
#define __ICOMMPARAMPARSER_INTERFACE_H_20030816__

#include <MyComDef.h>
#include <time.h>

#pragma pack(push,4)

// {DCF17160-8C37-4d19-8CF8-B9FB0BE17172}
static const GUID IID_ICOMMPARAMETERBLOCK =
{ 0xdcf17160, 0x8c37, 0x4d19, { 0x8c, 0xf8, 0xb9, 0xfb, 0xb, 0xe1, 0x71, 0x72 } };
// {69B42167-067E-4522-81C9-3553D1110BD2}
static const GUID IID_ICOMMPARAMPARSER =
{ 0x69b42167, 0x67e, 0x4522, { 0x81, 0xc9, 0x35, 0x53, 0xd1, 0x11, 0xb, 0xd2 } };


class ICommParameterBlock : public IMyUnknown
{
public:
	virtual BOOL SetAsULargeInteger( LPCSTR lpszVarName, ULONGLONG Value ) = 0;
	virtual BOOL SetAsLargeInteger( LPCSTR lpszVarName, LONGLONG nValue ) = 0;
	virtual BOOL SetAsDate( LPCSTR lpszVarName, time_t t ) = 0;
	virtual BOOL SetAsInt( LPCSTR lpszVarName, int nValue ) = 0;
	virtual BOOL SetAsDWORD( LPCSTR lpszVarName, DWORD dwValue ) = 0;
	virtual BOOL SetAsString( LPCSTR lpszVarName, LPCSTR lpszValue ) = 0;
	virtual BOOL SetAsLine( LPCSTR lpszLine ) = 0;

	virtual ULONGLONG GetAsULARGE_INTEGER( LPCSTR lpszKey, ULONGLONG uDefValue = 0 ) = 0;
	virtual LONGLONG GetAsLargeInteger( LPCSTR lpszKey, LONGLONG nDefValue = 0 ) = 0;
	virtual time_t GetAsDate( LPCSTR lpszKey, time_t DefTime = 0 ) = 0;
	virtual double GetAsDouble( LPCSTR lpszKey, double fDefValue = 0 ) = 0;
	virtual int GetAsInt( LPCSTR lpszKey, int nDefValue = 0 ) = 0;
	virtual DWORD GetAsDWORD( LPCSTR lpszKey, DWORD dwDefValue = 0 ) = 0;
	virtual LPCSTR GetAsString( LPCSTR lpszKey, LPCSTR lpszDefValue = NULL) = 0;

	virtual ICommParameterBlock * CloneBlock() = 0;
    virtual	LPCSTR GetBlockName() = 0;
};

class ICommParamParser
{
public:	
	virtual BOOL IsParameterBlockEnd(PBYTE pBuf, DWORD dwLen)= 0;
	virtual void RemoveAt( int nNo ) = 0;
	virtual BOOL Parse( LPCSTR lpszFileName ) = 0;
	virtual BOOL Parse( PBYTE pBuf, DWORD dwBufLen ) = 0;
    virtual int Write( PBYTE pBuf, DWORD dwLen ) = 0;
    virtual void Write( LPCSTR lpszFileName ) = 0;
	virtual ICommParameterBlock * ElementAt( int nNo ) = 0;
	virtual ICommParameterBlock * ElementAt( LPCSTR lpszArrayName ) = 0;
	virtual int GetSize() = 0;

    virtual ICommParamParser * CloneParser() = 0;
};

#pragma pack(pop)

//-----------------------------------------------
//	Interface define
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

ICommParamParser * CreateCommParser();

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __ICOMMPARAMPARSER_INTERFACE_H_20030816__
