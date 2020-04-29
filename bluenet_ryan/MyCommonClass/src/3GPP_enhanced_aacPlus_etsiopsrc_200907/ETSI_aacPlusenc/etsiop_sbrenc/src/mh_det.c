/*
   Missing harmonics detection
 */ 

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "sbr_misc.h"
#include "sbr.h"
#include "sbr_def.h"
#include "mh_det.h"
#include "sbr_ram.h"

#include "count.h"


/*!< Detector Parameters for AAC core codec. */

static const Word16 deltaTime=9;
static const Word16 maxComp=2; 
static const Word32 tonalityQuota = 0x0ccccccd;
static const Word32 diffQuota = 0x60000000;
static const Word32 thresHoldDiff = 0x0000a000;
static const Word32 thresHoldDiffGuide = 0x00000a14;
static const Word32 thresHoldTone = 0x00007800;
static const Word32 iThresHoldTone = 0x00000089;
static const Word32 thresHoldToneGuide = 0x00000a14;
static const Word32 sfmThresSbr = 0x26666666; 
static const Word32 sfmThresOrig = 0x0ccccccd; 
static const Word32 decayGuideOrig = 0x26666666; 
static const Word32 decayGuideDiff = 0x40000000;



/*
  table of pow(2.0,-0.25*x)
*/

static const Word16 pow_2_m025[4] ={
  0x7fff,
  0x6ba2,
  0x5a82,
  0x4c1c
};


/**************************************************************************/
/*!
  \brief     Calculates the difference in tonality between original and SBR
             for a given time and frequency region.
  
  \return    none.

*/
/**************************************************************************/
static void diff(Word32* pTonalityOrig,          /*!< */
                 Word32* pDiffMapped2Scfb,
                 const UWord16* pFreqBandTable,
                 Word16 nScfb,
                 Word16 * indexVector)
{

  Word32 i, ll, lu, k;
  Word32 maxValOrig, maxValSbr;

  
  for(i=0; i < nScfb; i++){
    
    ll = pFreqBandTable[i];
    lu = pFreqBandTable[i+1];

    maxValOrig = 0;                                                                        move32();
    maxValSbr = 0;                                                                         move32();

    for(k=ll;k<lu;k++){
      test(); L_sub(1, 1);
      if(pTonalityOrig[k] > maxValOrig)
        maxValOrig = pTonalityOrig[k];

      test(); L_sub(1, 1);
      if(pTonalityOrig[indexVector[k]] > maxValSbr)
        maxValSbr = pTonalityOrig[indexVector[k]];
    }
 
    test(); L_sub(1, 1);
    if(maxValSbr >= 0x00000800){
      pDiffMapped2Scfb[i] = mulScaleDiv(maxValOrig, 0x00000800, maxValSbr);                move32();
    }
    else
      pDiffMapped2Scfb[i] = maxValOrig;                                                    move32();
  }
}


/**************************************************************************/
/*!
  \brief     Calculates a flatness measure of the tonality measures.
          
  
  \return    none.

*/
/**************************************************************************/
static void calculateFlatnessMeasure(Word32* pQuotaBuffer,
                                     Word16* indexVector,
                                     Word32* pSfmOrigVec,
                                     Word32* pSfmSbrVec,
                                     const UWord16* pFreqBandTable,
                                     Word16 nSfb)
{
  Word32 i,j;

  
  for(i=0;i<nSfb;i++)
    {
      Word16 ll = pFreqBandTable[i];
      Word16 lu = pFreqBandTable[i+1];
      Word16 itmp16, shift;
      Word16 tmp16 = sub(lu, ll);; 
    
      pSfmOrigVec[i] = 1;
      pSfmSbrVec[i]  = 1;

      test(); sub(1, 1);
      if(tmp16 > 1){
        Word32 amOrig,amTransp,gmOrig,gmTransp,sfmOrig,sfmTransp;
        Word16 iNumbands;
        Word16 igmOrig,igmTransp;
        shift = norm_s(tmp16);
        itmp16 = div_s(0x2000, shl(tmp16, shift));
        shift = sub(13, shift);
        iNumbands = shr(itmp16, shift);
        igmOrig = 0;                                                                    move16();
        igmTransp = 0;                                                                  move16();
        amOrig = 0;                                                                     move32();
        amTransp = 0;                                                                   move32();
        gmOrig =  0x7fffffff;                                                           move32();
        gmTransp = 0x7fffffff;                                                          move32();

        for(j= ll;j<lu;j++){
          sfmOrig   = pQuotaBuffer[j];
          sfmTransp = pQuotaBuffer[indexVector[j]];
        
          amOrig   = L_add(amOrig, fixmul_32x16b(sfmOrig, iNumbands));
          amTransp = L_add(amTransp, fixmul_32x16b(sfmTransp, iNumbands));
        
          igmOrig = sub(igmOrig, ffr_iLog4(sfmOrig));
          igmTransp = sub(igmTransp, ffr_iLog4(sfmTransp));
        }
        igmOrig = mult(igmOrig, iNumbands);
        igmTransp = mult(igmTransp, iNumbands);
      
        /* dif 4 */
        gmOrig = L_shr(gmOrig, shr(igmOrig, 2));
        gmTransp = L_shr(gmTransp, shr(igmTransp, 2));
        /* mod 4 */
        gmOrig   = fixmul_32x16b(gmOrig, pow_2_m025[igmOrig & 3]);
        gmTransp = fixmul_32x16b(gmTransp, pow_2_m025[igmTransp & 3]); 

        test(); test(); L_sub(1, 1);
        if( (amOrig != 0x00000000) && (amOrig > gmOrig)) {
          shift = ffr_norm32(amOrig);          
          pSfmOrigVec[i] = L_deposit_h(div_s(etsiopround(L_shl(gmOrig, shift)), etsiopround(L_shl(amOrig, shift))));
        }
      
        test(); test(); L_sub(1, 1);
        if( (amTransp != 0x00000000) && (amTransp > gmTransp)) {
          shift = ffr_norm32(amTransp);          
          pSfmSbrVec[i] = L_deposit_h(div_s(etsiopround(L_shl(gmTransp, shift)), etsiopround(L_shl(amTransp, shift))));
        }
    
      }
    }
}






