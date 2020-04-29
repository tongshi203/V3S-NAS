//---------------------------------------------------------------------------

#ifndef __IPDefrag_H_20040419__
#define __IPDefrag_H_20040419__

#include "UDPCapture.h"

// Note: single thread mode

#ifdef _WIN32
    #include <afxtempl.h>
#else
    #include <MyArray.h>
#endif //_WIN32
//---------------------------------------------------------------------------

#pragma pack(push,4)

class CIPDefrag
{
public:
    CIPDefrag();
    ~CIPDefrag();

    void DeleteTimeOutIPs();        // delete the IP that timeout
    BOOL InsertIP( CIP_Packet * pIPPacket );

private:
#ifdef _WIN32
    CArray< CIP_Packet*, CIP_Packet*> m_aCatchedIP;
#else
    CMyArray< CIP_Packet*> m_aCatchedIP;
#endif //_WIN32
    time_t m_LastDeleteIPTime;

private:
    int FindIP(CIP_Packet*pIPData);
    void DeleteIP(CIP_Packet*pIPData);
};

#pragma pack(pop)

#endif   // __IPDefrag_H_20040419__
