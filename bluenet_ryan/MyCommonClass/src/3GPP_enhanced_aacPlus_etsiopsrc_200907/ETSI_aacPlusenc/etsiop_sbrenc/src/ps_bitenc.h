/*
   Parametric stereo bitstream encoder
 */
#ifndef __PS_BITENC_H
#define __PS_BITENC_H

#include "FFR_bitbuffer.h"
#include "sbr_main.h"

#define CODE_BOOK_LAV_IID 14
#define CODE_BOOK_LAV_ICC 7
#define NO_IID_STEPS                    ( 7 )
#define NO_ICC_STEPS                    ( 8 )


struct PS_ENC;

Word32 WritePsData (struct PS_ENC*  h_ps_e,
                    Word32          bHeaderActive);

#endif
