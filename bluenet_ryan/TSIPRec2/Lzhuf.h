#ifndef __LZHUF_H__
#define __LZHUF_H__

#include "UnCompressObj.h"

#define EnCode_Action          0
#define DeCode_Action          1

#define DeCode_Smart_Buf_Size	32768
#define _File_IN				0
#define _File_OUT				1

class CLzhuf32 : public CUnCompressObj
{
public:
// Huf
	   enum { THRESHOLD	= 2,
			F = 60,
			N = 4096,
			NIL = N
	   };
	   enum {
		   LZHUF_UNCMP_MAJORVERSION = 1,
		   LZHUF_UNCMP_MINORVERSION = 1
	   };
       enum { N_CHAR = (256 - THRESHOLD + F)};
				/* kinds of characters (character code = 0..N_CHAR-1) */
       enum { T = (N_CHAR * 2 - 1)};	/* size of table */
       enum { R = (T - 1)};		/* position of root */
       enum { MAX_FREQ = 0x8000	};	/* updates tree when the */
					/* root frequency comes to this value. */
// LZSS
       unsigned char text_buf[N + F - 1];
// Huf

       WORD freq[T + 1];	/* frequency table */

       short int prnt[T + N_CHAR];	/* pointers to parent nodes, except for the */
			       /* elements [T..T + N_CHAR - 1] which are used to get */
			       /* the positions of leaves corresponding to the codes. */

       short int son[T];     	       /* pointers to child nodes (son[], son[] + 1) */


       unsigned code, len;

		/* for decoding */
		static const BYTE d_code[256] ;
		static const BYTE d_len[256];
		static WORD getbuf ;
		static BYTE getlen ;


private:
	PBYTE m_pOutBufAutoAlloc;
      WORD GetBit(void);	/* get one bit */
      BYTE GetByte(void);	/* get one byte */
      void StartHuff(void);
      void reconst(void);
      void update(short c);
      short DecodeChar(void);
      short DecodePosition(void);
      void Decode(void);  /* recover */
      void PrepareData(); // init data
public:
	CLzhuf32();
	~CLzhuf32();
	virtual PBYTE DecodeOneFile(int nFileNo,CFileStatus & outfStatus,PBYTE pDstBuf=NULL);
	virtual int GetFileInfo(int nFileNo,CFileStatus & outfStatus);
	virtual int GetFileNum();
	virtual int Attach(int nFileLen,PBYTE pBuf);
	virtual DWORD GetCompressMethod();
	virtual int GetDecoderVersion();
	virtual void FreeMemory();
};

#endif //__LZHUF_H__
