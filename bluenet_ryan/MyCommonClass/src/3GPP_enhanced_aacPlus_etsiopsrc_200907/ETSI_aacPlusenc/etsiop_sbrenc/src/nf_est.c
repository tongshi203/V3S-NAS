/*
   Noise floor estimation
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "sbr_misc.h"
#include "sbr.h"
#include "nf_est.h"

#include "count.h"


static const Word32 smoothFilter[4]  = {0x077f8148, 0x1999999a, 0x2bb3b1ec, 0x33333333};



/**************************************************************************/
/*!
  \brief     The function applies smoothing to the noise levels.

  

  \return    none

*/
/**************************************************************************/
static void
smoothingOfNoiseLevels (Word32 *NoiseLevels,         /*!< pointer to noise-floor levels.*/
                        Word32 nEnvelopes,           /*!< Number of noise floor envelopes.*/
                        Word32 noNoiseBands,         /*!< Number of noise bands for every noise floor envelope. */
                        Word32 prevNoiseLevels[NF_SMOOTHING_LENGTH][MAX_NUM_NOISE_VALUES],/*!< Previous noise floor envelopes. */  
                        const Word32* smoothFilter)  /*!< filter used for smoothing the noise floor levels. */
{
  Word32 i,band,env;

  for(env = 0; env < nEnvelopes; env++){
  
    for (i = 1; i < NF_SMOOTHING_LENGTH; i++){
      memcpy(prevNoiseLevels[i - 1],prevNoiseLevels[i],noNoiseBands*sizeof(Word32));                           memop32(noNoiseBands);
    }

    memcpy(prevNoiseLevels[NF_SMOOTHING_LENGTH - 1],NoiseLevels+env*noNoiseBands,noNoiseBands*sizeof(Word32)); memop32(noNoiseBands);

    for (band = 0; band < noNoiseBands; band++){
      Word32 accu = 0;                                                                                         move32();
      NoiseLevels[band+ env*noNoiseBands] = 0;                                                                 move32();
      for (i = 0; i < NF_SMOOTHING_LENGTH; i++){
        accu = L_add(accu, fixmul(smoothFilter[i], prevNoiseLevels[i][band]));
      }
      NoiseLevels[band+ env*noNoiseBands] = accu;                                                              move32();
    }
  }
}