/**************************************************************************/
/*!
  \brief     Calculates the input to the missing harmonics detection.
          
  
  \return    none.

*/
/**************************************************************************/
static void calculateDetectorInput(Word32** pQuotaBuffer, /*!< Pointer to tonality matrix. */
                                   Word16* indexVector,
                                   Word32** tonalityDiff,
                                   Word32** pSfmOrig,
                                   Word32** pSfmSbr,
                                   const UWord16* freqBandTable,
                                   Word16 nSfb,
                                   Word16 noEstPerFrame,
                                   Word16 move,
                                   Word16 noQmfBands)
{
  Word32 est;

  
  /*
    Buffer values.
  */
  for(est =  0 ; est < move; est++){
    memcpy(tonalityDiff[est],
           tonalityDiff[est + noEstPerFrame],
           noQmfBands * sizeof(Word32));                              memop32(noQmfBands);

    memcpy(pSfmOrig[est],
           pSfmOrig[est + noEstPerFrame],
           noQmfBands * sizeof(Word32));                              memop32(noQmfBands);

    memcpy(pSfmSbr[est],
           pSfmSbr[est + noEstPerFrame],
           noQmfBands * sizeof(Word32));                              memop32(noQmfBands);

  }

  
  /*
    New estimate.
  */
  for(est = 0; est < noEstPerFrame; est++){
    
    diff(pQuotaBuffer[est+move],
         tonalityDiff[est+move],
         freqBandTable,
         nSfb,
         indexVector);

    calculateFlatnessMeasure(pQuotaBuffer[est+ move],
                             indexVector,
                             pSfmOrig[est + move],
                             pSfmSbr[est + move],
                             freqBandTable,
                             nSfb);
  }
}





/**************************************************************************/
/*!
  \brief     Checks if it is allowed to detect a missing tone, that wasn't
  detected previously.
          
  
  \return    newDetectionAllowed flag.

*/
/**************************************************************************/
static Word16 isDetectionOfNewToneAllowed(const SBR_FRAME_INFO *pFrameInfo,
                                          Word16 prevTransientFrame,
                                          Word16 prevTransientPos,
                                          Word16 prevTransientFlag,
                                          Word16 transientPosOffset,
                                          Word16 transientFlag,
                                          Word16 transientPos,
                                          Word16 deltaTime,
                                          HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector)
{
  Word16 transientFrame, newDetectionAllowed;


  /* Determine if this is a frame where a transient starts...
   * If the transient flag was set the previous frame but not the
   * transient frame flag, the transient frame flag is set in the current frame.
   *****************************************************************************/
  transientFrame = 0;                                                                      move16();
  test(); sub(1, 1);
  if(transientFlag){
    test(); sub(1, 1);
    if(add(transientPos, transientPosOffset) < pFrameInfo->borders[pFrameInfo->nEnvelopes])
      transientFrame = 1;                                                                            move16();
  }
  else{
    test(); test();
    if(prevTransientFlag && !prevTransientFrame){
      transientFrame = 1;                                                                            move16();
    }
  }

  /*
   * Determine if detection of new missing harmonics are allowed.
   */
  newDetectionAllowed = 0;                                                                           move16();
  test();
  if(transientFrame){
    newDetectionAllowed = 1;                                                                         move16();
  }
  else {
    test(); test(); sub(1, 1);
    if(prevTransientFrame && 
       abs_s(sub(pFrameInfo->borders[0], sub(add(prevTransientPos, transientPosOffset), 
                                     h_sbrMissingHarmonicsDetector->timeSlots))) < deltaTime)
      newDetectionAllowed = 1;                                                                       move16();
  }

  h_sbrMissingHarmonicsDetector->previousTransientFlag  = transientFlag;                             move16();
  h_sbrMissingHarmonicsDetector->previousTransientFrame = transientFrame;                            move16();
  h_sbrMissingHarmonicsDetector->previousTransientPos   = transientPos;                              move16();

  return (newDetectionAllowed);
}







