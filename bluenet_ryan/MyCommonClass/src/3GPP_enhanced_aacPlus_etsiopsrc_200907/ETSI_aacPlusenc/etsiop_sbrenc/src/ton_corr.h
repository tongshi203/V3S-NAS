/*
   General tonality correction detector module
 */
#ifndef _TON_CORR_EST_H
#define _TON_CORR_EST_H

#include "sbr.h"
#include "sbr_main.h"
#include "sbr_def.h"
#include "mh_det.h"
#include "invf_est.h"
#include "nf_est.h"


#define MAX_NUM_PATCHES 6

/** parameter set for one single patch */
typedef struct {
  Word16    sourceStartBand;         /*!< first band in lowbands where to take the samples from */ 
  Word16    sourceStopBand;          /*!< first band in lowbands which is not included in the patch anymore */
  Word16    guardStartBand;          /*!< first band in highbands to be filled with zeros in order to
                                       reduce interferences between patches */
  Word16    targetStartBand;         /*!< first band in highbands to be filled with whitened lowband signal */
  Word16    targetBandOffs;          /*!< difference between 'startTargetBand' and 'startSourceBand' */
  Word16    numBandsInPatch;         /*!< number of consecutive bands in this one patch */
} PATCH_PARAM; /* size Word16: 6 */




typedef struct
{
  Word16 switchInverseFilt;          /*!< Flag to enable dynamic adaption of invf. detection */
  Word16 noQmfChannels;
  Word16 bufferLength;               /*!< Length of the r and i buffers. */
  Word16 stepSize;                   /*!< Stride for the lpc estimate. */
  Word16 numberOfEstimates;          /*!< The total number of estiamtes, available in the quotaMatrix.*/
  Word16 numberOfEstimatesPerFrame;  /*!< The number of estiamtes per frame available in the quotaMatrix.*/
  Word16 lpcLength;                  /*!< Segment length used for second order LPC analysis.*/
  Word16 nextSample;                 /*!< Where to start the LPC analysis of the current frame.*/
  Word16 move;                       /*!< How many estaimates to move in the quotaMatrix, when buffering. */
  Word16 frameStartIndex;            /*!< The start index for the current frame in the r and i buffers. */
  Word16 startIndexMatrix;           /*!< The start index for the current frame in the quotaMatrix. */
  Word16 frameStartIndexInvfEst;     /*!< The start index of the inverse filtering, not the same as the others,
                                       dependent on what decoder is used (buffer opt, or no buffer opt). */
  Word16 prevTransientFlag;          /*!< The transisent flag (from the transient detector) for the previous frame. */
  Word16 transientNextFrame;         /*!< Flag to indicate that the transient will show up in the next frame. */
  Word16 transientPosOffset;         /*!< An offset value to match the transient pos as calculated by the transient detector
                                       with the actual position in the frame.*/
  Word32 *quotaMatrix[NO_OF_ESTIMATES];  /*!< Matrix holding the quota values for all estimates, all channels. */
  Word32 nrgVector[NO_OF_ESTIMATES];     /*!< Vector holding the averaged energies for every QMF band. */
  Word16 indexVector[QMF_CHANNELS];       /*!< Index vector pointing to the correct lowband channel,
                                              when indexing a highband channel, -1 represents a guard band */
  PATCH_PARAM  patchParam[MAX_NUM_PATCHES];     /*!< new parameter set for patching */
  Word16    guard;                              /*!< number of guardbands between every patch */
  Word16    shiftStartSb;                       /*!< lowest subband of source range to be included in the patches */
  Word16    noOfPatches;                        /*!< number of patches */

  SBR_MISSING_HARMONICS_DETECTOR sbrMissingHarmonicsDetector;  /*!< SBR_MISSING_HARMONICS_DETECTOR struct. */ /* size Word16: 43 */
  SBR_NOISE_FLOOR_ESTIMATE sbrNoiseFloorEstimate;              /*!< SBR_NOISE_FLOOR_ESTIMATE struct. */ /* size Word16: 120 */
  SBR_INV_FILT_EST sbrInvFilt;                                 /*!< SBR_INV_FILT_EST struct. */ /* size Word16: 423 */
}
SBR_TON_CORR_EST; /* size Word16: 717 */

typedef SBR_TON_CORR_EST *HANDLE_SBR_TON_CORR_EST;




void
TonCorrParamExtr(HANDLE_SBR_TON_CORR_EST hTonCorr,   /*!< Handle to SBR_TON_CORR struct. */
                 INVF_MODE* infVec,                  /*!< Vector where the inverse filtering levels will be stored. */ 
                 Word32* noiseLevels,                /*!< Vector where the noise levels will be stored. */
                 Word16* missingHarmonicFlag,        /*!< Flag set to one or zero, dependent on if any strong sines are missing.*/ 
                 UWord16* missingHarmonicsIndex,     /*!< Vector indicating where sines are missing. */
                 Word16* envelopeCompensation,        /*!< Vector to store compensation values for the energies in. */
                 const SBR_FRAME_INFO *frameInfo,    /*!< Frame info struct, contains the time and frequency grid of the current frame.*/
                 Word16* transientInfo,              /*!< Transient info.*/
                 UWord16 * freqBandTable,            /*!< Frequency band tables for high-res.*/
                 Word16 nSfb,                        /*!< Number of scalefactor bands for high-res. */
                 XPOS_MODE xposType                  /*!< Type of transposer used in the decoder.*/
                 ); 



Word32
CreateTonCorrParamExtr (Word16 chan,                          /*!< Channel index, needed for mem allocation */ 
                        HANDLE_SBR_TON_CORR_EST hTonCorr,     /*!< Pointer to handle to SBR_TON_CORR struct. */
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
                        );


void
DeleteTonCorrParamExtr(HANDLE_SBR_TON_CORR_EST hTonCorr);   /*!< Handle to SBR_TON_CORR struct. */


void 
CalculateTonalityQuotas(HANDLE_SBR_TON_CORR_EST hTonCorr,
                        Word16 **sourceBufferReal,
                        Word16 **sourceBufferImag,
                        Word16 usb,
                        Word16 qmfScale                    /*!< sclefactor of QMF subsamples */  
                        );


Word32
ResetTonCorrParamExtr(HANDLE_SBR_TON_CORR_EST hTonCorr,    /*!< Handle to SBR_TON_CORR struct. */
                      Word16 xposctrl,                     /*!< Different patch modes. */
                      Word16 highBandStartSb,              /*!< Start band of the SBR range. */
                      UWord16 *v_k_master,                 /*!< Master frequency table from which all other table are derived.*/
                      Word16 numMaster,                    /*!< Number of elements in the master table. */  
                      Word32 fs,                           /*!< Sampling frequency (of the SBR part). */
                      UWord16** freqBandTable,             /*!< Frequency band table for low-res and high-res. */  
                      Word16* nSfb,                        /*!< Number of frequency bands (hig-res and low-res). */  
                      Word16 noQmfChannels                 /*!< Number of QMF channels. */ 
                      ); 
#endif 

