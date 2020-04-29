/*
   Memory layout
 */
#ifndef __SBR_RAM_H
#define __SBR_RAM_H

#include "aacenc.h"
#include "ffr.h"
#include "sbr_def.h"
#include "hybrid.h"


extern Word16 sbr_envRBuffer[MAX_CHANNELS * QMF_TIME_SLOTS  * QMF_CHANNELS];
extern Word16 sbr_envIBuffer[MAX_CHANNELS * QMF_TIME_SLOTS  * QMF_CHANNELS];
extern Word16 sbr_QmfStatesAnalysis[MAX_CHANNELS * QMF_FILTER_LENGTH];
extern Word32 sbr_QmfStatesSynthesis[QMF_FILTER_STATE_SYN_SIZE];
extern Word32 sbr_envYBuffer[MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS]; 
extern Word32 sbr_quotaMatrix[MAX_CHANNELS * NO_OF_ESTIMATES * QMF_CHANNELS];
extern Word32 sbr_thresholds[MAX_CHANNELS * QMF_CHANNELS];
extern UWord16 sbr_freqBandTableLO[MAX_FREQ_COEFFS / 2 + 1];
extern UWord16 sbr_freqBandTableHI[MAX_FREQ_COEFFS + 1];
extern UWord16 sbr_v_k_master[MAX_FREQ_COEFFS + 1];
extern UWord16 sbr_detectionVectors[MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];/* Word16*/
extern Word16 sbr_prevEnvelopeCompensation[MAX_CHANNELS * MAX_FREQ_COEFFS];
extern UWord16 sbr_guideScfb[MAX_CHANNELS * MAX_FREQ_COEFFS];
extern UWord16 sbr_guideVectorDetected[MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];
extern Word32 sbr_toncorrBuff[5 * MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];
extern Word32 sbr_transients[MAX_CHANNELS * 3 * QMF_TIME_SLOTS];
extern Word32 ps_IccDataBuffer[NO_BINS];
extern Word32 ps_IidDataBuffer[NO_BINS];
extern Word16 ps_histQmfBuffer[2 * 2 * HYBRID_FILTER_DELAY * (NO_QMF_BANDS_IN_HYBRID + QMF_CHANNELS)];
extern Word16 ps_tempQmfBuffer[2 * 2 * HYBRID_FILTER_DELAY * (NO_QMF_BANDS_IN_HYBRID + QMF_CHANNELS)];

#endif

