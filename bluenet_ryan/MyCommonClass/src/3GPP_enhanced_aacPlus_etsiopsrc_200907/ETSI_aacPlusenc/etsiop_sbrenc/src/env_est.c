/*
   Envelope estimation
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "cmondata.h"
#include "sbr.h"
#include "env_est.h"
#include "tran_det.h"
#include "qmf_enc.h"
#include "fram_gen.h"
#include "bit_sbr.h"
#include "sbr_ram.h"
#include "sbr_misc.h"
#include "ps_enc.h"
#include "count.h"


#define QUANT_ERROR_THRES 200


/***************************************************************************/
/*!
 
\brief  Quantisation of the panorama value (balance)
  
\return the quantized pan value
 
****************************************************************************/
static Word16 
mapPanorama (Word16 nrgVal,     /*! integer value of the energy */
             Word16 ampRes,     /*! amplitude resolution [1.5/3dB] */
             Word32 *quantError /*! quantization error of energy val*/
             )
{
  Word16 i;
  Word32 min_val, val;
  Word16 panTable[2][10] = {{0,2,4,6,8,12,16,20,24},
                            { 0, 2, 4, 8, 12 }};
  Word16 maxIndex[2] = {9,5};

  Word16 panIndex;
  Word16 sign;

  test();
  sign = nrgVal > 0 ? 1 : -1;

  assert(abs(nrgVal) <= 0x7fff);
  nrgVal = abs_s(nrgVal);

  min_val = INT_MAX;                                                                                 move32();
  panIndex = 0;                                                                                      move32();
  for (i = 0; i < maxIndex[ampRes]; i++) {
    val = L_abs(L_sub(L_deposit_l(nrgVal), L_deposit_l(panTable[ampRes][i])));
    test(); L_sub(1, 1);
    if (val < min_val) {
      min_val = val;                                                                                 move32();
      panIndex = i;                                                                                  move16();
    }
  }

  *quantError=min_val;                                                                               move32();

  test(); sub(1, 1);
  if ( sign == 1 )
    return add(panTable[ampRes][maxIndex[ampRes]-1], panTable[ampRes][panIndex]);
  else
    return sub(panTable[ampRes][maxIndex[ampRes]-1], panTable[ampRes][panIndex]);
}


/***************************************************************************/
/*!
 
\brief  Quantisation of the noise floor levels
  
\return void
 
****************************************************************************/
static void 
sbrNoiseFloorLevelsQuantisation (Word16 *iNoiseLevels, /*! quantized noise levels */
                                 Word32 *NoiseLevels   /*! the noise levels  */
                                 )
{
  Word32 i;

  /* Quantisation, similar to sfb quant... */
  for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
    Word16 tmp;
    tmp = shr(sub(NOISE_FLOOR_OFFSET*4-2*4+2, ffr_iLog4(NoiseLevels[i])), 2); /* 2*4 : noise levels scaled by a factor of 4 */
    tmp = S_min(tmp,30);
    iNoiseLevels[i] = tmp;                                                                           move16();
  }
}

/***************************************************************************/
/*!
 
\brief  Quantisation of the noise floor levels, coupling mode
  
\return void
 
****************************************************************************/
static void 
sbrNoiseFloorLevelsQuantisationCoupling(Word16 *iNoiseLevelsLeft,  /*! quantized noise levels */
                                        Word16 *iNoiseLevelsRight, /*! quantized noise levels */
                                        Word32 *NoiseLevelsLeft,   /*! the noise levels  */
                                        Word32 *NoiseLevelsRight   /*! the noise levels  */
                                        )
{
  Word32 i;
  Word32 dummy;

  for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
    Word16 tmp;

    tmp = shr(sub(NOISE_FLOOR_OFFSET*4+2, ffr_iLog4(L_add(L_shr(NoiseLevelsLeft[i], 1), L_shr(NoiseLevelsRight[i], 1)))), 2);
    tmp = S_min(tmp,30);

    iNoiseLevelsLeft[i] = tmp;                                                                      move16();

    tmp = shr(add(sub(ffr_iLog4(NoiseLevelsLeft[i]), ffr_iLog4(NoiseLevelsRight[i])), 2), 2);
    tmp = S_min(tmp,30);
    tmp = S_max(tmp,-30);

    iNoiseLevelsRight[i] = mapPanorama (tmp,1,&dummy);                                              move16();

  }
}


/***************************************************************************/
/*!
 
\brief  calculates the envelope values from the energies, depending on
framing and stereo mode

\return void
 
****************************************************************************/
static void 
calculateSbrEnvelope (Word32 **YBufferLeft,             /*! energy buffer left */
                      Word32 **YBufferRight,            /*! energy buffer right */
                      Word16 *yBufferScaleLeft,         /*! Y-Buffer scale left */
                      Word16 *yBufferScaleRight,        /*! Y-Buffer scale right */
                      const SBR_FRAME_INFO *frame_info, /*! frame info vector */
                      Word16 *sfb_nrgLeft,              /*! sfb energy buffer left */
                      Word16 *sfb_nrgRight,             /*! sfb energy buffer right */
                      HANDLE_SBR_CONFIG_DATA h_con,     /*! handle to congif data   */
                      HANDLE_ENV_CHANNEL h_sbr,         /*! envelope channel handle */
                      SBR_STEREO_MODE stereoMode,       /*! stereo coding mode */
                      Word32* maxQuantError)            /*! maximum quantization error, for panorama. */
                      
