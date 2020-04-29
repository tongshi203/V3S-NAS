/*
   Sbr QMF inverse filtering detector
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "ffr.h"
#include "invf_est.h"
#include "sbr_misc.h"
#include "sbr_def.h"

#include "count.h"


#define MAX_NUM_REGIONS 10


static Word16 quantStepsSbr[4]  = {1, 10, 14, 19};                 
static Word16 quantStepsOrig[4] = {0,  3,  7, 10};
static Word16 nrgBorders[4] =     {25, 30, 35, 40}; 

static DETECTOR_PARAMETERS detectorParamsAAC = {
  quantStepsSbr,
  quantStepsOrig,
  nrgBorders,
  4,                              /* Number of borders SBR. */    
  4,                              /* Number of borders orig. */    
  4,                              /* Number of borders Nrg. */          
  {                               /* Region space. */
    {INVF_MID_LEVEL,   INVF_LOW_LEVEL,  INVF_OFF,        INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_MID_LEVEL,   INVF_LOW_LEVEL,  INVF_OFF,        INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_MID_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /* regionSbr */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}  /*    |      */
  },/*------------------------ regionOrig ---------------------------------*/
  {                               /* Region space transient. */
    {INVF_LOW_LEVEL,   INVF_LOW_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_LOW_LEVEL,   INVF_LOW_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_MID_LEVEL,  INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /* regionSbr */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}  /*    |      */
  },/*------------------------ regionOrig ---------------------------------*/
  {-4, -3, -2, -1, 0} /* Reduction factor of the inverse filtering for low energies.*/
};

static const Word16 hysteresis = 1;                      /* Delta value for hysteresis. */

/* 
 * AAC+SBR PARAMETERS for Speech
 *********************************/
static DETECTOR_PARAMETERS detectorParamsAACSpeech = {
  quantStepsSbr,
  quantStepsOrig,
  nrgBorders,
  4,                              /* Number of borders SBR. */    
  4,                              /* Number of borders orig. */    
  4,                              /* Number of borders Nrg. */          
  {                               /* Region space. */
    {INVF_MID_LEVEL,   INVF_MID_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_MID_LEVEL,   INVF_MID_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_MID_LEVEL,  INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /* regionSbr */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}  /*    |      */
  },/*------------------------ regionOrig ---------------------------------*/
  {                               /* Region space transient. */
    {INVF_MID_LEVEL,   INVF_MID_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_MID_LEVEL,   INVF_MID_LEVEL,  INVF_LOW_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_MID_LEVEL,  INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /* regionSbr */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}, /*    |      */
    {INVF_HIGH_LEVEL,  INVF_HIGH_LEVEL, INVF_MID_LEVEL,  INVF_OFF, INVF_OFF}  /*    |      */
  },/*------------------------ regionOrig ---------------------------------*/
  {-4, -3, -2, -1, 0} /* Reduction factor of the inverse filtering for low energies.*/
};









/*
 * Smoothing filters.
 ************************/

static const Word32 fir_0[] = { 0x7fffffff };
static const Word32 fir_1[] = { 0x2aaaaa63, 0x555554c6 };
static const Word32 fir_2[] = { 0x10000000, 0x30000000, 0x40000000 };
static const Word32 fir_3[] = { 0x077f80ea, 0x1999999a, 0x2bb3b24a, 0x33333333 };
static const Word32 fir_4[] = { 0x04130596, 0x0ebdaff9, 0x1becfa6a, 0x2697a4cd, 0x2aaaaa63 };

static const Word32 *fir_table[5] = {
  fir_0,
  fir_1,
  fir_2,
  fir_3,
  fir_4
};


