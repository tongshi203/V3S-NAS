/*
   parametric stereo encoding
 */
#ifndef __PS_ENC_H
#define __PS_ENC_H

#include "ps_bitenc.h"
#include "hybrid.h"
#include "sbr_def.h"
#include "sbr_ram.h"


/*############################################################################*/
/* Constant definitions                                                       */
/*############################################################################*/
#define NO_BINS                         ( 20 )

#define NO_IPD_BINS_EST                 ( NO_BINS )

#define NO_IPD_GROUPS                   ( NO_IPD_BINS_EST + 6 + 2 )

#define NO_QMF_BANDS                    ( 64 )


/*############################################################################*/
/* Type definitions                                                           */
/*############################################################################*/

struct PS_ENC{

  UWord8 bHiFreqResIidIcc;
  UWord8 iidIccBins;
  UWord8 bPrevZeroIid;
  UWord8 bPrevZeroIcc;

  Word32 psMode;

  struct BIT_BUF psBitBuf;                  /* size Word16: 8 */

  Word16 prevScale;

  Word32 *aaaIIDDataBuffer;
  Word32 *aaaICCDataBuffer;

  Word8   aLastIidIndex[NO_IID_BINS];
  Word8   aLastIccIndex[NO_ICC_BINS];

  Word32 *mHybridRealLeft[QMF_TIME_SLOTS];
  Word32 *mHybridImagLeft[QMF_TIME_SLOTS];
  Word32 *mHybridRealRight[QMF_TIME_SLOTS];
  Word32 *mHybridImagRight[QMF_TIME_SLOTS];

  Word32   powerLeft    [NO_BINS];
  Word32   powerRight   [NO_BINS];
  Word32   powerCorrReal[NO_BINS];
  Word32   powerCorrImag[NO_BINS];

  Word16 *histQmfLeftReal[QMF_BUFFER_MOVE];
  Word16 *histQmfLeftImag[QMF_BUFFER_MOVE];
  Word16 *histQmfRightReal[QMF_BUFFER_MOVE];
  Word16 *histQmfRightImag[QMF_BUFFER_MOVE];

  Word16 *tempQmfLeftReal[QMF_BUFFER_MOVE];
  Word16 *tempQmfLeftImag[QMF_BUFFER_MOVE];
  Word16 *tempQmfRightReal[QMF_BUFFER_MOVE];
  Word16 *tempQmfRightImag[QMF_BUFFER_MOVE];
}; /* size Word16: 431 */

typedef struct PS_ENC *HANDLE_PS_ENC;

/*############################################################################*/
/* Extern function prototypes                                                 */
/*############################################################################*/
Word32 GetPsMode(Word32 bitRate);

Word32
CreatePsEnc(HANDLE_PS_ENC h_ps_enc,
            Word32 psMode);

void
EncodePsFrame(HANDLE_PS_ENC h_ps_e,
              Word16 **iBufferLeft,
              Word16 **rBufferLeft,
              Word16 **iBufferRight,
              Word16 **rBufferRight,
              Word32  **energyValues,
              Word16 *leftScale,
              Word16 *rightScale,
              Word16 *energyScale);

void downmixToMono( HANDLE_PS_ENC h_ps_e,
                    Word16 **qmfLeftReal,
                    Word16 **qmfLeftImag,
                    Word16 **qmfRightReal,
                    Word16 **qmfRightImag,
                    Word32 **energyValues,
                    Word16 *scale);


#endif