/**************************************************************************/
/*!
  \brief     Cleans up the detection after a transient.
          
  
  \return    none.

*/
/**************************************************************************/
static void transientCleanUp(Word32** quotaBuffer,
                             Word16 nSfb,
                             UWord16 ** detectionVectors,
                             const UWord16* pFreqBandTable,
                             GUIDE_VECTORS guideVectors,
                             Word16 start,
                             Word16 stop)
{
  Word16 i,j,li, ui,est;

  UWord16 pHarmVec[MAX_FREQ_COEFFS];

  memset(pHarmVec,0,MAX_FREQ_COEFFS*sizeof(UWord16));                        memop16(MAX_FREQ_COEFFS);

  for(est = start; est < stop; est++){
    for(i=0;i<nSfb-1;i++){
      pHarmVec[i] = pHarmVec[i] || detectionVectors[est][i];                logic16(); move16();
    }
  }


  /*
   * Check for duplication of sines located
   * on the border of two scf-bands.
   *************************************************/
  for(i=0;i<nSfb-1;i++)
    {
      /* detection in adjacent channels.*/
      test(); test();
      if(pHarmVec[i] && pHarmVec[i+1]){
        Word32 maxVal1, maxVal2;
        Word16 maxPos1, maxPos2;

        li = pFreqBandTable[i];                                         
        ui = pFreqBandTable[i+1];                                         

        /* Find maximum tonality in the the two scf bands.*/
        maxPos1 = li;                                                       move16();
        maxVal1 = quotaBuffer[start][li];                                   move32();
        for(j = li; j<ui; j++){
          test(); L_sub(1, 1);
          if(quotaBuffer[start][j] > maxVal1){
            maxVal1 = quotaBuffer[start][j];                                move32();
            maxPos1 = j;                                                    move16();
          }
        }
        for(est = start + 1; est < stop; est++){
          for(j = li; j<ui; j++){
            test(); L_sub(1, 1);
            if(quotaBuffer[est][j] > maxVal1){
              maxVal1 = quotaBuffer[est][j];                                move32();
              maxPos1 = j;                                                  move16();
            }
          }
        }


        li = pFreqBandTable[i+1];                                     
        ui = pFreqBandTable[i+2];                                         

        /* Find maximum tonality in the the two scf bands.*/
        maxPos2 = li;
        maxVal2 = quotaBuffer[start][li];
        for(j = li; j<ui; j++){
          test(); L_sub(1, 1);
          if(quotaBuffer[start][j] > maxVal2){
            maxVal2 = quotaBuffer[start][j];                                move32();
            maxPos2 = j;                                                    move16();
          }
        }
        for(est = start + 1; est < stop; est++){
          for(j = li; j<ui; j++){
            test(); L_sub(1, 1);
            if(quotaBuffer[est][j] > maxVal2){
              maxVal2 = quotaBuffer[est][j];                                move32();
              maxPos2 = j;                                                  move16();
            }
          }
        }


        /* If the maximum values are in adjacent QMF-channels, we need to remove
           the lowest of the two.*/
        test(); sub(1, 1);
        if(sub(maxPos2, maxPos1) < 2){
        
          test(); L_sub(1, 1);
          if(maxVal1 > maxVal2){
            /* Keep the lower, remove the upper.*/
            guideVectors.guideVectorDetected[i+1] = 0;                                     move16();
            guideVectors.guideVectorOrig[i+1] = 0;                                         move32();
            guideVectors.guideVectorDiff[i+1] = 0;                                         move32();
            for(est = start; est<stop; est++){
              detectionVectors[est][i+1] = 0;                                              move16();
            }
          }
          else{
            /* Keep the upper, remove the lower.*/
            guideVectors.guideVectorDetected[i] = 0;                                       move16();
            guideVectors.guideVectorOrig[i] = 0;                                           move32();
            guideVectors.guideVectorDiff[i] = 0;                                           move32();
            for(est = start; est<stop; est++){
              detectionVectors[est][i] = 0;                                                move16();
            }
          }
        }
      }
    }
}









/**************************************************************************/
/*!
  \brief     Do detection for one tonality estimate.
          
  
  \return    none.

*/
/**************************************************************************/
static void detection(Word32* quotaBuffer,
                      Word32* pDiffVecScfb,
                      Word16 nSfb,
                      UWord16* pHarmVec,
                      const UWord16* pFreqBandTable,
                      Word32* sfmOrig,
                      Word32* sfmSbr,
                      GUIDE_VECTORS guideVectors,
                      GUIDE_VECTORS newGuideVectors)
{

  Word16 i,j,ll, lu;
  Word32 thresTemp,thresOrig;

 
  for(i=0;i<nSfb;i++)
    {
      test();
      thresTemp = (guideVectors.guideVectorDiff[i] != 0x00000000) ? 
        L_max(fixmul(decayGuideDiff, guideVectors.guideVectorDiff[i]),thresHoldDiffGuide): 
        thresHoldDiff;

      thresTemp = L_min(thresTemp, thresHoldDiff);
    
      test(); L_sub(1, 1);
      if(pDiffVecScfb[i] > thresTemp){
        pHarmVec[i] = 1;                                                                            move16();
        newGuideVectors.guideVectorDiff[i] = pDiffVecScfb[i];                                       move32();
      }
      else{
        test();
        if((Word32)guideVectors.guideVectorDiff[i] != 0x00000000){
          guideVectors.guideVectorOrig[i] = thresHoldToneGuide ;                                    move32();
        }
      }
    }


  for(i=0;i<nSfb;i++){
    ll = pFreqBandTable[i];                                                                   
    lu = pFreqBandTable[i+1];                                                                  

    thresOrig   = L_max(fixmul(guideVectors.guideVectorOrig[i], decayGuideOrig),thresHoldToneGuide );
    thresOrig   = L_min(thresOrig,thresHoldTone);

    test();
    if(guideVectors.guideVectorOrig[i] != 0){
      for(j= ll;j<lu;j++){
        test(); L_sub(1, 1);
        if(quotaBuffer[j] > thresOrig){
          pHarmVec[i] = 1;                                                                          move16();
          newGuideVectors.guideVectorOrig[i] = quotaBuffer[j];                                      move32();
        }
      }
    }
  }




  /* 
   * Check for multiple sines in the transposed signal
   */
  thresOrig = thresHoldTone;                                                                        move32();

  for(i=0;i<nSfb;i++){
    ll = pFreqBandTable[i];     
    lu = pFreqBandTable[i+1];                                                                    

    test(); sub(1, 1);
    if(sub(lu, ll) > 1){
      for(j= ll;j<lu;j++){
        test(); test(); test();
        if(L_sub(quotaBuffer[j], thresOrig) > 0 && (L_sub(sfmSbr[i], sfmThresSbr) > 0 && L_sub(sfmOrig[i], sfmThresOrig) < 0)){
          pHarmVec[i] = 1;                                                                          move16();
          newGuideVectors.guideVectorOrig[i] = quotaBuffer[j];                                      move32();
        }
      }
    }
    else{
      test(); sub(1, 1);
      if(i < sub(nSfb, 1)){
        ll = pFreqBandTable[i];

        test();
        if(i>0){
          test(); test(); test();
          if(L_sub(quotaBuffer[ll], thresHoldTone) > 0 && 
             (L_sub(pDiffVecScfb[+1], iThresHoldTone) < 0 ||
              L_sub(pDiffVecScfb[i-1], iThresHoldTone) < 0)
             ){
            pHarmVec[i] = 1;                                                                        move16();
            newGuideVectors.guideVectorOrig[i] = quotaBuffer[ll];                                   move32();
          }
        }
        else{
          test(); test();
          if(L_sub(quotaBuffer[ll], thresHoldTone) > 0 && 
             L_sub(pDiffVecScfb[i+1], iThresHoldTone) < 0){
            pHarmVec[i] = 1;                                                                        move16();
            newGuideVectors.guideVectorOrig[i] = quotaBuffer[ll];                                   move32();
          }
        }
      }
    }
  }
}








