/*
   SBR encoder top level processing prototypes
 */

#ifndef __SBR_MAIN_H
#define __SBR_MAIN_H

#include "ffr.h"

  /* core coder helpers */
#define MAX_TRANS_FAC         8
#define MAX_CODEC_FRAME_RATIO 2
#define MAX_PAYLOAD_SIZE    128


typedef struct
{
  Word32 bitRate;
  Word16 nChannels;
  Word32 sampleFreq;
  Word16 transFac;
  Word32 standardBitrate;
} CODEC_PARAM;


typedef enum
{
  SBR_MONO,
  SBR_LEFT_RIGHT,
  SBR_COUPLING,
  SBR_SWITCH_LRC
}
SBR_STEREO_MODE;

typedef struct sbrConfiguration
{
  /* 
     core coder dependent configurations
  */
  CODEC_PARAM codecSettings;  /*!< Core coder settings, to be set from core coder */
  Word16 SendHeaderDataTime;     /*!< SBR-Header send update frequency in msec */
  Word16 crcSbr;                 /*!< Flag: usage of SBR-CRC */
  Word16 detectMissingHarmonics; /*!< Flag: usage of missing harmonics detection */
  Word16 parametricCoding;       /*!< Flag: usage of parametric coding tool */
  
  
  /* 
     core coder dependent tuning parameters
  */
  Word32 tran_thr;             /*!< SBR transient detector threshold (* 100) */
  Word32 noiseFloorOffset;     /*! Noise floor offset      */
  UWord16 useSpeechConfig;     /*!< Flag: adapt tuning parameters according to speech */
  
  
  
  /* 
     core coder independent configurations
  */
  Word16 sbrFrameSize;           /*!< SBR frame size in samples, will be calculated from core coder settings */
  Word16 sbr_data_extra;         /*!< Flag usage of data extra */
  Word16 amp_res;                /*!< Amplitude resolution */
  Word32 ana_max_level;          /*!< Noise insertion maximum level */
  Word16 tran_fc;                /*!< Transient detector start frequency */
  Word16 tran_det_mode;          /*!< Transient detector mode */
  Word16 spread;                 /*!< Flag: usage of SBR spread */
  Word16 stat;                   /*!< Flag: usage of static framing */
  Word16 e;                      /*!< Number of envelopes when static framing is chosen */
  SBR_STEREO_MODE stereoMode; /*!< SBR stereo mode */
  Word16 deltaTAcrossFrames;     /*!< Flag: allow time-delta coding */
  Word16 sbr_invf_mode;          /*!< Inverse Filtering mode */
  Word16 sbr_xpos_mode;          /*!< Transposer mode */
  Word16 sbr_xpos_ctrl;          /*!< Transposer control */
  Word16 sbr_xpos_level;         /*!< Transposer 3rd order level */
  Word16 startFreq;              /*!< The start frequency table index */
  Word16 stopFreq;               /*!< The stop frequency table index */
  Word16 usePs;                  /*!< Flag: usage of parametric stereo */
  Word32 psMode;
  
  
  /* 
     header_extra1 configuration 
  */ 
  Word16 freqScale;              /*!< Frequency grouping */
  Word16 alterScale;             /*!< Scale resolution */
  Word16 sbr_noise_bands;        /*!< Number of noise bands */
  
  
  /* 
     header_extra2 configuration 
  */
  Word16 sbr_limiter_bands;      /*!< Number of limiter bands */
  Word16 sbr_limiter_gains;      /*!< Gain of limiter */
  Word16 sbr_interpol_freq;      /*!< Flag: use interpolation in freq. direction */
  Word16 sbr_smoothing_length;   /*!< Flag: choose length 4 or 0 (=on, off) */
  
} sbrConfiguration, *sbrConfigurationPtr ;


UWord32
IsSbrSettingAvail (UWord32 bitrate,
                   UWord16 numOutputChannels,
                   UWord32 sampleRateInput,
                   UWord32 *sampleRateCore);


UWord32 
AdjustSbrSettings (const sbrConfigurationPtr config,
                   UWord32 bitRate,
                   UWord16 numChannels,
                   UWord32 fsCore,
                   UWord16 transFac,
                   UWord32 standardBitrate);

UWord32
InitializeSbrDefaults (sbrConfigurationPtr config);


typedef struct SBR_ENCODER *HANDLE_SBR_ENCODER;



Word32
EnvOpen (HANDLE_SBR_ENCODER*  hEnvEncoder,
         sbrConfigurationPtr  params,
         Word16               *coreBandWith  /**< encoder (lowband) bandwith in Hz */      
         ); 

void 
EnvClose (HANDLE_SBR_ENCODER hEnvEnc);

Word32
SbrGetXOverFreq(HANDLE_SBR_ENCODER hEnv,
                Word32         xoverFreq );

Word32
SbrGetStopFreqRaw(HANDLE_SBR_ENCODER hEnv);

Word32
EnvEncodeFrame (HANDLE_SBR_ENCODER hEnvEncoder,
                Word16  *samples,                 /*!< time samples, always interleaved  */
                Word16  *pCoreBuffer,
                UWord16 timeInStride,
                Word16  *numAncBytes,
                UWord8  *ancData);

#endif /* ifndef __SBR_MAIN_H */