{
  Word16 i, j, k, l, m = 0;
  Word16 no_of_bands, start_pos, stop_pos, li, ui;
  FREQ_RES freq_res;
  Word16 logEnergy;

  Word16 ca = sub(2, h_sbr->encEnvData.init_sbr_amp_res);
  Word32 quantError;
  Word16 nEnvelopes = frame_info->nEnvelopes;
  Word16 short_env = sub(frame_info->shortEnv, 1);
  Word16 timeStep = h_sbr->sbrExtractEnvelope.time_step;
  Word16 missingHarmonic = 0;

  Word16 scfLeft0,scfLeft1;
  Word16 scfRight0 = 0, scfRight1 = 0;
  Word16 minScale;
  Word16 noCols = h_sbr->sbrExtractEnvelope.no_cols;
  
  /*
    determinate min scale factor
  */

  minScale = S_min(yBufferScaleLeft[0],yBufferScaleLeft[1]);
  test(); sub(1, 1);
  if(stereoMode == SBR_COUPLING){
    minScale = S_min(minScale,yBufferScaleRight[0]);
    minScale = S_min(minScale,yBufferScaleRight[1]);
    scfRight0 = sub(yBufferScaleRight[0], minScale);
    scfRight1 = sub(yBufferScaleRight[1], minScale);
  }
  scfLeft0 = sub(yBufferScaleLeft[0], minScale);
  scfLeft1 = sub(yBufferScaleLeft[1], minScale);

  test(); sub(1, 1);
  if (stereoMode == SBR_COUPLING) {
    *maxQuantError = 0;                                                                              move32();
  }

  for (i = 0; i < nEnvelopes; i++) {

    start_pos = ffr_Short_Mult(timeStep, frame_info->borders[i]);
    stop_pos =  ffr_Short_Mult(timeStep, frame_info->borders[i + 1]);
    freq_res = frame_info->freqRes[i];
    no_of_bands = h_con->nSfb[freq_res];

    test(); sub(1, 1);
    if (i == short_env)
      stop_pos = sub(stop_pos, timeStep);


    for (j = 0; j < no_of_bands; j++) {
      Word16 count;
      Word16 iCount;
      Word32 nrgRight = 0;
      Word32 nrgLeft = 0;
      Word32 nrgTemp = 0;
    
      li = h_con->freqBandTable[freq_res][j];
      ui = h_con->freqBandTable[freq_res][j + 1];

      test(); sub(1, 1);
      if(freq_res == FREQ_RES_HIGH){
        test(); sub(1, 1);
        if(j == 0 && sub(ui, li) > 1){
          li = add(li, 1);
        }
      }
      else{
        test(); sub(1, 1);
        if(j == 0 && sub(ui, li) > 2){
          li = add(li, 1);
        }
      }

      missingHarmonic = 0;

      test();
      if(h_sbr->encEnvData.addHarmonicFlag){

        test(); sub(1, 1);
        if(freq_res == FREQ_RES_HIGH){
          test();
          if(h_sbr->encEnvData.addHarmonic[j]){
            missingHarmonic = 1;
          }
        }
        else{
          Word32 i;
          Word16 startBandHigh = 0;
          Word16 stopBandHigh = 0;
          
          test();
          while(h_con->freqBandTable[FREQ_RES_HIGH][startBandHigh] < h_con->freqBandTable[FREQ_RES_LOW][j]) {
            startBandHigh = add(startBandHigh, 1);
            test();
          }
          test();
          while(h_con->freqBandTable[FREQ_RES_HIGH][stopBandHigh] < h_con->freqBandTable[FREQ_RES_LOW][j + 1]) {
            stopBandHigh = add(stopBandHigh, 1);
            test();
          }
          
          for(i = startBandHigh; i<stopBandHigh; i++){
            test();
            if(h_sbr->encEnvData.addHarmonic[i]){
              missingHarmonic = 1;
            }
          }
        }

      }

      test();
      if(missingHarmonic){        
        count = sub(stop_pos, start_pos);
        iCount = div_s(1, count);
 
        test(); test(); sub(1, 1); sub(1, 1); 
        if(sub(ui, li) > 2)
          iCount = mult_r(0x32f5, iCount); /* The maximum boost is 1.584893, so the maximum attenuation should be 
                                                  square(1/1.584893) = 0.398107267 */
        else if(ui-li > 1)
          iCount = mult_r(0x4000, iCount);
        
        for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
          nrgLeft = L_add(nrgLeft, L_shr(fixmul_32x16b(YBufferLeft[l/2][li], iCount), scfLeft0));
        }

        for (; l < stop_pos; l++) {
          nrgLeft = L_add(nrgLeft, L_shr(fixmul_32x16b(YBufferLeft[l/2][li], iCount), scfLeft1));
        }
        
        for (k = li+1; k < ui; k++){
          Word32 tmpNrg = 0;                                                                         move32();
      
          for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
            tmpNrg = L_add(tmpNrg, L_shr(fixmul_32x16b(YBufferLeft[l/2][k], iCount), scfLeft0));
          }
          for (; l < stop_pos; l++) {
            tmpNrg = L_add(tmpNrg, L_shr(fixmul_32x16b(YBufferLeft[l/2][k], iCount), scfLeft1));
          }

          test(); L_sub(1, 1);
          if(tmpNrg > nrgLeft){
            nrgLeft = tmpNrg;                                                                        move32();
          }
        }
        
        test(); sub(1, 1);
        if (stereoMode == SBR_COUPLING) {
          for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
            nrgRight = L_add(nrgRight, L_shr(fixmul_32x16b(YBufferRight[l/2][li], iCount), scfRight0));
          }

          for (; l < stop_pos; l++) {
            nrgRight = L_add(nrgRight, L_shr(fixmul_32x16b(YBufferRight[l/2][li], iCount), scfRight1));
          }

          
          for (k = li+1; k < ui; k++){
            Word32 tmpNrg = 0;                                                                       move32();
            
            for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
              tmpNrg = L_add(tmpNrg, L_shr(fixmul_32x16b(YBufferRight[l/2][k], iCount), scfRight0));
            }

            for (; l < stop_pos; l++) {
              tmpNrg = L_add(tmpNrg, L_shr(fixmul_32x16b(YBufferRight[l/2][k], iCount), scfRight1));
            }

            test(); L_sub(1, 1);
            if(tmpNrg > nrgRight){
              nrgRight = tmpNrg;                                                                     move32();
            }
          }
        
        }
        
        test(); sub(1, 1);
        if (stereoMode == SBR_COUPLING) {
          nrgTemp = nrgLeft;                                                                         move32();
          nrgLeft = L_shr(L_add(nrgRight, nrgLeft), 1);
        }
      } /* end missingHarmonic */
      else{
        count = ffr_Short_Mult(sub(stop_pos, start_pos), sub(ui, li));
        iCount = div_s(1, count);


        for (k = li; k < ui; k++){
          for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
            nrgLeft = L_add(nrgLeft, L_shr(fixmul_32x16b(YBufferLeft[l/2][k], iCount), scfLeft0));
          }
          for (; l < stop_pos; l++) {
            nrgLeft = L_add(nrgLeft, L_shr(fixmul_32x16b(YBufferLeft[l/2][k], iCount), scfLeft1));
          }


        }

        test(); sub(1, 1);
        if (stereoMode == SBR_COUPLING) {
          for (k = li; k < ui; k++){
            for (l = start_pos; l < S_min(noCols,stop_pos); l++) {
              nrgRight = L_add(nrgRight, L_shr(fixmul_32x16b(YBufferRight[l/2][k], iCount), scfRight0));
            }
            for (; l < stop_pos; l++) {
              nrgRight = L_add(nrgRight, L_shr(fixmul_32x16b(YBufferRight[l/2][k], iCount), scfRight1));
            }

            
          }
        }
        test(); sub(1, 1);
        if (stereoMode == SBR_COUPLING) {
          nrgTemp = nrgLeft;                                                                         move32();
        
          nrgLeft = L_shr(L_add(nrgRight, nrgLeft), 1);
        }
      }

      logEnergy = add(ffr_iLog4(nrgLeft), shl(sub(44-6, minScale), 2));  /*  44: qmfscale 6: h_sbr->sbrQmf.no_channels*/ 
      logEnergy = S_max(logEnergy,0);
      
      sfb_nrgLeft[m] = shr(add(ffr_Short_Mult(ca, logEnergy), 2), 2);                                                                 move16();

      test(); sub(1, 1);
      if (stereoMode == SBR_COUPLING) {

        logEnergy = sub(ffr_iLog4(nrgTemp), ffr_iLog4(nrgRight));
        
        sfb_nrgRight[m] = mapPanorama (shr(add(ffr_Short_Mult(ca, logEnergy), 2), 2),h_sbr->encEnvData.init_sbr_amp_res,&quantError); move16();
        move32(); test(); L_sub(1, 1);
        if(quantError > *maxQuantError) {
          *maxQuantError = quantError;                                                               move32();
        }
      }
      
      m = add(m, 1);
        
    } /* j */
	
    test();
    if (h_con->useParametricCoding) {
      m = sub(m, no_of_bands);
      for (j = 0; j < no_of_bands; j++) {
        test(); test(); sub(1, 1);
        if (freq_res==FREQ_RES_HIGH && h_sbr->sbrExtractEnvelope.envelopeCompensation[j]){
          sfb_nrgLeft[m] = sub(sfb_nrgLeft[m], 
               ffr_Short_Mult(ca, abs_s(h_sbr->sbrExtractEnvelope.envelopeCompensation[j])));           move16();
        }
        test();
        if(sfb_nrgLeft[m] < 0) {
          sfb_nrgLeft[m] = 0;                                                                           move16();
        }
        m = add(m, 1);
      }
    }
  } /* i*/
}


                   
/***************************************************************************/
/*!

\brief  calculates the noise floor and the envelope values from the 
energies, depending on framing and stereo mode

extractSbrEnvelope is the main function for encoding and writing the
envelope and the noise floor. The function includes the following processes:

-Analysis subband filtering.
-Encoding SA and pan parameters (if enabled).
-Transient detection.
-Determine time/frequency division of current granule.
-Sending transient info to bitstream.
-Set amp_res to 1.5 dB if the current frame contains only one envelope.
-Lock dynamic bandwidth frequency change if the next envelope not starts on a
frame boundary.
-MDCT transposer (needed to detect where harmonics will be missing).
-Spectrum Estimation (used for pulse train and missing harmonics detection).
-Pulse train detection.
-Inverse Filtering detection.
-Waveform Coding.
-Missing Harmonics detection.
-Extract envelope of current frame.
-Noise floor estimation.
-Noise floor quantisation and coding.
-Encode envelope of current frame.
-Send the encoded data to the bitstream.
-Write to bitstream.

****************************************************************************/
void
extractSbrEnvelope (const Word16 *timeInPtr,      /*!< time samples, always interleaved */
                    Word16 timeInStride,          /*!< stride of time domain input data */
                    HANDLE_SBR_CONFIG_DATA h_con, /*! handle to congif data   */
                    HANDLE_SBR_HEADER_DATA sbrHeaderData,
                    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData,
                    HANDLE_ENV_CHANNEL h_envChan[MAX_CHANNELS],
                    struct PS_ENC *hPsEnc,
                    HANDLE_SBR_QMF_FILTER_BANK hSynthesisQmfBank, /*!< Handle to QMF synthesis filterbank  */
                    Word16  *pCoreBuffer,                         /*!< Time data output for downmix */
                    HANDLE_COMMON_DATA hCmonData)
{
  Word16 ch, i, j, c;
  Word16 nEnvelopes[MAX_CHANNELS];
  Word16 transient_info[MAX_CHANNELS][2];
  
  const SBR_FRAME_INFO *frame_info[MAX_CHANNELS];

  Word32 nChannels = h_con->nChannels;
  Word32 nInChannels = (hPsEnc) ? 2:nChannels;

  SBR_STEREO_MODE stereoMode = h_con->stereoMode;

  FREQ_RES res[MAX_NUM_NOISE_VALUES];
  Word16 v_tuning[6] = { 0, 2, 4, 0, 0, 0 };
  Word32 maxQuantError;


  for (i=0; i<MAX_CHANNELS; i++) {
    for (j=0; j<2; j++) {
      transient_info[i][j] = 0;                                 move16();
    }
  }

  for(i=0; i<MAX_NUM_NOISE_VALUES; i++) {
    res[i] = FREQ_RES_HIGH;                                     move16();
  }

  for(ch = 0; ch < nInChannels;ch++)
    {
      /*
        Analysis subband filtering.

      */      
      sbrAnalysisFiltering    (timeInPtr ? timeInPtr+ch:NULL,
                               timeInStride, 
                               h_envChan[ch]->sbrExtractEnvelope.rBuffer+h_envChan[ch]->sbrExtractEnvelope.rBufferWriteOffset,
                               h_envChan[ch]->sbrExtractEnvelope.iBuffer+h_envChan[ch]->sbrExtractEnvelope.rBufferWriteOffset,
                               &h_envChan[ch]->sbrQmf);
    
    }


#ifndef MONO_ONLY

  test();
  if (hPsEnc) {
    EncodePsFrame(hPsEnc,
                  h_envChan[0]->sbrExtractEnvelope.iBuffer,
                  h_envChan[0]->sbrExtractEnvelope.rBuffer,
                  h_envChan[1]->sbrExtractEnvelope.iBuffer,
                  h_envChan[1]->sbrExtractEnvelope.rBuffer,
                  h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferWriteOffset,
                  &h_envChan[0]->sbrQmf.qmfScale,
                  &h_envChan[1]->sbrQmf.qmfScale,
                  &h_envChan[0]->sbrExtractEnvelope.yBufferScale[1]);


    /*
      Perform downsampling by applying a 32 channel QMF synthesis on the downmixed spectrum
    */
    SynthesisQmfFilteringEnc ((Word32**)h_envChan[1]->sbrExtractEnvelope.rBuffer,
                              (Word32**)h_envChan[1]->sbrExtractEnvelope.iBuffer,
                              h_envChan[0]->sbrQmf.qmfScale,
                              pCoreBuffer,
                              (HANDLE_SBR_QMF_FILTER_BANK)hSynthesisQmfBank);

  }

#endif /* #ifndef MONO_ONLY */


  for(ch = 0; ch < nChannels;ch++){
    test();
    if (!hPsEnc) {
      getEnergyFromCplxQmfData (h_envChan[ch]->sbrExtractEnvelope.YBuffer + h_envChan[ch]->sbrExtractEnvelope.YBufferWriteOffset,
                                h_envChan[ch]->sbrExtractEnvelope.rBuffer + h_envChan[ch]->sbrExtractEnvelope.rBufferReadOffset,
                                h_envChan[ch]->sbrExtractEnvelope.iBuffer + h_envChan[ch]->sbrExtractEnvelope.rBufferReadOffset,
                                h_envChan[ch]->sbrQmf.no_channels, h_envChan[ch]->sbrExtractEnvelope.no_cols);
    
      h_envChan[ch]->sbrExtractEnvelope.yBufferScale[1] = sub(shl(h_envChan[ch]->sbrQmf.qmfScale, 1), 1);
    }
    

    /*
      Precalculation of Tonality Quotas
    */
    CalculateTonalityQuotas(&h_envChan[ch]->TonCorr,
                            h_envChan[ch]->sbrExtractEnvelope.rBuffer+ h_envChan[ch]->sbrExtractEnvelope.rBufferWriteOffset,
                            h_envChan[ch]->sbrExtractEnvelope.iBuffer+ h_envChan[ch]->sbrExtractEnvelope.rBufferWriteOffset,
                            h_con->freqBandTable[HI][h_con->nSfb[HI]],
                            h_envChan[ch]->sbrQmf.qmfScale);


    /*
      Transient detection COEFF
    */
    transientDetect (h_envChan[ch]->sbrExtractEnvelope.YBuffer,
                     h_envChan[ch]->sbrExtractEnvelope.yBufferScale,
                     &h_envChan[ch]->sbrTransientDetector,
                     transient_info[ch],
                     h_envChan[ch]->sbrExtractEnvelope.time_step, 
                     h_envChan[ch]->SbrEnvFrame.frameMiddleSlot);
   
    /*
      frame Splitter COEFF
    */
    frameSplitter(h_envChan[ch]->sbrExtractEnvelope.YBuffer,
                  h_envChan[ch]->sbrExtractEnvelope.yBufferScale,
                  &h_envChan[ch]->sbrTransientDetector,
                  h_con->freqBandTable[1],
                  h_con->nSfb[1],
                  h_envChan[ch]->sbrExtractEnvelope.time_step,
                  h_envChan[ch]->sbrExtractEnvelope.no_cols,
                  transient_info[ch],
                  h_envChan[ch]->sbrExtractEnvelope.tmpEnergiesM);

  } /* ch */

  
#if (MAX_CHANNELS>1)
  /*
    Select stereo mode.
  */
  test(); sub(1, 1);
  if (stereoMode == SBR_COUPLING) {
    test(); test();
    if (transient_info[0][1] && transient_info[1][1]) {
      transient_info[0][0] =
        S_min (transient_info[1][0], transient_info[0][0]);
      transient_info[1][0] = transient_info[0][0];                                                   move16();
    }
    else {
      test(); test();
      if (transient_info[0][1] && !transient_info[1][1]) {
        transient_info[1][0] = transient_info[0][0];                                                 move16();
      }
      else {
        test(); test();
        if (!transient_info[0][1] && transient_info[1][1]) {
          transient_info[0][0] = transient_info[1][0];                                               move16();
        }
        else {
          transient_info[0][0] =
            S_max (transient_info[1][0], transient_info[0][0]);
          transient_info[1][0] = transient_info[0][0];                                               move16();
        }
      }
    }
  }
#endif

 
  /*
    Determine time/frequency division of current granule
  */
  frame_info[0] = frameInfoGenerator (&h_envChan[0]->SbrEnvFrame, 
                                      transient_info[0], 
                                      v_tuning);                                                     move16();

  h_envChan[0]->encEnvData.hSbrBSGrid = &h_envChan[0]->SbrEnvFrame.SbrGrid;                          move16();
  

#if (MAX_CHANNELS>1)

  test();
  switch (stereoMode) {
  case SBR_LEFT_RIGHT:
  case SBR_SWITCH_LRC:
    frame_info[1] = frameInfoGenerator (&h_envChan[1]->SbrEnvFrame,
                                        transient_info[1], 
                                        v_tuning);                                                   move16();

    h_envChan[1]->encEnvData.hSbrBSGrid = &h_envChan[1]->SbrEnvFrame.SbrGrid;                        move16();
    
    /* compare left and right frame_infos */
    test(); sub(1, 1);
    if (frame_info[0]->nEnvelopes != frame_info[1]->nEnvelopes) {
      stereoMode = SBR_LEFT_RIGHT;
    } else {
      for (i = 0; i < frame_info[0]->nEnvelopes + 1; i++) {
        test(); sub(1, 1);
        if (frame_info[0]->borders[i] != frame_info[1]->borders[i]) {
          stereoMode = SBR_LEFT_RIGHT;
          break;
        }
      }
      for (i = 0; i < frame_info[0]->nEnvelopes; i++) {
        test(); sub(1, 1);
        if (frame_info[0]->freqRes[i] != frame_info[1]->freqRes[i]) {
          stereoMode = SBR_LEFT_RIGHT;
          break;
        }
      }
      test(); sub(1, 1);
      if (frame_info[0]->shortEnv != frame_info[1]->shortEnv) {
        stereoMode = SBR_LEFT_RIGHT;
      }
    }
    break;
  case SBR_COUPLING:
    frame_info[1] = frame_info[0];                                                                   move16();
    h_envChan[1]->encEnvData.hSbrBSGrid = &h_envChan[0]->SbrEnvFrame.SbrGrid;                        move16();
    break;
  case SBR_MONO:
    /* nothing to do */
    break;
  default:
    assert (0);
  }

#endif /* (MAX_CHANNELS>1) */


  for(ch = 0; ch < nChannels;ch++){
    /*
      Send transient info to bitstream and store for next call
    */
    h_envChan[ch]->sbrExtractEnvelope.pre_transient_info[0] = transient_info[ch][0];/* tran_pos */             move16();
    h_envChan[ch]->sbrExtractEnvelope.pre_transient_info[1] = transient_info[ch][1];/* tran_flag */            move16();
    h_envChan[ch]->encEnvData.noOfEnvelopes = nEnvelopes[ch] = frame_info[ch]->nEnvelopes;                     move16();
   

    /*
      Save number of scf bands per envelope
    */
    for (i = 0; i < nEnvelopes[ch]; i++){
      test();
      h_envChan[ch]->encEnvData.noScfBands[i] =
        (frame_info[ch]->freqRes[i] == FREQ_RES_HIGH ? h_con->nSfb[FREQ_RES_HIGH] : h_con->nSfb[FREQ_RES_LOW]); move16();
    }



    /*
      Check if the current frame is divided into one envelope only.
    */
    test(); test(); sub(1, 1); sub(1, 1);
    if( ( h_envChan[ch]->encEnvData.hSbrBSGrid->frameClass == FIXFIX ) &&
        ( nEnvelopes[ch] == 1 ) ) {
      test(); sub(1, 1);
      if( h_envChan[ch]->encEnvData.init_sbr_amp_res != SBR_AMP_RES_1_5 ) {

        InitSbrHuffmanTables (&h_envChan[ch]->encEnvData,
                              &h_envChan[ch]->sbrCodeEnvelope,
                              &h_envChan[ch]->sbrCodeNoiseFloor,
                              SBR_AMP_RES_1_5);
      }
    } 
    else {
      test(); sub(1, 1);
      if(sbrHeaderData->sbr_amp_res != h_envChan[ch]->encEnvData.init_sbr_amp_res ) {

        InitSbrHuffmanTables (&h_envChan[ch]->encEnvData,
                              &h_envChan[ch]->sbrCodeEnvelope,
                              &h_envChan[ch]->sbrCodeNoiseFloor,
                              sbrHeaderData->sbr_amp_res);
      }
    }

    
    /*
      Tonality correction parameter extraction.
    */
    memset( h_envChan[ch]->noiseFloor, 0, sizeof(*h_envChan[ch]->noiseFloor) * MAX_NUM_NOISE_VALUES );                   memop32(MAX_NUM_NOISE_VALUES);

    TonCorrParamExtr(&h_envChan[ch]->TonCorr,
                     h_envChan[ch]->encEnvData.sbr_invf_mode_vec,
                     h_envChan[ch]->noiseFloor,
                     &h_envChan[ch]->encEnvData.addHarmonicFlag,
                     h_envChan[ch]->encEnvData.addHarmonic,
                     h_envChan[ch]->sbrExtractEnvelope.envelopeCompensation,
                     frame_info[ch],
                     transient_info[ch],
                     h_con->freqBandTable[HI],
                     h_con->nSfb[HI],
                     h_envChan[ch]->encEnvData.sbr_xpos_mode);

    h_envChan[ch]->encEnvData.sbr_invf_mode = h_envChan[ch]->encEnvData.sbr_invf_mode_vec[0];                  move16();
    h_envChan[ch]->encEnvData.noOfnoisebands = h_envChan[ch]->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;      move16();
    



  } /* ch */ 

  /*
    Extract envelope of current frame.
  */
  test();
  switch (stereoMode) {

  case SBR_MONO:
    calculateSbrEnvelope (h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferReadOffset, NULL, 
                          h_envChan[0]->sbrExtractEnvelope.yBufferScale,NULL,
                          frame_info[0], h_envChan[0]->sfb_nrg,
                          NULL,h_con,h_envChan[0], SBR_MONO,NULL);
    break;

#if (MAX_CHANNELS>1)

  case SBR_LEFT_RIGHT:
    calculateSbrEnvelope (h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferReadOffset, NULL, 
                          h_envChan[0]->sbrExtractEnvelope.yBufferScale,NULL,
                          frame_info[0], h_envChan[0]->sfb_nrg,
                          NULL, h_con, h_envChan[0], SBR_MONO,NULL);
    calculateSbrEnvelope (h_envChan[1]->sbrExtractEnvelope.YBuffer + h_envChan[1]->sbrExtractEnvelope.YBufferReadOffset, NULL, 
                          h_envChan[1]->sbrExtractEnvelope.yBufferScale,NULL,
                          frame_info[1], h_envChan[1]->sfb_nrg,
                          NULL, h_con,h_envChan[1], SBR_MONO,NULL);
    break;

  case SBR_COUPLING:
    calculateSbrEnvelope (h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferReadOffset, h_envChan[1]->sbrExtractEnvelope.YBuffer + h_envChan[1]->sbrExtractEnvelope.YBufferReadOffset, 
                          h_envChan[0]->sbrExtractEnvelope.yBufferScale,h_envChan[1]->sbrExtractEnvelope.yBufferScale,
                          frame_info[0],
                          h_envChan[0]->sfb_nrg, h_envChan[1]->sfb_nrg, h_con,h_envChan[0], SBR_COUPLING,&maxQuantError);
    break;

  case SBR_SWITCH_LRC:
    calculateSbrEnvelope (h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferReadOffset, NULL, 
                          h_envChan[0]->sbrExtractEnvelope.yBufferScale,NULL,
                          frame_info[0], h_envChan[0]->sfb_nrg,
                          NULL, h_con,h_envChan[0], SBR_MONO,NULL);
    calculateSbrEnvelope (h_envChan[1]->sbrExtractEnvelope.YBuffer + h_envChan[1]->sbrExtractEnvelope.YBufferReadOffset, NULL, 
                          h_envChan[1]->sbrExtractEnvelope.yBufferScale,NULL,
                          frame_info[1], h_envChan[1]->sfb_nrg,
                          NULL, h_con,h_envChan[1], SBR_MONO,NULL);
    calculateSbrEnvelope (h_envChan[0]->sbrExtractEnvelope.YBuffer + h_envChan[0]->sbrExtractEnvelope.YBufferReadOffset, h_envChan[1]->sbrExtractEnvelope.YBuffer + h_envChan[1]->sbrExtractEnvelope.YBufferReadOffset, 
                          h_envChan[0]->sbrExtractEnvelope.yBufferScale,h_envChan[1]->sbrExtractEnvelope.yBufferScale,
                          frame_info[0],
                          h_envChan[0]->sfb_nrg_coupling, h_envChan[1]->sfb_nrg_coupling, h_con,h_envChan[0], SBR_COUPLING,&maxQuantError);
    break;

#endif /* (MAX_CHANNELS>1) */

  default:
    assert(0);
  }



  /*
    Noise floor quantisation and coding.
  */
  test();
  switch (stereoMode) {

  case SBR_MONO:
    sbrNoiseFloorLevelsQuantisation (h_envChan[0]->noise_level, h_envChan[0]->noiseFloor);
   
    codeEnvelope (h_envChan[0]->noise_level, res,
                  &h_envChan[0]->sbrCodeNoiseFloor,
                  h_envChan[0]->encEnvData.domain_vec_noise, 0,
                  (frame_info[0]->nEnvelopes > 1 ? 2 : 1), 0,
                  sbrBitstreamData->HeaderActive);
    break;

#if (MAX_CHANNELS>1)

  case SBR_LEFT_RIGHT:
    sbrNoiseFloorLevelsQuantisation (h_envChan[0]->noise_level,h_envChan[0]->noiseFloor);
    test();
    codeEnvelope (h_envChan[0]->noise_level, res,
                  &h_envChan[0]->sbrCodeNoiseFloor,
                  h_envChan[0]->encEnvData.domain_vec_noise, 0,
                  (frame_info[0]->nEnvelopes > 1 ? 2 : 1), 0,
                  sbrBitstreamData->HeaderActive);
  

    sbrNoiseFloorLevelsQuantisation (h_envChan[1]->noise_level,h_envChan[1]->noiseFloor);
    test();
    codeEnvelope (h_envChan[1]->noise_level, res,
                  &h_envChan[1]->sbrCodeNoiseFloor,
                  h_envChan[1]->encEnvData.domain_vec_noise, 0,
                  (frame_info[1]->nEnvelopes > 1 ? 2 : 1), 0,
                  sbrBitstreamData->HeaderActive);
    break;

  case SBR_COUPLING:
    sbrNoiseFloorLevelsQuantisationCoupling (h_envChan[0]->noise_level,h_envChan[1]->noise_level,
                                             h_envChan[0]->noiseFloor,h_envChan[1]->noiseFloor);
    test();
    codeEnvelope (h_envChan[0]->noise_level, res,
                  &h_envChan[0]->sbrCodeNoiseFloor,
                  h_envChan[0]->encEnvData.domain_vec_noise, 1,
                  (frame_info[0]->nEnvelopes > 1 ? 2 : 1), 0,
                  sbrBitstreamData->HeaderActive);
  
    test();
    codeEnvelope (h_envChan[1]->noise_level, res,
                  &h_envChan[1]->sbrCodeNoiseFloor,
                  h_envChan[1]->encEnvData.domain_vec_noise, 1,
                  (frame_info[1]->nEnvelopes > 1 ? 2 : 1), 1,
                  sbrBitstreamData->HeaderActive);
    break;

  case SBR_SWITCH_LRC:
    sbrNoiseFloorLevelsQuantisation (h_envChan[0]->noise_level,h_envChan[0]->noiseFloor);
    sbrNoiseFloorLevelsQuantisation (h_envChan[1]->noise_level,h_envChan[1]->noiseFloor);
    sbrNoiseFloorLevelsQuantisationCoupling (h_envChan[0]->noise_level_coupling,h_envChan[1]->noise_level_coupling,
                                             h_envChan[0]->noiseFloor,h_envChan[1]->noiseFloor);
    break;


#endif /* (MAX_CHANNELS>1) */

  default:
    assert(0);
  }


  /*
    Encode envelope of current frame.
  */
  test();
  switch (stereoMode) {

  case SBR_MONO:
    sbrHeaderData->coupling = 0;                                                                     move16();
    h_envChan[0]->encEnvData.balance = 0;                                                            move16();
    
    codeEnvelope (h_envChan[0]->sfb_nrg, frame_info[0]->freqRes,
                  &h_envChan[0]->sbrCodeEnvelope,
                  h_envChan[0]->encEnvData.domain_vec,
                  sbrHeaderData->coupling, 
                  frame_info[0]->nEnvelopes, 0,
                  sbrBitstreamData->HeaderActive);
    break;

#if (MAX_CHANNELS>1)

  case SBR_LEFT_RIGHT:
    sbrHeaderData->coupling = 0;                                                                     move16();

    h_envChan[0]->encEnvData.balance = 0;                                                            move16();
    h_envChan[1]->encEnvData.balance = 0;                                                            move16();

    
    codeEnvelope (h_envChan[0]->sfb_nrg, frame_info[0]->freqRes,
                  &h_envChan[0]->sbrCodeEnvelope,
                  h_envChan[0]->encEnvData.domain_vec,
                  sbrHeaderData->coupling, 
                  frame_info[0]->nEnvelopes, 0,
                  sbrBitstreamData->HeaderActive);
    codeEnvelope (h_envChan[1]->sfb_nrg, frame_info[1]->freqRes,
                  &h_envChan[1]->sbrCodeEnvelope,
                  h_envChan[1]->encEnvData.domain_vec,
                  sbrHeaderData->coupling, 
                  frame_info[1]->nEnvelopes, 0,
                  sbrBitstreamData->HeaderActive);
    break;

  case SBR_COUPLING:
    sbrHeaderData->coupling = 1;                                                                     move16();
    h_envChan[0]->encEnvData.balance = 0;                                                            move16();
    h_envChan[1]->encEnvData.balance = 1;                                                            move16();

    codeEnvelope (h_envChan[0]->sfb_nrg, frame_info[0]->freqRes,
                  &h_envChan[0]->sbrCodeEnvelope,
                  h_envChan[0]->encEnvData.domain_vec,
                  sbrHeaderData->coupling, 
                  frame_info[0]->nEnvelopes, 0,
                  sbrBitstreamData->HeaderActive);
    codeEnvelope (h_envChan[1]->sfb_nrg, frame_info[1]->freqRes,
                  &h_envChan[1]->sbrCodeEnvelope,
                  h_envChan[1]->encEnvData.domain_vec,
                  sbrHeaderData->coupling, 
                  frame_info[1]->nEnvelopes, 1,
                  sbrBitstreamData->HeaderActive);
    break;

  case SBR_SWITCH_LRC:
    {
      Word32 payloadbitsLR;
      Word32 payloadbitsCOUPLING;

      Word16 sfbNrgPrevTemp[MAX_CHANNELS][MAX_FREQ_COEFFS];
      Word16 noisePrevTemp[MAX_CHANNELS][MAX_NUM_NOISE_COEFFS];
      Word16 upDateNrgTemp[MAX_CHANNELS];
      Word16 upDateNoiseTemp[MAX_CHANNELS];
      Word16 domainVecTemp[MAX_CHANNELS][MAX_ENVELOPES];
      Word16 domainVecNoiseTemp[MAX_CHANNELS][MAX_ENVELOPES];

      Word16 tempFlagRight = 0;
      Word16 tempFlagLeft = 0;
      
      /* 
         Store previous values, in order to be able to "undo" what is being done.
      */      
      for(ch = 0; ch < nChannels;ch++){
        memcpy (sfbNrgPrevTemp[ch], h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev,
                MAX_FREQ_COEFFS * sizeof (*h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev));            memop16(MAX_FREQ_COEFFS);
      
        memcpy (noisePrevTemp[ch], h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev,
                MAX_NUM_NOISE_COEFFS * sizeof (*h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev));     memop16(MAX_NUM_NOISE_COEFFS);
      
        upDateNrgTemp[ch] = h_envChan[ch]->sbrCodeEnvelope.upDate;                                   move16();
        upDateNoiseTemp[ch] = h_envChan[ch]->sbrCodeNoiseFloor.upDate;                               move16();
        
        test();
        if(sbrHeaderData->prev_coupling){
          h_envChan[ch]->sbrCodeEnvelope.upDate = 0;                                                 move16();
          h_envChan[ch]->sbrCodeNoiseFloor.upDate = 0;                                               move16();
        }
      } /* ch */

      
      /* 
         Code ordinary Left/Right stereo
      */
      codeEnvelope (h_envChan[0]->sfb_nrg, frame_info[0]->freqRes,
                    &h_envChan[0]->sbrCodeEnvelope,
                    h_envChan[0]->encEnvData.domain_vec, 0, 
                    frame_info[0]->nEnvelopes, 0,
                    sbrBitstreamData->HeaderActive);
      codeEnvelope (h_envChan[1]->sfb_nrg, frame_info[1]->freqRes,
                    &h_envChan[1]->sbrCodeEnvelope,
                    h_envChan[1]->encEnvData.domain_vec, 0,
                    frame_info[1]->nEnvelopes, 0,
                    sbrBitstreamData->HeaderActive);
      
      c = 0;
      for (i = 0; i < nEnvelopes[0]; i++) {
        for (j = 0; j < h_envChan[0]->encEnvData.noScfBands[i]; j++) 
          {
            h_envChan[0]->encEnvData.ienvelope[i][j] = h_envChan[0]->sfb_nrg[c];                     move16();
            h_envChan[1]->encEnvData.ienvelope[i][j] = h_envChan[1]->sfb_nrg[c];                     move16();
            c = add(c, 1);
          }
      }

      
      test();
      codeEnvelope (h_envChan[0]->noise_level, res,
                    &h_envChan[0]->sbrCodeNoiseFloor,
                    h_envChan[0]->encEnvData.domain_vec_noise, 0,
                    (frame_info[0]->nEnvelopes > 1 ? 2 : 1), 0,
                    sbrBitstreamData->HeaderActive);
      
      
      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++)
        h_envChan[0]->encEnvData.sbr_noise_levels[i] = h_envChan[0]->noise_level[i];                 move16();

      test();
      codeEnvelope (h_envChan[1]->noise_level, res,
                    &h_envChan[1]->sbrCodeNoiseFloor,
                    h_envChan[1]->encEnvData.domain_vec_noise, 0,
                    (frame_info[1]->nEnvelopes > 1 ? 2 : 1), 0,
                    sbrBitstreamData->HeaderActive);
      
      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
        h_envChan[1]->encEnvData.sbr_noise_levels[i] = h_envChan[1]->noise_level[i];                 move16();
      }
      
      sbrHeaderData->coupling = 0;                                                                   move16();
      h_envChan[0]->encEnvData.balance = 0;                                                          move16();
      h_envChan[1]->encEnvData.balance = 0;                                                          move16();

      payloadbitsLR = CountSbrChannelPairElement (sbrHeaderData,
                                                  sbrBitstreamData,
                                                  &h_envChan[0]->encEnvData,
                                                  &h_envChan[1]->encEnvData,
                                                  hCmonData);

      /*
        swap saved stored with current values
      */
      for(ch = 0; ch < nChannels;ch++){
        Word16 itmp;
        for(i=0;i<MAX_FREQ_COEFFS;i++){
          /*
            swap sfb energies
          */
          itmp =  h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev[i];
          h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev[i]=sfbNrgPrevTemp[ch][i];                      move16();
          sfbNrgPrevTemp[ch][i]=itmp;                                                                move16();
        }
        for(i=0;i<MAX_NUM_NOISE_COEFFS;i++){
          /*
            swap noise energies
          */
          itmp =  h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev[i];
          h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev[i]=noisePrevTemp[ch][i];                     move16();
          noisePrevTemp[ch][i]=itmp;                                                                 move16();
        }
        /* swap update flags */
        itmp  = h_envChan[ch]->sbrCodeEnvelope.upDate;
        h_envChan[ch]->sbrCodeEnvelope.upDate=upDateNrgTemp[ch];                                     move16();
        upDateNrgTemp[ch] = itmp;                                                                    move16();
        
        itmp =  h_envChan[ch]->sbrCodeNoiseFloor.upDate;
        h_envChan[ch]->sbrCodeNoiseFloor.upDate=upDateNoiseTemp[ch];                                 move16();
        upDateNoiseTemp[ch]=itmp;                                                                    move16();
        
        /*
          save domain vecs
        */
        memcpy(domainVecTemp[ch],h_envChan[ch]->encEnvData.domain_vec,sizeof(*h_envChan[ch]->encEnvData.domain_vec)*MAX_ENVELOPES);               
        memop16(MAX_ENVELOPES);
        memcpy(domainVecNoiseTemp[ch],h_envChan[ch]->encEnvData.domain_vec_noise,sizeof(*h_envChan[ch]->encEnvData.domain_vec_noise)*MAX_ENVELOPES);    
        memop16(MAX_ENVELOPES);

        if(!sbrHeaderData->prev_coupling){
          h_envChan[ch]->sbrCodeEnvelope.upDate = 0;                                                 move16();
          h_envChan[ch]->sbrCodeNoiseFloor.upDate = 0;                                               move16();
        }
      } /* ch */



      /* 
         Coupling
      */
      codeEnvelope (h_envChan[0]->sfb_nrg_coupling, frame_info[0]->freqRes,
                    &h_envChan[0]->sbrCodeEnvelope,
                    h_envChan[0]->encEnvData.domain_vec, 1, 
                    frame_info[0]->nEnvelopes, 0,
                    sbrBitstreamData->HeaderActive);
      
      codeEnvelope (h_envChan[1]->sfb_nrg_coupling, frame_info[1]->freqRes,
                    &h_envChan[1]->sbrCodeEnvelope,
                    h_envChan[1]->encEnvData.domain_vec, 1, 
                    frame_info[1]->nEnvelopes, 1,
                    sbrBitstreamData->HeaderActive);
 

      c = 0;
      for (i = 0; i < nEnvelopes[0]; i++) {
        for (j = 0; j < h_envChan[0]->encEnvData.noScfBands[i]; j++) {
          h_envChan[0]->encEnvData.ienvelope[i][j] = h_envChan[0]->sfb_nrg_coupling[c];              move16();
          h_envChan[1]->encEnvData.ienvelope[i][j] = h_envChan[1]->sfb_nrg_coupling[c];              move16();
          c = add(c, 1);
        }
      }

      
      test();
      codeEnvelope (h_envChan[0]->noise_level_coupling, res,
                    &h_envChan[0]->sbrCodeNoiseFloor,
                    h_envChan[0]->encEnvData.domain_vec_noise, 1,
                    (frame_info[0]->nEnvelopes > 1 ? 2 : 1), 0,
                    sbrBitstreamData->HeaderActive);
      
      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
        h_envChan[0]->encEnvData.sbr_noise_levels[i] = h_envChan[0]->noise_level_coupling[i];        move16();
      }

      test();
      codeEnvelope (h_envChan[1]->noise_level_coupling, res,
                    &h_envChan[1]->sbrCodeNoiseFloor,
                    h_envChan[1]->encEnvData.domain_vec_noise, 1,
                    (frame_info[1]->nEnvelopes > 1 ? 2 : 1), 1,
                    sbrBitstreamData->HeaderActive);
      
      for (i = 0; i < MAX_NUM_NOISE_VALUES; i++) {
        h_envChan[1]->encEnvData.sbr_noise_levels[i] = h_envChan[1]->noise_level_coupling[i];        move16();
      }

      sbrHeaderData->coupling = 1;                                                                   move16();
      
      h_envChan[0]->encEnvData.balance  = 0;                                                         move16();
      h_envChan[1]->encEnvData.balance  = 1;                                                         move16();

      tempFlagLeft  = h_envChan[0]->encEnvData.addHarmonicFlag;
      tempFlagRight = h_envChan[1]->encEnvData.addHarmonicFlag;

      payloadbitsCOUPLING =
        CountSbrChannelPairElement (sbrHeaderData,
                                    sbrBitstreamData,
                                    &h_envChan[0]->encEnvData,
                                    &h_envChan[1]->encEnvData,
                                    hCmonData);

      h_envChan[0]->encEnvData.addHarmonicFlag = tempFlagLeft;                                       move16();
      h_envChan[1]->encEnvData.addHarmonicFlag = tempFlagRight;                                      move16();

      test(); sub(1, 1);
      if (payloadbitsCOUPLING < payloadbitsLR) {

        /*
          copy coded coupling envelope and noise data to l/r
        */
        for(ch = 0; ch < nChannels;ch++){
          memcpy (h_envChan[ch]->sfb_nrg, h_envChan[ch]->sfb_nrg_coupling,
                  MAX_NUM_ENVELOPE_VALUES * sizeof (*h_envChan[ch]->sfb_nrg_coupling));              memop16(MAX_NUM_ENVELOPE_VALUES);
          memcpy (h_envChan[ch]->noise_level, h_envChan[ch]->noise_level_coupling,
                  MAX_NUM_NOISE_VALUES * sizeof (*h_envChan[ch]->noise_level_coupling));             memop16(MAX_NUM_NOISE_VALUES);
        }
          
        sbrHeaderData->coupling = 1;                                                                 move16();
        h_envChan[0]->encEnvData.balance  = 0;                                                       move16();
        h_envChan[1]->encEnvData.balance  = 1;                                                       move16();
      }
      else{
        /*
          restore saved l/r items
        */
        for(ch = 0; ch < nChannels;ch++){
           
          memcpy (h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev,
                  sfbNrgPrevTemp[ch], MAX_FREQ_COEFFS * sizeof (*h_envChan[ch]->sbrCodeEnvelope.sfb_nrg_prev));   
          memop16(MAX_FREQ_COEFFS);
          
          h_envChan[ch]->sbrCodeEnvelope.upDate = upDateNrgTemp[ch];                                 move16();
       
          memcpy (h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev,
                  noisePrevTemp[ch], MAX_NUM_NOISE_COEFFS * sizeof (*h_envChan[ch]->sbrCodeNoiseFloor.sfb_nrg_prev));
          memop16(MAX_NUM_NOISE_COEFFS);
         
          memcpy(h_envChan[ch]->encEnvData.domain_vec,domainVecTemp[ch],sizeof(*h_envChan[ch]->encEnvData.domain_vec)*MAX_ENVELOPES);            
          memop16(MAX_ENVELOPES);
          memcpy(h_envChan[ch]->encEnvData.domain_vec_noise,domainVecNoiseTemp[ch],sizeof(*h_envChan[ch]->encEnvData.domain_vec_noise)*MAX_ENVELOPES); 
          memop16(MAX_ENVELOPES);
          
          h_envChan[ch]->sbrCodeNoiseFloor.upDate = upDateNoiseTemp[ch];                             move16();
        }

        sbrHeaderData->coupling = 0;                                                                 move16();
        h_envChan[0]->encEnvData.balance  = 0;                                                       move16();
        h_envChan[1]->encEnvData.balance  = 0;                                                       move16();
      }
    }
    break;