/**************************************************************************/
/*!
  \brief     Do detection for every tonality estimate, using forward prediction.
          
  
  \return    none.

*/
/**************************************************************************/
static void detectionWithPrediction(Word32** quotaBuffer,
                                    Word32** pDiffVecScfb,
                                    Word16 nSfb,
                                    const UWord16* pFreqBandTable,
                                    Word32** sfmOrig,
                                    Word32** sfmSbr,
                                    UWord16 ** detectionVectors,
                                    UWord16* prevFrameSfbHarm,
                                    GUIDE_VECTORS* guideVectors,
                                    Word16 noEstPerFrame,
                                    Word16 totNoEst,
                                    Word16 newDetectionAllowed,
                                    UWord16* pAddHarmonicsScaleFactorBands)
{
  Word16 est = 0,i;
  Word16 start;

  memset(pAddHarmonicsScaleFactorBands,0,nSfb*sizeof(UWord16));                                      memop16(nSfb);

  test();
  if(newDetectionAllowed){

    test(); sub(1, 1);
    if(totNoEst > 1){
      start = noEstPerFrame;                                                                        move16();
  
      memcpy(guideVectors[noEstPerFrame].guideVectorDiff,guideVectors[0].guideVectorDiff,nSfb*sizeof(Word32));         memop32(nSfb);
      memcpy(guideVectors[noEstPerFrame].guideVectorOrig,guideVectors[0].guideVectorOrig,nSfb*sizeof(Word32));         memop32(nSfb);
      memset(guideVectors[noEstPerFrame-1].guideVectorDetected,0,nSfb*sizeof(UWord16));              memop16(nSfb);
    }
    else{
      start = 0;                                                                                    move16();
    }
  }
  else{
    start = 0;                                                                                      move16();
  }


  for(est = start; est < totNoEst; est++){

    /*
     * Do detection on the current frame using
     * guide-info from the previous.
     *******************************************/
    test();
    if(est > 0){
      memcpy(guideVectors[est].guideVectorDetected,detectionVectors[est-1],nSfb*sizeof(UWord16));    memop16(nSfb);
    }

    memset(detectionVectors[est], 0, nSfb*sizeof(UWord16));                                          memop16(nSfb);

    test(); sub(1, 1);
    if(est < sub(totNoEst, 1)){
      memset(guideVectors[est+1].guideVectorDiff,0,nSfb*sizeof(Word32));                            memop32(nSfb);
      memset(guideVectors[est+1].guideVectorOrig,0,nSfb*sizeof(Word32));                            memop32(nSfb);
      memset(guideVectors[est+1].guideVectorDetected,0,nSfb*sizeof(UWord16));                        memop16(nSfb);

      detection(quotaBuffer[est],
                pDiffVecScfb[est],
                nSfb,
                detectionVectors[est],
                pFreqBandTable,
                sfmOrig[est],
                sfmSbr[est],
                guideVectors[est],
                guideVectors[est+1]);
    }
    else{
      memset(guideVectors[est].guideVectorDiff,0,nSfb*sizeof(Word32));                              memop32(nSfb);
      memset(guideVectors[est].guideVectorOrig,0,nSfb*sizeof(Word32));                              memop32(nSfb);
      memset(guideVectors[est].guideVectorDetected,0,nSfb*sizeof(UWord16));                         memop16(nSfb);

      detection(quotaBuffer[est],
                pDiffVecScfb[est],
                nSfb,
                detectionVectors[est],
                pFreqBandTable,
                sfmOrig[est],
                sfmSbr[est],
                guideVectors[est],
                guideVectors[est]);     
    }
   

  }


  test();
  if(newDetectionAllowed){
    /* Use a different cleanUp algortihm compared to the stationary case. */
    test(); sub(1, 1);
    if(totNoEst > 1){
      transientCleanUp(quotaBuffer,
                       nSfb,
                       detectionVectors,
                       pFreqBandTable,
                       guideVectors[noEstPerFrame],
                       start,
                       totNoEst);     
    }
    else{
      transientCleanUp(quotaBuffer,
                       nSfb,
                       detectionVectors,
                       pFreqBandTable,
                       guideVectors[0],
                       start,
                       totNoEst);     
    }
  }



  for(i = 0; i< nSfb; i++){
    for(est = start; est < totNoEst; est++){
      test();
      pAddHarmonicsScaleFactorBands[i] = pAddHarmonicsScaleFactorBands[i] || detectionVectors[est][i];         move16();
    }
  }


  test();
  if(!newDetectionAllowed){
    /*
     * If a missing harmonic wasn't missing the previous frame
     * the transient-flag needs to be set in order to be allowed to detect it. 
     *************************************************************************/
    for(i=0;i<nSfb;i++){
      test();
      if(sub(pAddHarmonicsScaleFactorBands[i], prevFrameSfbHarm[i]) > 0)
        pAddHarmonicsScaleFactorBands[i] = 0;                                                                  move16();
    }
  }

}



