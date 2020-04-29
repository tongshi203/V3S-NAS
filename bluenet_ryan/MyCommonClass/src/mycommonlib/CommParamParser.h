// CommParamParser.h: interface for the CCommParamParser class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __COMMPARAMPARSER_H_20030816__
#define __COMMPARAMPARSER_H_20030816__

#pragma pack(push,4)


#include <time.h>
#include "ICommParamParser.h"
#include <MyMap.h>
#include <MyString.h>
#include <MyFile.h>
#include <MyArray.h>
#include <MyArchive.h>

class CCommParamParser;

class CCommParameterBlock :\
	public ICommParameterBlock,\
	public CMyMap<CMyString,LPCSTR,CMyString,LPCSTR>
{
	friend CCommParamParser;
public:
	CCommParameterBlock();
	virtual ~CCommParameterBlock();

public:	//	IUnkown
    virtual long AddRef(void);
	virtual long Release(void);
	virtual DWORD QueryInterface( REFIID iid, void **ppvObject);

public:
  	virtual BOOL SetAsULargeInteger( LPCSTR lpszVarName, ULONGLONG Value );
	virtual BOOL SetAsLargeInteger( LPCSTR lpszVarName, LONGLONG nValue );
	virtual BOOL SetAsDate( LPCSTR lpszVarName, time_t t );
	virtual BOOL SetAsInt( LPCSTR lpszVarName, int nValue );
	virtual BOOL SetAsDWORD( LPCSTR lpszVarName, DWORD dwValue );
	virtual BOOL SetAsString( LPCSTR lpszVarName, LPCSTR lpszValue );
	virtual BOOL SetAsLine( LPCSTR lpszLine );

    BOOL SetValue( CMyString & strLine );

	virtual ULONGLONG GetAsULARGE_INTEGER( LPCSTR lpszKey, ULONGLONG uDefValue = 0 );
	virtual LONGLONG GetAsLargeInteger( LPCSTR lpszKey, LONGLONG nDefValue = 0 );
	virtual time_t GetAsDate( LPCSTR lpszKey, time_t DefTime = 0 );
	virtual double GetAsDouble( LPCSTR lpszKey, double fDefValue = 0 );
	virtual int GetAsInt( LPCSTR lpszKey, int nDefValue = 0 );
	virtual DWORD GetAsDWORD( LPCSTR lpszKey, DWORD dwDefValue = 0 );
	virtual LPCSTR GetAsString( LPCSTR lpszKey, LPCSTR lpszDefValue = NULL);

	virtual ICommParameterBlock * CloneBlock();
    virtual	LPCSTR GetBlockName();

    CCommParameterBlock & operator=( const CCommParameterBlock & src );

public:						
	CMyString m_strBlockName;				//	¿éÃû³Æ£¬Èç Parameter

protected:
	bool LoadFromStream( CArchive & ar, BOOL bGetSectionHead = FALSE );
	CMyString GetSectionHead( CArchive & ar );
	bool WriteToStream( CArchive & ar, BOOL bWriteEndFlag = TRUE );

private:
	CMyString m_GetAsStringRetVal;		//	the return value of GetAsString
};

class CCommParamParser :
 	public ICommParamParser,
    public CCommParameterBlock
{
public:
	virtual BOOL IsParameterBlockEnd(PBYTE pBuf, DWORD dwLen);
	virtual void RemoveAt( int nNo );
	virtual BOOL Parse( LPCSTR lpszFileName );
	virtual BOOL Parse( PBYTE pBuf, DWORD dwBufLen );
    virtual int Write( PBYTE pBuf, DWORD dwLen );
    virtual void Write( LPCSTR lpszFileName );
	virtual ICommParameterBlock * ElementAt( int nNo );
	virtual ICommParameterBlock * ElementAt( LPCSTR lpszArrayName );
	virtual int GetSize();
    virtual ICommParamParser * CloneParser();
    
    void Serialize( CArchive & ar );

public:
    virtual long AddRef(void);
	virtual long Release(void);
	virtual DWORD QueryInterface( REFIID iid, void **ppvObject);

public:
	CCommParamParser();
    virtual ~CCommParamParser();
    BOOL Write( CFile * pFile );
    BOOL Parse( CFile * pFile );

private:
	bool LoadFromStream( CArchive & ar );

private:
	CMyArray<CCommParameterBlock> m_ParameterArray;
    long m_nRefTimes;
};

#pragma pack(pop)

#endif // __COMMPARAMPARSER_H_20030816__
