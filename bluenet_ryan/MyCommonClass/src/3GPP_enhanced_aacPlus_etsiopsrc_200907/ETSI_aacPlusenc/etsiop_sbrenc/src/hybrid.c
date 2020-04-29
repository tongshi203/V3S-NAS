/*
   Hybrid Filter Bank
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hybrid.h"
#include "sbr_ram.h"
#include "sbr_rom.h"
#include "sbr_def.h"

#include "count.h"



/*!
  \brief  4-channel complex-valued filtering with 6-tap delay
*/
static void fourChannelFiltering( const Word32 *pQmfReal,
                                  const Word32 *pQmfImag,
                                  Word32 **mHybridReal,
                                  Word32 **mHybridImag,
                                  Word32 chOffset)
{
  Word32 i, k, n;
  Word32 midTap = HYBRID_FILTER_DELAY;
  Word32 real, imag;
  Word32 cum[8];


  for(i = 0; i < QMF_TIME_SLOTS; i++) {
    /*
      Apply polyphase filters
    */

    /* First filter has 4 taps */
    cum[5] = cum[4] = 0;                                                               move32(); move32();
    for(k = 0; k < 4; k++) {
      cum[5] = L_sub(cum[5], fixmul(p4_13[4*k], pQmfReal[i+4*k]));
      cum[4] = L_add(cum[4], fixmul(p4_13[4*k], pQmfImag[i+4*k]));
    }

    /* Second filter has three taps */
    real = imag = 0;                                                                   move32(); move32();
    for(k = 0; k < 3; k++) {
      real = L_add(real, fixmul(p4_13[4*k+3], pQmfReal[i+4*k+3]));
      imag = L_add(imag, fixmul(p4_13[4*k+3], pQmfImag[i+4*k+3]));
    }

    /* Multiply with twiddle factors */
    cum[6] = fixmul(L_add(imag, real), 0x5a82799a);
    cum[7] = fixmul(L_sub(imag, real), 0x5a82799a);

    /* Third filter has one non-zero tap */
    cum[0] = fixmul(p4_13[midTap], pQmfReal[i+midTap]);
    cum[1] = fixmul(p4_13[midTap], pQmfImag[i+midTap]);

    /* Fourth filter has three taps */
    real = imag = 0;                                                                   move32(); move32();
    for(k = 0; k < 3; k++) {
      real = L_add(real, fixmul(p4_13[4*k+1], pQmfReal[i+4*k+1]));
      imag = L_add(imag, fixmul(p4_13[4*k+1], pQmfImag[i+4*k+1]));
    }

    /* Multiply with twiddle factors */
    cum[2] = fixmul(L_sub(real, imag ), 0x5a82799a);
    cum[3] = fixmul(L_add(real, imag ), 0x5a82799a);

    /*
      4-point inverse FFT
    */
    inv_dit_fft_4pt(cum ,0 );


    /*
      Copy to output
    */
    for(n = 0; n < 4; n++) {
      mHybridReal[i][n + chOffset] = cum[2*n];                                                       move32();
      mHybridImag[i][n + chOffset] = cum[2*n+1];                                                     move32();
    }
  }
}


/*!
  \brief  8-channel complex-valued filtering with 6-tap delay
*/
static void eightChannelFiltering( const Word32 *pQmfReal,
                                   const Word32 *pQmfImag,
                                   Word32 *mHybridReal,
                                   Word32 *mHybridImag)
{
  Word32 n;
  Word32 real, imag;
  Word32 cum[16];

  /*
    Apply polyphase filters
  */

  /* First filter has two taps. */
  real = L_add(fixmul(p8_13_enc[4], pQmfReal[4]), fixmul(p8_13_enc[12], pQmfReal[12]));
  imag = L_add(fixmul(p8_13_enc[4], pQmfImag[4]), fixmul(p8_13_enc[12], pQmfImag[12]));
  /* Multiply with twiddle factors. */
  cum[4] =  fixmul(L_sub(imag, real), 0x5a82799a);
  cum[5] =  fixmul(L_negate(L_add(imag, real)), 0x5a82799a);

  /* Second filter has two taps. */
  real = L_add(fixmul(p8_13_enc[3], pQmfReal[3]), fixmul(p8_13_enc[11], pQmfReal[11]));
  imag = L_add(fixmul(p8_13_enc[3], pQmfImag[3]), fixmul(p8_13_enc[11], pQmfImag[11]));
  /* Multiply with twiddle factors. */
  cum[6] = L_sub(fixmul(imag, 0x7641af3d), fixmul(real, 0x30fbc54d));
  cum[7] = L_negate(L_add(fixmul(imag, 0x30fbc54d), fixmul(real, 0x7641af3d)));

  /* Third filter has two taps. */
  cum[9] = L_negate(L_add(fixmul(p8_13_enc[2], pQmfReal[2]), fixmul(p8_13_enc[10], pQmfReal[10])));
  cum[8] = L_add(fixmul(p8_13_enc[2], pQmfImag[2]), fixmul(p8_13_enc[10], pQmfImag[10]));

  /* Fourth filter has two taps. */
  real = L_add(fixmul(p8_13_enc[1], pQmfReal[1]), fixmul(p8_13_enc[9], pQmfReal[9]));
  imag = L_add(fixmul(p8_13_enc[1], pQmfImag[1]), fixmul(p8_13_enc[9], pQmfImag[9]));
  /* Multiply with twiddle factors. */
  cum[10] = L_add(fixmul(imag, 0x7641af3d), fixmul(real, 0x30fbc54d));
  cum[11] = L_sub(fixmul(imag, 0x30fbc54d), fixmul(real, 0x7641af3d));

  /* Fifth filter has two taps. */
  real = L_add(fixmul(p8_13_enc[0], pQmfReal[0]), fixmul(p8_13_enc[8], pQmfReal[8]));
  imag = L_add(fixmul(p8_13_enc[0], pQmfImag[0]), fixmul(p8_13_enc[8], pQmfImag[8]));
  /* Multiply with twiddle factors. */
  cum[12] = fixmul(L_add(imag, real), 0x5a82799a);
  cum[13] = fixmul(L_sub(imag, real), 0x5a82799a);

  /* Sixth filter has one tap. */
  real = fixmul(p8_13_enc[7], pQmfReal[7]);
  imag = fixmul(p8_13_enc[7], pQmfImag[7]);
  /* Multiply with twiddle factors. */
  cum[14] = L_add(fixmul(imag, 0x30fbc54d), fixmul(real, 0x7641af3d));
  cum[15] = L_sub(fixmul(imag, 0x7641af3d), fixmul(real, 0x30fbc54d));

  /* Seventh filter has one tap. */
  cum[0] = fixmul(p8_13_enc[HYBRID_FILTER_DELAY], pQmfReal[HYBRID_FILTER_DELAY]);
  cum[1] = fixmul(p8_13_enc[HYBRID_FILTER_DELAY], pQmfImag[HYBRID_FILTER_DELAY]);

  /* Eighth filter has one tap. */
  real = fixmul(p8_13_enc[5], pQmfReal[5]);
  imag = fixmul(p8_13_enc[5], pQmfImag[5]);
  /* Multiply with twiddle factors. */
  cum[2] = L_sub(fixmul(real, 0x7641af3d), fixmul(imag, 0x30fbc54d));
  cum[3] = L_add(fixmul(real, 0x30fbc54d), fixmul(imag, 0x7641af3d));

  /*
    8-point inverse FFT
  */
  inv_dit_fft_8pt_enc(cum ,0);		   
  

  /*
    Copy to output
  */
  for(n = 0; n < 8; n++) {
    mHybridReal[n] = cum[2*n];                                                                       move32();
    mHybridImag[n] = cum[2*n+1];                                                                     move32();
  }

}


