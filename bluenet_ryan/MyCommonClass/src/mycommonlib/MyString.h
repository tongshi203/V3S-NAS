#if !defined(__MYSTRING_H_20030723__)
#define __MYSTRING_H_20030723__

#include <string.h>

#pragma pack(push,4)

class CMyString
{
public:
	static char m_NilString;
	void ReleaseBuffer( int nNewLen = -1 );
	char * GetBuffer( int nSize );
	CMyString Left( int nCount );
	int Find( const char *  lpszDstString, int nStart = 0 );
	int Find( char DstChar, int nStart = 0);
	int ReverseFind( char DstChar ) const;
	int Compare( const char * pszValue ) const;
	int CompareNoCase( const char * pszValue ) const;
	int GetLength()const;
	void SetAt( int nIndex, char newChar );
	char GetAt( int nIndex) const;
	bool IsEmpty();
	void MakeLower();
	void MakeUpper();
	void TrimRight();
	void TrimLeft();
    void Delete( int nIndex, int nCount = 1 );
    void Replace( char cOld, char cNew );
    void Replace( LPCSTR pszOld, LPCSTR pszNew );
    int FindOneOf( LPCSTR pszCharSet ) const;
    CMyString Mid( int nFirst, int nCount = -1 );
    CMyString Right( int nCount );
    int Insert( int nIndex, char NewChar );
    int Insert( int nIndex, const char * pszNewStr );
	void Remove( char cChar );

	// constructors and destructor
	CMyString(const CMyString& str);
	CMyString(const char* str);
	CMyString(const char var);

	CMyString()
	{
	#if defined(_DEBUG) && 0
		char * pAddr1 = (char*)&m_pString;
		char * pAddr2 = (char *)(&m_nLength);
		ASSERT( pAddr2 - pAddr1 >= 4 );
		char * pAddr3 = (char * )this;
		ASSERT( pAddr1 == pAddr3 );
	#endif //_DEBUG

		m_nBufSize = 0;
		m_nLength = 0;
		m_pString = &m_NilString;
		m_nBufSize = 0;
	}

	~CMyString();

	// operator overloading
	CMyString& operator  =(const char* str);
	CMyString& operator  =(const CMyString& str);
	CMyString& operator  =(const double var)	{ VarToString(var); return *this; }
	CMyString& operator +=(const char str)		{ return *this += (CMyString)str; }
	CMyString& operator +=(const char* str)	{ return *this += (CMyString)str; }
	CMyString& operator +=(const CMyString& str);


	// add more logic comparison operators as following, for example, although not efficient
//	bool operator !=(char* str)	{ return strcmp(str, m_pString) != 0; }
//    bool operator ==( const char * str ){ return strcmp( str, m_pString) == 0; }
//	bool operator ==( const CMyString & str ){ return strcmp( str.m_pString, m_pString) == 0; }

	// c type string conversion
	operator char* ()					{ return m_pString; }
	operator const char* ()	const		{ return m_pString; }
	char* GetChar()						{ return m_pString; }
	char operator[](int nIndex){ return GetAt(nIndex); };

	// search the match string : WildCards can be '?' and '*' combination
	// return value : true (pattern matchs string), false (no match)
	bool Search(const char* WildCards)	{ return Match((char*)WildCards, m_pString); }

	// format string
	int Format(const char* format, ...);

protected:
	// can use faster algorithm for search ?
	bool Match(char*, char*);
	bool Scan(char*&, char*&);

	// have any good conversion method ?
	void VarToString(const double var);

	// numeric conversion helpers
	bool NumericParse(void* pvar, char flag);
	bool GetVar(bool& var)				{ return NumericParse((void*)&var, 'b'); }
	bool GetVar(char& var)				{ return NumericParse((void*)&var, 'c'); }
	bool GetVar(short& var)				{ return NumericParse((void*)&var, 's'); }
	bool GetVar(int& var)				{ return NumericParse((void*)&var, 'i'); }
	bool GetVar(long& var)				{ return NumericParse((void*)&var, 'l'); }
	bool GetVar(float& var)				{ return NumericParse((void*)&var, 'f'); }
	bool GetVar(double& var)			{ return NumericParse((void*)&var, 'd'); }
	bool GetVar(unsigned char& var)		{ return NumericParse((void*)&var, 'C'); }
	bool GetVar(unsigned short& var)	{ return NumericParse((void*)&var, 'S'); }
	bool GetVar(unsigned int& var)		{ return NumericParse((void*)&var, 'I'); }
	bool GetVar(unsigned long& var)		{ return NumericParse((void*)&var, 'L'); }

	// data block
	char* m_pString;		//	pointer to string buffer
	int   m_nLength;		//	string length
	int	  m_nBufSize;		//	buffer size
};

inline bool operator==(const CMyString& s1, const CMyString& s2)
    { return (s1.GetLength() == s2.GetLength()) && (s1.Compare(s2) == 0); }
inline bool operator==(const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) == 0; }
inline bool operator==(const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) == 0; }
inline bool operator!=(const CMyString& s1, const CMyString& s2)
    { return (s1.GetLength() != s2.GetLength()) || (s1.Compare(s2) != 0); }
inline bool operator!=(const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) != 0; }
inline bool operator!=(const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) != 0; }
inline bool operator< (const CMyString& s1, const CMyString& s2)
    { return s1.Compare(s2) < 0; }
inline bool operator< (const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) <  0; }
inline bool operator< (const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) >  0; }
inline bool operator> (const CMyString& s1, const CMyString& s2)
    { return s1.Compare(s2) >  0; }
inline bool operator> (const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) >  0; }
inline bool operator> (const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) <  0; }
inline bool operator<=(const CMyString& s1, const CMyString& s2)
    { return s1.Compare(s2) <= 0; }
inline bool operator<=(const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) <= 0; }
inline bool operator<=(const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) >= 0; }
inline bool operator>=(const CMyString& s1, const CMyString& s2)
    { return s1.Compare(s2) >= 0; }
inline bool operator>=(const CMyString& s1, const char  * s2)
    { return s1.Compare(s2) >= 0; }
inline bool operator>=(const char  * s1, const CMyString& s2)
    { return s2.Compare(s1) <= 0; }

#pragma pack(pop)

#endif	//	__MYSTRING_H_20030723__
