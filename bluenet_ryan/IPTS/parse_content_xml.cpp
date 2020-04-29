#include "parse_content_xml.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <unistd.h>

const QString CONTENT_FILE( "content.xml" );

// 配置文件节点字符串
const QString ROOT_NODE( "configs" );
const QString CONTENT_NODE( "content" );
const QString IP_NODE( "ip" );
const QString PORT_NODE( "port" );
const QString PID_NODE( "pid" );


CParseContentXml::CParseContentXml()
{
    ReadContenXmlFile();
}

CParseContentXml::~CParseContentXml()
{

}

bool CParseContentXml::ReadContenXmlFile()
{
    QDomDocument doc;
    QString strFileName =  CONTENT_FILE;
    if( !ReadFile( doc, strFileName ) )
        return false;

    // root 节点 (configs)
    QDomElement rootElem = doc.documentElement();
    if( ROOT_NODE != rootElem.nodeName() )
    {
        qWarning() << qPrintable( strFileName ) << "format is error, Please checkout your xml file!";
        return false;
    }

    QDomNodeList contentList = rootElem.childNodes();
    int nCount = contentList.count();
    for( int i = 0; i < nCount; ++i )
    {
        QDomElement contentElem = contentList.at(i).toElement();
        if( !contentElem.isElement() \
                || 0 != QString::compare( contentElem.nodeName(), CONTENT_NODE, Qt::CaseInsensitive )  )
        {
            continue;
        }
        QDomNodeList contentChildList = contentElem.childNodes();
        int nChildCount = contentChildList.count();
        CONTENT_INFO childContentInfo;

        for( int j = 0; j < nChildCount; ++j )
        {
            QDomElement ChildElem = contentChildList.at(j).toElement();
            if( !ChildElem.isElement() )
                continue;

            QString strNodeName = ChildElem.nodeName();
            QString strText = ChildElem.text();
            int nValue = strText.toInt();
            if( 0 == QString::compare( strNodeName, IP_NODE, Qt::CaseInsensitive )  )
                childContentInfo.m_strIP = strText;
            else if( 0 == QString::compare( strNodeName, PORT_NODE, Qt::CaseInsensitive ) )
                childContentInfo.m_nPort = nValue;
            else if( 0 == QString::compare( strNodeName, PID_NODE, Qt::CaseInsensitive ) )
                childContentInfo.m_byPID = strText.toInt(0,16);
        }

        // save to vector
        if( !IsExist( childContentInfo ) )
        {
            m_vecContentInfo.append( childContentInfo );
        }
    }
    return true;
}

bool CParseContentXml::ReadFile( QDomDocument& doc, const QString& fileName )
{
    QFile file( fileName );
    // 检查文件是否存在
    if( !file.exists() )
    {
        qDebug() << fileName << "is not exist!";
        return false;
    }

    // 打开文件
    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
        qWarning() << "Error: Cannot read file " << qPrintable( fileName )
                   << ": " << qPrintable( file.errorString() );
        return false;
    }

    // 指定内容给QDomDocument解析，并检查配置文件的格式
    QString strErrorInfo;
    int nErrorLine, nErrorCol;
    if( !doc.setContent( &file, true, &strErrorInfo, &nErrorLine, &nErrorCol ) )
    {
        file.close();
        qWarning() << "Error:" << strErrorInfo << " line:" << nErrorLine << " col:" << nErrorCol;
        return false;
    }
    file.close();

    return true;
}

bool CParseContentXml::IsExist( CONTENT_INFO NewInfo )
{
    int nCount = m_vecContentInfo.count();
    for( int i=0; i<nCount; ++i )
    {
        if( m_vecContentInfo[i].m_strIP == NewInfo.m_strIP \
                || m_vecContentInfo[i].m_nPort == NewInfo.m_nPort \
                || m_vecContentInfo[i].m_byPID == NewInfo.m_byPID )
            return true;
    }

    return false;
}
