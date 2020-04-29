/*
   parametric stereo encoding
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <memory.h>
#endif
#include <assert.h>
#include "ps_enc.h"
#include "sbr_def.h"

#include "sbr_rom.h"
#include "sbr_ram.h"

#include "count.h"


#define SUBQMF_BINS_ENERGY              ( 8 )
#define SUBQMF_GROUPS_MIX               ( 16 )
#define MAX_MIX_COEFF                   0x7fff
#define MIN_MIX_COEFF                   0x2000

#define PS_MODE_LOW_FREQ_RES_IID_ICC    ( 0x00020000 )


/*!
  \brief  PS tuning expert

  A finer frequency resolution for the IID and ICC parameters
  is selected for bitrates above 21000 bps.
*/
Word32 GetPsMode(Word32 bitRate) /*!< Overall output bitrate of the encoder */
{
  Word32 psMode = 0;

  test(); L_sub(1, 1);
  if(bitRate < 21000){
    psMode = PS_MODE_LOW_FREQ_RES_IID_ICC;                        move32();
  }
  else {
    test(); L_sub(1, 1);
    if(bitRate < 128000) {
      psMode = 0;                                                 move32();
    }
  }

  return psMode;
}


/*!
  \brief  Allocate memory and set default values

  \return 0 on successful completion
*/
Word32 CreatePsEnc(HANDLE_PS_ENC h_ps_e, /*!< Parametric stereo encoder instance */
                   Word32 psMode           /*!< Selects coarse or fine frequency resolution */
                   )
{
  Word32 i;

  Word32 *ptr1 = (Word32*)&sbr_envYBuffer[QMF_TIME_SLOTS * QMF_CHANNELS];
  Word16 *ptr2 = ps_histQmfBuffer;
  Word16 *ptr3 = ps_tempQmfBuffer;
  Word32 *ptr4 = &sbr_toncorrBuff[5 * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];

  h_ps_e->psMode = psMode;                                                                 move32();

  h_ps_e->bPrevZeroIid = 0;                                                                move32();
  h_ps_e->bPrevZeroIcc = 0;                                                                move32();

  logic32(); test(); test();
  h_ps_e->bHiFreqResIidIcc = ( ( psMode & PS_MODE_LOW_FREQ_RES_IID_ICC ) != 0 )? 0: 1;     move32();
  test();
  h_ps_e->iidIccBins = ( h_ps_e->bHiFreqResIidIcc )? NO_IID_BINS: NO_LOW_RES_IID_BINS;     move32();

  h_ps_e->aaaICCDataBuffer = ps_IccDataBuffer;                                             move32();
  h_ps_e->aaaIIDDataBuffer = ps_IidDataBuffer;                                             move32();

  /* Setup pointers to hybrid filterbank subband samples */
  for (i=0 ; i < QMF_TIME_SLOTS ; i++) {
    h_ps_e->mHybridRealLeft[i] = ptr1;                                                     move32();
    ptr1 = (Word32*)L_add((Word32)ptr1, NO_HYBRID_BANDS * sizeof(*ptr1));

    h_ps_e->mHybridImagLeft[i] = ptr1;                                                     move32();
    ptr1 = (Word32*)L_add((Word32)ptr1, NO_HYBRID_BANDS * sizeof(*ptr1));

    h_ps_e->mHybridRealRight[i] = ptr1;                                                    move32();
    ptr1 = (Word32*)L_add((Word32)ptr1, NO_HYBRID_BANDS * sizeof(*ptr1));

    h_ps_e->mHybridImagRight[i] = ptr1;                                                    move32();
    ptr1 = (Word32*)L_add((Word32)ptr1, NO_HYBRID_BANDS * sizeof(*ptr1));
  }

  /* Setup QMF delay buffer: past 12 subband-samples for 3 channels... */
  for (i=0 ; i < HYBRID_FILTER_DELAY ; i++) {
    h_ps_e->histQmfLeftReal[i] = ptr2;                                                     move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr2));

    h_ps_e->histQmfLeftImag[i] = ptr2;                                                     move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr2));

    h_ps_e->histQmfRightReal[i] = ptr2;                                                    move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr2));

    h_ps_e->histQmfRightImag[i] = ptr2;                                                    move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr2));


    h_ps_e->tempQmfLeftReal[i] = ptr3;                                                     move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr3));

    h_ps_e->tempQmfLeftImag[i] = ptr3;                                                     move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr3));

    h_ps_e->tempQmfRightReal[i] = ptr3;                                                    move16();
    ptr3 =(Word16*) L_add((Word32)ptr3, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr3));

    h_ps_e->tempQmfRightImag[i] = ptr3;                                                    move16();
    ptr3 =(Word16*) L_add((Word32)ptr3, NO_QMF_BANDS_IN_HYBRID * sizeof(*ptr3));
  }
  /* ...and all 64 channels for the past 6 subband-samples */
  for (i=HYBRID_FILTER_DELAY ; i < QMF_BUFFER_MOVE ; i++) {
    h_ps_e->histQmfLeftReal[i] = ptr2;                                                     move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, QMF_CHANNELS * sizeof(*ptr2));

    h_ps_e->histQmfLeftImag[i] = ptr2;                                                     move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, QMF_CHANNELS * sizeof(*ptr2));

    h_ps_e->histQmfRightReal[i] = ptr2;                                                    move16();
    ptr2 = (Word16*)L_add((Word32)ptr2, QMF_CHANNELS * sizeof(*ptr2));

    h_ps_e->histQmfRightImag[i] = ptr2;                                                    move16();
    ptr2 =(Word16*) L_add((Word32)ptr2, QMF_CHANNELS * sizeof(*ptr2));


    h_ps_e->tempQmfLeftReal[i] = ptr3;                                                     move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, QMF_CHANNELS * sizeof(*ptr3));

    h_ps_e->tempQmfLeftImag[i] = ptr3;                                                     move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, QMF_CHANNELS * sizeof(*ptr3));

    h_ps_e->tempQmfRightReal[i] = ptr3;                                                    move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, QMF_CHANNELS * sizeof(*ptr3));

    h_ps_e->tempQmfRightImag[i] = ptr3;                                                    move16();
    ptr3 = (Word16*)L_add((Word32)ptr3, QMF_CHANNELS * sizeof(*ptr3));
  }

  /* ptr4 is used as temporary bitstream buffer */
  test();
  if (h_ps_e->psBitBuf.isValid == 0) {
    CreateBitBuffer(&h_ps_e->psBitBuf,
                    (UWord8*)ptr4,
                    (255+15));

    for (i=0; i<h_ps_e->iidIccBins; i++) {
      h_ps_e->aaaICCDataBuffer[i] = 0x7fffffff;                                            move32();
    }
  }

  return 0;
}


