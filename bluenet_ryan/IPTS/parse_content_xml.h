#ifndef __PARSE_CONTENT_XML_H__
#define __PARSE_CONTENT_XML_H__

#include <QDomDocument>
#include <QVector>
#include <QStringList>
#include "value.h"

typedef struct tagContentInfo
{
    QString  m_strIP;
    DWORD    m_nPort;
    BYTE     m_byPID;

}CONTENT_INFO,*PCONTENT_INFO;

class CParseContentXml
{
public:
    CParseContentXml();
    ~CParseContentXml();

    QVector<CONTENT_INFO> GetContentInfo(){ return m_vecContentInfo;}

private:
    bool ReadContenXmlFile();
    bool ReadFile( QDomDocument& doc, const QString& fileName );
    bool IsExist( CONTENT_INFO NewInfo );

private:
    QVector<CONTENT_INFO> m_vecContentInfo;
};

#endif // __PARSE_CONTENT_XML_H__
