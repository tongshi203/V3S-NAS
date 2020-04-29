/*
   General tonality correction detector module
 */
#include <stdlib.h>
#include <string.h>

#include "sbr.h"
#include "sbr_ram.h"
#include "ton_corr.h"
#include "invf_est.h"
#include "sbr_misc.h"
#include "sbr_def.h"

#include "count.h"

/*!
  Auto correlation coefficients.
*/
typedef struct {
  Word16  r00r;
  Word16  r11r;
  Word16  r01r;
  Word16  r01i;
  Word16  r02r;
  Word16  r02i;
  Word16  r12r;
  Word16  r12i;
  Word16  r22r;
  Word32  det;
  Word16  mScale;      /*! common scale of energies */
} ACORR_COEFS;



#define LPC_ORDER 2
#define SHIFT 5


/**************************************************************************/
/*!
  \brief     Calculates the second order auto correlation.
          
  Calculates the second order autocorrelation without windowing, using the
  covariance method.

  \return    none.

*/
/**************************************************************************/
static void
calcAutoCorrSecondOrder (ACORR_COEFS *ac, /*!< Pointer to autocorrelation coeficients struct. */
                         Word16 *realBuf, /*!< QMF-matrix real part. */
                         Word16 *imagBuf, /*!< QMF-matrix imaginary part. */
                         Word16 len)      /*!< Length of segment. */
{  
  Word32 j;
 
  Word32 accu1,accu2;
  Word32 tmp1, tmp2;
  Word16 stmp1, stmp2;
  Word16 *realBufPtr;
  Word16 *imagBufPtr;
  Word16 *realBufPtr1;
  Word16 *imagBufPtr1;
  Word16 *realBufPtr2;
  Word16 *imagBufPtr2;
  Word16 *realBufPtr11;
  Word16 *imagBufPtr11;
  Word32  r00r;
  Word32  r11r;
  Word32  r01r;
  Word32  r01i;
  Word32  r02r;
  Word32  r02i;
  Word32  r12r;
  Word32  r12i;
  Word32  r22r;

 
  /*
    r00r
  */
  accu1 = 0;
  realBufPtr=realBuf;
  imagBufPtr=imagBuf;
  for ( j = 0; j < len; j++ ) {
    stmp1 = *realBufPtr;
    stmp2 = *imagBufPtr;
    tmp1 = L_mult( stmp1, stmp1 );
    tmp2 = L_mult( stmp2, stmp2 );
    accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
    accu1 = L_add( accu1, L_shr(tmp2, SHIFT) ); 
    realBufPtr+=64;
    imagBufPtr+=64;
  }

  r00r = accu1;

  /*
    r11r,r22r
  */
  accu1 = 0;
  realBufPtr1=realBuf-64;
  imagBufPtr1=imagBuf-64;
  realBufPtr11=realBufPtr1;
  imagBufPtr11=imagBufPtr1;
  for ( j = 0; j < len - 1; j++ ) {
    tmp1 = L_mult( *realBufPtr1, *realBufPtr1 );
    tmp2 = L_mult( *imagBufPtr1, *imagBufPtr1 );
    accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
    accu1 = L_add( accu1, L_shr(tmp2, SHIFT) );
    realBufPtr1+=64;
    imagBufPtr1+=64; 
  }

  realBufPtr2=realBuf-2*64;
  imagBufPtr2=imagBuf-2*64;
  tmp1 = L_mult( *realBufPtr2, *realBufPtr2);
  tmp2 = L_mult( *imagBufPtr2, *imagBufPtr2 );
  accu2 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu2 = L_add( accu2, L_shr(tmp2, SHIFT) );
  
  tmp1 = L_mult( *realBufPtr1, *realBufPtr1 );
  tmp2 = L_mult( *imagBufPtr1, *imagBufPtr1 );
  accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu1 = L_add( accu1, L_shr(tmp2, SHIFT) ); 

  r11r = accu1;
  r22r = accu2;

  /*
    r01r,r12r
  */
  accu1 = 0;
  realBufPtr=realBuf;
  imagBufPtr=imagBuf;
  realBufPtr1=realBuf-64;
  imagBufPtr1=imagBuf-64;    
  for ( j = 0; j < len - 1; j++ ) {
    tmp1 = L_mult( *realBufPtr, *realBufPtr1 );
    tmp2 = L_mult( *imagBufPtr, *imagBufPtr1 );
    accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
    accu1 = L_add( accu1, L_shr(tmp2, SHIFT) ); 
    realBufPtr+=64;
    imagBufPtr+=64; 
    
    realBufPtr1+=64;
    imagBufPtr1+=64; 
  }
   
  tmp1 = L_mult( *realBufPtr11, *realBufPtr2 );
  tmp2 = L_mult( *imagBufPtr11, *imagBufPtr2);
  accu2 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu2 = L_add( accu2, L_shr(tmp2, SHIFT) );
  
  tmp1 = L_mult( *realBufPtr, *realBufPtr1 );
  tmp2 = L_mult( *imagBufPtr, *imagBufPtr1 );
  accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu1 = L_add( accu1, L_shr(tmp2, SHIFT) ); 
  
  r01r = accu1;
  r12r = accu2;

  /*
    r01i, r12i
  */
  accu1 = 0;
  realBufPtr=realBuf;
  imagBufPtr=imagBuf;
  realBufPtr1=realBuf-64;
  imagBufPtr1=imagBuf-64;   
  for ( j = 0; j < len - 1; j++ ) {
    tmp1 = L_mult( *imagBufPtr, *realBufPtr1 );
    tmp2 = L_mult( *realBufPtr, *imagBufPtr1 );
    accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
    accu1 = L_sub( accu1, L_shr(tmp2, SHIFT) ); 
    realBufPtr+=64;
    imagBufPtr+=64; 
    
    realBufPtr1+=64;
    imagBufPtr1+=64;
  }  

  tmp1 = L_mult( *imagBufPtr11 , *realBufPtr2 );
  tmp2 = L_mult( *realBufPtr11 , *imagBufPtr2 );
  accu2 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu2 = L_sub( accu2, L_shr(tmp2, SHIFT) );
  
  tmp1 = L_mult( *imagBufPtr, *realBufPtr1 );
  tmp2 = L_mult( *realBufPtr, *imagBufPtr1 );
  accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
  accu1 = L_sub( accu1, L_shr(tmp2, SHIFT) ); 
  
  r01i = accu1;
  r12i = accu2;

  /* 
     r02r, r02i
  */  
  accu1=accu2=0;
  realBufPtr=realBuf;
  imagBufPtr=imagBuf;
  realBufPtr1=realBuf-2*64;
  imagBufPtr1=imagBuf-2*64;   
  
  
  for ( j = 0; j < len; j++ ){
    tmp1 = L_mult( *realBufPtr, *realBufPtr1 );
    tmp2 = L_mult( *imagBufPtr, *imagBufPtr1 );
    accu1 = L_add( accu1, L_shr(tmp1, SHIFT) ); 
    accu1 = L_add( accu1, L_shr(tmp2, SHIFT) ); 

    tmp1 = L_mult( *imagBufPtr, *realBufPtr1);
    tmp2 = L_mult( *realBufPtr, *imagBufPtr1 );
    accu2 = L_add( accu2, L_shr(tmp1, SHIFT) ); 
    accu2 = L_sub( accu2, L_shr(tmp2, SHIFT) );
    
    realBufPtr+=64;
    imagBufPtr+=64; 
    
    realBufPtr1+=64;
    imagBufPtr1+=64; 
  }
  
  r02r = accu1;
  r02i = accu2;

  /* scale to common scale factor */
  logic32();
  logic32();
  logic32();
  logic32();
  logic32();
  logic32();
  logic32();
  logic32();
  ac->mScale = ffr_norm32(L_abs(r00r) | L_abs(r11r) | L_abs(r22r) | L_abs(r01r) | L_abs(r12r) | L_abs(r01i) | L_abs(r12i) | L_abs(r02r) | L_abs(r02i)); 


  ac->r00r = etsiopround(L_shl(r00r, ac->mScale));
  ac->r11r = etsiopround(L_shl(r11r, ac->mScale));
  ac->r22r = etsiopround(L_shl(r22r, ac->mScale));
  ac->r01r = etsiopround(L_shl(r01r, ac->mScale));
  ac->r12r = etsiopround(L_shl(r12r, ac->mScale));
  ac->r01i = etsiopround(L_shl(r01i, ac->mScale));
  ac->r12i = etsiopround(L_shl(r12i, ac->mScale));
  ac->r02r = etsiopround(L_shl(r02r, ac->mScale));
  ac->r02i = etsiopround(L_shl(r02i, ac->mScale));

  ac->det = L_sub( L_mult( ac->r11r, ac->r22r), L_add( L_mult(ac->r12r, ac->r12r), L_mult(ac->r12i, ac->r12i) ) );
}