/***************************************************************************/
/*!
  \brief  Extracts the parameters (IID, ICC) of the current frame.
 
****************************************************************************/
void
EncodePsFrame(HANDLE_PS_ENC pms,     /*!< Parametric stereo encoder instance */
              Word16 **iBufferLeft,  /*!< Imaginary part of QMF matrix for left channel */
              Word16 **rBufferLeft,  /*!< Real part of QMF matrix for left channel */
              Word16 **iBufferRight, /*!< Imaginary part of QMF matrix for right channel */
              Word16 **rBufferRight, /*!< Real part of QMF matrix for right channel */
              Word32 **energyValues, /*!< Resulting energies */
              Word16 *leftScale,     /*!< Scale of left channel (may be modified) */
              Word16 *rightScale,    /*!< Scale of right channel (may be modified) */
              Word16 *energyScale)       
{
  Word32  env;
  Word32  i;
  Word16  shift, nrgShift;
  Word32  slot,bin;
  Word32  subband, maxSubband;
  Word16  startSample, stopSample;
  Word16  commonScale;

  Word16 **hybrLeftImag = NULL;
  Word16 **hybrLeftReal = NULL;
  Word16 **hybrRightImag = NULL;
  Word16 **hybrRightReal = NULL;

  Word32 **hybrLeftReal32;
  Word32 **hybrLeftImag32;
  Word32 **hybrRightReal32;
  Word32 **hybrRightImag32;

  /*
    Determine common scale
  */
  commonScale = S_min(*leftScale, *rightScale);  /*! \todo 1 bit of headroom? */

  test(); sub(1, 1);
  if (pms->prevScale < commonScale) {
    /* Old values may be larger, check for headroom */
    Word16 headroom = SHORT_BITS;                                                       move16();
    for (slot=0; slot<HYBRID_FILTER_DELAY; slot++) {
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfLeftReal[slot], NO_QMF_BANDS_IN_HYBRID));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfLeftImag[slot], NO_QMF_BANDS_IN_HYBRID));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfRightReal[slot], NO_QMF_BANDS_IN_HYBRID));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfRightImag[slot], NO_QMF_BANDS_IN_HYBRID));
    }
    for ( ; slot<QMF_BUFFER_MOVE; slot++) {
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfLeftReal[slot], QMF_CHANNELS));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfLeftImag[slot], QMF_CHANNELS));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfRightReal[slot], QMF_CHANNELS));
      headroom = S_min(headroom, ffr_getScalefactorOfShortVector(pms->histQmfRightImag[slot], QMF_CHANNELS));
    }
    commonScale = S_min(commonScale, add(pms->prevScale, headroom));
  }

  /* Rescale old values in delay buffer */
  shift = sub(pms->prevScale, commonScale);
  test(); test();
  if (shift > 0) {
    for (slot=0; slot<HYBRID_FILTER_DELAY; slot++) {
      for (bin=0; bin<NO_QMF_BANDS_IN_HYBRID; bin++) {
        pms->histQmfLeftReal[slot][bin] = etsiopround(L_shr(pms->histQmfLeftReal[slot][bin], sub(shift, 16)));     move16();
        pms->histQmfLeftImag[slot][bin] = etsiopround(L_shr(pms->histQmfLeftImag[slot][bin], sub(shift, 16)));     move16();
        pms->histQmfRightReal[slot][bin] = etsiopround(L_shr(pms->histQmfRightReal[slot][bin], sub(shift, 16)));   move16();
        pms->histQmfRightImag[slot][bin] = etsiopround(L_shr(pms->histQmfRightImag[slot][bin], sub(shift, 16)));   move16();
      }
    }
    for ( ; slot<QMF_BUFFER_MOVE; slot++) {
      for (bin=0; bin<QMF_CHANNELS; bin++) {
        pms->histQmfLeftReal[slot][bin] = etsiopround(L_shr(pms->histQmfLeftReal[slot][bin], sub(shift, 16)));     move16();
        pms->histQmfLeftImag[slot][bin] = etsiopround(L_shr(pms->histQmfLeftImag[slot][bin], sub(shift, 16)));     move16();
        pms->histQmfRightReal[slot][bin] = etsiopround(L_shr(pms->histQmfRightReal[slot][bin], sub(shift, 16)));   move16();
        pms->histQmfRightImag[slot][bin] = etsiopround(L_shr(pms->histQmfRightImag[slot][bin], sub(shift, 16)));   move16();
      }
    }
  }
  else if (shift < 0) {
    for (slot=0; slot<HYBRID_FILTER_DELAY; slot++) {
      for (bin=0; bin<NO_QMF_BANDS_IN_HYBRID; bin++) {
        pms->histQmfLeftReal[slot][bin] = shr(pms->histQmfLeftReal[slot][bin], shift);                             move16();
        pms->histQmfLeftImag[slot][bin] = shr(pms->histQmfLeftImag[slot][bin], shift);                             move16();
        pms->histQmfRightReal[slot][bin] = shr(pms->histQmfRightReal[slot][bin], shift);                           move16();
        pms->histQmfRightImag[slot][bin] = shr(pms->histQmfRightImag[slot][bin], shift);                           move16();
      }
    }
    for ( ; slot<QMF_BUFFER_MOVE; slot++) {
      for (bin=0; bin<QMF_CHANNELS; bin++) {
        pms->histQmfLeftReal[slot][bin] = shr(pms->histQmfLeftReal[slot][bin], shift);                             move16();
        pms->histQmfLeftImag[slot][bin] = shr(pms->histQmfLeftImag[slot][bin], shift);                             move16();
        pms->histQmfRightReal[slot][bin] = shr(pms->histQmfRightReal[slot][bin], shift);                           move16();
        pms->histQmfRightImag[slot][bin] = shr(pms->histQmfRightImag[slot][bin], shift);                           move16();
      }
    }
  }

  /* Rescale input data of left channel */
  shift = sub(*leftScale, commonScale);
  assert(shift >= 0);

  test();
  if (shift) {
    for (slot=0; slot<QMF_TIME_SLOTS; slot++) {
      for (bin=0; bin<QMF_CHANNELS; bin++) {
        iBufferLeft[slot][bin] = shr(iBufferLeft[slot][bin], shift);                                               move16();
        rBufferLeft[slot][bin] = shr(rBufferLeft[slot][bin], shift);                                               move16();
      }
    }
  }
  /* Rescale input data of right channel */
  shift = sub(*rightScale, commonScale);
  assert(shift >= 0);

  test();
  if (shift) {
    for (slot=0; slot<QMF_TIME_SLOTS; slot++) {
      for (bin=0; bin<QMF_CHANNELS; bin++) {
        iBufferRight[slot][bin] = shr(iBufferRight[slot][bin], shift);                                             move16();
        rBufferRight[slot][bin] = shr(rBufferRight[slot][bin], shift);                                             move16();
      }
    }
  }

  nrgShift = shl(sub(commonScale, pms->prevScale), 1);
  pms->prevScale = commonScale;                                                                                    move16();
  *leftScale = commonScale;                                                                                        move16();
  *rightScale = commonScale;                                                                                       move16();
  *energyScale = sub(shl(*leftScale, 1), 1);                                                                       move16();

  /* Transform lowest QMF bands of left channel to hybrid domain */
  HybridAnalysis ( (const Word16**) rBufferLeft,
                   (const Word16**) iBufferLeft,
                   pms->mHybridRealLeft,
                   pms->mHybridImagLeft,
                   pms->histQmfLeftReal,
                   pms->histQmfLeftImag);

  /* Transform lowest QMF bands of right channel to hybrid domain */
  HybridAnalysis ( (const Word16**) rBufferRight,
                   (const Word16**) iBufferRight,
                   pms->mHybridRealRight,
                   pms->mHybridImagRight,
                   pms->histQmfRightReal,
                   pms->histQmfRightImag);

  /* Analyze spectral data */
  for ( env = 0; env < 2; env++ ) {
    hybrLeftReal32  = pms->mHybridRealLeft;
    hybrLeftImag32  = pms->mHybridImagLeft;
    hybrRightReal32 = pms->mHybridRealRight;
    hybrRightImag32 = pms->mHybridImagRight;

    test();
    if ( env == 0  ) {
      startSample   = 0;                                                          move16();
      stopSample    = QMF_TIME_SLOTS/2;                                           move16();
    }
    else {
      startSample   = QMF_TIME_SLOTS/2;                                           move16();
      stopSample    = QMF_TIME_SLOTS;                                             move16();
    }

    for ( bin = 0; bin < NO_BINS; bin++ ) {
      test(); sub(1, 1);
      if ( bin >= SUBQMF_BINS_ENERGY ) {
        test();
        if ( env == 0 ) {
          hybrLeftReal  = (Word16**)L_add((Word32)pms->histQmfLeftReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrLeftImag  = (Word16**)L_add((Word32)pms->histQmfLeftImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrRightReal = (Word16**)L_add((Word32)pms->histQmfRightReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrRightImag = (Word16**)L_add((Word32)pms->histQmfRightImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
        }
        else {
          hybrLeftReal  = (Word16**)L_sub((Word32)rBufferLeft, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrLeftImag  = (Word16**)L_sub((Word32)iBufferLeft, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrRightReal = (Word16**)L_sub((Word32)rBufferRight, HYBRID_FILTER_DELAY*sizeof(Word16**));
          hybrRightImag = (Word16**)L_sub((Word32)iBufferRight, HYBRID_FILTER_DELAY*sizeof(Word16**));
        }
      }

      test(); sub(1, 1);
      maxSubband = ( bin < SUBQMF_BINS_ENERGY )? add(hiResBandBorders[bin], 1): hiResBandBorders[bin + 1];  logic16(); logic16(); /* Word8 read access */

    
      {
        Word32 accu1;
        Word32 accu2;
        Word32 accu3;
        Word32 accu4;

        accu1 = 0;                                                                move32();
        accu2 = 0;                                                                move32();
        accu3 = 0;                                                                move32();
        accu4 = 0;                                                                move32();

        for ( i = startSample; i < stopSample; i++ ) {

          test(); test(); sub(1, 1); sub(1, 1);
          if ( bin >= SUBQMF_BINS_ENERGY && i == HYBRID_FILTER_DELAY ) {
            hybrLeftReal  = (Word16**)L_sub((Word32)rBufferLeft, HYBRID_FILTER_DELAY*sizeof(Word16**));
            hybrLeftImag  = (Word16**)L_sub((Word32)iBufferLeft, HYBRID_FILTER_DELAY*sizeof(Word16**));
            hybrRightReal = (Word16**)L_sub((Word32)rBufferRight, HYBRID_FILTER_DELAY*sizeof(Word16**));
            hybrRightImag = (Word16**)L_sub((Word32)iBufferRight, HYBRID_FILTER_DELAY*sizeof(Word16**));
          }

          logic16(); logic16(); /* Word8 read access */
          for ( subband = hiResBandBorders[bin]; subband < maxSubband; subband++ ) {

            Word16 tempLeftReal;
            Word16 tempLeftImag;
            Word16 tempRightReal;
            Word16 tempRightImag;

            test(); sub(1, 1);
            if (bin >= SUBQMF_BINS_ENERGY) {
              tempLeftReal  = hybrLeftReal [i][subband];
              tempLeftImag  = hybrLeftImag [i][subband];
              tempRightReal = hybrRightReal[i][subband];
              tempRightImag = hybrRightImag[i][subband];
            }
            else {
              tempLeftReal  = etsiopround(hybrLeftReal32 [i][subband]);
              tempLeftImag  = etsiopround(hybrLeftImag32 [i][subband]);
              tempRightReal = etsiopround(hybrRightReal32[i][subband]);
              tempRightImag = etsiopround(hybrRightImag32[i][subband]);
            }

            /* The bins are shifted down so that no overflow bits
               are required for storage.
            */
            shift = 3;
            tempLeftReal  = shr(tempLeftReal, shift);
            tempLeftImag  = shr(tempLeftImag, shift);
            tempRightReal  = shr(tempRightReal, shift);
            tempRightImag  = shr(tempRightImag, shift);

            accu1 = L_mac( accu1, tempLeftReal, tempLeftReal);
            accu1 = L_mac( accu1, tempLeftImag, tempLeftImag);
          
            accu2 = L_mac( accu2, tempRightReal, tempRightReal);
            accu2 = L_mac( accu2, tempRightImag, tempRightImag);
          
            accu3 = L_mac( accu3, tempLeftReal, tempRightReal);
            accu3 = L_mac( accu3, tempLeftImag, tempRightImag);
          
            accu4 = L_mac( accu4, tempLeftImag, tempRightReal);
            accu4 = L_msu( accu4, tempLeftReal, tempRightImag);
          }
        }   /* for (i) */

        test();
        if ( env == 0 ) {
          test();  test();
          if (nrgShift >= 0) {
            pms->powerLeft[bin] = L_add(pms->powerLeft[bin], L_shr(accu1, nrgShift));                        move32();
            pms->powerRight[bin] = L_add(pms->powerRight[bin], L_shr(accu2, nrgShift));                      move32();
            pms->powerCorrReal[bin] = L_add(pms->powerCorrReal[bin], L_shr(accu3, nrgShift));                move32();
            pms->powerCorrImag[bin] = L_add(pms->powerCorrImag[bin], L_shr(accu4, nrgShift));                move32();
          }
          else if (nrgShift < 0) {
            pms->powerLeft[bin] = L_add(L_shl(pms->powerLeft[bin], nrgShift), accu1);                        move32();
            pms->powerRight[bin] = L_add(L_shl(pms->powerRight[bin], nrgShift), accu2);                      move32();
            pms->powerCorrReal[bin] = L_add(L_shl(pms->powerCorrReal[bin], nrgShift), accu3);                move32();
            pms->powerCorrImag[bin] = L_add(L_shl(pms->powerCorrImag[bin], nrgShift), accu4);                move32();
          }
        }
        else {
          pms->powerLeft[bin] = accu1;                                                                       move32();
          pms->powerRight[bin] = accu2;                                                                      move32();
          pms->powerCorrReal[bin] = accu3;                                                                   move32();
          pms->powerCorrImag[bin] = accu4;                                                                   move32();
        }
      }

    } /* bins loop */

    test();
    if (env == 0) {
      Word32  tempLeft;
      Word32  tempRight;
      Word32  tempCorrR;
      Word32  tempCorrI;
      Word32  lr;
      Word16  shiftNrg, shiftCorr, bin;

      for ( bin = 0; bin < pms->iidIccBins; bin++ ) {
        test();
        if ( pms->bHiFreqResIidIcc ) {
          tempLeft  = pms->powerLeft    [bin];
          tempRight = pms->powerRight   [bin];
          tempCorrR = pms->powerCorrReal[bin];
          tempCorrI = pms->powerCorrImag[bin];
        }
        else {
          tempLeft  = L_add(pms->powerLeft    [2 * bin], pms->powerLeft    [2 * bin + 1]);
          tempRight = L_add(pms->powerRight   [2 * bin], pms->powerRight   [2 * bin + 1]);
          tempCorrR = L_add(pms->powerCorrReal[2 * bin], pms->powerCorrReal[2 * bin + 1]);
          tempCorrI = L_add(pms->powerCorrImag[2 * bin], pms->powerCorrImag[2 * bin + 1]);
        }

        /*
          Shift left to gain accuracy
        */
        shiftNrg = S_min( ffr_norm32(tempRight), ffr_norm32(tempLeft) );
        tempRight = L_shl(tempRight, shiftNrg);
        tempLeft = L_shl(tempLeft, shiftNrg);

        /*
          Derive ICC
        */
        test(); sub(1, 1);
        if (bin > NO_IPD_BINS) {
          shiftCorr = sub(S_min( ffr_norm32(tempCorrR), ffr_norm32(tempCorrI) ), 1);
          tempCorrR = L_shl(tempCorrR, shiftCorr);
          tempCorrI = L_shl(tempCorrI, shiftCorr);
          tempCorrR = L_add(fixmul(tempCorrR, tempCorrR), fixmul(tempCorrI, tempCorrI));
        }
        else {
          shiftCorr = ffr_norm32(tempCorrR);
          tempCorrR = L_shl(tempCorrR, shiftCorr);

          test();
          if (tempCorrR > 0x00000000) {
            tempCorrR = fixmul(tempCorrR, tempCorrR);
          }
          else {
            tempCorrR = L_negate(fixmul(tempCorrR, tempCorrR));
          }
        }
        lr = fixmul(tempLeft, tempRight);
        tempCorrR = L_shl(tempCorrR , shl(sub(shiftNrg, shiftCorr), 1));

        test(); L_sub(1, 1);
        if (L_abs(tempCorrR) >= L_abs(lr)) {
          test();
          pms->aaaICCDataBuffer[bin] = fixmul(tempCorrR, lr) < 0x00000000 ? 0x80000000 : 0x7fffffff;         move32();
        }
        else {
          pms->aaaICCDataBuffer[bin] = ffr_div32_32(tempCorrR, lr);                                          move32();
        }

        /*
          Derive IID
        */
        tempLeft = L_shr(tempLeft, 8);
        test(); L_sub(1, 1);
        if (tempLeft >= tempRight) {
          pms->aaaIIDDataBuffer[bin] = 0x7fffffff;                                                           move32();
        }
        else {
          pms->aaaIIDDataBuffer[bin] = ffr_div32_32(tempLeft, tempRight);                                    move32();  
        }
      }

    } /* if (env==0) */
  } /* envelopes loop */

  downmixToMono( pms, rBufferLeft, iBufferLeft, rBufferRight, iBufferRight, energyValues, leftScale);
}


/***************************************************************************/
/*!
  \brief  Generates mono downmix and energy array

****************************************************************************/
void
downmixToMono( HANDLE_PS_ENC pms,
               Word16 **qmfLeftReal,  /*!< Real part of the QMF subsamples (left channel input) */
               Word16 **qmfLeftImag,  /*!< Imaginary part of the QMF subsamples (left channel input) */
               Word16 **qmfRightReal, /*!< Real part of the QMF subsamples (right channel input / downmixed output) */
               Word16 **qmfRightImag, /*!< Imaginary part of the QMF subsamples (right channel input / downmixed output) */
               Word32 **energyValues, /*!< Resulting energies */
               Word16 *scale)         /*!< Scale of subband samples (will be modified) */
{
  Word16   i;
  Word16   group;
  Word16   subband;
  Word16   maxSubband;
  Word32   tmp1, tmp2;
  Word32   energy;    /* Total energy of left and right channel */
  Word32   energyAv;  /* Energy of averaged channels (L+R)/2    */
  Word32   corr;

  Word16 **hybrLeftReal = NULL;
  Word16 **hybrLeftImag = NULL;
  Word16 **hybrRightReal = NULL;
  Word16 **hybrRightImag = NULL;

  Word32 **hybrLeftReal32;
  Word32 **hybrLeftImag32;
  Word32 **hybrRightReal32;
  Word32 **hybrRightImag32;

  Word32 **qmfMixedReal = (Word32 **)qmfRightReal;
  Word32 **qmfMixedImag = (Word32 **)qmfRightImag;

  *scale = sub(*scale, 2);

  for ( i = 0; i < QMF_BUFFER_MOVE; i++ ) {
    Word32 bands = i < HYBRID_FILTER_DELAY ? NO_QMF_BANDS_IN_HYBRID : QMF_CHANNELS;   test();
    memcpy( pms->tempQmfLeftReal [i], qmfLeftReal[QMF_TIME_SLOTS-QMF_BUFFER_MOVE+i], bands * sizeof(Word16) );    memop16(bands);
    memcpy( pms->tempQmfLeftImag [i], qmfLeftImag[QMF_TIME_SLOTS-QMF_BUFFER_MOVE+i], bands * sizeof(Word16) );    memop16(bands);
    memcpy( pms->tempQmfRightReal[i], qmfRightReal[QMF_TIME_SLOTS-QMF_BUFFER_MOVE+i], bands * sizeof(Word16) );   memop16(bands);
    memcpy( pms->tempQmfRightImag[i], qmfRightImag[QMF_TIME_SLOTS-QMF_BUFFER_MOVE+i], bands * sizeof(Word16) );   memop16(bands);
  }

  hybrLeftReal32  = pms->mHybridRealLeft;
  hybrLeftImag32  = pms->mHybridImagLeft;
  hybrRightReal32 = pms->mHybridRealRight;
  hybrRightImag32 = pms->mHybridImagRight;

  for ( group = NO_IPD_GROUPS-1; group >= 0; group-- ) {
    test(); sub(1, 1);
    if ( group >= SUBQMF_GROUPS_MIX ) {
      hybrLeftReal  = (Word16**)L_sub((Word32)qmfLeftReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
      hybrLeftImag  = (Word16**)L_sub((Word32)qmfLeftImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
      hybrRightReal = (Word16**)L_sub((Word32)qmfRightReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
      hybrRightImag = (Word16**)L_sub((Word32)qmfRightImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
    }

    test(); sub(1, 1);
    maxSubband = ( group < SUBQMF_GROUPS_MIX )? add(groupBordersMix[group], 1): groupBordersMix[group + 1];        logic16(); logic16(); /* Word8 read access */


    for ( i = QMF_TIME_SLOTS-1; i >= 0; i-- ) {
      test(); test(); sub(1, 1); sub(1, 1);
      if ( group >= SUBQMF_GROUPS_MIX && i == HYBRID_FILTER_DELAY-1 ) {
        hybrLeftReal  = (Word16**)L_add((Word32)pms->histQmfLeftReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
        hybrLeftImag  = (Word16**)L_add((Word32)pms->histQmfLeftImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
        hybrRightReal = (Word16**)L_add((Word32)pms->histQmfRightReal, HYBRID_FILTER_DELAY*sizeof(Word16**));
        hybrRightImag = (Word16**)L_add((Word32)pms->histQmfRightImag, HYBRID_FILTER_DELAY*sizeof(Word16**));
      }

      test(); logic16(); /* Word8 test access */
      for ( subband = sub(maxSubband, 1); subband >= groupBordersMix[group]; subband-- ) {
        Word16 mixcoeff;
        Word16 shift;
        Word16 tempLeftReal;
        Word16 tempLeftImag;
        Word16 tempRightReal;
        Word16 tempRightImag;

        /* Calculate energy and correlation ...
           ... for the first sample of the SBR timeslot */
        test(); sub(1, 1);
        if (group >= SUBQMF_GROUPS_MIX) {
          tempLeftReal  = hybrLeftReal [i][subband];
          tempLeftImag  = hybrLeftImag [i][subband];
          tempRightReal = hybrRightReal[i][subband];
          tempRightImag = hybrRightImag[i][subband];
        }
        else {
          tempLeftReal  = etsiopround(hybrLeftReal32 [i][subband]);
          tempLeftImag  = etsiopround(hybrLeftImag32 [i][subband]);
          tempRightReal = etsiopround(hybrRightReal32[i][subband]);
          tempRightImag = etsiopround(hybrRightImag32[i][subband]);
        }

        tmp1 = L_add( L_shr(L_mult(tempLeftReal, tempLeftReal), 3), L_shr(L_mult(tempLeftImag, tempLeftImag), 3) );
        tmp2 = L_add( L_shr(L_mult(tempRightReal, tempRightReal), 3), L_shr(L_mult(tempRightImag, tempRightImag), 3) );
        energy = L_add( tmp1, tmp2 );

        /* Store total energy of all 4 complex subband samples
           (2 channels, 2 time slots), calc energies below SUBQMF_GROUPS_MIX later */
        test(); sub(1, 1);
        if (group >= SUBQMF_GROUPS_MIX) {
          test(); logic16();
          if (i%2) { /* i+1 */
            energyValues[(i-1)>>1][subband] = energy;                                             move32();
          }
          else {     /* i */
            energyValues[i>>1][subband] = L_add(energyValues[i>>1][subband], energy);             move32();
          }
        }

        corr = L_add( L_shr(L_mult(tempLeftReal, tempRightReal), 2), L_shr(L_mult(tempLeftImag, tempRightImag), 2) );

        energyAv = L_add( energy, corr );

        /*
          Calculate a compensation factor for the energy loss introduced
          by different phases of the left and the right channel.
        */
        shift = sub(ffr_norm32(energy), 2);

        energy = L_shl( energy, shift );
        energy = L_shr( energy, 3 );
        energyAv = L_shl( energyAv, shift );

        test(); L_sub(1, 1);
        if (energy >= energyAv) {
          test();
          if (energy != 0)
            mixcoeff = MAX_MIX_COEFF;
          else
            mixcoeff = MIN_MIX_COEFF;
        }
        else {
          energy = ffr_div32_32(energy, energyAv);
          mixcoeff = etsiopround( ffr_sqrt(energy, 16) );
        }
        test(); sub(1, 1);
        if (group >= SUBQMF_GROUPS_MIX) {
          tmp1 = L_mult(extract_l(L_shr(L_add( tempLeftReal, tempRightReal ), 1)), mixcoeff );
          tmp2 = L_mult(extract_l(L_shr(L_add( tempLeftImag, tempRightImag ), 1)), mixcoeff );
        }
        else {
          tmp1 = fixmul_32x16b( L_add( L_shr(hybrLeftReal32[i][subband], 1), L_shr(hybrRightReal32[i][subband], 1) ), mixcoeff );
          tmp2 = fixmul_32x16b( L_add( L_shr(hybrLeftImag32[i][subband], 1), L_shr(hybrRightImag32[i][subband], 1) ), mixcoeff );
        }

        test(); sub(1, 1);
        if (group < SUBQMF_GROUPS_MIX) {
          /* store mono hybrid values in left channel */
          hybrLeftReal32[i][subband] = tmp1;                                         move32();
          hybrLeftImag32[i][subband] = tmp2;                                         move32();
        }
        else {
          /* only lower half is needed for qmf synthesis */
          test(); sub(1, 1);
          if (subband < NO_SYNTHESIS_CHANNELS) {
            qmfMixedReal[i][subband] = tmp1;                                         move32();
            qmfMixedImag[i][subband] = tmp2;                                         move32();
          }
          /* store mono short values in left channel */
          qmfLeftReal [i][subband]   = etsiopround( tmp1 );                          move16();
          qmfLeftImag [i][subband]   = etsiopround( tmp2 );                          move16();
        }
      }

    }
  }

  HybridSynthesis( ( const Word32** )hybrLeftReal32,
                   ( const Word32** )hybrLeftImag32,
                   (Word32**)qmfMixedReal,
                   (Word32**)qmfMixedImag,
                   qmfLeftReal,
                   qmfLeftImag);

  /* Store total energy of all 4 complex subband samples
     (2 channels, 2 time slots) */
  for ( subband = NO_QMF_BANDS_IN_HYBRID-1; subband >= 0; subband-- ) {
    for ( i = QMF_TIME_SLOTS-1; i >= 0; i-- ) {
      energy = L_add( L_shl(L_mult(qmfLeftReal[i][subband], qmfLeftReal[i][subband]), 2),
                      L_shl(L_mult(qmfLeftImag[i][subband], qmfLeftImag[i][subband]), 2) );
      test(); logic16();
      if (i%2) { /* i+1 */
        energyValues[i>>1][subband] = energy;                                                move32();
      }
      else {    /* i */
        energyValues[i>>1][subband] = L_add(energyValues[i>>1][subband], energy);            move32();
      }
    }
  }

  for ( i = 0; i < QMF_BUFFER_MOVE; i++ ) {
    Word32 bands = i < HYBRID_FILTER_DELAY ? NO_QMF_BANDS_IN_HYBRID : QMF_CHANNELS;  test(); sub(1, 1);
    memcpy( pms->histQmfLeftReal [i], pms->tempQmfLeftReal [i], bands * sizeof(Word16) );    memop16(bands);
    memcpy( pms->histQmfLeftImag [i], pms->tempQmfLeftImag [i], bands * sizeof(Word16) );    memop16(bands);
    memcpy( pms->histQmfRightReal[i], pms->tempQmfRightReal[i], bands * sizeof(Word16) );    memop16(bands);
    memcpy( pms->histQmfRightImag[i], pms->tempQmfRightImag[i], bands * sizeof(Word16) );    memop16(bands);
  }

}