#endif /* (MAX_CHANNELS>1) */

  default:
    assert(0);

  } /* end switch(stereoMode) */


  /*
    Send the encoded data to the bitstream
  */
  for(ch = 0; ch < nChannels;ch++){
    c = 0;
    for (i = 0; i < nEnvelopes[ch]; i++) {
      for (j = 0; j < h_envChan[ch]->encEnvData.noScfBands[i]; j++) {
        h_envChan[ch]->encEnvData.ienvelope[i][j] = h_envChan[ch]->sfb_nrg[c];                       move16();
        c = add(c, 1);
      }
    }
    for (i = 0; i < MAX_NUM_NOISE_VALUES; i++){
      h_envChan[ch]->encEnvData.sbr_noise_levels[i] = h_envChan[ch]->noise_level[i];                 move16();
      
    }

  }/* ch */

  	

  /*
    Write bitstream
  */
  test(); sub(1, 1);
  if (nChannels == 2) {
    WriteEnvChannelPairElement(sbrHeaderData,
                               sbrBitstreamData,
                               &h_envChan[0]->encEnvData,
                               &h_envChan[1]->encEnvData,
                               hCmonData);
  }
  else {
    WriteEnvSingleChannelElement(sbrHeaderData,
                                 sbrBitstreamData,
                                 &h_envChan[0]->encEnvData,
                                 hPsEnc,
                                 hCmonData);
  }

  /*
    Delay SBR payload by 1 frame only in case of param. stereo
  */
  test();
  if (hPsEnc)
    {
      Word16 tmpHdrBits = hCmonData->sbrHdrBitsPrev;
      Word16 tmpDataBits = hCmonData->sbrDataBitsPrev;

      /* Update previous payload size */
      hCmonData->sbrHdrBitsPrev = hCmonData->sbrHdrBits;                                             move16();
      hCmonData->sbrDataBitsPrev = hCmonData->sbrDataBits;                                           move16();

      test();
      if (hCmonData->sbrBitbufPrev.cntBits) {
        /* Swap previous and current payload */
        struct BIT_BUF tmpBitbuf;
        tmpBitbuf = hCmonData->sbrBitbufPrev;
        hCmonData->sbrBitbufPrev = hCmonData->sbrBitbuf;                                             move16();
        hCmonData->sbrBitbuf = tmpBitbuf;                                                            move16();
        /* Swap previous and current payload sizes */
        hCmonData->sbrHdrBits = tmpHdrBits;                                                          move16();
        hCmonData->sbrDataBits = tmpDataBits;                                                        move16();
      }
      else {
        CopyBitBufAll(&hCmonData->sbrBitbuf, &hCmonData->sbrBitbufPrev);
      }
    }

  /*
    Update buffers
  */
  for(ch = 0; ch < nChannels;ch++){
    for (i = 0; i < h_envChan[ch]->sbrExtractEnvelope.YBufferWriteOffset; i++) {
      Word32 *temp;

      temp = h_envChan[ch]->sbrExtractEnvelope.YBuffer[i];                                                     move16(); move16();
      h_envChan[ch]->sbrExtractEnvelope.YBuffer[i] = h_envChan[ch]->sbrExtractEnvelope.YBuffer[i + h_envChan[ch]->sbrExtractEnvelope.no_cols/2]; 
      h_envChan[ch]->sbrExtractEnvelope.YBuffer[i + h_envChan[ch]->sbrExtractEnvelope.no_cols/2] = temp;
    }

    

    h_envChan[ch]->sbrExtractEnvelope.yBufferScale[0] = h_envChan[ch]->sbrExtractEnvelope.yBufferScale[1];      move16();
  }/* ch */

  sbrHeaderData->prev_coupling = sbrHeaderData->coupling;                                                      move16();
}