/**************************************************************************/
/*!
  \brief     Does the noise floor level estiamtion.

  

  \return    none

*/
/**************************************************************************/
static void
qmfBasedNoiseFloorDetection(Word32 *noiseLevel,               /*!< Pointer to vector to store the noise levels in.*/
                            Word32** quotaMatrixOrig,         /*!< Matrix holding the quota values of the original. */
                            Word16* indexVector,              /*!< Index vector to obtain the patched data. */ 
                            Word16 startIndex,                /*!< Start index. */
                            Word16 stopIndex,                 /*!< Stop index. */
                            Word16 startChannel,              /*!< Start channel of the current noise floor band.*/
                            Word16 stopChannel,               /*!< Stop channel of the current noise floor band. */
                            Word32 ana_max_level,             /*!< Maximum level of the adaptive noise.*/
                            Word32 noiseFloorOffset,          /*!< Noise floor offset. */
                            Word16 missingHarmonicFlag,       /*!< Flag indicating if a strong tonal component is missing.*/
                            Word32 weightFac,                 /*!< Weightening factor for the difference between orig and sbr. */
                            INVF_MODE diffThres,              /*!< Threshold value to control the inverse filtering decision.*/
                            INVF_MODE inverseFilteringLevel)  /*!< Inverse filtering level of the current band.*/  
{
  Word32 l,k;
  Word32 meanOrig=0, meanSbr=0, diff;
  Word32 tonalityOrig, tonalitySbr;
  Word16 iNumIndex;
  Word16 iNumChannels;

  iNumIndex = div_s(1, sub(stopIndex, startIndex));

  iNumChannels = div_s(1, sub(stopChannel, startChannel));

  /*
    Calculate the mean value, over the current time segment, for the original, the HFR
    and the difference, over all channels in the current frequency range. 
  */
  test(); sub(1, 1);
  if(missingHarmonicFlag == 1){

    for(l = startChannel; l < stopChannel;l++){
      
      tonalityOrig = 0;                                                                              move32();
      for(k = startIndex ; k < stopIndex; k++){
        tonalityOrig = L_add(tonalityOrig, fixmul_32x16b(quotaMatrixOrig[k][l], iNumIndex));
      }
      tonalitySbr = 0;                                                                               move32();
      for(k = startIndex ; k < stopIndex; k++){
        tonalitySbr = L_add(tonalitySbr, fixmul_32x16b(quotaMatrixOrig[k][indexVector[l]], iNumIndex));
      }

      test(); L_sub(1, 1);
      if(tonalityOrig > meanOrig) {
        meanOrig = tonalityOrig;                                                                     move32();
      }
      test(); L_sub(1, 1);
      if(tonalitySbr > meanSbr) {
        meanSbr = tonalitySbr;                                                                       move32();
      }
    }
  }
  else{

    for(l = startChannel; l < stopChannel;l++){
      
      tonalityOrig = 0;                                                                              move32();
      for(k = startIndex ; k < stopIndex; k++){
        tonalityOrig = L_add(tonalityOrig, fixmul_32x16b(quotaMatrixOrig[k][l], iNumIndex));
      }

      tonalitySbr = 0;                                                                               move32();
      for(k = startIndex ; k < stopIndex; k++){
        tonalitySbr = L_add(tonalitySbr, fixmul_32x16b(quotaMatrixOrig[k][indexVector[l]], iNumIndex));
      }
  
      meanOrig = L_add(meanOrig, fixmul_32x16b(tonalityOrig, iNumChannels));
      meanSbr = L_add(meanSbr, fixmul_32x16b(tonalitySbr, iNumChannels));
    }
  }

  /* Small fix to avoid noise during silent passages.*/
  test(); test();
  if(L_sub(meanOrig, 0x00000002) <= 0 && L_sub(meanSbr, 0x00000002) <= 0 ){ /*-30dB*/
    meanOrig = 0x00032cc0; /* 20dB */                                                                move32();
    meanSbr  = 0x00032cc0;                                                                           move32();
  }


  test(); L_sub(1, 1);
  if(meanOrig < 0x00000800) {
    meanOrig = 0x00000800;                                                                           move32();
  }
  test(); L_sub(1, 1);
  if(meanSbr < 0x00000800) {
    meanSbr  = 0x00000800;                                                                           move32();
  }
  

  test(); sub(1, 1);
  if(missingHarmonicFlag == 1){
    diff = 0x00000800;                                                                               move32();
  }
  else{
    diff = L_max(0x00000800,  fixmul(mulScaleDiv(meanSbr,0x00000800,meanOrig), weightFac));
  } 

  test(); test(); test();
  if(sub(inverseFilteringLevel, INVF_MID_LEVEL) == 0 ||
     sub(inverseFilteringLevel, INVF_LOW_LEVEL) == 0 ||
     sub(inverseFilteringLevel, INVF_OFF) == 0){
    diff = 0x00000800;                                                                               move32();
  }

  test();
  if (sub(inverseFilteringLevel, diffThres) <= 0) {
    diff = 0x00000800;                                                                               move32();
  }


  /* 
   * noise Level is now a positive value, i.e. 
   * the more harmonic the signal is the higher noise level,
   * this makes no sense so we change the sign.
   *********************************************************/
  
  /*
    scale noise levels by a factor of 4
  */

  diff = L_shr(diff, 2);  

  test(); L_sub(1, 1);
  if(meanOrig > diff) {
    *noiseLevel = ffr_div32_32(diff, meanOrig);
  }
  else {
    *noiseLevel = 0x7fffffff;                                                                        move32();
  }

  /*
   * Add a noise floor offset to compensate for bias in the detector
   *****************************************************************/
  *noiseLevel = fixmul(*noiseLevel, noiseFloorOffset);


  /*
   * check to see that we don't exceed the maximum allowed level
   **************************************************************/
  *noiseLevel = L_min (*noiseLevel, ana_max_level);

}


/**************************************************************************/
/*!
  \brief     Does the noise floor level estiamtion.
  The function calls the Noisefloor estimation function 
  for the time segments decided based upon the transient 
  information. The block is always divided into one or two segments.


  \return    none

*/
/**************************************************************************/
void
sbrNoiseFloorEstimateQmf (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                          const SBR_FRAME_INFO *frame_info,   /*!< Time frequency grid of the current frame. */
                          Word32 *noiseLevels,                /*!< Pointer to vector to store the noise levels in.*/
                          Word32** quotaMatrixOrig,           /*!< Matrix holding the quota values of the original. */
                          Word16* indexVector,                /*!< Index vector to obtain the patched data. */
                          Word16 missingHarmonicsFlag,        /*!< Flag indicating if a strong tonal component will be missing. */
                          Word16 startIndex,                  /*!< Start index. */
                          INVF_MODE* pInvFiltLevels           /*!< Pointer to the vector holding the inverse filtering levels. */
                          )  
                          
