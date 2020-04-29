/*
   aac coder functions
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "aacenc.h"
#include "bitenc.h"

#include "psy_configuration.h"
#include "psy_main.h"
#include "qc_main.h"
#include "psy_main.h"
#include "channel_map.h"
#include "stprepro.h"
#include "count.h"

#define GetBitsAvail(hB) ((hB)->cntBits)


struct AAC_ENCODER {

  AACENC_CONFIG config;     /* Word16 size: 8 */

  ELEMENT_INFO elInfo;      /* Word16 size: 4 */

  QC_STATE qcKernel;        /* Word16 size: 6 + 5(PADDING) + 7(ELEMENT_BITS) + 54(ADJ_THR_STATE) = 72 */
  QC_OUT   qcOut;           /* Word16 size: MAX_CHANNELS*920(QC_OUT_CHANNEL) + 5(QC_OUT_ELEMENT) + 7 = 932 / 1852 */

  PSY_OUT    psyOut;        /* Word16 size: MAX_CHANNELS*186 + 2 = 188 / 374 */
  PSY_KERNEL psyKernel;     /* Word16 size:  2587 / 4491 */

  struct BITSTREAMENCODER_INIT bseInit; /* Word16 size: 6 */
#ifndef MONO_ONLY
  struct STEREO_PREPRO stereoPrePro;    /* Word16 size: 32 */
#endif
  struct BIT_BUF  bitStream;            /* Word16 size: 8 */
  HANDLE_BIT_BUF  hBitStream;

}; /* Word16 size: 3809 / 6851 */


static struct AAC_ENCODER aacEncoder;



/*-----------------------------------------------------------------------------

functionname: AacInitDefaultConfig
description:  gives reasonable default configuration
returns:      ---

------------------------------------------------------------------------------*/
void AacInitDefaultConfig(AACENC_CONFIG *config)
{
  /* make the pre initialization of the structs flexible */
  memset(config, 0, sizeof(AACENC_CONFIG));

  /* default configurations */
  config->bitRate         = 128000;                     move16();
  config->bandWidth       = 0;                          move16();
}

/*---------------------------------------------------------------------------

functionname: AacEncOpen
description:  allocate and initialize a new encoder instance
returns:      0 if success

---------------------------------------------------------------------------*/