/**************************************************************************/
/*!
  \brief     Calculates a compensation vector for the energy data.
                       
  
  \return    none.

*/
/**************************************************************************/
static void calculateCompVector(UWord16* pAddHarmonicsScaleFactorBands, /*!<  */
                                Word32** tonality,
                                Word16* envelopeCompensation,
                                Word16 nSfb,
                                const UWord16* freqBandTable,
                                Word32** diff,
                                Word16 totNoEst,
                                Word16 maxComp,
                                Word32 tonalityQuota,
                                Word32 diffQuota,
                                Word16 *prevEnvelopeCompensation,
                                Word16 newDetectionAllowed)
{ 
  Word16 i,j,l,ll,lu,maxPosF,maxPosT;
  Word32 maxVal;
  Word16 compValue;

  memset(envelopeCompensation,0,nSfb*sizeof(Word16));                                                memop16(nSfb);

  for(i=0 ; i < nSfb; i++){

    test();
    if(pAddHarmonicsScaleFactorBands[i]){ /* A missing sine was detected*/
      ll = freqBandTable[i];                                                                        move16();
      lu = freqBandTable[i+1];                                                                      move16();

      maxPosF = 0;                        /* First find the maximum*/                               move16();
      maxPosT = 0;                                                                                  move16();
      maxVal = 0;                                                                                   move32();

      for(j=0;j<totNoEst;j++){
        for(l=ll; l<lu; l++){
          test(); L_sub(1, 1);
          if(tonality[j][l] > maxVal)
            {
              maxVal = tonality[j][l];                                                              move32();
              maxPosF = l;                                                                          move16();
              maxPosT = j;                                                                          move16();
            }
        }
      }

      
      test(); test(); sub(1, 1);
      if(maxPosF == ll && i){                   /* First check below*/
        compValue = shr(add(abs_s(add(ffr_iLog4(diff[maxPosT][i - 1]), 4*20)), 2), 2);      /* 20 : log2(RELAXATION) */
        
        test(); sub(1, 1);
        if (compValue > maxComp) {
          compValue = maxComp;                                                                      move16();
        }

        test();
        if(!pAddHarmonicsScaleFactorBands[i-1]) {     /* No detection below*/
          test(); L_sub(1, 1);
          if(tonality[maxPosT][maxPosF -1] > fixmul(tonalityQuota, tonality[maxPosT][maxPosF])){
            envelopeCompensation[i-1] = negate(compValue);                                                   move16();
          }
        }
      }

      test(); test(); sub(1, 1); sub(1, 1);
      if(maxPosF == sub(lu, 1) && add(i, 1) < nSfb){        /* Upper border*/
        compValue = shr(add(abs_s(add(ffr_iLog4(diff[maxPosT][i + 1]), 4*20)), 2), 2);      /* 20 : log2(RELAXATION) */

        test(); sub(1, 1);
        if (compValue > maxComp) {
          compValue = maxComp;                                                                                 move16();
        }

        test();
        if(!pAddHarmonicsScaleFactorBands[i+1]) {
          test(); L_sub(1, 1);
          if(tonality[maxPosT][maxPosF+1] > fixmul(tonalityQuota, tonality[maxPosT][maxPosF])){
            envelopeCompensation[i+1] = compValue;                                                             move16();
          }
        }
      }

      
      /* If an SBR tone will show up in a scalefactor band, where there shouldn't
         be any strong tone, and there will be a tone missing or a very weak tone in 
         the adjacent channel, this needs to be compensated for.*/
      test(); test(); sub(1, 1);
      if(i && i < sub(nSfb, 1)){
        compValue = shr(add(abs_s(add(ffr_iLog4(diff[maxPosT][i - 1]), 4*20)), 2), 2);      /* 20 : log2(RELAXATION) */

        test(); sub(1, 1);
        if (compValue > maxComp) {
          compValue = maxComp;                                                                                 move16();
        }

        test(); L_sub(1, 1);
        if(0x00000800  > fixmul(fixmul(diffQuota, diff[maxPosT][i]), diff[maxPosT][i-1])){
          envelopeCompensation[i-1] = negate(compValue);                                                       move16();
        }

        compValue = shr(add(abs_s(add(ffr_iLog4(diff[maxPosT][i + 1]), 4*20)), 2), 2);      /* 20 : log2(RELAXATION) */
        
        test(); sub(1, 1);
        if (compValue > maxComp) {
          compValue = maxComp;                                                                                 move16();
        }

        test(); L_sub(1, 1);
        if(0x00000800 > fixmul(fixmul(diffQuota, diff[maxPosT][i]), diff[maxPosT][i+1])){
          envelopeCompensation[i+1] = compValue;                                                               move16();
        }
      }
    }
  }


  test();
  if(!newDetectionAllowed){
    for(i=0;i<nSfb;i++){
      test(); test();
      if(envelopeCompensation[i] != 0 && prevEnvelopeCompensation[i] == 0)
        envelopeCompensation[i] = 0;                                                                           move16();
    }
  }

}