{

  Word16 nNoiseEnvelopes, startPos[2], stopPos[2], env, band;

  Word16 noNoiseBands      = h_sbrNoiseFloorEstimate->noNoiseBands;
  Word16 *freqBandTable    = h_sbrNoiseFloorEstimate->freqBandTableQmf;



  nNoiseEnvelopes = frame_info->nNoiseEnvelopes;
  test(); sub(1, 1);
  if(nNoiseEnvelopes == 1){
    startPos[0] = startIndex;                                                       move16();
    stopPos[0]  = add(startIndex, 2);
  }
  else{
    startPos[0] = startIndex;                                                       move16();
    stopPos[0]  = add(startIndex, 1);
    startPos[1] = add(startIndex, 1);
    stopPos[1]  = add(startIndex, 2);
  }


  /*
   * Estimate the noise floor.
   **************************************/
  for(env = 0; env < nNoiseEnvelopes; env++){

    for(band = 0; band < noNoiseBands; band++){


      qmfBasedNoiseFloorDetection(&noiseLevels[band + env*noNoiseBands],
                                  quotaMatrixOrig,
                                  indexVector,
                                  startPos[env],
                                  stopPos[env],
                                  freqBandTable[band],
                                  freqBandTable[band+1],
                                  h_sbrNoiseFloorEstimate->ana_max_level,
                                  h_sbrNoiseFloorEstimate->noiseFloorOffset[band],
                                  missingHarmonicsFlag,
                                  h_sbrNoiseFloorEstimate->weightFac,
                                  h_sbrNoiseFloorEstimate->diffThres,
                                  pInvFiltLevels[band]);

    }

  }

  /*
   * Smoothing of the values.
   **************************/
  smoothingOfNoiseLevels(noiseLevels,
                         nNoiseEnvelopes,
                         h_sbrNoiseFloorEstimate->noNoiseBands,
                         h_sbrNoiseFloorEstimate->prevNoiseLevels,
                         h_sbrNoiseFloorEstimate->smoothFilter);

}


  
/**************************************************************************/
/*!
  \brief     


  \return    errorCode, noError if successful

*/
/**************************************************************************/
static Word32
downSampleLoRes(Word16 *v_result,              /*!<    */
                Word16 num_result,             /*!<    */
                const UWord16 *freqBandTableRef,/*!<    */
                Word16 num_Ref)                /*!<    */    
{
  Word16 step;
  Word16 i,j;
  Word16 org_length,result_length;
  Word16 v_index[MAX_FREQ_COEFFS/2];
  
  /* init */
  org_length=num_Ref;
  result_length=num_result;
  
  v_index[0]=0;	/* Always use left border */
  i=0;                                                                                     move16();
  test();
  while(org_length > 0)	/* Create downsample vector */
    {
      i = add(i, 1);
      step = ffr_Short_Div(org_length, result_length); /* floor; */
      org_length = sub(org_length, step);
      result_length = sub(result_length, 1);
      v_index[i] = add(v_index[i-1], step);
      test();
    }

  test(); sub(1, 1);
  if(i != num_result )	/* Should never happen */
    return (1);/* error downsampling */
  
  for(j=0;j<=i;j++)	/* Use downsample vector to index LoResolution vector. */
    {
      v_result[j]=freqBandTableRef[v_index[j]];                                            move16();
    }

  return (0);
}


