/*
   Inverse Filtering detection prototypes
 */
#ifndef _INV_FILT_DET_H
#define _INV_FILT_DET_H

#include "sbr_main.h"
#include "sbr_def.h"
#include "ffr.h"

#define INVF_SMOOTHING_LENGTH 2

typedef struct
{
  Word16 *quantStepsSbr;
  Word16 *quantStepsOrig;
  Word16 *nrgBorders;
  Word16   numRegionsSbr;
  Word16   numRegionsOrig;
  Word16   numRegionsNrg;
  INVF_MODE regionSpace[5][5];
  INVF_MODE regionSpaceTransient[5][5];
  Word32 EnergyCompFactor[5];

}DETECTOR_PARAMETERS; /* size Word16: 66 */



typedef struct
{
  Word32  origQuotaMean[INVF_SMOOTHING_LENGTH+1];
  Word32  sbrQuotaMean[INVF_SMOOTHING_LENGTH+1];
  Word32  origQuotaMeanStrongest[INVF_SMOOTHING_LENGTH+1];
  Word32  sbrQuotaMeanStrongest[INVF_SMOOTHING_LENGTH+1];
  
  Word32 origQuotaMeanFilt;
  Word32 sbrQuotaMeanFilt;
  Word32 origQuotaMeanStrongestFilt;
  Word32 sbrQuotaMeanStrongestFilt;
  
  Word32 origQuotaMax;
  Word32 sbrQuotaMax;

  Word32 avgNrg;
 
}DETECTOR_VALUES; /* size Word16: 38 */



typedef struct
{
  Word16 numberOfStrongest;
  
  Word16 prevRegionSbr[MAX_NUM_NOISE_VALUES];
  Word16 prevRegionOrig[MAX_NUM_NOISE_VALUES];
    
  Word16 freqBandTableInvFilt[MAX_NUM_NOISE_VALUES];
  Word16 noDetectorBands;
  Word16 noDetectorBandsMax;

  DETECTOR_PARAMETERS *detectorParams;
  INVF_MODE prevInvfMode[MAX_NUM_NOISE_VALUES];
  DETECTOR_VALUES detectorValues[MAX_NUM_NOISE_VALUES];
}
SBR_INV_FILT_EST; /* size Word16: 423 */

typedef SBR_INV_FILT_EST *HANDLE_SBR_INV_FILT_EST;


void
qmfInverseFilteringDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,  /*!<  */
                             Word32 ** quotaMatrix,
                             Word32 *nrgVector,
                             Word16 * indexVector,
                             Word16 startIndex,
                             Word16 stopIndex,
                             Word16 transientFlag,                 /*!<  */
                             INVF_MODE* infVec);                 /*!<  */



Word32
createInvFiltDetector (HANDLE_SBR_INV_FILT_EST hInvFilt,   /*!<  */
                       Word16* freqBandTableDetector,
                       Word16 numDetectorBands,
                       UWord16 useSpeechConfig);
  
void
deleteInvFiltDetector (HANDLE_SBR_INV_FILT_EST hInvFilt);


Word32
resetInvFiltDetector(HANDLE_SBR_INV_FILT_EST hInvFilt,
                     Word16* freqBandTableDetector,
                     Word16 numDetectorBands);



#endif /* _QMF_INV_FILT_H */

