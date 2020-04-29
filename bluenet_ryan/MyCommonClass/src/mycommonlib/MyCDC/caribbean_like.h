/*****************************************************************
*** Copyright (c) 2001 Sigma Designs Inc. All rights reserved. ***
*****************************************************************/

/**
  @file   caribbean_like.h
  @brief  

  <long description>

  @author Emmanuel Michon
  @date   2002-02-21
*/

#ifndef __CARIBBEAN_LIKE_H__
#define __CARIBBEAN_LIKE_H__

#ifndef NULL
#define NULL (void *)(0)
#endif
//modified by yuanlii@29bbs.net, according to rmbasic.h
//typedef	unsigned char	RMbool;
typedef int		RMbool;
// by yuanlii@29bbs.net
#ifndef TRUE
#define TRUE (RMbool)1
#endif
#ifndef FALSE
#define FALSE (RMbool)0
#endif

typedef unsigned char RMuint8;
typedef          char RMint8;

typedef unsigned short RMuint16;
typedef          short RMint16;

typedef unsigned long RMuint32;
typedef          long RMint32;

typedef unsigned long long RMuint64;
typedef          long long RMint64;

typedef char RMascii;
typedef char RMnonAscii;

// by yuanlii@29bbs.net, 1/24/2005 12:13:36
//typedef unsigned long UINT;
// by yuanlii@29bbs.net

typedef unsigned char *PUCHAR;
#define OVERLAPPED void

typedef enum {
	RM_ERROR_FIRST__ = 0,
	RM_OK,
	RM_ERROR,
	RM_ERROR_LAST__,
} RMstatus;

#define RM_MAX_STRING 1024
#define ENABLE TRUE, __FILE__, __LINE__
#define DISABLE FALSE, __FILE__, __LINE__
#define RMDBGLOG(x) DebugPrintInfoFile x

void *RMMalloc(RMuint32 submittedSize);
void RMFree(void *ptr);
void RMPrintAscii(RMascii *dest, const RMascii *fmt, ...);
RMint32 RMPseudoFileOpen(const RMnonAscii *name);
RMint32 RMPseudoFileRead(RMint32 fd,void *buf,RMuint32 count);
RMint32 RMPseudoFileIoctl(RMint32 fd,RMint32 request,void *argp);
RMint32 RMPseudoFileSimplifiedSelect(RMint32 fd,RMuint64 timeOutMicroSeconds);
void RMPseudoFileClose(RMint32 fd);
RMuint64 RMGetTimeInMicroSeconds(void);
void DebugPrintInfoFile(RMbool active, const RMnonAscii *file, RMint32 line, const RMascii *text, ...);
void RMPanic(RMstatus x);

#endif // __CARIBBEAN_LIKE_H__