/**************************************************************************/
/*!
  \brief    Creates an instance of the noise floor level estimation module. 


  \return    errorCode, noError if successful

*/
/**************************************************************************/
Word32
CreateSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE  h_sbrNoiseFloorEstimate,   /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                             Word32 ana_max_level,            /*!< Maximum level of the adaptive noise. */
                             const UWord16 *freqBandTable,    /*!< Frequany band table. */
                             Word16 nSfb,                     /*!< Number of frequency bands. */
                             Word16 noiseBands,               /*!< Number of noise bands per octave. */
                             Word32 noiseFloorOffset,         /*!< Noise floor offset. */
                             Word16 timeSlots,                /*!< Number of time slots in a frame. */
                             UWord16 useSpeechConfig          /*!< Flag: adapt tuning parameters according to speech */
                             )
{
  Word16 i;

  assert(ana_max_level%3 == 0);
  assert(ana_max_level <= 6);
  assert(noiseFloorOffset%3 == 0);
  assert(noiseFloorOffset <= 0);
  
 
  memset(h_sbrNoiseFloorEstimate,0,sizeof(SBR_NOISE_FLOOR_ESTIMATE));           memop16(sizeof(SBR_NOISE_FLOOR_ESTIMATE)/sizeof(Word16));
  
 
  
  h_sbrNoiseFloorEstimate->smoothFilter = smoothFilter;                         move32();
  test();
  if (useSpeechConfig) {
    h_sbrNoiseFloorEstimate->weightFac = 0x7fffffff;                            move32();
    h_sbrNoiseFloorEstimate->diffThres = INVF_LOW_LEVEL;                        move16();
  }
  else {
    h_sbrNoiseFloorEstimate->weightFac = 0x20000000;                            move32();
    h_sbrNoiseFloorEstimate->diffThres = INVF_MID_LEVEL;                        move16();
  }

  h_sbrNoiseFloorEstimate->timeSlots     = timeSlots;                           move16();
  test(); 
  if ( ana_max_level < 0 ) {
    h_sbrNoiseFloorEstimate->ana_max_level = L_shl(0x20000000, extract_l(L_negate(ffr_Integer_Div(L_negate(ana_max_level),3)))); move16();
  }
  else {
    h_sbrNoiseFloorEstimate->ana_max_level = L_shl(0x20000000, extract_l(ffr_Integer_Div(ana_max_level,3)));          move16();
  }
  h_sbrNoiseFloorEstimate->noiseBands    = noiseBands;                                                                move16();

  
  /*
    calculate number of noise bands and allocate
  */
  test();
  if(resetSbrNoiseFloorEstimate(h_sbrNoiseFloorEstimate,freqBandTable,nSfb))
    return(1);


  for(i=0;i<h_sbrNoiseFloorEstimate->noNoiseBands;i++) {
    h_sbrNoiseFloorEstimate->noiseFloorOffset[i] = L_shl(0x7fffffff, extract_l(ffr_Integer_Div(noiseFloorOffset,3))); move32();
  }
  
  return (0);
}







/**************************************************************************/
/*!
  \brief     Resets the current instance of the noise floor estiamtion
  module.


  \return    errorCode, noError if successful

*/
/**************************************************************************/
Word32
resetSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                            const UWord16 *freqBandTable,            /*!< Frequany band table. */
                            Word16 nSfb)                             /*!< Number of bands in the frequency band table. */
{
  Word16 k2,kx;
 
  /*
   * Calculate number of noise bands
   ***********************************/
  k2=freqBandTable[nSfb];                                                                  move16();
  kx=freqBandTable[0];                                                                     move16();
  test();
  if(h_sbrNoiseFloorEstimate->noiseBands == 0){
    h_sbrNoiseFloorEstimate->noNoiseBands = 1;                                             move16();
  }
  else{
    /* 
     * Calculate number of noise bands 1,2 or 3 bands/octave
     ********************************************************/
    
    Word16 temp;

    /* Fetch number of octaves divided by 32 */
    temp = shr(ffr_getNumOctavesDiv8(kx,k2), 2);

    /* Integer-Multiplication with number of bands: */
    temp = ffr_Short_Mult(temp, h_sbrNoiseFloorEstimate->noiseBands);

    /* Add scaled 0.5 for rounding: */
    temp = add(temp, 0x0200);

    /* Convert to right-aligned integer: */
    temp = shr(temp, (SHORT_BITS - 1 - 5));

   
    h_sbrNoiseFloorEstimate->noNoiseBands = temp;                                          move16();
    test();
    if( h_sbrNoiseFloorEstimate->noNoiseBands==0) {
      h_sbrNoiseFloorEstimate->noNoiseBands=1;                                             move16();
    }
  }
  
  return(downSampleLoRes(h_sbrNoiseFloorEstimate->freqBandTableQmf,
                         h_sbrNoiseFloorEstimate->noNoiseBands,
                         freqBandTable,nSfb));
}


/**************************************************************************/
/*!
  \brief     Deletes the current instancce of the noise floor level
  estimation module.


  \return    none

*/
/**************************************************************************/
void
deleteSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate)  /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
{
  /*
    nothing to do
  */
  (void)h_sbrNoiseFloorEstimate;
}
