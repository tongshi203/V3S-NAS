/************************************************************************
 *
 *	Test main
 *
 * Chen Yongjian @ zhoi
 * 2014.10.22 @ xi'an
 *
 ***********************************************************************/
#include <stdafx.h>
#include <mydatatype.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <mydvbpsi/dvbpsitablesdefine.h>
#include <mydvbpsi/psitablegenerator.h>
#include <mydvbpsi/dvbdescriptorsdefine.h>
#include <mydvbpsi/tspacket.h>
#include <mydvbpsi/pespacket.h>


DVB_TS_PACKET			g_aTSBuf[8];
int						g_nTSIndex = 0;
bool					g_bHasAudio= false;

//----------------------------------------------
#define MY_OUTPUT_PCR_PID           8190
#define MY_OUTPUT_VIDEO_PID         200
#define MY_OUTPUT_AUDIO_PID         201
#define MY_OUTPUT_PMT_PID			100
#define MY_OUTPUT_SID				1

//-------------------------------------------------------------
class CMyPATGenerator :public CDVBPSI_PATGenerator
{
	//	分配一个TS分组，返回 NULL 失败
	virtual PDVB_TS_PACKET AllocateTSPacketBuffer()
	{
		return g_aTSBuf + g_nTSIndex;
	}
	//	一个TS分组完成
	virtual void OnTSPacketReady( PDVB_TS_PACKET pTSPacket )
	{
		g_nTSIndex ++;
	}
};

//-------------------------------------------------------------
class CMyPMTGeneraor :public CDVBPSI_PMTGenerator
{
//	分配一个TS分组，返回 NULL 失败
	virtual PDVB_TS_PACKET AllocateTSPacketBuffer()
	{
		return g_aTSBuf + g_nTSIndex;
	}
	//	一个TS分组完成
	virtual void OnTSPacketReady( PDVB_TS_PACKET pTSPacket )
	{
		g_nTSIndex ++;
	}
};

//-------------------------------------------------------------
void PrintHelp()
{
	fprintf( stderr, "genpatpmt 			to generator H264 video only, no audio\n" );
	fprintf( stderr, "genpatpmt AAC			to generator AAC audio\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "output file is: my_pat_pmt_bin.cpp\n");
}

//-------------------------------------------------------------
void print_pat_pmt()
{
	FILE * fp = fopen( "my_pat_pmt_bin.cpp", "wt");

	unsigned char * pBuf = &g_aTSBuf[0].m_abyData[0];

	fprintf( fp, "// Video: H.264, Audio: %d\n", g_bHasAudio );
	fprintf( fp, "int g_nPAT_PMT_DataLen=%d;\n", g_nTSIndex*188 );
	fprintf( fp, "unsigned char g_pbyPAT_PMT[%d]=\n", g_nTSIndex*188 );
	fprintf( fp, "{\n");
	for(int i=0; i<g_nTSIndex; i++ )
	{
		fprintf( fp, "    " );
		for(int j=0; j<188; j++)
		{
			fprintf( fp, "0x%02X,", *pBuf );
			pBuf ++;
			if( (j&7) == 7 )
				fprintf( fp, " " );
			if( (j&15) == 15 )
				fprintf( fp, "\n    " );
		}
		fprintf( fp, "\n" );
	}
	fprintf( fp, "};\n" );
}

//-------------------------------------------------------------
int main( int argc, char * argv[] )
{
	PrintHelp();

	g_nTSIndex = 0;

	CMyPATGenerator pat;
	CMyPMTGeneraor pmt;

	pmt.CreateStream( DVBPES_STREAM_TYPE_VIDEO_H264, MY_OUTPUT_VIDEO_PID, NULL, 0 );

	if( argc > 1 && 0 == strcasecmp( argv[1], "aac") )
	{
		fprintf( stderr, "Audio: AAC \n" );
		//		unsigned char abyAACData[] = { 0x7C, 2, 0x13, 0 };
		unsigned char abyAACData[]={ 0x2B, 4, 0x02, 0x80, 0x08, 0x00 };
		pmt.CreateStream( DVBPES_STREAM_TYPE_AUDIO_MPEG4, MY_OUTPUT_AUDIO_PID, abyAACData, sizeof(abyAACData) );
		g_bHasAudio = true;
	}
	else
	{
		fprintf( stderr, "No Audio\n" );
		g_bHasAudio = false;
	}

	pmt.SetPID( MY_OUTPUT_PMT_PID );
	pmt.SetPCR_PID( MY_OUTPUT_PCR_PID );
	pmt.SetSID( MY_OUTPUT_SID );

	pat.SetSID_PMT( MY_OUTPUT_SID, MY_OUTPUT_PMT_PID );

	pat.Build();
	pmt.Build();

	print_pat_pmt();

	return 0;
}