/**************************************************************************/
/*!
  \brief     Detects where strong tonal components will be missing after
  HFR in the decoder.
         
  
  \return    none.

*/
/**************************************************************************/
void
SbrMissingHarmonicsDetectorQmf(HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMHDet,
                               Word32 ** pQuotaBuffer,
                               Word16* indexVector,
                               const SBR_FRAME_INFO *pFrameInfo,
                               const Word16* pTranInfo,
                               Word16* pAddHarmonicsFlag,
                               UWord16* pAddHarmonicsScaleFactorBands,
                               const UWord16* freqBandTable,
                               Word16 nSfb,
                               Word16* envelopeCompensation)
{

  Word16 i;
  Word16 transientFlag = pTranInfo[1];
  Word16 transientPos  = pTranInfo[0];
  Word16 newDetectionAllowed;

  
  UWord16 ** detectionVectors  = h_sbrMHDet->detectionVectors;
  Word16 move                 = h_sbrMHDet->move;  
  Word16 noEstPerFrame        = h_sbrMHDet->noEstPerFrame;
  Word16 totNoEst             = h_sbrMHDet->totNoEst;
  Word32** sfmSbr             = h_sbrMHDet->sfmSbr;
  Word32** sfmOrig            = h_sbrMHDet->sfmOrig;
  Word32** tonalityDiff       = h_sbrMHDet->tonalityDiff;
  Word16 prevTransientFlag    = h_sbrMHDet->previousTransientFlag;
  Word16 prevTransientFrame   = h_sbrMHDet->previousTransientFrame;
  Word16 transientPosOffset   = h_sbrMHDet->transientPosOffset;
  Word16 prevTransientPos     = h_sbrMHDet->previousTransientPos;
  GUIDE_VECTORS* guideVectors = h_sbrMHDet->guideVectors;
    
  Word16 noQmfBands =  sub(freqBandTable[nSfb], freqBandTable[0]);


  newDetectionAllowed = isDetectionOfNewToneAllowed(pFrameInfo,
                                                    prevTransientFrame,
                                                    prevTransientPos,
                                                    prevTransientFlag,
                                                    transientPosOffset,
                                                    transientFlag,
                                                    transientPos,
                                                    deltaTime,
                                                    h_sbrMHDet);
  

  calculateDetectorInput(pQuotaBuffer,
                         indexVector,
                         tonalityDiff,
                         sfmOrig,
                         sfmSbr,
                         freqBandTable,
                         nSfb,
                         noEstPerFrame,
                         move,
                         noQmfBands);


  detectionWithPrediction(pQuotaBuffer,
                          tonalityDiff,
                          nSfb,
                          freqBandTable,
                          sfmOrig,
                          sfmSbr,
                          detectionVectors,
                          h_sbrMHDet->guideScfb,
                          guideVectors,
                          noEstPerFrame,
                          totNoEst,
                          newDetectionAllowed,
                          pAddHarmonicsScaleFactorBands);



  /*
   * Calculate the comp vector, so that the energy can be
   * compensated for a sine between two QMF-bands.
   ******************************************************/
  calculateCompVector(pAddHarmonicsScaleFactorBands,
                      pQuotaBuffer,
                      envelopeCompensation,
                      nSfb,
                      freqBandTable,
                      tonalityDiff,
                      totNoEst,
                      maxComp, 
                      tonalityQuota,
                      diffQuota,
                      h_sbrMHDet->prevEnvelopeCompensation,
                      newDetectionAllowed); 



  /*
   * Set flag...
   **********************/
  *pAddHarmonicsFlag = 0;                                                                           move16();
  for(i=0;i<nSfb;i++){
    test();
    if(pAddHarmonicsScaleFactorBands[i]){
      *pAddHarmonicsFlag = 1;                                                                       move16();
      break;
    }
  }


   
  /*
   * Remember values.
   *******************/
  memcpy(h_sbrMHDet->prevEnvelopeCompensation, envelopeCompensation, nSfb*sizeof(Word16));          memop16(nSfb);
  memcpy(h_sbrMHDet->guideScfb, pAddHarmonicsScaleFactorBands, nSfb*sizeof(UWord16));               memop16(nSfb);
  memcpy(guideVectors[0].guideVectorDetected,pAddHarmonicsScaleFactorBands,nSfb*sizeof(UWord16));   memop16(nSfb);

  test(); sub(1, 1);
  if(totNoEst > noEstPerFrame){ 
    memcpy(guideVectors[0].guideVectorDiff,guideVectors[noEstPerFrame].guideVectorDiff,nSfb*sizeof(Word32));   memop32(nSfb);
    memcpy(guideVectors[0].guideVectorOrig,guideVectors[noEstPerFrame].guideVectorOrig,nSfb*sizeof(Word32));   memop32(nSfb);
  }
  else {
    memcpy(guideVectors[0].guideVectorDiff,guideVectors[noEstPerFrame-1].guideVectorDiff,nSfb*sizeof(Word32)); memop32(nSfb);
    memcpy(guideVectors[0].guideVectorOrig,guideVectors[noEstPerFrame-1].guideVectorOrig,nSfb*sizeof(Word32)); memop32(nSfb);
  }

  for(i=0;i<nSfb;i++){
    test();
    if(!pAddHarmonicsScaleFactorBands[i]){
      guideVectors[0].guideVectorDiff[i] = 0;                                                                  move16();
      guideVectors[0].guideVectorOrig[i] = 0;                                                                  move16();
    }
  }

}