/**************************************************************************/
/*!
  \brief     Calculates the values used for the detector.
  

  \return    none

*/
/**************************************************************************/
static void 
calculateDetectorValues(Word32 **quotaMatrixOrig,            /*!<  Matrix holding the tonality values of the original. */
                        Word16 * indexVector,                /*!< Index vector to obtain the patched data. */ 
                        Word32* nrgVector,                   /*!< Energy vector. */
                        DETECTOR_VALUES *detectorValues,     /*!< pointer to DETECTOR_VALUES struct. */
                        Word16 startChannel,                 /*!< Start channel. */
                        Word16 stopChannel,                  /*!< Stop channel. */
                        Word16 startIndex,                   /*!< Start index. */
                        Word16 stopIndex,                    /*!< Stop index. */
                        Word16 numberOfStrongest             /*!< The number of sorted tonal components to be considered. */
                        )
{
  Word16 i,temp, j;

  Word32 origQuota,sbrQuota; 
    
  Word32  origQuotaMeanStrongest,sbrQuotaMeanStrongest;

  const Word32* filter = fir_table[INVF_SMOOTHING_LENGTH];

  Word32 quotaVecOrig[64], quotaVecSbr[64];
  Word16 iNumChannels,numChannels,iNumIndex,iTemp;
  memset(quotaVecOrig,0,64*sizeof(Word32));                                                                    memop32(64);
  memset(quotaVecSbr ,0,64*sizeof(Word32));                                                                    memop32(64);

  iNumIndex = div_s(1, sub(stopIndex, startIndex));
  
  /* The original, the sbr signal and the total energy */
  detectorValues->avgNrg = 0;                                                                                  move32();
    
  for(j = startIndex ; j < stopIndex ; j++){
    for(i = startChannel; i < stopChannel; i++) {
      quotaVecOrig[i] = L_add(quotaVecOrig[i], fixmul_32x16b(quotaMatrixOrig[j][i], iNumIndex));               move32();

      test(); sub(1, 1);
      if(indexVector[i] != -1) {
        quotaVecSbr[i] = L_add(quotaVecSbr[i], fixmul_32x16b(quotaMatrixOrig[j][indexVector[i]], iNumIndex));  move32();
      }
    }
    detectorValues->avgNrg = L_add(detectorValues->avgNrg, fixmul_32x16b(nrgVector[j], iNumIndex));            move32();
  }

  numChannels = sub(stopChannel, startChannel);
  iNumChannels = div_s(1, numChannels);

  origQuota = 0;
  sbrQuota  = 0;
  for(i = startChannel; i < stopChannel; i++) {
    origQuota = L_add(origQuota, fixmul_32x16b(quotaVecOrig[i], iNumChannels));
    sbrQuota  = L_add(sbrQuota, fixmul_32x16b(quotaVecSbr[i], iNumChannels));
  }

  
  /*
    Calculate the mean value for the x strongest components
  */
  Shellsort_fract((Word32*)L_add((Word32)quotaVecOrig, startChannel*sizeof(*quotaVecOrig)),
                  numChannels);

  Shellsort_fract((Word32*)L_add((Word32)quotaVecSbr, startChannel*sizeof(*quotaVecOrig)),
                  numChannels);

  origQuotaMeanStrongest = 0;
  sbrQuotaMeanStrongest  = 0;


  temp = S_min(numChannels, numberOfStrongest);
  iTemp = div_s(1, temp);

  for(i = 0; i < temp; i++) {
    origQuotaMeanStrongest = L_add(origQuotaMeanStrongest, fixmul_32x16b(quotaVecOrig[i + stopChannel - temp], iTemp));
    sbrQuotaMeanStrongest  = L_add(sbrQuotaMeanStrongest, fixmul_32x16b(quotaVecSbr[i + stopChannel - temp], iTemp));
  }


  /*
    The value for the strongest component
  */
  detectorValues->origQuotaMax = quotaVecOrig[stopChannel - 1];                                                move32();
  detectorValues->sbrQuotaMax  = quotaVecSbr[stopChannel - 1];                                                 move32();
  

  /*
    Buffer values
  */
  memmove(detectorValues->origQuotaMean, detectorValues->origQuotaMean + 1, INVF_SMOOTHING_LENGTH*sizeof(Word32));
  memop32(INVF_SMOOTHING_LENGTH);
  memmove(detectorValues->sbrQuotaMean, detectorValues->sbrQuotaMean + 1, INVF_SMOOTHING_LENGTH*sizeof(Word32));
  memop32(INVF_SMOOTHING_LENGTH);
  memmove(detectorValues->origQuotaMeanStrongest, detectorValues->origQuotaMeanStrongest + 1, INVF_SMOOTHING_LENGTH*sizeof(Word32));
  memop32(INVF_SMOOTHING_LENGTH);
  memmove(detectorValues->sbrQuotaMeanStrongest, detectorValues->sbrQuotaMeanStrongest + 1, INVF_SMOOTHING_LENGTH*sizeof(Word32));
  memop32(INVF_SMOOTHING_LENGTH);


  detectorValues->origQuotaMean[INVF_SMOOTHING_LENGTH]          = origQuota;                                   move32();
  detectorValues->sbrQuotaMean[INVF_SMOOTHING_LENGTH]           = sbrQuota;                                    move32();
  detectorValues->origQuotaMeanStrongest[INVF_SMOOTHING_LENGTH] = origQuotaMeanStrongest;                      move32();
  detectorValues->sbrQuotaMeanStrongest[INVF_SMOOTHING_LENGTH]  = sbrQuotaMeanStrongest;                       move32();



  /*
    Filter values
  */
  detectorValues->origQuotaMeanFilt = 0;                                                                       move32();
  detectorValues->sbrQuotaMeanFilt = 0;                                                                        move32();
  detectorValues->origQuotaMeanStrongestFilt = 0;                                                              move32();
  detectorValues->sbrQuotaMeanStrongestFilt = 0;                                                               move32();
  
  for(i=0;i<INVF_SMOOTHING_LENGTH+1;i++) {
    detectorValues->origQuotaMeanFilt = L_add(detectorValues->origQuotaMeanFilt, fixmul(detectorValues->origQuotaMean[i], filter[i]));
    move32();
    detectorValues->sbrQuotaMeanFilt  = L_add(detectorValues->sbrQuotaMeanFilt, fixmul(detectorValues->sbrQuotaMean[i], filter[i]));
    move32();
    detectorValues->origQuotaMeanStrongestFilt = L_add(detectorValues->origQuotaMeanStrongestFilt, fixmul(detectorValues->origQuotaMeanStrongest[i], filter[i]));
    move32();
    detectorValues->sbrQuotaMeanStrongestFilt  = L_add(detectorValues->sbrQuotaMeanStrongestFilt, fixmul(detectorValues->sbrQuotaMeanStrongest[i], filter[i]));
    move32();
  }

}


