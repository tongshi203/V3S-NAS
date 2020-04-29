/*
   Hybrid Filter Bank header file
 */

#ifndef _HYBRID_H
#define _HYBRID_H

#include "ffr.h"
#include "ffr.h"
#include "fft.h"
#include "sbr_def.h"

#define HYBRID_FILTER_LENGTH  13
#define QMF_BUFFER_MOVE       (HYBRID_FILTER_LENGTH - 1)
#define HYBRID_FILTER_DELAY    6
#define NO_QMF_BANDS_IN_HYBRID 3
#define NO_HYBRID_BANDS       16

typedef enum {

  HYBRID_2_REAL = 2,
  HYBRID_4_CPLX = 4,
  HYBRID_8_CPLX = 8

} HYBRID_RES;


void
HybridAnalysis ( const Word16 **mQmfReal,
                 const Word16 **mQmfImag,
                 Word32 **mHybridReal,
                 Word32 **mHybridImag,
                 Word16 **histQmfReal,
                 Word16 **histQmfImag );
void
HybridSynthesis ( const Word32 **mHybridReal,
                  const Word32 **mHybridImag,
                  Word32 **mQmfReal,
                  Word32 **mQmfImag,
                  Word16 **mQmfRealShort,
                  Word16 **mQmfImagShort);


#endif /* _HYBRID_H */