/*
  Calculates the tonal to noise ration for different frequency bands
  and time segments.

  The ratio between the predicted energy (tonal energy A) and the total
  energy (A + B) is calculated. This is converted to the ratio between
  the predicted energy (tonal energy A) and the non-predictable energy
  (noise energy B). Hence the quota-matrix contains A/B = q/(1-q).       
   
  quotaMatrix scale compared to orig: RELAXATION
  nrgVector scale compared to orig: 2^4   
*/


void 
CalculateTonalityQuotas (HANDLE_SBR_TON_CORR_EST hTonCorr,  /*!< Handle to SBR_TON_CORR struct. */
                         Word16 **sourceBufferReal,         /*!< The real part of the QMF-matrix.  */
                         Word16 **sourceBufferImag,         /*!< The imaginary part of the QMF-matrix. */
                         Word16 usb,                        /*!< upper side band, highest + 1 QMF band in the SBR range. */
                         Word16 qmfScale                    /*!< sclefactor of QMF subsamples */   
                         )
{
  Word32    i, k, r, timeIndex;
  Word32  alphar[2], alphai[2];
  ACORR_COEFS ac;
 
  

  Word16     startIndexMatrix  = hTonCorr->startIndexMatrix;
  Word16     totNoEst          = hTonCorr->numberOfEstimates;
  Word16     noEstPerFrame     = hTonCorr->numberOfEstimatesPerFrame;
  Word16     move              = hTonCorr->move;
  Word16     noQmfChannels     = hTonCorr->noQmfChannels;
  Word16     buffLen           = hTonCorr->bufferLength;
  Word16     stepSize          = hTonCorr->stepSize;
  Word16     blockLength       = hTonCorr->lpcLength;
  Word32*    nrgVector         = hTonCorr->nrgVector;
  Word32**   quotaMatrix       = hTonCorr->quotaMatrix;

  /*
   * Buffering of the quotaMatrix and the quotaMatrixTransp.
   *********************************************************/
  for(i =  0 ; i < move; i++){
    memcpy(quotaMatrix[i],
           quotaMatrix[i + noEstPerFrame],
           noQmfChannels * sizeof(Word32));                                           memop32(noQmfChannels);
  }




  memmove(nrgVector,nrgVector+noEstPerFrame,move*sizeof(Word32));                     memop32(move);
  memset(nrgVector+startIndexMatrix,0,(totNoEst-startIndexMatrix)*sizeof(Word32));    memop32(totNoEst-startIndexMatrix);


  /*
   * Calculate the quotas for the current time steps.
   **************************************************/

  for (r = 0; r < usb; r++) {
    k = hTonCorr->nextSample;
    timeIndex = startIndexMatrix;
    while(k <= buffLen - blockLength) { 
      Word32 d0;

      Word16 *tempsrcBufferReal= (Word16 *)&(sourceBufferReal[k][r]);
      Word16 *tempsrcBufferImag= (Word16 *)&(sourceBufferImag[k][r]);
      calcAutoCorrSecondOrder (&ac, (Word16 *)(tempsrcBufferReal), (Word16 *)(tempsrcBufferImag), blockLength);
     
      test();
      if(ac.det > 0){
        alphar[1] = L_sub( L_sub( L_mult(ac.r01r, ac.r12r), L_mult(ac.r01i, ac.r12i)), L_mult(ac.r02r, ac.r11r) );
        alphai[1] = L_sub( L_add( L_mult(ac.r01i, ac.r12r), L_mult(ac.r01r, ac.r12i)), L_mult(ac.r02i, ac.r11r) );
    
        alphar[0] = L_add( L_add( fixmul_32x16b(ac.det, ac.r01r), fixmul_32x16b(alphar[1], ac.r12r)), fixmul_32x16b(alphai[1], ac.r12i) );
        alphai[0] = L_sub( L_add( fixmul_32x16b(ac.det, ac.r01i), fixmul_32x16b(alphai[1], ac.r12r)), fixmul_32x16b(alphar[1], ac.r12i) );

        d0 = fixmul_32x16b(fixmul_32x16b(ac.det, ac.r00r), ac.r11r);
      }
      else{
        alphar[1] = 0;
        alphai[1] = 0;

        alphar[0] =  L_deposit_h(ac.r01r);
        alphai[0] =  L_deposit_h(ac.r01i);
         
        d0 = L_mult(ac.r00r, ac.r11r);
      }

      test(); test(); sub(1, 1);
      if(d0 > 0 && ac.mScale < 23){
        Word32 tmp = L_sub(L_sub(L_add(fixmul_32x16b(alphar[0], ac.r01r), fixmul_32x16b(alphai[0], ac.r01i)), 
                                 fixmul_32x16b(fixmul_32x16b(alphar[1], ac.r02r), ac.r11r)), fixmul_32x16b(fixmul_32x16b(alphai[1], ac.r02i),ac.r11r));

        tmp = L_min(tmp, d0);


        quotaMatrix[timeIndex][r] = mulScaleDiv(tmp,0x00000800,L_add(L_sub(d0, tmp), 0x00000800));                            move32();

      }
      else {
        quotaMatrix[timeIndex][r] = 0;                                                                                        move32();
      }
 
      nrgVector[timeIndex] = L_add(nrgVector[timeIndex], 
         L_shr(L_deposit_h(ac.r00r), S_min((add(add(ac.mScale,shl(qmfScale, 1)),-SHIFT+4)),INT_BITS-1)));                    move32();
     
      k += stepSize;
      timeIndex++;
    }
  }
}









