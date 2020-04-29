//---------------------------------------------------------------------
//	
//		My unknown implement
//
//---------------------------------------------------------------------

#ifndef __MY_UNKNOWN_IMPLEMENT_H_20031120__
#define __MY_UNKNOWN_IMPLEMENT_H_20031120__

template < class TBase >	
class CMyIUnknownImpl : public TBase
{
//	interface of IUnknown
public:
	CMyIUnknownImpl()
	{
		m_nRef = 0;
	}

	virtual long AddRef(void)
	{
#ifdef _WIN32
		AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
		return ::InterlockedIncrement( &m_nRef );
	}

	virtual long Release(void)
	{
#ifdef _WIN32
		AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32
		if( 0 == ::InterlockedDecrement( &m_nRef ) )
		{
			SafeDelete();
			return 0;
		}
		return m_nRef;
	}

	virtual HRESULT QueryInterface( REFIID iid,void ** ppvObject)
	{
#ifdef _WIN32
		AFX_MANAGE_STATE(AfxGetStaticModuleState())
#endif //_WIN32

#ifdef _WIN32
		if( IID_IUnknown == iid )
#else
		if( IID_IMyUnknown == iid )
#endif //_WIN32
		{
			AddRef();
			*ppvObject = static_cast<IUnknown*>(this);
			return 0;	//	S_OK;
		}
		return 0x80004002L;		//	E_NOINTERFACE;
	}

	virtual void SafeDelete()=0;

protected:
	long m_nRef;
};


#endif // __MY_UNKNOWN_IMPLEMENT_H_20031120__