/**************************************************************************/
/*!
  \brief     Creates an instance of the missing harmonics detector.
          
  
  \return    errorCode, noError if OK.

*/
/**************************************************************************/
Word32
CreateSbrMissingHarmonicsDetector (Word16 chan,
                                   HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet,
                                   Word32 sampleFreq,
                                   Word16 nSfb,
                                   Word16 qmfNoChannels,
                                   Word16 totNoEst,
                                   Word16 move,
                                   Word16 noEstPerFrame
                                   )
{
  Word32 i;
  Word32 *ptr;

  HANDLE_SBR_MISSING_HARMONICS_DETECTOR hs =hSbrMHDet;


  assert(totNoEst == NO_OF_ESTIMATES);
  
  memset(hs,0,sizeof( SBR_MISSING_HARMONICS_DETECTOR));               memop16((sizeof(SBR_MISSING_HARMONICS_DETECTOR)+1)/sizeof(Word16));
    
  hs->transientPosOffset = FRAME_MIDDLE_SLOT_2048;                            move16();
  hs->timeSlots          = NUMBER_TIME_SLOTS_2048;                            move16();

  hs->qmfNoChannels = qmfNoChannels;                                          move16();
  hs->sampleFreq = sampleFreq;                                                move32();
  hs->nSfb = nSfb;                                                            move16();

  hs->totNoEst = totNoEst;                                                    move16();
  hs->move = move;                                                            move16();
  hs->noEstPerFrame = noEstPerFrame;                                          move16();

  ptr = &sbr_toncorrBuff[chan*5*NO_OF_ESTIMATES*MAX_FREQ_COEFFS];

  for(i=0; i < totNoEst; i++) {

    hs->tonalityDiff[i] = ptr; ptr = (Word32*)L_add((Word32)ptr, MAX_FREQ_COEFFS*sizeof(*ptr));                            move32();
    memset(hs->tonalityDiff[i],0,sizeof(Word32)*MAX_FREQ_COEFFS);                                    memop32(MAX_FREQ_COEFFS);

    hs->sfmOrig[i] = ptr; ptr =(Word32*) L_add((Word32)ptr, MAX_FREQ_COEFFS*sizeof(*ptr));                                 move32();
    memset(hs->sfmOrig[i],0,sizeof(Word32)*MAX_FREQ_COEFFS);                                         memop32(MAX_FREQ_COEFFS);

    hs->sfmSbr[i]  = ptr; ptr = (Word32*)L_add((Word32)ptr, MAX_FREQ_COEFFS*sizeof(*ptr));                                 move32();
    memset(hs->sfmSbr[i],0,sizeof(Word32)*MAX_FREQ_COEFFS);                                          memop32(MAX_FREQ_COEFFS);
    
    hs->guideVectors[i].guideVectorDiff = ptr; ptr = (Word32*)L_add((Word32)ptr, MAX_FREQ_COEFFS*sizeof(*ptr));            move32();
    memset(hs->guideVectors[i].guideVectorDiff,0,sizeof(Word32)*MAX_FREQ_COEFFS);                    memop32(MAX_FREQ_COEFFS);

    hs->guideVectors[i].guideVectorOrig = ptr; ptr = (Word32*)L_add((Word32)ptr, MAX_FREQ_COEFFS*sizeof(*ptr));            move32();
    memset(hs->guideVectors[i].guideVectorOrig,0,sizeof(Word32)*MAX_FREQ_COEFFS);                    memop32(MAX_FREQ_COEFFS);

    hs->detectionVectors[i] = &(sbr_detectionVectors[chan*NO_OF_ESTIMATES*MAX_FREQ_COEFFS + i*MAX_FREQ_COEFFS]); move32();
    memset(hs->detectionVectors[i],0,sizeof(UWord16)*MAX_FREQ_COEFFS);                   memop16(MAX_FREQ_COEFFS);

    hs->guideVectors[i].guideVectorDetected = &(sbr_guideVectorDetected[chan*NO_OF_ESTIMATES*MAX_FREQ_COEFFS + i*MAX_FREQ_COEFFS]); move32();
    memset(hs->guideVectors[i].guideVectorDetected,0,sizeof(UWord16)*MAX_FREQ_COEFFS);   memop16(MAX_FREQ_COEFFS);

  }

  hs->prevEnvelopeCompensation = &(sbr_prevEnvelopeCompensation[chan*MAX_FREQ_COEFFS]); move32();
  memset( hs->prevEnvelopeCompensation,0, sizeof(Word16)*MAX_FREQ_COEFFS);               memop16(MAX_FREQ_COEFFS);

  hs->guideScfb = &(sbr_guideScfb[chan*MAX_FREQ_COEFFS]);                               move32();
  memset( hs->guideScfb,0, sizeof(UWord16)*MAX_FREQ_COEFFS);                             memop16(MAX_FREQ_COEFFS);
  
  
  
  hs->previousTransientFlag = 0;                                                        move16();
  hs->previousTransientFrame = 0;                                                       move16();
  hs->previousTransientPos = 0;                                                         move16();

  return (0);
}