/**************************************************************************/
/*!
  \brief     Returns the region in which the input value belongs.

  

  \return    region.

*/
/**************************************************************************/
static Word16
findRegion(Word16 currVal,         /*!< The current value. */
           const Word16* borders,  /*!< The border of the regions. */
           const Word16 numBorders /*!< THe number of borders. */  
           )
{
  Word16 i;

  test(); sub(1, 1);
  if(currVal < borders[0])
    return 0;

  for(i = 1; i < numBorders; i++){
    test(); test(); sub(1, 1); sub(1, 1);
    if( currVal >= borders[i-1] && currVal < borders[i])
      return i;
  }

  test(); sub(1, 1);
  if(currVal > borders[numBorders-1])
    return numBorders;

  return 0;  /* We never get here, it's just to avoid compiler warnings.*/
}



/**************************************************************************/
/*!
  \brief     Makes a clever decision based on the quota vector.


  \return     decision on which invf mode to use

*/
/**************************************************************************/
static INVF_MODE 
decisionAlgorithm(const DETECTOR_PARAMETERS* detectorParams,  /*!< Struct with the detector parameters. */
                  DETECTOR_VALUES detectorValues,             /*!< Struct with the detector values. */
                  Word16 transientFlag,                       /*!< Flag indicating if there is a transient present.*/            
                  Word16* prevRegionSbr,                      /*!< The previous region in which the Sbr value was. */
                  Word16* prevRegionOrig                      /*!< The previous region in which the Orig value was. */    
                  )
{
  Word32 invFiltLevel;
  Word16 regionSbr, regionOrig, regionNrg;

  /*
    Current thresholds.
  */
  const Word16 *quantStepsSbr  = detectorParams->quantStepsSbr;
  const Word16 *quantStepsOrig = detectorParams->quantStepsOrig;
  const Word16 *nrgBorders     = detectorParams->nrgBorders;
  const Word16 numRegionsSbr   = detectorParams->numRegionsSbr;
  const Word16 numRegionsOrig  = detectorParams->numRegionsOrig;
  const Word16 numRegionsNrg   = detectorParams->numRegionsNrg;
 
  Word16 quantStepsSbrTmp[MAX_NUM_REGIONS];
  Word16 quantStepsOrigTmp[MAX_NUM_REGIONS];

  /*
    Current detector values.
  */
  Word16 origQuotaMeanFilt;
  Word16 sbrQuotaMeanFilt;
  Word16  nrg;
 

  nrg = add(ffr_iLog4(detectorValues.origQuotaMeanFilt), 20*4);
  origQuotaMeanFilt = shr(add(nrg, add(nrg, nrg)), 2);
  nrg = add(ffr_iLog4(detectorValues.sbrQuotaMeanFilt), 20*4);
  sbrQuotaMeanFilt  = shr(add(nrg, add(nrg, nrg)), 2);
  nrg = add(ffr_iLog4(detectorValues.avgNrg), (44 + 4)*4);
  nrg = shr(add(nrg, add(nrg, nrg)), 3);



  memcpy(quantStepsSbrTmp,quantStepsSbr,numRegionsSbr*sizeof(Word16));                                         memop32(numRegionsSbr);
  memcpy(quantStepsOrigTmp,quantStepsOrig,numRegionsOrig*sizeof(Word16));                                      memop32(numRegionsOrig);


  test(); sub(1, 1);
  if(*prevRegionSbr < numRegionsSbr) {
    quantStepsSbrTmp[*prevRegionSbr] = add(quantStepsSbr[*prevRegionSbr], hysteresis);                         move16();
  }
  test();
  if(*prevRegionSbr > 0) {
    quantStepsSbrTmp[*prevRegionSbr - 1] = sub(quantStepsSbr[*prevRegionSbr - 1], hysteresis);                 move16();
  }
  test(); sub(1, 1);
  if(*prevRegionOrig < numRegionsOrig) {
    quantStepsOrigTmp[*prevRegionOrig] = add(quantStepsOrig[*prevRegionOrig], hysteresis);                     move16();
  }
  test();
  if(*prevRegionOrig > 0) {
    quantStepsOrigTmp[*prevRegionOrig - 1] = sub(quantStepsOrig[*prevRegionOrig - 1], hysteresis);             move16();
  }

  regionSbr  = findRegion(sbrQuotaMeanFilt, quantStepsSbrTmp, numRegionsSbr);
  regionOrig = findRegion(origQuotaMeanFilt, quantStepsOrigTmp, numRegionsOrig);
  regionNrg  = findRegion(nrg,nrgBorders,numRegionsNrg);


  *prevRegionSbr = regionSbr;                                                                                  move16();
  *prevRegionOrig = regionOrig;                                                                                move16();

  /* Use different settings if a transient is present*/
  test(); sub(1, 1);
  if(transientFlag == 1){
    invFiltLevel = detectorParams->regionSpaceTransient[regionSbr][regionOrig]; 
  }
  else{
    invFiltLevel = detectorParams->regionSpace[regionSbr][regionOrig];
  }

  /* Compensate for low energy.*/
  invFiltLevel = L_max(L_add(invFiltLevel, detectorParams->EnergyCompFactor[regionNrg]),0);

  return (INVF_MODE) (invFiltLevel);
}