/**************************************************************************/
/*!
  \brief   HybridAnalysis

  Transform the lowest QMF bands into a hybrid domain with higher
  frequency resolution.

*/
/**************************************************************************/
void
HybridAnalysis ( const Word16 **mQmfReal,
                 const Word16 **mQmfImag,
                 Word32 **mHybridReal,
                 Word32 **mHybridImag,
                 Word16 **histQmfReal,
                 Word16 **histQmfImag )
{
  Word32  n, band;
  HYBRID_RES hybridRes;
  Word32  chOffset = 0;
  Word32 workReal[QMF_TIME_SLOTS + QMF_BUFFER_MOVE];
  Word32 workImag[QMF_TIME_SLOTS + QMF_BUFFER_MOVE];

  for(band = 0; band < NO_QMF_BANDS_IN_HYBRID; band++) {
    hybridRes = (HYBRID_RES)aHybridResolution[band];                                   move32();

    /* Fill up workbuffer for current band */

    for(n = 0; n < QMF_BUFFER_MOVE; n++) {
      workReal[n] = L_deposit_h(histQmfReal[n][band]);
      workImag[n] = L_deposit_h(histQmfImag[n][band]);
    }

    for(n = 0; n < QMF_TIME_SLOTS; n++) {
      workReal [QMF_BUFFER_MOVE + n] = L_deposit_h(mQmfReal[n][band]);
      workImag [QMF_BUFFER_MOVE + n] = L_deposit_h(mQmfImag[n][band]);
    }

    test(); test();
    switch(hybridRes) {
    case HYBRID_4_CPLX:
      fourChannelFiltering( workReal,
                            workImag,
                            mHybridReal,
                            mHybridImag,
                            chOffset );

     break;
    case HYBRID_8_CPLX:
      for(n = 0; n < QMF_TIME_SLOTS; n++) {
        eightChannelFiltering( workReal+n,
                               workImag+n,
                               mHybridReal[n],
                               mHybridImag[n]);
      }
      break;
    default:
      assert(0);
    }

    chOffset = L_add(chOffset, hybridRes);
  }
}


/**************************************************************************/
/*!
  \brief  Hybrid filterbank synthesis

  The lowest QMF bands are transformed back to the QMF domain.
*/
/**************************************************************************/
void
HybridSynthesis ( const Word32 **mHybridReal,
                  const Word32 **mHybridImag,
                  Word32 **mQmfReal,
                  Word32 **mQmfImag,
                  Word16 **mQmfRealShort,
                  Word16 **mQmfImagShort)
{
  Word16 k, n, band;
  HYBRID_RES hybridRes;
  Word16 chOffset = 0;                                                     move16();

  for(band = 0; band < NO_QMF_BANDS_IN_HYBRID; band++) {

    hybridRes = (HYBRID_RES)aHybridResolution[band];                       move16();

    for(n = 0; n < QMF_TIME_SLOTS; n++) {

      mQmfReal[n][band] = mQmfImag[n][band] = 0;                  move32(); move32();

      for(k = 0; k < (Word16)hybridRes; k++) {
        mQmfReal[n][band] = L_add(mQmfReal[n][band], mHybridReal[n][chOffset + k]);                  move32();
        mQmfImag[n][band] = L_add(mQmfImag[n][band], mHybridImag[n][chOffset + k]);                  move32();
      }

      mQmfRealShort[n][band] = etsiopround(mQmfReal[n][band]);                                       move16();
      mQmfImagShort[n][band] = etsiopround(mQmfImag[n][band]);                                       move16();
    }
    chOffset = add(chOffset, hybridRes);
  }
}