/**************************************************************************/
/*!
  \brief Extracts the parameters required in the decoder to obtain the
  correct tonal to noise ratio after SBR.

  Estimates the tonal to noise ratio of the original signal (using LPC).
  Predicts the tonal to noise ration of the SBR signal (in the decoder) by
  patching the tonal to noise ratio values similar to the patching of the
  lowband in the decoder. Given the tonal to noise ratio of the original
  and the SBR signal, it estimates the required amount of inverse filtering,
  additional noise as well as any additional sines.

  \return none.

*/
/**************************************************************************/
void
TonCorrParamExtr(HANDLE_SBR_TON_CORR_EST hTonCorr,   /*!< Handle to SBR_TON_CORR struct. */
                 INVF_MODE* infVec,                  /*!< Vector where the inverse filtering levels will be stored. */ 
                 Word32* noiseLevels,                /*!< Vector where the noise levels will be stored. */
                 Word16* missingHarmonicFlag,        /*!< Flag set to one or zero, dependent on if any strong sines are missing.*/ 
                 UWord16 * missingHarmonicsIndex,    /*!< Vector indicating where sines are missing. */
                 Word16 * envelopeCompensation,      /*!< Vector to store compensation values for the energies in. */
                 const SBR_FRAME_INFO *frameInfo,    /*!< Frame info struct, contains the time and frequency grid of the current frame.*/
                 Word16* transientInfo,              /*!< Transient info.*/
                 UWord16* freqBandTable,             /*!< Frequency band tables for high-res.*/
                 Word16 nSfb,                        /*!< Number of scalefactor bands for high-res. */
                 XPOS_MODE xposType                  /*!< Type of transposer used in the decoder.*/
                 )         
{
  Word16 band;
  Word16 transientFlag = transientInfo[1] ;    /*!< Flag indicating if a transient is present in the current frame. */
  Word16 transientPos  = transientInfo[0];     /*!< Position of the transient.*/
  Word16 transientFrame, transientFrameInvfEst;
  INVF_MODE* infVecPtr;
  
  
  /* Determine if this is a frame where a transient starts...
    
  The detection of noise-floor, missing harmonics and invf_est, is not in sync for the  
  non-buf-opt decoder such as AAC. Hence we need to keep track on the transient in the
  present frame as well as in the next.
  */
  transientFrame = 0;  
  test();
  if(hTonCorr->transientNextFrame){       /* The transient was detected in the previous frame, but is actually */
    transientFrame = 1;                   /*   located in the current frame.*/      move16();
    hTonCorr->transientNextFrame = 0;                                               move16();

    test();
    if(transientFlag){
      test(); sub(1, 1);
      if(add(transientPos, hTonCorr->transientPosOffset) >= frameInfo->borders[frameInfo->nEnvelopes]){
        hTonCorr->transientNextFrame = 1;                                           move16();
      }
    }
  }
  else{
    test();
    if(transientFlag){
      test(); sub(1, 1);
      if(add(transientPos, hTonCorr->transientPosOffset) < frameInfo->borders[frameInfo->nEnvelopes]){
        transientFrame = 1;                                                         move16();
        hTonCorr->transientNextFrame = 0;                                           move16();
      }
      else{
        hTonCorr->transientNextFrame = 1;                                           move16();
      }
    }
  }


  transientFrameInvfEst = hTonCorr->transientNextFrame;
  
  /*
    Estimate the required invese filtereing level. 
  */
  test();
  if (hTonCorr->switchInverseFilt)
    qmfInverseFilteringDetector (&hTonCorr->sbrInvFilt,
                                 hTonCorr->quotaMatrix,
                                 hTonCorr->nrgVector,
                                 hTonCorr->indexVector,
                                 hTonCorr->frameStartIndexInvfEst,
                                 hTonCorr->numberOfEstimatesPerFrame + hTonCorr->frameStartIndexInvfEst,
                                 transientFrameInvfEst, 
                                 infVec);   
                                
  /* 
     Detect what tones will be missing. 
  */
  test(); sub(1, 1);
  if (xposType == XPOS_LC ){   
    SbrMissingHarmonicsDetectorQmf(&hTonCorr->sbrMissingHarmonicsDetector,
                                   hTonCorr->quotaMatrix,
                                   hTonCorr->indexVector,
                                   frameInfo,
                                   transientInfo,
                                   missingHarmonicFlag,
                                   missingHarmonicsIndex,
                                   freqBandTable,
                                   nSfb,
                                   envelopeCompensation);
  }
  else{
    *missingHarmonicFlag = 0;                                                                        move16();
    memset(missingHarmonicsIndex,0,nSfb*sizeof(UWord16));                                            memop16(nSfb);
  }


  /*
    Noise floor estimation
  */

  infVecPtr = hTonCorr->sbrInvFilt.prevInvfMode;

  sbrNoiseFloorEstimateQmf (&hTonCorr->sbrNoiseFloorEstimate,
                            frameInfo,
                            noiseLevels,
                            hTonCorr->quotaMatrix,
                            hTonCorr->indexVector,
                            *missingHarmonicFlag,
                            hTonCorr->frameStartIndex,
                            infVecPtr);
    
  /* Store the invfVec data for the next frame...*/
  for(band = 0 ; band < hTonCorr->sbrInvFilt.noDetectorBands; band++){ 
    hTonCorr->sbrInvFilt.prevInvfMode[band] = infVec[band];                               move16();
  }

}