/**************************************************************************/
/*!
  \brief     Estiamtion of the inverse filtering level required
             in the decoder.
          
  \return    none.

*/
/**************************************************************************/
void
qmfInverseFilteringDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,  /*!< Handle to the SBR_INV_FILT_EST struct. */
                             Word32 ** quotaMatrix,             /*!< THe matrix holding the tonality values of the original. */  
                             Word32 *nrgVector,                 /*!< The energy vector. */
                             Word16* indexVector,               /*!< Index vector to obtain the patched data. */
                             Word16 startIndex,                 /*!< Start index. */
                             Word16 stopIndex,                  /*!< Stop index. */
                             Word16 transientFlag,              /*!< Flag indicating if a transient is present or not.*/
                             INVF_MODE* infVec                  /*!< Vector holding the inverse filtering levels. */
                             )
{
  Word32 band;
  
  
  /*
   * Do the inverse filtering level estimation.
   *****************************************************/
  for(band = 0 ; band < hInvFilt->noDetectorBands; band++){
    Word16 startChannel = hInvFilt->freqBandTableInvFilt[band];
    Word16 stopChannel  = hInvFilt->freqBandTableInvFilt[band+1];


    calculateDetectorValues(quotaMatrix,
                            indexVector,
                            nrgVector,
                            &hInvFilt->detectorValues[band],
                            startChannel,
                            stopChannel,
                            startIndex,
                            stopIndex,
                            hInvFilt->numberOfStrongest);

    
    infVec[band]= decisionAlgorithm(hInvFilt->detectorParams,
                                    hInvFilt->detectorValues[band],
                                    transientFlag,
                                    &hInvFilt->prevRegionSbr[band],
                                    &hInvFilt->prevRegionOrig[band]);                                          move32();

  }
}

 
/**************************************************************************/
/*!
  \brief     Creates an instance of the inverse filtering level estimator.
             
   
  \return   errorCode, noError if successful.

*/
/**************************************************************************/
Word32
createInvFiltDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,       /*!< Pointer to a handle to the SBR_INV_FILT_EST struct. */
                       Word16* freqBandTableDetector,          /*!< Frequency band table for the inverse filtering. */
                       Word16 numDetectorBands,                /*!< Number of inverse filtering bands. */
                       UWord16 useSpeechConfig                 /*!< Flag: adapt tuning parameters according to speech*/
                       )        
{
  Word32 i;
  
  

  memset( hInvFilt,0,sizeof(SBR_INV_FILT_EST));                                 memop16((sizeof(SBR_INV_FILT_EST)+1)/sizeof(Word16));

  test(); sub(1, 1);
  if (useSpeechConfig) {
    hInvFilt->detectorParams=&detectorParamsAACSpeech;                          move32();
  }
  else {
    hInvFilt->detectorParams=&detectorParamsAAC;                                move32();
  }
  
  hInvFilt->noDetectorBandsMax = numDetectorBands;                              move32();
 

  /* 
     Memory initialisation
  */
  for(i=0;i<hInvFilt->noDetectorBandsMax;i++){
    memset(&hInvFilt->detectorValues[i],0,sizeof(DETECTOR_VALUES));             memop16((sizeof(DETECTOR_VALUES)+1)/sizeof(Word16));
    hInvFilt->prevInvfMode[i]   = INVF_OFF;                                     move32();
    hInvFilt->prevRegionOrig[i] = 0;                                            move32();
    hInvFilt->prevRegionSbr[i]  = 0;                                            move32();
  }
  

  /* 
     Reset the inverse filtering detector.
  */
  resetInvFiltDetector(hInvFilt,
                       freqBandTableDetector,
                       hInvFilt->noDetectorBandsMax);


 
  return (0);
}


/**************************************************************************/
/*!
  \brief     resets sbr inverse filtering structure.
             
   

  \return   errorCode, noError if successful.

*/
/**************************************************************************/
Word32
resetInvFiltDetector(HANDLE_SBR_INV_FILT_EST hInvFilt,    /*!< Handle to the SBR_INV_FILT_EST struct. */
                     Word16* freqBandTableDetector,       /*!< Frequency band table for the inverse filtering. */
                     Word16 numDetectorBands)             /*!< Number of inverse filtering bands. */
{

  hInvFilt->numberOfStrongest     = 1;                                                                         move16();
  memcpy(hInvFilt->freqBandTableInvFilt,freqBandTableDetector,(numDetectorBands+1)*sizeof(Word16));            memop16(numDetectorBands+1);
  hInvFilt->noDetectorBands = numDetectorBands;                                                                move16();

  return (0);
}


/**************************************************************************/
/*!
  \brief    deletes sbr inverse filtering structure.
             
   

  \return   none

*/
/**************************************************************************/
void
deleteInvFiltDetector (HANDLE_SBR_INV_FILT_EST hs) /*!< Handle to the SBR_INV_FILT_EST struct. */
{
  /*
    nothing to do
  */
  (void)hs;
}