/***************************************************************************/
/*!
 
\brief  creates an envelope extractor handle

\return error status
 
****************************************************************************/
Word32
CreateExtractSbrEnvelope (Word16 chan,
                          HANDLE_SBR_EXTRACT_ENVELOPE  hSbrCut,
                          Word16 no_cols,
                          Word16 no_rows,
                          Word16 start_index,
                          Word16 time_slots, 
                          Word16 time_step                          )
{
  Word16 i;
  Word16 YBufferLength, rBufferLength;

  hSbrCut->YBufferWriteOffset = no_cols;                                       move16();
  hSbrCut->YBufferReadOffset  = 0;                                             move16();
  hSbrCut->rBufferWriteOffset = 0;                                             move16();
  hSbrCut->rBufferReadOffset  = 0;                                             move16();


  YBufferLength = add(hSbrCut->YBufferWriteOffset, no_cols);
  rBufferLength = add(hSbrCut->rBufferWriteOffset, no_cols);

  
  
  hSbrCut->pre_transient_info[0] = 0;                                          move16();
  hSbrCut->pre_transient_info[1] = 0;                                          move16();

  
  hSbrCut->no_cols = no_cols;                                                  move16();
  hSbrCut->no_rows = no_rows;                                                  move16();
  hSbrCut->start_index = start_index;                                          move16();

  hSbrCut->time_slots = time_slots;                                            move16();
  hSbrCut->time_step = time_step;                                              move16();


 
  assert(rBufferLength  ==   QMF_TIME_SLOTS);
  assert(YBufferLength  == 2*QMF_TIME_SLOTS);
  assert(no_rows        ==   QMF_CHANNELS);
  YBufferLength = shr(YBufferLength, 1);
  hSbrCut->YBufferWriteOffset = shr(hSbrCut->YBufferWriteOffset, 1);
  hSbrCut->YBufferReadOffset  = shr(hSbrCut->YBufferReadOffset, 1);

  if (hSbrCut->YBuffer[0] == NULL) { /* this is the first initialization */
    hSbrCut->yBufferScale[0] = INT_BITS-1;                                     move16();
  }

  for (i = 0; i < YBufferLength; i++) {
    hSbrCut->YBuffer[i] = &sbr_envYBuffer[chan*YBufferLength*no_rows + i*QMF_CHANNELS];              move16();
  }

  for (i = 0; i < rBufferLength; i++) {
    hSbrCut->rBuffer[i] = &sbr_envRBuffer[chan*rBufferLength*no_rows + i*QMF_CHANNELS];              move16();
    hSbrCut->iBuffer[i] = &sbr_envIBuffer[chan*rBufferLength*no_rows + i*QMF_CHANNELS];              move16();
  }

  for (i = 0; i < NUMBER_TIME_SLOTS_2048; i++) { /* re-use iBuffer memory  */
    hSbrCut->tmpEnergiesM[i] = ((Word32*)sbr_envIBuffer) + i * MAX_FREQ_COEFFS;                      move16();
  }

 
  memset(hSbrCut->envelopeCompensation,0,sizeof(Word8)*MAX_FREQ_COEFFS);                             memop16((MAX_FREQ_COEFFS+1)/sizeof(Word16));
 
  
  
  return (0);
}




/***************************************************************************/
/*!
 
\brief  deinitializes an envelope extractor handle

\return void
 
****************************************************************************/
void
deleteExtractSbrEnvelope (HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut)
{
  if (hSbrCut) {
  }
}