/**************************************************************************/
/*!
  \brief     Searches for the closest match in the frequency master table.
             
   

  \return   closest entry.

*/
/**************************************************************************/
static Word16 
findClosestEntry(Word16 goalSb,      /*!< */
                 UWord16 *v_k_master, /*!< */
                 Word16 numMaster,   /*!< */
                 Word16 direction)   /*!< */
{
  Word16 index;

  test(); sub(1, 1);
  if( goalSb <= v_k_master[0] )
    return v_k_master[0];

  test(); sub(1, 1);
  if( goalSb >= v_k_master[numMaster] )
    return v_k_master[numMaster];

  test();
  if(direction) {
    index = 0;                                                                     move16();
    test();
    while( v_k_master[index] < goalSb ) {
      index = add(index, 1);
      test();
    }
  } else {
    index = numMaster;                                                             move16();
    test();
    while( v_k_master[index] > goalSb ) {
      index = sub(index, 1);
      test();
    }
  }

  return v_k_master[index];
}








/**************************************************************************/
/*!
  \brief     resets the patch
             
   

  \return   errorCode, noError if successful.

*/
/**************************************************************************/
static Word32
resetPatch(HANDLE_SBR_TON_CORR_EST hTonCorr,     /*!< Handle to SBR_TON_CORR struct. */
           Word16 xposctrl,                      /*!< Different patch modes. */
           Word16 highBandStartSb,               /*!< Start band of the SBR range. */
           UWord16 *v_k_master,                  /*!< Master frequency table from which all other table are derived.*/
           Word16 numMaster,                     /*!< Number of elements in the master table. */  
           Word32 fs,                            /*!< Sampling frequency. */
           Word16 noChannels)                    /*!< Number of QMF-channels. */
{
  Word16 patch,k,i;
  Word16 targetStopBand;
  
  PATCH_PARAM  *patchParam = hTonCorr->patchParam;

  Word16 sbGuard = hTonCorr->guard;
  Word16 sourceStartBand;
  Word16 patchDistance;
  Word16 numBandsInPatch;
  
  Word16 lsb = v_k_master[0];                                 /* Lowest subband related to the synthesis filterbank */ 
  Word16 usb = v_k_master[numMaster];                         /* Stop subband related to the synthesis filterbank */ 
  Word16 xoverOffset = sub(highBandStartSb, v_k_master[0]);   /* Calculate distance in subbands between k0 and kx */

  Word16 goalSb;

  
  /*
   * Initialize the patching parameter
   */
  test(); sub(1, 1);
  if (xposctrl == 1) {
    lsb = add(lsb, xoverOffset);
    xoverOffset = 0;                                                                                 move16();
  }
  
  /* goalSb = (2 * noChannels * 16000 * 2 / fs  + 1 )>>1; */ /* 16 kHz band */
  goalSb = shr(add(extract_l(ffr_Integer_Div(L_mult(noChannels, 2 * 16000), fs)), 1 ), 1); /* 16 kHz band */
  goalSb = findClosestEntry(goalSb, v_k_master, numMaster, 1); /* Adapt region to master-table */

  /* First patch */ 
  sourceStartBand = add(hTonCorr->shiftStartSb, xoverOffset);
  targetStopBand = add(lsb, xoverOffset);

  /* even (odd) numbered channel must be patched to even (odd) numbered channel */  
  patch = 0;                                                                        move16();
  test();
  while(targetStopBand < usb) {

    /* To many patches */
    test(); sub(1, 1);
    if (patch >= MAX_NUM_PATCHES)
      return(1); /*Number of patches to high */

    patchParam[patch].guardStartBand = targetStopBand;                              move16();
    targetStopBand = add(targetStopBand, sbGuard);
    patchParam[patch].targetStartBand = targetStopBand;                             move16();

    numBandsInPatch = sub(goalSb, targetStopBand);                   /* get the desired range of the patch */

    test(); sub(1, 1);
    if ( numBandsInPatch >= sub(lsb, sourceStartBand) ) {
      /* desired number bands are not available -> patch whole source range */
      patchDistance   = sub(targetStopBand, sourceStartBand);        /* get the targetOffset */
      patchDistance   = patchDistance & ~1;    logic16();              /* etsioprounding off odd numbers and make all even */
      numBandsInPatch = sub(lsb, sub(targetStopBand, patchDistance)); 
      numBandsInPatch = sub(findClosestEntry(add(targetStopBand, numBandsInPatch), v_k_master, numMaster, 0),
        targetStopBand);  /* Adapt region to master-table */
    }
    
    /* desired number bands are available -> get the minimal even patching distance */
    patchDistance   = sub(add(numBandsInPatch, targetStopBand), lsb);    /* get minimal distance */
    patchDistance   = add(patchDistance, 1) & ~1;   logic16();           /* etsioprounding up odd numbers and make all even */
    
    test(); 
    if (numBandsInPatch <= 0) {
      patch = sub(patch, 1);
    } else {
      patchParam[patch].sourceStartBand = sub(targetStopBand, patchDistance);                      move16();
      patchParam[patch].targetBandOffs  = patchDistance;                                           move16();
      patchParam[patch].numBandsInPatch = numBandsInPatch;                                         move16();
      patchParam[patch].sourceStopBand  = add(patchParam[patch].sourceStartBand, numBandsInPatch); move16();
    
      targetStopBand = add(targetStopBand, patchParam[patch].numBandsInPatch);
    }

    /* All patches but first */
    sourceStartBand = hTonCorr->shiftStartSb;

    /* Check if we are close to goalSb */
    test(); sub(1, 1);
    if( abs(sub(targetStopBand, goalSb)) < 3) {
      goalSb = usb;
    }
    
    patch = add(patch, 1);
    test();
  }

  patch = sub(patch, 1);

  /* if highest patch contains less than three subband: skip it */
  test(); test(); sub(1, 1);
  if ( patchParam[patch].numBandsInPatch < 3 && patch > 0 ) {
    patch = sub(patch, 1);
    targetStopBand = add(patchParam[patch].targetStartBand, patchParam[patch].numBandsInPatch);
  } 

  hTonCorr->noOfPatches = add(patch, 1);                                   move16();


  /* Assign the index-vector, so we know where to look for the high-band.
     -1 represents a guard-band. */
  for(k = 0; k < hTonCorr->patchParam[0].guardStartBand; k++) {
    hTonCorr->indexVector[k] = k;                                          move16();
  }

  for(i = 0; i < hTonCorr->noOfPatches; i++)
    {
      Word16 sourceStart    = hTonCorr->patchParam[i].sourceStartBand;
      Word16 targetStart    = hTonCorr->patchParam[i].targetStartBand;
      Word16 numberOfBands  = hTonCorr->patchParam[i].numBandsInPatch;
      Word16 startGuardBand = hTonCorr->patchParam[i].guardStartBand;

      for(k = 0; k < sub(targetStart, startGuardBand); k++) {
        hTonCorr->indexVector[startGuardBand+k] = -1;                      move16();
      }

      for(k = 0; k < numberOfBands; k++) {
        hTonCorr->indexVector[targetStart+k] = add(sourceStart, k);        move16();
      }
    }

  return (0);
}






