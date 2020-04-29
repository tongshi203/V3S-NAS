/*
   Main SBR structs definitions
 */

#ifndef __SBR_H
#define __SBR_H

#include "qmf_enc.h"
#include "tran_det.h"
#include "fram_gen.h"
#include "nf_est.h"
#include "mh_det.h"
#include "invf_est.h"
#include "env_est.h"
#include "code_env.h"
#include "sbr_main.h"
#include "ton_corr.h"

struct SBR_BITSTREAM_DATA
{
  Word32 TotalBits;
  Word32 PayloadBits;
  Word32 FillBits;
  Flag   HeaderActive;
  Word16 NrSendHeaderData;
  Word16 CountSendHeaderData;         /* modulo count */
};

typedef struct SBR_BITSTREAM_DATA  *HANDLE_SBR_BITSTREAM_DATA;

struct SBR_HEADER_DATA
{

  Word16 protocol_version;
  AMP_RES sbr_amp_res;
  Word16 sbr_start_frequency;
  Word16 sbr_stop_frequency;
  Word16 sbr_xover_band;
  Word16 sbr_noise_bands;
  Flag   sbr_data_extra;
  Flag    header_extra_1;
  Flag    header_extra_2;
  Word16 sbr_limiter_bands;
  Word16 sbr_limiter_gains;
  Word16 sbr_interpol_freq;
  Word16 sbr_smoothing_length;
  Word16 alterScale;
  Word16 freqScale;

  /*
    element of sbrdata
  */
  SR_MODE sampleRateMode;

  /*
    element of channelpairelement
  */
  Word16 coupling;
  Word16 prev_coupling;

};

typedef struct SBR_HEADER_DATA *HANDLE_SBR_HEADER_DATA;
  
struct SBR_CONFIG_DATA
{
  Word16 nChannels;                                /* number of channels  */
  
  Word16 nSfb[2];                                  /* number of SBR scalefactor bands for LO_RES and HI_RES (?) */
  Word16 num_Master;		                   /* number of elements in v_k_master */
  Word32 sampleFreq;                               /* SBR sampling frequency */
  Word16 xOverFreq;                                /* the SBR start frequency */

  UWord16 *freqBandTable[2];                        /* frequency table for low and hires, only MAX_FREQ_COEFFS/2 +1 coeefs actually needed for lowres */
  UWord16 *v_k_master;		                   /* master BandTable which freqBandTable is derived from */

  SBR_STEREO_MODE stereoMode;

  Flag useParametricCoding;
};

typedef struct SBR_CONFIG_DATA *HANDLE_SBR_CONFIG_DATA;

struct SBR_ENV_DATA
{
 
  Word16 sbr_xpos_ctrl;
  Word16 freq_res_fixfix;
  

  INVF_MODE sbr_invf_mode;
  INVF_MODE sbr_invf_mode_vec[MAX_NUM_NOISE_VALUES];
  
  XPOS_MODE sbr_xpos_mode;

  Word32 ienvelope[MAX_ENVELOPES][MAX_FREQ_COEFFS];
 
  Word16 codeBookScfLavBalance;
  Word16 codeBookScfLav;
  const Word32 *hufftableTimeC;
  const Word32 *hufftableFreqC;
  const UWord8 *hufftableTimeL;
  const UWord8 *hufftableFreqL;

  const Word32 *hufftableLevelTimeC;
  const Word32 *hufftableBalanceTimeC;
  const Word32 *hufftableLevelFreqC;
  const Word32 *hufftableBalanceFreqC;
  const UWord8 *hufftableLevelTimeL;
  const UWord8 *hufftableBalanceTimeL;
  const UWord8 *hufftableLevelFreqL;
  const UWord8 *hufftableBalanceFreqL;


  const UWord8 *hufftableNoiseTimeL;
  const Word32 *hufftableNoiseTimeC;
  const UWord8 *hufftableNoiseFreqL;
  const Word32 *hufftableNoiseFreqC;

  const UWord8 *hufftableNoiseLevelTimeL;
  const Word32 *hufftableNoiseLevelTimeC;
  const UWord8 *hufftableNoiseBalanceTimeL;
  const Word32 *hufftableNoiseBalanceTimeC;
  const UWord8 *hufftableNoiseLevelFreqL;
  const Word32 *hufftableNoiseLevelFreqC;
  const UWord8 *hufftableNoiseBalanceFreqL;
  const Word32 *hufftableNoiseBalanceFreqC;
  
  HANDLE_SBR_GRID hSbrBSGrid;

  
  Word16 syntheticCoding;
  Word16 noHarmonics;
  Word16 addHarmonicFlag;
  UWord16 addHarmonic[MAX_FREQ_COEFFS];


  /* calculated helper vars */
  Word16 si_sbr_start_env_bits_balance;
  Word16 si_sbr_start_env_bits;
  Word16 si_sbr_start_noise_bits_balance;
  Word16 si_sbr_start_noise_bits;

  Word16 noOfEnvelopes;
  Word16 noScfBands[MAX_ENVELOPES];
  Word16 domain_vec[MAX_ENVELOPES];
  Word16 domain_vec_noise[MAX_ENVELOPES];
  Word16 sbr_noise_levels[MAX_FREQ_COEFFS];
  Word16 noOfnoisebands;

  Word16 balance;
  AMP_RES init_sbr_amp_res;
}; /* size Word16: 402 */


typedef struct SBR_ENV_DATA *HANDLE_SBR_ENV_DATA;

struct ENV_CHANNEL
{
  
  SBR_TRANSIENT_DETECTOR sbrTransientDetector;  /* size Word16:  14 */
  SBR_CODE_ENVELOPE sbrCodeEnvelope;            /* size Word16:  46 */
  SBR_CODE_ENVELOPE sbrCodeNoiseFloor;          /* size Word16:  46 */
  SBR_EXTRACT_ENVELOPE sbrExtractEnvelope;      /* size Word16: 200 */
  SBR_QMF_FILTER_BANK sbrQmf;                   /* size Word16:  12 */
  SBR_ENVELOPE_FRAME SbrEnvFrame;               /* size Word16:  88 */
  SBR_TON_CORR_EST   TonCorr;                   /* size Word16: 717 */
  
  struct SBR_ENV_DATA encEnvData;               /* size Word16: 402 */

  Word16 *sfb_nrg;
  Word16 *sfb_nrg_coupling; /* only used if stereomode = SWITCH_L_R_C */   
  Word32 *noiseFloor;
  Word16 *noise_level;
  Word16 *noise_level_coupling; /* only used if stereomode = SWITCH_L_R_C */   
}; /* size Word16: 1530 */

typedef struct ENV_CHANNEL *HANDLE_ENV_CHANNEL;


#endif /* __SBR_H */
