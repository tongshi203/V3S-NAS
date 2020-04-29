/*
   Core Coder's and SBR's shared data structure definition
 */
#ifndef __SBR_CMONDATA_H
#define __SBR_CMONDATA_H

#include "FFR_bitbuffer.h"


struct COMMON_DATA {
  Word16                sbrHdrBits;             /* number of SBR header bits */
  Word16                sbrDataBits;            /* number of SBR data bits */
  Word16                sbrFillBits;            /* number of SBR fill bits */
  struct BIT_BUF        sbrBitbuf;              /* the SBR data bitbuffer */
  struct BIT_BUF        sbrBitbufPrev;          /* Delayed SBR payload */
  Word16                sbrHdrBitsPrev;         /* Number of SBR header bits in delayed payload */
  Word16                sbrDataBitsPrev;        /* Number of SBR data bits in delayed payload */
  Word16                sbrNumChannels;         /* number of channels (meaning mono or stereo) */
};/* size Word16: 22  */

typedef struct COMMON_DATA *HANDLE_COMMON_DATA;



#endif