Word16  AacEncOpen(  struct AAC_ENCODER**     phAacEnc,        /* pointer to an encoder handle, initialized on return */
                     const  AACENC_CONFIG     config           /* pre-initialized config struct */
                     )
{
  Word16 error = 0;
  Word16 profile = 1;
  ELEMENT_INFO *elInfo = NULL;
  struct AAC_ENCODER *hAacEnc ;
  move16(); move16();
    
  hAacEnc = &aacEncoder;

  test();
  if (phAacEnc==0) {
    error=1;                                    move16();
  }

  test();
  if (!error) {
    /* sanity checks on config structure */
    test(); test();
    test(); test();
    test(); test();
    test(); test(); test();
    move16();
    error = (&config == 0            || phAacEnc == 0             ||
             sub(config.nChannelsIn, 1) < 0  || sub(config.nChannelsIn, MAX_CHANNELS) > 0  ||
             sub(config.nChannelsOut, 1) < 0 || sub(config.nChannelsOut, MAX_CHANNELS) > 0 ||
             (config.bitRate!=0 && (L_sub(ffr_divideWord32(config.bitRate, config.nChannelsOut), 8000) < 0      ||
                                    L_sub(ffr_divideWord32(config.bitRate, config.nChannelsOut), 160000) > 0)));
  }

  /* check sample rate */
  test();
  if (!error) {
    test();
    switch (config.sampleRate) {
      case 16000: case 22050: case 24000:
        break;
      default:
        error = 1;                              move16();
        break;
    }
  }

  /* check if bit rate is not too high for sample rate */
  test();
  if (!error) {
    if (L_sub(config.bitRate, (ffr_Integer_Mult(config.sampleRate, ffr_Integer_Mult16x16(6,config.nChannelsOut)))) > 0) {
      error=1;                                          move16();
    }
  }

    
  test();
  if (!error) {
    hAacEnc->config = config;
  }

  test();
  if (!error) {
    error = InitElementInfo (config.nChannelsOut,
                             &hAacEnc->elInfo);
  }
  test();
  if (!error) {
    elInfo = &hAacEnc->elInfo;
  }


  /* allocate the Psy aud Psy Out structure */
  test();
  if (!error) {
    error = (PsyNew(&hAacEnc->psyKernel, elInfo->nChannelsInEl) ||
             PsyOutNew(&hAacEnc->psyOut));
  }
  test();
  if (!error) {
    Word16 tnsMask=3;                                                         move16();

    error = psyMainInit(&hAacEnc->psyKernel,
                        config.sampleRate,
                        config.bitRate,
                        elInfo->nChannelsInEl,
                        tnsMask,
                        hAacEnc->config.bandWidth);
  }


  /* allocate the Q&C Out structure */
  test();
  if (!error) {
    error = QCOutNew(&hAacEnc->qcOut,
                     elInfo->nChannelsInEl);
  }

  /* allocate the Q&C kernel */
  test();
  if (!error) {
    error = QCNew(&hAacEnc->qcKernel);
  }
  test();
  if (!error) {
    struct QC_INIT qcInit;

    /*qcInit.channelMapping = &hAacEnc->channelMapping;*/
    qcInit.elInfo = &hAacEnc->elInfo;

    qcInit.maxBits = (Word16) ffr_Integer_Mult16x16(6144, elInfo->nChannelsInEl);
    qcInit.bitRes = qcInit.maxBits;                                             move16();
    qcInit.averageBits = (Word16) ffr_divideWord32(ffr_Integer_Mult(config.bitRate, FRAME_LEN_LONG), config.sampleRate);

    qcInit.padding.paddingRest = config.sampleRate;                             move32();

    qcInit.meanPe = (Word16) ffr_divideWord32(ffr_Integer_Mult16x16(10 * FRAME_LEN_LONG, hAacEnc->config.bandWidth),
                                              L_shr(config.sampleRate,1));
    test();
    qcInit.maxBitFac = (Word16) ffr_divideWord32(ffr_Integer_Mult(100, (ffr_Integer_Mult16x16(6144-744, elInfo->nChannelsInEl))),
                                                 qcInit.averageBits?qcInit.averageBits:1);

    qcInit.bitrate = config.bitRate;                                            move16();

    error = QCInit(&hAacEnc->qcKernel, &qcInit);
  }

  /* init bitstream encoder */
  test();
  if (!error) {
    hAacEnc->bseInit.nChannels   = elInfo->nChannelsInEl;                         move16();
    hAacEnc->bseInit.bitrate     = config.bitRate;                                move32();
    hAacEnc->bseInit.sampleRate  = config.sampleRate;                             move32();
    hAacEnc->bseInit.profile     = profile;                                       move16();
  }

#ifndef MONO_ONLY
  test();
  if (!error) {
    test(); test(); test();
    if (sub(elInfo->elType, ID_CPE) == 0 &&
        (L_sub(config.sampleRate, 24000) <= 0 &&
         L_sub(L_shl(ffr_divideWord32(config.bitRate,elInfo->nChannelsInEl),1), 60000) < 0)) {
      Word16 scfUsedRatio = (Word16) ffr_divideWord32(ffr_Integer_Mult16x16(hAacEnc->psyKernel.psyConfLong.sfbActive, 100),
                                                      hAacEnc->psyKernel.psyConfLong.sfbCnt);

      error = InitStereoPreProcessing(&(hAacEnc->stereoPrePro),
                                      elInfo->nChannelsInEl, 
                                      config.bitRate, 
                                      config.sampleRate,
                                      scfUsedRatio);
    }
  }
#endif

  test();
  if (error) {
    AacEncClose(hAacEnc);
    hAacEnc=0;
  }

  *phAacEnc = hAacEnc;

  return error;
}


