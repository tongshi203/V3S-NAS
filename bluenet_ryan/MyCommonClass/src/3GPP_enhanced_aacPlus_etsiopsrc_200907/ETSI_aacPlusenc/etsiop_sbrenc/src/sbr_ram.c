/*
   This module declares all static and dynamic memory spaces
 */
#include <stdio.h>
#include "sbr_ram.h"
#include "sbr_def.h"
#include "sbr_main.h"
#include "env_est.h"
#include "sbr.h"
#include "cmondata.h"
#include "ps_enc.h"


Word16 sbr_envRBuffer[MAX_CHANNELS * QMF_TIME_SLOTS  * QMF_CHANNELS];                       /* dynamic: 4096 16-bit Words */
Word16 sbr_envIBuffer[MAX_CHANNELS * QMF_TIME_SLOTS  * QMF_CHANNELS];                       /* dynamic: 4096 16-bit Words */
Word32 sbr_transients[MAX_CHANNELS * 3 * QMF_TIME_SLOTS];                                   /* dynamic:  384 16-bit Words */
#ifndef MONO_ONLY
Word16 ps_tempQmfBuffer[2 * 2 * HYBRID_FILTER_DELAY * (NO_QMF_BANDS_IN_HYBRID + QMF_CHANNELS)]; /* dynamic: 1608 16-bit Words */
#endif

Word16 sbr_QmfStatesAnalysis[MAX_CHANNELS * QMF_FILTER_LENGTH];                             /* static: 1280 16-bit Words */
#ifndef MONO_ONLY
Word32 sbr_QmfStatesSynthesis[QMF_FILTER_STATE_SYN_SIZE];                                   /* static:  640 16-bit Words */
#endif
Word32 sbr_envYBuffer[MAX_CHANNELS * QMF_TIME_SLOTS * QMF_CHANNELS];                        /* static: 8192 16-bit Words */
Word32 sbr_quotaMatrix[MAX_CHANNELS * NO_OF_ESTIMATES * QMF_CHANNELS];                      /* static: 1024 16-bit Words */
Word32 sbr_thresholds[MAX_CHANNELS * QMF_CHANNELS];                                         /* static:  256 16-bit Words */
UWord16 sbr_freqBandTableLO[MAX_FREQ_COEFFS / 2 + 1];                                       /* static:   14 16-bit Words */
UWord16 sbr_freqBandTableHI[MAX_FREQ_COEFFS + 1];                                           /* static:   28 16-bit Words */
UWord16 sbr_v_k_master[MAX_FREQ_COEFFS + 1];                                                /* static:   28 16-bit Words */
UWord16 sbr_detectionVectors[MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];             /* static:  216 16-bit Words */
Word16 sbr_prevEnvelopeCompensation[MAX_CHANNELS * MAX_FREQ_COEFFS];                        /* static:   54 16-bit Words */
UWord16 sbr_guideScfb[MAX_CHANNELS * MAX_FREQ_COEFFS];                                      /* static:   54 16-bit Words */
UWord16 sbr_guideVectorDetected[MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];          /* static:  216 16-bit Words */
Word32 sbr_toncorrBuff[5 * MAX_CHANNELS * NO_OF_ESTIMATES * MAX_FREQ_COEFFS];               /* static: 2160 16-bit Words */
#ifndef MONO_ONLY
Word32 ps_IccDataBuffer[NO_BINS];                                                               /* static:   40 16-bit Words */
Word32 ps_IidDataBuffer[NO_BINS];                                                               /* static:   40 16-bit Words */
Word16 ps_histQmfBuffer[2 * 2 * HYBRID_FILTER_DELAY * (NO_QMF_BANDS_IN_HYBRID + QMF_CHANNELS)]; /* static: 1608 16-bit Words */
#endif









