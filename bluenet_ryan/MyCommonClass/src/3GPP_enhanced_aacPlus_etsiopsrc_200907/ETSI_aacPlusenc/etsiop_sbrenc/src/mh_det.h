/*
   missing harmonics detection header file
 */  

#ifndef __MH_DETECT_H
#define __MH_DETECT_H


#include "sbr_main.h"
#include "fram_gen.h"



typedef struct
{
  Word32* guideVectorDiff;
  Word32* guideVectorOrig;
  UWord16* guideVectorDetected;
}GUIDE_VECTORS;





typedef struct
{
  Word16 qmfNoChannels;
  Word16 nSfb;
  Word32 sampleFreq;
  Word16 previousTransientFlag;
  Word16 previousTransientFrame;
  Word16 previousTransientPos;

  Word16 noVecPerFrame;
  Word16 transientPosOffset;
  
  Word16 move;
  Word16 totNoEst;
  Word16 noEstPerFrame;
  Word16 timeSlots;

  UWord16* guideScfb;
  Word16 *prevEnvelopeCompensation;
  UWord16* detectionVectors[NO_OF_ESTIMATES];
  Word32* tonalityDiff[NO_OF_ESTIMATES];
  Word32* sfmOrig[NO_OF_ESTIMATES];
  Word32* sfmSbr[NO_OF_ESTIMATES];
  GUIDE_VECTORS guideVectors[NO_OF_ESTIMATES]; /* size Word16: 12 */
}
SBR_MISSING_HARMONICS_DETECTOR; /* size Word16: 43 */

typedef SBR_MISSING_HARMONICS_DETECTOR *HANDLE_SBR_MISSING_HARMONICS_DETECTOR;



void
SbrMissingHarmonicsDetectorQmf(HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
                               Word32 ** pQuotaBuffer,
                               Word16 *indexVector,
                               const SBR_FRAME_INFO *pFrameInfo,
                               const Word16* pTranInfo,
                               Word16* pAddHarmonicsFlag,
                               UWord16* pAddHarmonicsScaleFactorBands,
                               const UWord16* freqBandTable,
                               Word16 nSfb,
                               Word16 * envelopeCompensation);


Word32
CreateSbrMissingHarmonicsDetector(Word16 chan,
                                  HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector,
                                  Word32 sampleFreq,
                                  Word16 nSfb,
                                  Word16 qmfNoChannels,
                                  Word16 totNoEst,
                                  Word16 move,
                                  Word16 noEstPerFrame);

void
DeleteSbrMissingHarmonicsDetector (HANDLE_SBR_MISSING_HARMONICS_DETECTOR h_sbrMissingHarmonicsDetector);


Word32
ResetSbrMissingHarmonicsDetector (HANDLE_SBR_MISSING_HARMONICS_DETECTOR hSbrMissingHarmonicsDetector,
                                  Word16 nSfb);




#endif