Word16 AacEncEncode(struct AAC_ENCODER *aacEnc, /*!< an encoder handle */
                    Word16 *timeSignal,         /*!< BLOCKSIZE*nChannels audio samples, interleaved */
                    const UWord8 *ancBytes,     /*!< pointer to ancillary data bytes */
                    Word16 *numAncBytes,       /*!< number of ancillary Data Bytes */
                    UWord8 *outBytes,           /*!< pointer to output buffer (must be 6144/8*MAX_CHANNELS bytes large) */
                    Word32 *numOutBytes         /*!< number of bytes in output buffer after processing */
                    )
{
  ELEMENT_INFO *elInfo = &aacEnc->elInfo;
  Word16 globUsedBits;
  Word16 ancDataBytes, ancDataBytesLeft;
  
  ancDataBytes = ancDataBytesLeft = *numAncBytes;                       move16(); move16();

  aacEnc->hBitStream = CreateBitBuffer(&aacEnc->bitStream, outBytes, 6144*MAX_CHANNELS/8);

#ifndef MONO_ONLY
  /* advance psychoacoustic */
  test();
  if (sub(elInfo->elType, ID_CPE) == 0) {
      ApplyStereoPreProcess(&aacEnc->stereoPrePro,
                            MAX_CHANNELS,
                            elInfo,
                            timeSignal,
                            FRAME_LEN_LONG);
  }
#endif

  psyMain(MAX_CHANNELS,    
          elInfo,
          timeSignal,
          &aacEnc->psyKernel.psyData[elInfo->ChannelIndex[0]],
          &aacEnc->psyKernel.tnsData[elInfo->ChannelIndex[0]],
          &aacEnc->psyKernel.psyConfLong,
          &aacEnc->psyKernel.psyConfShort,
          &aacEnc->psyOut.psyOutChannel[elInfo->ChannelIndex[0]],
          &aacEnc->psyOut.psyOutElement,
          aacEnc->psyKernel.pScratchTns);

  AdjustBitrate(&aacEnc->qcKernel,
                aacEnc->config.bitRate,
                aacEnc->config.sampleRate);
    
  QCMain(&aacEnc->qcKernel,
         elInfo->nChannelsInEl,
         &aacEnc->qcKernel.elementBits,
         &aacEnc->qcKernel.adjThr.adjThrStateElem,
         &aacEnc->psyOut.psyOutChannel[elInfo->ChannelIndex[0]],
         &aacEnc->psyOut.psyOutElement,
         &aacEnc->qcOut.qcChannel[elInfo->ChannelIndex[0]],
         &aacEnc->qcOut.qcElement,
         S_min(ancDataBytesLeft,ancDataBytes));

#ifndef MONO_ONLY
  UpdateStereoPreProcess(&aacEnc->psyOut.psyOutChannel[elInfo->ChannelIndex[0]],
                         &aacEnc->qcOut.qcElement,
                         &aacEnc->stereoPrePro,
                         aacEnc->psyOut.psyOutElement.weightMsLrPeRatio);
#endif

  ancDataBytesLeft = sub(ancDataBytesLeft, ancDataBytes);

  FinalizeBitConsumption(&aacEnc->qcKernel,
                         &aacEnc->qcOut);

  WriteBitstream(aacEnc->hBitStream,
                 *elInfo,
                 &aacEnc->qcOut,
                 &aacEnc->psyOut,
                 &globUsedBits,
                 ancBytes);

  updateBitres(&aacEnc->qcKernel,
               &aacEnc->qcOut);

  /* write out the bitstream */
  *numOutBytes = shr(GetBitsAvail(aacEnc->hBitStream), 3);

  /* assert this frame is not too large */
  assert(*numOutBytes*8 <= 6144*elInfo->nChannelsInEl);

  return 0;
}


/*---------------------------------------------------------------------------

functionname:AacEncClose
description: deallocate an encoder instance

---------------------------------------------------------------------------*/

void AacEncClose (struct AAC_ENCODER* hAacEnc)
{
  Word16 error=0;                       move16();

  test();
  if (hAacEnc) {  
    QCDelete(&hAacEnc->qcKernel);

    QCOutDelete(&hAacEnc->qcOut);

    error = PsyDelete(&hAacEnc->psyKernel);

    error = PsyOutDelete(&hAacEnc->psyOut);

    DeleteBitBuffer(&hAacEnc->hBitStream);

    hAacEnc=0;
  }
}