/**************************************************************************/
/*!
  \brief     Creates an instance of the tonality correction parameter module.

  The module includes modules for inverse filtering level estimation, 
  missing harmonics detection and noise floor level estimation.
   

  \return   errorCode, noError if successful.

*/
/**************************************************************************/
Word32
CreateTonCorrParamExtr (Word16 chan,                          /*!< Channel index, needed for mem allocation */ 
                        HANDLE_SBR_TON_CORR_EST hTonCorr,     /*!< PoWord32er to handle to SBR_TON_CORR struct. */
                        Word16 timeSlots,                     /*!< Number of time-slots per frame */
                        Word16 nCols,                         /*!< Number of columns per frame. */ 
                        Word32 fs,                            /*!< Sampling frequency (of the SBR part). */
                        Word16 noQmfChannels,                 /*!< Number of QMF channels. */  
                        Word16 xposCtrl,                      /*!< Different patch modes. */
                        Word16 highBandStartSb,               /*!< Start band of the SBR range. */  
                        UWord16 *v_k_master,                  /*!< Master frequency table from which all other table are derived.*/
                        Word16 numMaster,                     /*!< Number of elements in the master table. */  
                        Word32 ana_max_level,                 /*!< Maximum level of the adaptive noise. */  
                        UWord16 *freqBandTable[2],            /*!< Frequency band table for low-res and high-res. */  
                        Word16* nSfb,                         /*!< Number of frequency bands (hig-res and low-res). */  
                        Word16 noiseBands,                    /*!< Number of noise bands per octave. */  
                        Word32 noiseFloorOffset,              /*!< Noise floor offset. */  
                        UWord16 useSpeechConfig               /*!< Speech or music tuning. */
                        )  
{
  Word32 i;

  memset(hTonCorr,0,sizeof(SBR_TON_CORR_EST));                                      memop16(sizeof(SBR_TON_CORR_EST)/sizeof(Word16));

  
  /*
    Reset the patching and allocate memory for the quota matrix.
    Assing parameters for the LPC analysis.
  */

  hTonCorr->lpcLength                 = sub(16, LPC_ORDER);                         move16();
  hTonCorr->numberOfEstimates         = NO_OF_ESTIMATES;                            move16();
  hTonCorr->numberOfEstimatesPerFrame = 2;                                          move16();
  hTonCorr->frameStartIndexInvfEst    = 0;                                          move16();
  hTonCorr->transientPosOffset        = FRAME_MIDDLE_SLOT_2048;                     move16();
 
  hTonCorr->bufferLength              = nCols;                                      move16();
  hTonCorr->stepSize                  = add(hTonCorr->lpcLength, LPC_ORDER);        move16();
  hTonCorr->nextSample                = LPC_ORDER;                                  move16();
  /* Number of estimates to move when buffering.*/
  hTonCorr->move                      = sub(hTonCorr->numberOfEstimates, hTonCorr->numberOfEstimatesPerFrame); move16();
  /* Where to store the latest estimations in the tonality Matrix.*/
  hTonCorr->startIndexMatrix          = sub(hTonCorr->numberOfEstimates, hTonCorr->numberOfEstimatesPerFrame); move16();
  /* Where in the tonality matrix the current frame (to be sent to the decoder) starts. */
  hTonCorr->frameStartIndex           = 0;                                          move16();
  hTonCorr->prevTransientFlag         = 0;                                          move16();
  hTonCorr->transientNextFrame        = 0;                                          move16();
  
  hTonCorr->noQmfChannels = noQmfChannels;                                          move16();
  
 
  for(i=0;i<hTonCorr->numberOfEstimates;i++) {
    hTonCorr->quotaMatrix[i] = &(sbr_quotaMatrix[chan* NO_OF_ESTIMATES*QMF_CHANNELS + i*noQmfChannels]);  move32();
    memset(hTonCorr->quotaMatrix[i] ,0, sizeof(Word32)*QMF_CHANNELS);                                     memop32(QMF_CHANNELS);
  }

  /* Reset the patch.*/
  hTonCorr->guard = 0;                                                              move16();
  hTonCorr->shiftStartSb = 1;                                                       move16();

  test();
  if(resetPatch(hTonCorr,
                xposCtrl,
                highBandStartSb,
                v_k_master,
                numMaster,
                fs,
                noQmfChannels))
    return(1);
  
  test();
  if(CreateSbrNoiseFloorEstimate (&hTonCorr->sbrNoiseFloorEstimate,
                                  ana_max_level,
                                  freqBandTable[LO],
                                  nSfb[LO],
                                  noiseBands,
                                  noiseFloorOffset,
                                  timeSlots,
                                  useSpeechConfig))
    return(1);
              
  
  test();
  if(createInvFiltDetector(&hTonCorr->sbrInvFilt,   
                           hTonCorr->sbrNoiseFloorEstimate.freqBandTableQmf,
                           hTonCorr->sbrNoiseFloorEstimate.noNoiseBands,
                           useSpeechConfig))
    return(1);
  
  
  test();
  if(CreateSbrMissingHarmonicsDetector (chan,
                                        &hTonCorr->sbrMissingHarmonicsDetector,
                                        fs,
                                        nSfb[HI],
                                        noQmfChannels,
                                        hTonCorr->numberOfEstimates,
                                        hTonCorr->move,
                                        hTonCorr->numberOfEstimatesPerFrame))
    return(1);
  

 
  return (0);
}