/**************************************************************************/
/*!
  \brief     Deletes an instance of the missing harmonics detector.
          
  
  \return    none.

*/
/**************************************************************************/
void
DeleteSbrMissingHarmonicsDetector(HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMHDet)
{
  /* nothing to do */
  (void)hSbrMHDet;
}

/**************************************************************************/
/*!
  \brief     Resets an instance of the missing harmonics detector.
          
  
  \return    error code, noError if OK.

*/
/**************************************************************************/
Word32
ResetSbrMissingHarmonicsDetector (HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMissingHarmonicsDetector,
                                  Word16 nSfb)
{
  Word32 i;
  Word32 tempGuide[MAX_FREQ_COEFFS];
  Word16 tempGuideInt[MAX_FREQ_COEFFS];
  Word16 nSfbPrev;
  
  nSfbPrev = hSbrMissingHarmonicsDetector->nSfb;
  hSbrMissingHarmonicsDetector->nSfb = nSfb;                                                         move16();
  
  memcpy( tempGuideInt, hSbrMissingHarmonicsDetector->guideScfb, nSfbPrev * sizeof(UWord16) );       memop16(nSfbPrev);

  test();
  if ( nSfb > nSfbPrev ) {
    for ( i = 0; i < (nSfb - nSfbPrev); i++ ) {
      hSbrMissingHarmonicsDetector->guideScfb[i] = 0;                                                move16();
    }
    
    for ( i = 0; i < nSfbPrev; i++ ) {
      hSbrMissingHarmonicsDetector->guideScfb[i + (nSfb - nSfbPrev)] = tempGuideInt[i];              move16();
    }
  }
  else {
    for ( i = 0; i < nSfb; i++ ) {
      hSbrMissingHarmonicsDetector->guideScfb[i] = tempGuideInt[i + (nSfbPrev-nSfb)];                move16();
    }
  }

  memcpy( tempGuide, hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff, nSfbPrev * sizeof(Word32) );  memop32(nSfbPrev);

  test(); sub(1, 1);
  if (nSfb > nSfbPrev ) {
    for ( i = 0; i < (nSfb - nSfbPrev); i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff[i] = 0;                                             move32();
    }
    
    for ( i = 0; i < nSfbPrev; i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff[i + (nSfb - nSfbPrev)] = tempGuide[i];              move32();
    }
  }
  else {
    for ( i = 0; i < nSfb; i++  ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDiff[i] = tempGuide[i + (nSfbPrev-nSfb)];                move32();
    }
  }

  memcpy( tempGuide, hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig, nSfbPrev * sizeof(Word32) );        memop32(nSfbPrev);

  test(); sub(1, 1);
  if ( nSfb > nSfbPrev ) {
    for ( i = 0; i< (nSfb - nSfbPrev); i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig[i] = 0;                                             move32();
    }

    for ( i = 0; i < nSfbPrev; i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig[i + (nSfb - nSfbPrev)] = tempGuide[i];              move32();
    }
  }
  else {
    for ( i = 0; i < nSfb; i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorOrig[i] = tempGuide[i + (nSfbPrev-nSfb)];                move32();
    }
  }

  memcpy( tempGuideInt, hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected, nSfbPrev * sizeof(Word16) ); memop16(nSfbPrev);

  test(); sub(1, 1);
  if ( nSfb > nSfbPrev ) {
    for ( i = 0; i < (nSfb - nSfbPrev); i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected[i] = 0;                                         move32();
    }

    for ( i = 0; i < nSfbPrev; i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected[i + (nSfb - nSfbPrev)] = tempGuideInt[i];       move32();
    }
  }
  else {
    for ( i = 0; i < nSfb; i++ ) {
      hSbrMissingHarmonicsDetector->guideVectors[0].guideVectorDetected[i] = tempGuideInt[i + (nSfbPrev-nSfb)];         move32();
    }
  }


  memcpy( tempGuideInt, hSbrMissingHarmonicsDetector->prevEnvelopeCompensation, nSfbPrev * sizeof(Word16) );            memop16(nSfbPrev);

  test(); sub(1, 1);
  if ( nSfb > nSfbPrev ) {
    for ( i = 0; i < (nSfb - nSfbPrev); i++ ) {
      hSbrMissingHarmonicsDetector->prevEnvelopeCompensation[i] = 0;                                                    move32();
    }

    for ( i = 0; i < nSfbPrev; i++ ) {
      hSbrMissingHarmonicsDetector->prevEnvelopeCompensation[i + (nSfb - nSfbPrev)] = tempGuideInt[i];                  move32();
    }
  }
  else {
    for ( i = 0; i < nSfb; i++ ) {
      hSbrMissingHarmonicsDetector->prevEnvelopeCompensation[i] = tempGuideInt[i + (nSfbPrev-nSfb)];                    move32();
    }
  }
  return 0;
}

