/*
   SBR encoder top level processing
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "sbr_main.h"
#include "sbr.h"
#include "sbr_ram.h"
#include "freq_sca.h"
#include "ps_enc.h"
#include "env_bit.h"
#include "cmondata.h"
#include "ffr.h"

#include "count.h"

#define INVALID_TABLE_IDX -1


#define GetBitsAvail(hB) ((hB)->cntBits)
/* 
   tuningTable 
*/
static const struct
{
  UWord32    bitrateFrom ;      /*!< inclusive */
  UWord32    bitrateTo ;        /*!< exclusive */
  
  UWord32    sampleRate ;
  UWord16    numChannels ;
  
  UWord16    startFreq ;        /*!< bs_start_freq */
  UWord16    stopFreq ;         /*!< bs_stop_freq */

  Word16 numNoiseBands ;
  Word16 noiseFloorOffset ;
  Word16 noiseMaxLevel ;
  SBR_STEREO_MODE stereoMode ;
  Word16 freqScale ;

} tuningTable[] = {

  /* sf,sfsp,sf,sfsp,nnb,nfo,saml,SM,FS */

  /*** mono ***/
  { 10000, 12000,  16000, 1,  1, 3,  1, 0, 6, SBR_MONO, 3 }, /* nominal: 10 kbit/s */
  { 12000, 16000,  16000, 1,  2, 0,  1, 0, 6, SBR_MONO, 3 }, /* nominal: 12 kbit/s */
  { 16000, 18000,  16000, 1,  2, 3,  1, 0, 6, SBR_MONO, 3 }, /* nominal: 16 kbit/s */

  { 12000, 18000,  22050, 1,  1, 4,  1, 0, 6, SBR_MONO, 3 }, /* nominal: 14 kbit/s */
  { 18000, 22000,  22050, 1,  3, 5,  2, 0, 6, SBR_MONO, 2 }, /* nominal: 20 kbit/s */
  { 22000, 28000,  22050, 1,  7, 8,  2, 0, 6, SBR_MONO, 2 }, /* nominal: 24 kbit/s */
  { 28000, 36000,  22050, 1, 11, 9,  2, 0, 3, SBR_MONO, 2 }, /* nominal: 32 kbit/s */
  { 36000, 44001,  22050, 1, 11, 9,  2, 0, 3, SBR_MONO, 1 }, /* nominal: 40 kbit/s */

  { 12000, 18000,  24000, 1,  1, 4,  1, 0, 6, SBR_MONO, 3 }, /* nominal: 14 kbit/s */
  { 18000, 22000,  24000, 1,  3, 5,  2, 0, 6, SBR_MONO, 2 }, /* nominal: 20 kbit/s */
  { 22000, 28000,  24000, 1,  7, 8,  2, 0, 6, SBR_MONO, 2 }, /* nominal: 24 kbit/s */
  { 28000, 36000,  24000, 1, 10, 9,  2, 0, 3, SBR_MONO, 2 }, /* nominal: 32 kbit/s */
  { 36000, 44001,  24000, 1, 11, 9,  2, 0, 3, SBR_MONO, 1 }, /* nominal: 40 kbit/s */

  /*** stereo ***/
  { 18000, 24000,  16000, 2,  4, 2,  1, 0, -3, SBR_SWITCH_LRC, 3 }, /* nominal: 18 kbit/s */

  { 24000, 28000,  22050, 2,  5, 6,  1, 0, -3, SBR_SWITCH_LRC, 3 }, /* nominal: 24 kbit/s */
  { 28000, 36000,  22050, 2,  7, 8,  2, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 32 kbit/s */
  { 36000, 44000,  22050, 2, 10, 9,  2, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 40 kbit/s */
  { 44000, 52000,  22050, 2, 12, 9,  3, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 48 kbit/s */

  { 24000, 28000,  24000, 2,  5, 6,  1, 0, -3, SBR_SWITCH_LRC, 3 }, /* nominal: 24 kbit/s */
  { 28000, 36000,  24000, 2,  7, 8,  2, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 32 kbit/s */
  { 36000, 44000,  24000, 2, 10, 9,  2, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 40 kbit/s */
  { 44000, 52000,  24000, 2, 12, 9,  3, 0, -3, SBR_SWITCH_LRC, 2 }, /* nominal: 48 kbit/s */
};



struct SBR_ENCODER
{
  
  struct SBR_CONFIG_DATA    sbrConfigData;                    /* size Word16: 12 */
  struct SBR_HEADER_DATA    sbrHeaderData;                    /* size Word16: 18 */
  struct SBR_BITSTREAM_DATA sbrBitstreamData;                 /* size Word16:  9 */
  struct ENV_CHANNEL       *hEnvChannel[MAX_CHANNELS];
  struct ENV_CHANNEL        EnvChannel[MAX_CHANNELS];         /* size Word16: MAX_NUM_CHANNELS * 1505  */
  struct COMMON_DATA        CmonData;                         /* size Word16: 23  */
  struct PS_ENC            *hPsEnc;  
  SBR_QMF_FILTER_BANK      *hSynthesisQmfBank;
#ifndef MONO_ONLY
  struct PS_ENC             PsEnc;                            /* size Word16: 431  */
  SBR_QMF_FILTER_BANK       SynthesisQmfBank;                 /* size Word16: 12  */
#endif
  UWord8                    sbrPayloadPrev[MAX_PAYLOAD_SIZE]; /* size Word16: 64 */
  UWord8                    sbrPayload[MAX_PAYLOAD_SIZE];     /* size Word16: 64 */
  Word16                    sbrPayloadSize;
} ; /* size Word16: 1698 / 3650 */


/*
  static sbr encoder instance for one encoder (2 channels)
  all major static and dynamic memory areas are located
  in module sbr_ram and sbr rom
*/

static struct SBR_ENCODER sbrEncoder;


/***************************************************************************/
/*!
 
\brief  Selects the SBR tuning settings to use dependent on number of
channels, bitrate, sample rate and core coder
  
\return Index to the appropriate table
 
****************************************************************************/
static Word16 
getSbrTuningTableIndex(UWord32 bitrate,    /*! the total bitrate in bits/sec */
                       UWord32 numChannels,/*! the number of channels for the core coder */
                       UWord32 sampleRate  /*! the sampling rate of the core coder */
                       )
{
  Word16 i, paramSets = sizeof (tuningTable) / sizeof (tuningTable [0]);

  for (i = 0 ; i < paramSets ; i++)  {
    test(); sub(1, 1);
    if (numChannels == tuningTable [i].numChannels) {
      test(); L_sub(1, 1);
      test(); L_sub(1, 1);
      test(); L_sub(1, 1);
      if ((sampleRate == tuningTable [i].sampleRate) &&
          (bitrate >= tuningTable [i].bitrateFrom) &&
          (bitrate < tuningTable [i].bitrateTo)) {
        return i ;
      }
    }
  }
  
  return INVALID_TABLE_IDX;
}

/***************************************************************************/
/*!
 
\brief  tells us, if for the given coreCoder, bitrate, number of channels
and input sampling rate an SBR setting is available. If yes, it
tells us also the core sampling rate we would need to run with
  
\return a flag indicating success: yes (1) or no (0)
 
****************************************************************************/
UWord32
IsSbrSettingAvail (UWord32 bitrate,          /*! the total bitrate in bits/sec */
                   UWord16 numOutputChannels,/*! the number of channels for the core coder */
                   UWord32 sampleRateInput,  /*! the input sample rate [in Hz] */
                   UWord32 *sampleRateCore   /*! output: the core sample rate [in Hz] */
                   )
{
  Word16 idx = INVALID_TABLE_IDX;

  test(); L_sub(1, 1);
  if (sampleRateInput < 32000)
    return 0;

  /* try half the input sampling rate */
  *sampleRateCore = L_shr(sampleRateInput, 1);                                   move32();
  idx = getSbrTuningTableIndex(bitrate,numOutputChannels, *sampleRateCore);      move16();

  
  test(); sub(1, 1);
  return (idx == INVALID_TABLE_IDX) ? 0 : 1;
}


/***************************************************************************/
/*!
 
\brief  Adjusts the SBR settings according to the chosen core coder
settings which are accessible via config->codecSettings
  
\return A flag indicating success: yes (1) or no (0)
 
****************************************************************************/
UWord32 
AdjustSbrSettings (const sbrConfigurationPtr config, /*! output, modified */
                   UWord32 bitRate,             /*! the total bitrate in bits/sec */
                   UWord16 numChannels,         /*! the core coder number of channels */
                   UWord32 fsCore,              /*! the core coder sampling rate in Hz */
                   UWord16 transFac,            /*! the short block to long block ratio */
                   UWord32 standardBitrate      /*! the standard bitrate per channel in bits/sec */
                   )
{
  Word16 idx = INVALID_TABLE_IDX;
  UWord32 sampleRate;
 
  /* set the codec settings */
  config->codecSettings.bitRate         = bitRate;               move32();
  config->codecSettings.nChannels       = numChannels;           move16();
  config->codecSettings.sampleFreq      = fsCore;                move32();
  config->codecSettings.transFac        = transFac;              move16();
  config->codecSettings.standardBitrate = standardBitrate;       move32();
  sampleRate  = L_shl(fsCore, 1);

  idx = getSbrTuningTableIndex(bitRate,numChannels,fsCore);

  test(); sub(1, 1);
  if (idx != INVALID_TABLE_IDX) {
    config->startFreq       = tuningTable [idx].startFreq ;      move16();
    config->stopFreq        = tuningTable [idx].stopFreq ;       move16();
    config->sbr_noise_bands = tuningTable [idx].numNoiseBands ;  move16();

    config->noiseFloorOffset= tuningTable[idx].noiseFloorOffset; move16();

    config->ana_max_level   = tuningTable [idx].noiseMaxLevel ;  move32();
    config->stereoMode      = tuningTable[idx].stereoMode ;      move16();
    config->freqScale       = tuningTable[idx].freqScale ;       move16();

    test(); L_sub(1, 1);
    if (bitRate <= 20000) {
      config->parametricCoding = 0;                              move16();
      config->useSpeechConfig  = 1;                              move16();
    }

#ifndef MONO_ONLY
    test();
    if(config->usePs) {
      config->psMode = GetPsMode(bitRate);                       move16();
    }
#endif

    return 1 ;
  }
  
  return 0 ;
}


/*****************************************************************************

 functionname: InitializeSbrDefaults
 description:  initializes the SBR confifuration
 returns:      error status
 input:        - core codec type,
               - fac of SBR to core frame length,
               - core frame length
 output:       initialized SBR configuration
 
*****************************************************************************/
UWord32
InitializeSbrDefaults (sbrConfigurationPtr config)
{
 
  
  config->SendHeaderDataTime     = 500;                          move16();
  config->tran_thr               = 13000;                        move32();
  config->parametricCoding       = 1;                            move16();
  config->useSpeechConfig        = 0;                            move16();
  
  
  /* sbr default parameters */
  config->sbr_data_extra         = 0;                            move16();
  config->amp_res                = SBR_AMP_RES_3_0 ;             move16();
  config->tran_fc                = 0 ;                           move16();
  config->tran_det_mode          = 1 ;                           move16();
  config->spread                 = 1 ;                           move16();
  config->stat                   = 0 ;                           move16();
  config->e                      = 1 ;                           move16();
  config->deltaTAcrossFrames     = 1 ;                           move16();

  config->sbr_invf_mode   = INVF_SWITCHED;                       move16();
  config->sbr_xpos_mode   = XPOS_LC;                             move16();
  config->sbr_xpos_ctrl   = SBR_XPOS_CTRL_DEFAULT;               move16();
  config->sbr_xpos_level  = 0;                                   move16();

  config->usePs           = 0;                                   move16();
  config->psMode          = -1;                                  move32();

  /* the following parameters are overwritten by the AdjustSbrSettings() function since
     they are included in the tuning table */
  config->stereoMode             = SBR_SWITCH_LRC;               move16();
  config->ana_max_level          = 6;                            move32();
  config->noiseFloorOffset       = 0;                            move16();
  config->startFreq              = 5;                            move16();
  config->stopFreq               = 9;                            move16();


  /* header_extra_1 */
  config->freqScale       = SBR_FREQ_SCALE_DEFAULT;              move16();
  config->alterScale      = SBR_ALTER_SCALE_DEFAULT;             move16();
  config->sbr_noise_bands = SBR_NOISE_BANDS_DEFAULT;             move16();

  /* header_extra_2 */
  config->sbr_limiter_bands    = SBR_LIMITER_BANDS_DEFAULT;      move16();
  config->sbr_limiter_gains    = SBR_LIMITER_GAINS_DEFAULT;      move16();
  config->sbr_interpol_freq    = SBR_INTERPOL_FREQ_DEFAULT;      move16();
  config->sbr_smoothing_length = SBR_SMOOTHING_LENGTH_DEFAULT;   move16();

  return 1;
}


/*****************************************************************************

 functionname: DeleteEnvChannel
 description:  frees memory of one SBR channel
 returns:      -
 input:        handle of channel
 output:       released handle
 
*****************************************************************************/
static void
deleteEnvChannel (HANDLE_ENV_CHANNEL hEnvCut)
{

  test();
  if (hEnvCut) {

    deleteFrameInfoGenerator (&hEnvCut->SbrEnvFrame);
    
    deleteQmfBank (&hEnvCut->sbrQmf);
    
    deleteSbrCodeEnvelope (&hEnvCut->sbrCodeEnvelope);
    
    deleteSbrCodeEnvelope (&hEnvCut->sbrCodeNoiseFloor);
    
    
    deleteSbrTransientDetector (&hEnvCut->sbrTransientDetector);
    
    deleteExtractSbrEnvelope (&hEnvCut->sbrExtractEnvelope);

   
    DeleteTonCorrParamExtr(&hEnvCut->TonCorr);
   
  }

}


/*****************************************************************************

 functionname: EnvClose
 description:  close the envelope coding handle
 returns:      
 input:        hEnv
 output:       
 
*****************************************************************************/
void
EnvClose (HANDLE_SBR_ENCODER hEnvEnc)
{
  Word16 i;

  test();
  if (hEnvEnc != NULL) {
    for (i = 0; i < MAX_CHANNELS; i++) {
      test();
      if (hEnvEnc->hEnvChannel[i] != NULL) {
        deleteEnvChannel (hEnvEnc->hEnvChannel[i]);
        hEnvEnc->hEnvChannel[i] = NULL;                         move16();
      }
    }
  }
}

/*****************************************************************************

 functionname: updateFreqBandTable
 description:  updates vk_master
 returns:      -
 input:        config handle
 output:       error info
 
*****************************************************************************/
static Word32 updateFreqBandTable(HANDLE_SBR_CONFIG_DATA  sbrConfigData,
                                  HANDLE_SBR_HEADER_DATA  sbrHeaderData,
                                  Word16 noQmfChannels)
{

  
  Word16 k0, k2;

  test();
  if(FindStartAndStopBand(sbrConfigData->sampleFreq,
                          noQmfChannels,
                          sbrHeaderData->sbr_start_frequency, 
                          sbrHeaderData->sbr_stop_frequency,
                          sbrHeaderData->sampleRateMode,
                          &k0, &k2))
    return(1);



  test();
  if(UpdateFreqScale(sbrConfigData->v_k_master, &sbrConfigData->num_Master,
                     k0, k2, sbrHeaderData->freqScale,
                     sbrHeaderData->alterScale))
    return(1);


  sbrHeaderData->sbr_xover_band=0;                                        move16();
  
  
  test();
  if(UpdateHiRes(sbrConfigData->freqBandTable[HI], 
                 &sbrConfigData->nSfb[HI],
                 sbrConfigData->v_k_master, 
                 sbrConfigData->num_Master , 
                 &sbrHeaderData->sbr_xover_band, 
                 sbrHeaderData->sampleRateMode,
                 noQmfChannels))
    return(1);

  
  UpdateLoRes(sbrConfigData->freqBandTable[LO],
              &sbrConfigData->nSfb[LO],
              sbrConfigData->freqBandTable[HI],
              sbrConfigData->nSfb[HI]);

  sbrConfigData->xOverFreq = extract_l(L_shr(L_add(ffr_Integer_Div(ffr_Integer_Mult(sbrConfigData->freqBandTable[LOW_RES][0], 
                                         sbrConfigData->sampleFreq), noQmfChannels), 1), 1)); move32();



  return (0);
}


/*****************************************************************************

 functionname: EnvEncodeFrame
 description: performs the sbr envelope calculation
 returns:      
 input:        
 output:       
 
*****************************************************************************/
Word32
EnvEncodeFrame (HANDLE_SBR_ENCODER hEnvEncoder,
                Word16  *samples,                 /*!< time samples, always interleaved */
                Word16  *pCoreBuffer,             /*!< Downmixed time samples (output) */
                UWord16 timeInStride,
                Word16  *numAncBytes,
                UWord8  *ancData)
{
  test();
  if (hEnvEncoder != NULL) {
    /* header bitstream handling */ 

    HANDLE_SBR_BITSTREAM_DATA sbrBitstreamData = &hEnvEncoder->sbrBitstreamData;

 
    sbrBitstreamData->HeaderActive = 0;                                                        move16();

    test();
    if (sbrBitstreamData->CountSendHeaderData == 0) {
      sbrBitstreamData->HeaderActive = 1;                                                      move16();
    }
      
    test();
    if (sbrBitstreamData->NrSendHeaderData == 0) {
      sbrBitstreamData->CountSendHeaderData = 1;                                               move16();
    } 
    else {
      sbrBitstreamData->CountSendHeaderData = add(sbrBitstreamData->CountSendHeaderData, 1);   move16();
      test(); sub(1, 1);
      if (sbrBitstreamData->CountSendHeaderData >= sbrBitstreamData->NrSendHeaderData) {
        sbrBitstreamData->CountSendHeaderData = 0;                                             move16();
      }
    }


    /*
      allocate space for dummy header
    */
    InitSbrBitstream(&hEnvEncoder->CmonData);

    /*
      calculate envelope data and write payload
    */
    
    extractSbrEnvelope(samples,
                       timeInStride,
                       &hEnvEncoder->sbrConfigData,
                       &hEnvEncoder->sbrHeaderData,
                       &hEnvEncoder->sbrBitstreamData,
                       hEnvEncoder->hEnvChannel,
                       hEnvEncoder->hPsEnc,
                       hEnvEncoder->hSynthesisQmfBank,
                       pCoreBuffer,
                       &hEnvEncoder->CmonData);




    /* 
       format payload
    */
    
    AssembleSbrBitstream(&hEnvEncoder->CmonData);
                          
                      
    assert(GetBitsAvail(&hEnvEncoder->CmonData.sbrBitbuf) % 8 == 0);

    /*
      save new payload, set to zero length if greater than MAX_PAYLOAD_SIZE
    */
    hEnvEncoder->sbrPayloadSize = shr(GetBitsAvail(&hEnvEncoder->CmonData.sbrBitbuf), 3);          move16();

    test(); sub(1, 1);
    if(hEnvEncoder->sbrPayloadSize > MAX_PAYLOAD_SIZE) {
      hEnvEncoder->sbrPayloadSize=0;                                                               move16();
    }
 
    test();
    if(ancData){
      *numAncBytes = hEnvEncoder->sbrPayloadSize;                                                  move16();
      memcpy(ancData,hEnvEncoder->CmonData.sbrBitbuf.pBitBufBase, hEnvEncoder->sbrPayloadSize); 
      memop16(hEnvEncoder->sbrPayloadSize/sizeof(Word16));  
    } 
  }  
  return (0);
}

/*****************************************************************************

 functionname: createEnvChannel
 description:  initializes parameters and allocates memory
 returns:      error status
 input:        
 output:       hEnv
 
*****************************************************************************/

static Word32
createEnvChannel (Word16 chan,
                  HANDLE_SBR_CONFIG_DATA sbrConfigData,
                  HANDLE_SBR_HEADER_DATA sbrHeaderData,
                  HANDLE_ENV_CHANNEL     hEnv,
                  sbrConfigurationPtr params
                  )
{

  Word16 e;
  Word16 tran_fc;
  Word16 timeSlots, timeStep, startIndex;
  Word16 noiseBands[2] = { 3, 3 };
  Word32 *tmpPtr;
  



  e = shl(1, params->e);

  hEnv->encEnvData.freq_res_fixfix = FREQ_RES_HIGH;                    move16();

  hEnv->encEnvData.sbr_xpos_mode = (XPOS_MODE)params->sbr_xpos_mode;   move16();
  hEnv->encEnvData.sbr_xpos_ctrl = params->sbr_xpos_ctrl;              move16();
 
  
  test();
  if(createQmfBank (chan,&hEnv->sbrQmf)){
    return (1); /* QMF initialisation failed */
  }

  startIndex = sub(hEnv->sbrQmf.p_filter_length, hEnv->sbrQmf.no_channels);
  
  timeSlots = 16;

  timeStep = shr(hEnv->sbrQmf.no_col, 4);


  test();
  if(CreateTonCorrParamExtr (chan,
                             &hEnv->TonCorr,   
                             timeSlots,       
                             hEnv->sbrQmf.no_col,
                             sbrConfigData->sampleFreq,  
                             hEnv->sbrQmf.no_channels,
                             params->sbr_xpos_ctrl,
                             sbrConfigData->freqBandTable[LOW_RES][0],
                             sbrConfigData->v_k_master,
                             sbrConfigData->num_Master,
                             params->ana_max_level,
                             sbrConfigData->freqBandTable,
                             sbrConfigData->nSfb,
                             sbrHeaderData->sbr_noise_bands,
                             params->noiseFloorOffset,
                             params->useSpeechConfig
                             ))
    return(1);

  hEnv->encEnvData.noOfnoisebands = hEnv->TonCorr.sbrNoiseFloorEstimate.noNoiseBands;     move16();
  
  noiseBands[0] = hEnv->encEnvData.noOfnoisebands;                                        move16();
  noiseBands[1] = hEnv->encEnvData.noOfnoisebands;                                        move16();

  hEnv->encEnvData.sbr_invf_mode = (INVF_MODE)params->sbr_invf_mode;                      move16();

  test(); sub(1, 1);
  if (hEnv->encEnvData.sbr_invf_mode == INVF_SWITCHED) {
    hEnv->encEnvData.sbr_invf_mode = INVF_MID_LEVEL;                                      move16();
    hEnv->TonCorr.switchInverseFilt = TRUE;                                               move16();
  }
  else {
    hEnv->TonCorr.switchInverseFilt = FALSE;                                              move16();
  }

  
  tran_fc  = params->tran_fc;

  test();
  if (tran_fc == 0) {
    tran_fc = S_min (5000, getSbrStartFreqRAW (sbrHeaderData->sbr_start_frequency,64,sbrConfigData->sampleFreq));
  }
  
  tran_fc = extract_l(L_shr(L_add(ffr_Integer_Div(ffr_Integer_Mult(L_deposit_l(shl(tran_fc, 2)),
                      L_deposit_l(hEnv->sbrQmf.no_channels)), sbrConfigData->sampleFreq), 1), 1));

  test();
  if(CreateExtractSbrEnvelope (chan,
                               &hEnv->sbrExtractEnvelope, hEnv->sbrQmf.no_col,
                               hEnv->sbrQmf.no_channels, startIndex,
                               timeSlots, timeStep))
    return(1);
  
  test();
  if(CreateSbrCodeEnvelope (&hEnv->sbrCodeEnvelope,
                            sbrConfigData->nSfb,
                            params->deltaTAcrossFrames))
    return(1);

  test();
  if(CreateSbrCodeEnvelope (&hEnv->sbrCodeNoiseFloor,
                            noiseBands,
                            params->deltaTAcrossFrames))
    return(1);
  
  test();
  if(InitSbrHuffmanTables (&hEnv->encEnvData,
                           &hEnv->sbrCodeEnvelope,
                           &hEnv->sbrCodeNoiseFloor,
                           sbrHeaderData->sbr_amp_res))
    return(1);

  createFrameInfoGenerator (&hEnv->SbrEnvFrame,
                            params->spread,
                            e,
                            params->stat,
                            timeSlots,
                            hEnv->encEnvData.freq_res_fixfix);

  
  test();
  if(CreateSbrTransientDetector (chan,
                                 &hEnv->sbrTransientDetector,
                                 sbrConfigData->sampleFreq,
                                 params->codecSettings.standardBitrate *
                                 params->codecSettings.nChannels,
                                 params->codecSettings.bitRate,
                                 params->tran_thr, 
                                 params->tran_det_mode,
                                 tran_fc, 
                                 hEnv->sbrQmf.no_col,
                                 hEnv->sbrQmf.no_channels
                                 ))
    return(1);

  
  hEnv->encEnvData.noHarmonics = sbrConfigData->nSfb[HI];                    move16();
  hEnv->encEnvData.syntheticCoding = 1;                                      move16(); /* PATCH: can be removed ? */
  hEnv->encEnvData.addHarmonicFlag = 0;                                      move16();

  tmpPtr = (Word32*)&sbr_envRBuffer[chan*QMF_TIME_SLOTS*QMF_CHANNELS];
  hEnv->sfb_nrg = (Word16*)tmpPtr;                                           move16();

  tmpPtr = (Word32*)L_add((Word32)tmpPtr, MAX_NUM_ENVELOPE_VALUES<<1);
  hEnv->sfb_nrg_coupling = (Word16*)tmpPtr;                                  move16();

  tmpPtr = (Word32*)L_add((Word32)tmpPtr, MAX_NUM_ENVELOPE_VALUES<<1);
  hEnv->noiseFloor = (Word32*)tmpPtr;                                        move16();

  tmpPtr = (Word32*)L_add((Word32)tmpPtr, MAX_NUM_NOISE_VALUES<<2);
  hEnv->noise_level = (Word16*)tmpPtr;                                       move16();

  tmpPtr = (Word32*)L_add((Word32)tmpPtr, MAX_NUM_NOISE_VALUES<<1);
  hEnv->noise_level_coupling = (Word16*)tmpPtr;                              move16();

  return 0;
}


/*****************************************************************************

 functionname: EnvOpen
 description:  initializes parameters and allocates memory
 returns:      error status
 input:        
 output:       hEnv
 
*****************************************************************************/
Word32
EnvOpen (HANDLE_SBR_ENCODER * hEnvEncoder, 
         sbrConfigurationPtr params,
         Word16      *coreBandWith)
{
  HANDLE_SBR_ENCODER hEnvEnc ;
  Word16 ch;

  /* initialize the encoder handle  and structs*/
  
  *hEnvEncoder=0;
  hEnvEnc = &sbrEncoder;
  
  hEnvEnc->hEnvChannel[0] = &hEnvEnc->EnvChannel[0];         move16();
#if (MAX_CHANNELS>1)
  hEnvEnc->hEnvChannel[1] = &hEnvEnc->EnvChannel[1];         move16();
#endif

  test(); test(); sub(1, 1); sub(1, 1);
  if ((params->codecSettings.nChannels < 1) || (params->codecSettings.nChannels > MAX_CHANNELS)){
    EnvClose (hEnvEnc);
    return(1);
  }
  
  
  hEnvEnc->sbrConfigData.freqBandTable[LO] = sbr_freqBandTableLO;                          move16();
  memset(hEnvEnc->sbrConfigData.freqBandTable[LO],0,sizeof(UWord16)*MAX_FREQ_COEFFS/2+1);  memop16(MAX_FREQ_COEFFS/2+1);

  hEnvEnc->sbrConfigData.freqBandTable[HI] = sbr_freqBandTableHI;                          move16();
  memset(hEnvEnc->sbrConfigData.freqBandTable[HI],0,sizeof(UWord16)*MAX_FREQ_COEFFS+1);    memop16(MAX_FREQ_COEFFS+1);

  hEnvEnc->sbrConfigData.v_k_master = sbr_v_k_master;                                      move16();
  memset(hEnvEnc->sbrConfigData.v_k_master,0,sizeof(UWord16)*MAX_FREQ_COEFFS+1);           memop16(MAX_FREQ_COEFFS+1);

  test();
  if (hEnvEnc->CmonData.sbrBitbuf.isValid == 0) {
    CreateBitBuffer(&hEnvEnc->CmonData.sbrBitbuf,
                    hEnvEnc->sbrPayload,
                    sizeof(hEnvEnc->sbrPayload));
  }
  test();
  if (hEnvEnc->CmonData.sbrBitbufPrev.isValid == 0) {
    CreateBitBuffer(&hEnvEnc->CmonData.sbrBitbufPrev,
                    hEnvEnc->sbrPayloadPrev,
                    sizeof(hEnvEnc->sbrPayloadPrev));
  }

  /*
    now initialize sbrConfigData, sbrHeaderData and sbrBitstreamData, 
  */

  hEnvEnc->sbrConfigData.nChannels = params->codecSettings.nChannels;                      move16();

  test(); sub(1, 1);
  if(params->codecSettings.nChannels == 2) {
    hEnvEnc->sbrConfigData.stereoMode = params->stereoMode;                                move16(); 
  }
  else {
    hEnvEnc->sbrConfigData.stereoMode = SBR_MONO;                                          move16();
  }
  


  /* implicit rule for sampleRateMode */
  test(); L_sub(1, 1);
  if (params->codecSettings.sampleFreq <= 24000) {
    /* run in "multirate" mode where sbr fs is 2 * codec fs */
    hEnvEnc->sbrHeaderData.sampleRateMode = DUAL_RATE;                                     move16();
    hEnvEnc->sbrConfigData.sampleFreq = L_shl(params->codecSettings.sampleFreq, 1);        move32();
  }
  else {
    /* run in "single rate" mode where sbr fs is codec fs */
    hEnvEnc->sbrHeaderData.sampleRateMode = SINGLE_RATE;                                   move16();
    hEnvEnc->sbrConfigData.sampleFreq = params->codecSettings.sampleFreq;                  move32();
  }
  
  hEnvEnc->sbrBitstreamData.CountSendHeaderData = 0;                                       move16();
  test();
  if (params->SendHeaderDataTime > 0 ) {
    hEnvEnc->sbrBitstreamData.NrSendHeaderData = extract_l(ffr_Integer_Div(
         ffr_Integer_Mult(params->SendHeaderDataTime, hEnvEnc->sbrConfigData.sampleFreq), 
         (1000 * 2048)));                       move16();
    hEnvEnc->sbrBitstreamData.NrSendHeaderData = S_max(hEnvEnc->sbrBitstreamData.NrSendHeaderData,1);
  }
  else {
    hEnvEnc->sbrBitstreamData.NrSendHeaderData = 0;                                        move16();
  }
  
  hEnvEnc->sbrHeaderData.sbr_data_extra = params->sbr_data_extra;                          move16();
  hEnvEnc->sbrBitstreamData.HeaderActive = 0;                                              move16();
  hEnvEnc->sbrHeaderData.sbr_start_frequency = params->startFreq;                          move16();
  hEnvEnc->sbrHeaderData.sbr_stop_frequency  = params->stopFreq;                           move16();
  hEnvEnc->sbrHeaderData.sbr_xover_band = 0;                                               move16();

  /* data_extra */
  test(); sub(1, 1);
  if (params->sbr_xpos_ctrl!= SBR_XPOS_CTRL_DEFAULT) {
    hEnvEnc->sbrHeaderData.sbr_data_extra = 1;                                             move16();
  }

  hEnvEnc->sbrHeaderData.protocol_version = SI_SBR_PROTOCOL_VERSION_ID;                    move16();

  hEnvEnc->sbrHeaderData.sbr_amp_res = (AMP_RES)params->amp_res;                           move16();


  /* header_extra_1 */
  hEnvEnc->sbrHeaderData.freqScale  = params->freqScale;                                   move16();
  hEnvEnc->sbrHeaderData.alterScale = params->alterScale;                                  move16();
  hEnvEnc->sbrHeaderData.sbr_noise_bands = params->sbr_noise_bands;                        move16();
  hEnvEnc->sbrHeaderData.header_extra_1 = 0;                                               move16();
  test(); test(); test();
  if (sub(params->freqScale, SBR_FREQ_SCALE_DEFAULT) != 0 ||
      sub(params->alterScale, SBR_ALTER_SCALE_DEFAULT) != 0 ||
      sub(params->sbr_noise_bands, SBR_NOISE_BANDS_DEFAULT) != 0) {
    hEnvEnc->sbrHeaderData.header_extra_1 = 1;                                             move16();
  }
  
  /* header_extra_2 */
  hEnvEnc->sbrHeaderData.sbr_limiter_bands = params->sbr_limiter_bands;                    move16();
  hEnvEnc->sbrHeaderData.sbr_limiter_gains = params->sbr_limiter_gains;                    move16();
  hEnvEnc->sbrHeaderData.sbr_interpol_freq = params->sbr_interpol_freq;                    move16();
  hEnvEnc->sbrHeaderData.sbr_smoothing_length = params->sbr_smoothing_length;              move16();
  hEnvEnc->sbrHeaderData.header_extra_2 = 0;                                               move16();
  test(); test(); test(); test();
  if (sub(params->sbr_limiter_bands, SBR_LIMITER_BANDS_DEFAULT) != 0 ||
      sub(params->sbr_limiter_gains, SBR_LIMITER_GAINS_DEFAULT) != 0 ||
      sub(params->sbr_interpol_freq, SBR_INTERPOL_FREQ_DEFAULT) != 0 ||
      sub(params->sbr_smoothing_length, SBR_SMOOTHING_LENGTH_DEFAULT) != 0) {
    hEnvEnc->sbrHeaderData.header_extra_2 = 1;                                             move16();
  }


  /* other switches */

  hEnvEnc->sbrConfigData.useParametricCoding       = params->parametricCoding;             move16(); /* PATCH: can be removed ? */

  
  /* init freq band table */
  test();
  if(updateFreqBandTable(&hEnvEnc->sbrConfigData,
                         &hEnvEnc->sbrHeaderData,
                         QMF_CHANNELS)){
    EnvClose (hEnvEnc);
    return(1);
  }
  
  /* now create envelope ext for each available channel */
  for ( ch = 0; ch < hEnvEnc->sbrConfigData.nChannels; ch++ ) {
    test();
    if(createEnvChannel(ch,
                        &hEnvEnc->sbrConfigData,
                        &hEnvEnc->sbrHeaderData,
                        hEnvEnc->hEnvChannel[ch],
                        params
                        )){
      EnvClose (hEnvEnc);
      return(1);
    }
  }

  hEnvEnc->hPsEnc = NULL;                                                                  move16();
  hEnvEnc->hSynthesisQmfBank = NULL;

#ifndef MONO_ONLY
  test();
  if (params->usePs) {

    test();
    if(createQmfBank (1, &hEnvEnc->hEnvChannel[1]->sbrQmf)){
      return 1;
    }

    if(CreateExtractSbrEnvelope (1,
                                 &hEnvEnc->hEnvChannel[1]->sbrExtractEnvelope,
                                 hEnvEnc->hEnvChannel[1]->sbrQmf.no_col,
                                 hEnvEnc->hEnvChannel[1]->sbrQmf.no_channels,
                                 576,16,2
                                 )) {
      return 1;
    }

    hEnvEnc->hSynthesisQmfBank = &hEnvEnc->SynthesisQmfBank;                               move16();

    test();
    if ( CreateSynthesisQmfBank(hEnvEnc->hSynthesisQmfBank, 32) ) {
      return 1;
    }

    hEnvEnc->hPsEnc = &hEnvEnc->PsEnc;                                                     move16();

    test();
    if ( CreatePsEnc(hEnvEnc->hPsEnc, params->psMode) ) {
      return 1;
    }
  }
#endif /* #ifndef MONO_ONLY */


  hEnvEnc->CmonData.sbrNumChannels  = hEnvEnc->sbrConfigData.nChannels;                    move16();

  hEnvEnc->sbrPayloadSize = 0;                                                             move16();

  *hEnvEncoder = hEnvEnc;                                                                  move16();
  *coreBandWith = hEnvEnc->sbrConfigData.xOverFreq;                                        move16();

  return 0;
}
 