/**************************************************************************/
/*!
  \brief     resets tonality correction parameter module.
             
   

  \return   errorCode, noError if successful.

*/
/**************************************************************************/
Word32
ResetTonCorrParamExtr(HANDLE_SBR_TON_CORR_EST hTonCorr,    /*!< Handle to SBR_TON_CORR struct. */
                      Word16 xposctrl,                     /*!< Different patch modes. */
                      Word16 highBandStartSb,              /*!< Start band of the SBR range. */
                      UWord16 *v_k_master,                 /*!< Master frequency table from which all other table are derived.*/
                      Word16 numMaster,                    /*!< Number of elements in the master table. */  
                      Word32 fs,                           /*!< Sampling frequency (of the SBR part). */
                      UWord16 ** freqBandTable,            /*!< Frequency band table for low-res and high-res. */  
                      Word16* nSfb,                        /*!< Number of frequency bands (hig-res and low-res). */  
                      Word16 noQmfChannels                 /*!< Number of QMF channels. */ 
                      )  
{
  
  /* Reset the patch.*/
  hTonCorr->guard = 0;                                                              move16();
  hTonCorr->shiftStartSb = 1;                                                       move16();

  test();
  if(resetPatch(hTonCorr,
                xposctrl,
                highBandStartSb,
                v_k_master,
                numMaster,
                fs,
                noQmfChannels))
    return(1);
  


  /* Reset the noise floor estimate.*/
  test();
  if(resetSbrNoiseFloorEstimate (&hTonCorr->sbrNoiseFloorEstimate,
                                 freqBandTable[LO],
                                 nSfb[LO]))
    return(1);

  /* 
     Reset the inveerse filtereing detector.
  */
  test();
  if(resetInvFiltDetector(&hTonCorr->sbrInvFilt,
                          hTonCorr->sbrNoiseFloorEstimate.freqBandTableQmf, 
                          hTonCorr->sbrNoiseFloorEstimate.noNoiseBands))
    return(1);
  /* Reset the missing harmonics detector. */
  test();
  if(ResetSbrMissingHarmonicsDetector (&hTonCorr->sbrMissingHarmonicsDetector,
                                       nSfb[HI]))
    return(1);

  return (0);
}





/**************************************************************************/
/*!
  \brief  Deletes the tonality correction paramtere module.  
             
   

  \return   none

*/
/**************************************************************************/
void
DeleteTonCorrParamExtr (HANDLE_SBR_TON_CORR_EST hTonCorr) /*!< Handle to SBR_TON_CORR struct. */
{
 
  if (hTonCorr) {
  
  
  
    DeleteSbrMissingHarmonicsDetector (&hTonCorr->sbrMissingHarmonicsDetector);

    deleteInvFiltDetector (&hTonCorr->sbrInvFilt); 

  }
}
