/*
   Noise floor estimation structs and prototypes
 */

#ifndef __NF_EST_H
#define __NF_EST_H

#include "sbr_main.h"
#include "sbr_def.h"
#include "fram_gen.h"


#define NF_SMOOTHING_LENGTH 4                   /*!< Smoothing length of the noise floors. */


typedef struct
{
  Word32 prevNoiseLevels[NF_SMOOTHING_LENGTH][MAX_NUM_NOISE_VALUES];/*!< The previous noise levels. */
  Word16 freqBandTableQmf[MAX_NUM_NOISE_VALUES + 1]; /*!< Frequncy band table for the noise floor bands.*/
  Word32 ana_max_level;                              /*!< Max level allowed.   */
  Word32 weightFac;                                  /*!< Weightening factor for the difference between orig and sbr. */
  Word16 noNoiseBands;                               /*!< Number of noisebands. */
  Word16 noiseBands;                                 /*!< NoiseBands switch 4 bit.*/
  Word32 noiseFloorOffset[MAX_NUM_NOISE_VALUES];     /*!< Noise floor offset. */
  Word16 timeSlots;                                  /*!< Number of timeslots in a frame. */    
  const Word32* smoothFilter;                        /*!< Smoothing filter to use. */  
  INVF_MODE diffThres;                               /*!< Threshold value to control the inverse filtering decision */
}
SBR_NOISE_FLOOR_ESTIMATE; /* size Word16: 120 */

typedef SBR_NOISE_FLOOR_ESTIMATE *HANDLE_SBR_NOISE_FLOOR_ESTIMATE;



void
sbrNoiseFloorEstimateQmf (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                          const SBR_FRAME_INFO *frame_info,   /*!< Time frequency grid of the current frame. */
                          Word32 *noiseLevels,                /*!< Pointer to vector to store the noise levels in.*/
                          Word32** quotaMatrixOrig,           /*!< Matrix holding the quota values of the original. */
                          Word16* indexVector,                /*!< Index vector to obtain the patched data. */
                          Word16 missingHarmonicsFlag,        /*!< Flag indicating if a strong tonal component will be missing. */
                          Word16 startIndex,                  /*!< Start index. */
                          INVF_MODE* pInvFiltLevels           /*!< Pointer to the vector holding the inverse filtering levels. */
                          );


Word32
CreateSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE  h_sbrNoiseFloorEstimate,   /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                             Word32 ana_max_level,            /*!< Maximum level of the adaptive noise. */
                             const UWord16 *freqBandTable,    /*!< Frequany band table. */
                             Word16 nSfb,                     /*!< Number of frequency bands. */
                             Word16 noiseBands,               /*!< Number of noise bands per octave. */
                             Word32 noiseFloorOffset,         /*!< Noise floor offset. */
                             Word16 timeSlots,                /*!< Number of time slots in a frame. */
                             UWord16 useSpeechConfig          /*!< Flag: adapt tuning parameters according to speech */
                             );



Word32
resetSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate, /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */
                            const UWord16 *freqBandTable,     /*!< Frequany band table. */
                            Word16 nSfb);                     /*!< Number of bands in the frequency band table. */




void deleteSbrNoiseFloorEstimate (HANDLE_SBR_NOISE_FLOOR_ESTIMATE h_sbrNoiseFloorEstimate); /*!< Handle to SBR_NOISE_FLOOR_ESTIMATE struct */

#endif
