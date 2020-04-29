/*
   Psychoacoustic major function block
 */
#include <stdlib.h>
#include <string.h>

#include "psy_const.h"
#include "block_switch.h"
#include "transform.h"
#include "spreading.h"
#include "pre_echo_control.h"
#include "band_nrg.h"
#include "psy_configuration.h"
#include "psy_data.h"
#include "ms_stereo.h"
#include "interface.h"
#include "psy_main.h"
#include "grp_data.h"
#include "tns_func.h"
#include "aac_ram.h"
#include "count.h"

/*                                    long       start       short       stop */
static Word16 blockType2windowShape[] = {KBD_WINDOW,SINE_WINDOW,SINE_WINDOW,KBD_WINDOW};

/*
  forward definitions
*/
static Word16 advancePsychLong(PSY_DATA* psyData,
                               TNS_DATA* tnsData,
                               PSY_CONFIGURATION_LONG *hPsyConfLong,
                               PSY_OUT_CHANNEL* psyOutChannel,
                               Word32 *pScratchTns,
                               const TNS_DATA *tnsData2,
                               const Word16 ch);

static Word16 advancePsychLongMS (PSY_DATA  psyData[MAX_CHANNELS],
                                  const PSY_CONFIGURATION_LONG *hPsyConfLong);

static Word16 advancePsychShort(PSY_DATA* psyData,
                                TNS_DATA* tnsData,
                                const PSY_CONFIGURATION_SHORT *hPsyConfShort,
                                PSY_OUT_CHANNEL* psyOutChannel,
                                Word32 *pScratchTns,
                                const TNS_DATA *tnsData2,
                                const Word16 ch);

static Word16 advancePsychShortMS (PSY_DATA  psyData[MAX_CHANNELS],
                                   const PSY_CONFIGURATION_SHORT *hPsyConfShort);


/*****************************************************************************

    functionname: PsyNew
    description:  allocates memory for psychoacoustic
    returns:      an error code
    input:        pointer to a psych handle

*****************************************************************************/
Word16 PsyNew(PSY_KERNEL *hPsy, Word32 nChan)
{
  Word16 i;
   
  for (i=0; i<nChan; i++){
    /*
      reserve memory for mdct delay buffer
    */
    hPsy->psyData[i].mdctDelayBuffer = &mdctDelayBuffer[i*BLOCK_SWITCHING_OFFSET];
      
    /*
      reserve memory for mdct spectum
    */
      
    hPsy->psyData[i].mdctSpectrum = &mdctSpectrum[i*FRAME_LEN_LONG];
  }

  hPsy->pScratchTns = scratchTNS;

  return 0;
}


/*****************************************************************************

    functionname: PsyDelete
    description:  allocates memory for psychoacoustic
    returns:      an error code

*****************************************************************************/
Word16 PsyDelete(PSY_KERNEL  *hPsy)
{
  /*
    nothing to do
  */
  hPsy=NULL;
  return 0;
}


/*****************************************************************************

    functionname: PsyOutNew
    description:  allocates memory for psyOut struc
    returns:      an error code
    input:        pointer to a psych handle

*****************************************************************************/
Word16 PsyOutNew(PSY_OUT *hPsyOut)
{
  memset(hPsyOut,0,sizeof(PSY_OUT));
  /*
    alloc some more stuff, tbd
  */
  return 0;
}

/*****************************************************************************

    functionname: PsyOutDelete
    description:  allocates memory for psychoacoustic
    returns:      an error code

*****************************************************************************/
Word16 PsyOutDelete(PSY_OUT *hPsyOut)
{
  hPsyOut=NULL;
  return 0;
}


/*****************************************************************************

    functionname: psyMainInit
    description:  initializes psychoacoustic
    returns:      an error code

*****************************************************************************/

Word16 psyMainInit(PSY_KERNEL *hPsy,
                   Word32 sampleRate,
                   Word32 bitRate,
                   Word16 channels,
                   Word16 tnsMask,
                   Word16 bandwidth)
{
  Word16 ch, err;
  Word32 channelBitRate = ffr_divideWord32(bitRate, channels);

  err = InitPsyConfigurationLong(channelBitRate,
                                 sampleRate,
                                 bandwidth,
                                 &(hPsy->psyConfLong));

  test();
  if (!err) {
    err = InitTnsConfigurationLong(bitRate, sampleRate, channels,
                                   &hPsy->psyConfLong.tnsConf, &hPsy->psyConfLong, tnsMask&2);
    logic16();
  }

  test();
  if (!err)
    err = InitPsyConfigurationShort(channelBitRate,
                                    sampleRate,
                                    bandwidth,
                                    &hPsy->psyConfShort);
  test();
  if (!err) {
    err = InitTnsConfigurationShort(bitRate, sampleRate, channels,
                                    &hPsy->psyConfShort.tnsConf, &hPsy->psyConfShort, tnsMask&1);
    logic16();
  }

  test();
  if (!err)
    for(ch=0;ch < channels;ch++){
  
      InitBlockSwitching(&hPsy->psyData[ch].blockSwitchingControl,
                         bitRate, channels);

      InitPreEchoControl(hPsy->psyData[ch].sfbThresholdnm1,
                         hPsy->psyConfLong.sfbCnt,
                         hPsy->psyConfLong.sfbThresholdQuiet);
      hPsy->psyData[ch].mdctScalenm1 = 0;                               move16();
    }

  return(err);
}







/*****************************************************************************

    functionname: psyMain
    description:  psychoacoustic
    returns:      an error code

        This function assumes that enough input data is in the modulo buffer.

*****************************************************************************/

Word16 psyMain(Word16                   nChannels,
               ELEMENT_INFO            *elemInfo,
               Word16                  *timeSignal, 
               PSY_DATA                 psyData[MAX_CHANNELS],
               TNS_DATA                 tnsData[MAX_CHANNELS],
               PSY_CONFIGURATION_LONG  *hPsyConfLong,
               PSY_CONFIGURATION_SHORT *hPsyConfShort,
               PSY_OUT_CHANNEL          psyOutChannel[MAX_CHANNELS],
               PSY_OUT_ELEMENT         *psyOutElement,
               Word32                  *pScratchTns)
{
  Word16 maxSfbPerGroup[MAX_CHANNELS];
  Word16 groupedSfbOffset[MAX_CHANNELS][MAX_GROUPED_SFB+1];  /* plus one for last dummy offset ! */
  Word16 groupedSfbMinSnr[MAX_CHANNELS][MAX_GROUPED_SFB];
  Word16 mdctScalingArray[MAX_CHANNELS];

  Word16 ch;   /* counts through channels          */
  Word16 sfb;  /* counts through scalefactor bands */
  Word16 line; /* counts through lines             */
  Word16 channels;
  Word16 maxScale;

  channels = elemInfo->nChannelsInEl;                           move16();
  maxScale = 0;                                                 move16();

  /* block switching */
  for(ch = 0; ch < channels; ch++) {
    BlockSwitching(&psyData[ch].blockSwitchingControl,
                   timeSignal+elemInfo->ChannelIndex[ch],
                   nChannels);
  }

  /* synch left and right block type */
  SyncBlockSwitching(&psyData[0].blockSwitchingControl,
                     &psyData[1].blockSwitchingControl,
                     channels);

  /* transform
     and get maxScale (max mdctScaling) for all channels */
  for(ch=0; ch<channels; ch++) {
    Transform_Real(psyData[ch].mdctDelayBuffer,
                   timeSignal+elemInfo->ChannelIndex[ch],
                   nChannels,
                   psyData[ch].mdctSpectrum,
                   &(mdctScalingArray[ch]),
                   psyData[ch].blockSwitchingControl.windowSequence);
    maxScale = S_max(maxScale, mdctScalingArray[ch]);
  }

  /* common scaling for all channels */
  for (ch=0; ch<channels; ch++) {
    Word16 scaleDiff = sub(maxScale, mdctScalingArray[ch]);
    test();
    if (scaleDiff > 0) {
      for(line=0; line<FRAME_LEN_LONG; line++) {
        psyData[ch].mdctSpectrum[line] = L_shr(psyData[ch].mdctSpectrum[line], scaleDiff);
      }
    }
    psyData[ch].mdctScale = maxScale;                                   move16();
  }

  for (ch=0; ch<channels; ch++) {
    test();
    if(sub(psyData[ch].blockSwitchingControl.windowSequence, SHORT_WINDOW) != 0) {
      advancePsychLong(&psyData[ch],
                       &tnsData[ch],
                       hPsyConfLong,
                       &psyOutChannel[ch],
                       pScratchTns,
                       &tnsData[sub(1,ch)],
                       ch);

      /* determine maxSfb */
      for (sfb=hPsyConfLong->sfbCnt-1; sfb>=0; sfb--) {
        for (line=sub(hPsyConfLong->sfbOffset[sfb+1],1); line>=hPsyConfLong->sfbOffset[sfb]; line--) {
          test();
          if (psyData[ch].mdctSpectrum[line] != 0) break;
        }
        if (sub(line, hPsyConfLong->sfbOffset[sfb]) >= 0) break;
      }
      maxSfbPerGroup[ch] = add(sfb, 1);

      /* Calc bandwise energies for mid and side channel
         Do it only if 2 channels exist */
      test();
      if (sub(ch,1) == 0)
        advancePsychLongMS(psyData, hPsyConfLong);
    }
    else {
      advancePsychShort(&psyData[ch],
                        &tnsData[ch],
                        hPsyConfShort,
                        &psyOutChannel[ch],
                        pScratchTns,
                        &tnsData[sub(1,ch)],
                        ch);

      /* Calc bandwise energies for mid and side channel
         Do it only if 2 channels exist */
      test();
      if (sub(ch,1) == 0)
        advancePsychShortMS (psyData, hPsyConfShort);
    }
  }

  /* group short data */
  for(ch=0; ch<channels; ch++) {
    test();
    if (sub(psyData[ch].blockSwitchingControl.windowSequence, SHORT_WINDOW) == 0) {
      groupShortData(psyData[ch].mdctSpectrum,
                     pScratchTns,
                     &psyData[ch].sfbThreshold,
                     &psyData[ch].sfbEnergy,
                     &psyData[ch].sfbEnergyMS,
                     &psyData[ch].sfbSpreadedEnergy,
                     hPsyConfShort->sfbCnt,
                     hPsyConfShort->sfbOffset,
                     hPsyConfShort->sfbMinSnr,
                     groupedSfbOffset[ch],
                     &maxSfbPerGroup[ch],
                     groupedSfbMinSnr[ch],
                     psyData[ch].blockSwitchingControl.noOfGroups,
                     psyData[ch].blockSwitchingControl.groupLen);
    }
  }


#if (MAX_CHANNELS>1)
  /*
    stereo Processing
  */
  if (sub(channels, 2) == 0) {
    psyOutElement->toolsInfo.msDigest = MS_NONE;                move16();
    maxSfbPerGroup[0] = maxSfbPerGroup[1] = S_max(maxSfbPerGroup[0], maxSfbPerGroup[1]);

    test();
    if (sub(psyData[0].blockSwitchingControl.windowSequence, SHORT_WINDOW) != 0)
      MsStereoProcessing(psyData[0].sfbEnergy.sfbLong,
                         psyData[1].sfbEnergy.sfbLong,
                         psyData[0].sfbEnergyMS.sfbLong,
                         psyData[1].sfbEnergyMS.sfbLong,
                         psyData[0].mdctSpectrum,
                         psyData[1].mdctSpectrum,
                         psyData[0].sfbThreshold.sfbLong,
                         psyData[1].sfbThreshold.sfbLong,
                         psyData[0].sfbSpreadedEnergy.sfbLong,
                         psyData[1].sfbSpreadedEnergy.sfbLong,
                         (Word16*)&psyOutElement->toolsInfo.msDigest,
                         (Word16*)psyOutElement->toolsInfo.msMask,
                         hPsyConfLong->sfbCnt,
                         hPsyConfLong->sfbCnt,
                         maxSfbPerGroup[0],
                         (const Word16*)hPsyConfLong->sfbOffset,
                         &psyOutElement->weightMsLrPeRatio);
      else
        MsStereoProcessing(psyData[0].sfbEnergy.sfbLong,
                           psyData[1].sfbEnergy.sfbLong,
                           psyData[0].sfbEnergyMS.sfbLong,
                           psyData[1].sfbEnergyMS.sfbLong,
                           psyData[0].mdctSpectrum,
                           psyData[1].mdctSpectrum,
                           psyData[0].sfbThreshold.sfbLong,
                           psyData[1].sfbThreshold.sfbLong,
                           psyData[0].sfbSpreadedEnergy.sfbLong,
                           psyData[1].sfbSpreadedEnergy.sfbLong,
                           (Word16*)&psyOutElement->toolsInfo.msDigest,
                           (Word16*)psyOutElement->toolsInfo.msMask,
                           psyData[0].blockSwitchingControl.noOfGroups*hPsyConfShort->sfbCnt,
                           hPsyConfShort->sfbCnt,
                           maxSfbPerGroup[0],
                           (const Word16*)groupedSfbOffset[0],
                           &psyOutElement->weightMsLrPeRatio);
  }

#endif /* (MAX_CHANNELS>1) */

  /*
    build output
  */
  for(ch=0;ch<channels;ch++) {
    test();
    if (sub(psyData[ch].blockSwitchingControl.windowSequence, SHORT_WINDOW) != 0)
      BuildInterface(psyData[ch].mdctSpectrum,
                     psyData[ch].mdctScale,
                     &psyData[ch].sfbThreshold,
                     &psyData[ch].sfbEnergy,
                     &psyData[ch].sfbSpreadedEnergy,
                     psyData[ch].sfbEnergySum,
                     psyData[ch].sfbEnergySumMS,
                     psyData[ch].blockSwitchingControl.windowSequence,
                     blockType2windowShape[psyData[ch].blockSwitchingControl.windowSequence],
                     hPsyConfLong->sfbCnt,
                     hPsyConfLong->sfbOffset,
                     maxSfbPerGroup[ch],
                     hPsyConfLong->sfbMinSnr,
                     psyData[ch].blockSwitchingControl.noOfGroups,
                     psyData[ch].blockSwitchingControl.groupLen,
                     &psyOutChannel[ch]);
    else
      BuildInterface(psyData[ch].mdctSpectrum,
                     psyData[ch].mdctScale,
                     &psyData[ch].sfbThreshold,
                     &psyData[ch].sfbEnergy,
                     &psyData[ch].sfbSpreadedEnergy,
                     psyData[ch].sfbEnergySum,
                     psyData[ch].sfbEnergySumMS,
                     SHORT_WINDOW,
                     SINE_WINDOW,
                     psyData[0].blockSwitchingControl.noOfGroups*hPsyConfShort->sfbCnt,
                     groupedSfbOffset[ch],
                     maxSfbPerGroup[ch],
                     groupedSfbMinSnr[ch],
                     psyData[ch].blockSwitchingControl.noOfGroups,
                     psyData[ch].blockSwitchingControl.groupLen,
                     &psyOutChannel[ch]);
  }

  return(0); /* no error */
}






/*****************************************************************************

    functionname: advancePsychLong
    description:  psychoacoustic for long blocks

*****************************************************************************/

static Word16 advancePsychLong(PSY_DATA* psyData,
                               TNS_DATA* tnsData,
                               PSY_CONFIGURATION_LONG *hPsyConfLong,
                               PSY_OUT_CHANNEL* psyOutChannel,
                               Word32 *pScratchTns,
                               const TNS_DATA* tnsData2,
                               const Word16 ch)
{
  Word16 i;
  Word16 normEnergyShift = shl(add(psyData->mdctScale,1), 1); /* in reference code, mdct spectrum must be multipied with 2, so +1 */
  Word32 clipEnergy = L_shr(hPsyConfLong->clipEnergy, normEnergyShift);

  /* low pass */
  for(i=hPsyConfLong->lowpassLine; i<FRAME_LEN_LONG; i++) {
    psyData->mdctSpectrum[i] = 0;                               move32();
  }

  /* Calc sfb-bandwise mdct-energies for left and right channel */
  CalcBandEnergy( psyData->mdctSpectrum,
                  hPsyConfLong->sfbOffset,
                  hPsyConfLong->sfbActive,
                  psyData->sfbEnergy.sfbLong,
                  &psyData->sfbEnergySum.sfbLong);

  /*
    TNS
  */
  TnsDetect(tnsData,
            hPsyConfLong->tnsConf,
            pScratchTns,
            (const Word16*)hPsyConfLong->sfbOffset,
            psyData->mdctSpectrum,
            0,
            psyData->blockSwitchingControl.windowSequence,
            psyData->sfbEnergy.sfbLong);

  /*  TnsSync */
  test();
  if (sub(ch,1) == 0) {
    TnsSync(tnsData,
            tnsData2,
            hPsyConfLong->tnsConf,
            0,
            psyData->blockSwitchingControl.windowSequence);
  }

  TnsEncode(&psyOutChannel->tnsInfo,
            tnsData,
            hPsyConfLong->sfbCnt,
            hPsyConfLong->tnsConf,
            hPsyConfLong->lowpassLine,
            psyData->mdctSpectrum,
            0,
            psyData->blockSwitchingControl.windowSequence);

  /* first part of threshold calculation */
  for (i=0; i<hPsyConfLong->sfbCnt; i++) {
    psyData->sfbThreshold.sfbLong[i] = fixmul_32x16b(psyData->sfbEnergy.sfbLong[i], hPsyConfLong->ratio);
    psyData->sfbThreshold.sfbLong[i] = L_min(psyData->sfbThreshold.sfbLong[i], clipEnergy);
  }

  /* Calc sfb-bandwise mdct-energies for left and right channel again */
  test();
  if (tnsData->dataRaw.tnsLong.subBlockInfo.tnsActive!=0) {
    Word16 tnsStartBand = hPsyConfLong->tnsConf.tnsStartBand;                           move16();
    CalcBandEnergy( psyData->mdctSpectrum,
                    hPsyConfLong->sfbOffset+tnsStartBand,
                    sub(hPsyConfLong->sfbActive, tnsStartBand),
                    psyData->sfbEnergy.sfbLong+tnsStartBand,
                    &psyData->sfbEnergySum.sfbLong);
    for (i=0; i<tnsStartBand; i++)
      psyData->sfbEnergySum.sfbLong = L_add(psyData->sfbEnergySum.sfbLong, psyData->sfbEnergy.sfbLong[i]);
  }


  /* spreading */
  SpreadingMax(hPsyConfLong->sfbCnt,
               hPsyConfLong->sfbMaskLowFactor,
               hPsyConfLong->sfbMaskHighFactor,
               psyData->sfbThreshold.sfbLong);

  /* threshold in quiet */
  for (i=0; i<hPsyConfLong->sfbCnt; i++)
    psyData->sfbThreshold.sfbLong[i] = L_max(psyData->sfbThreshold.sfbLong[i],
                                             L_shr(hPsyConfLong->sfbThresholdQuiet[i], normEnergyShift));

  /* preecho control */
  test();
  if (sub(psyData->blockSwitchingControl.windowSequence, STOP_WINDOW) == 0) {
    for (i=0; i<hPsyConfLong->sfbCnt;i++) {
      psyData->sfbThresholdnm1[i] = MAX_32;                             move32();
    }
    psyData->mdctScalenm1 = 0;                                          move16();
  }

  PreEchoControl( psyData->sfbThresholdnm1,
                  hPsyConfLong->sfbCnt,
                  hPsyConfLong->maxAllowedIncreaseFactor,
                  hPsyConfLong->minRemainingThresholdFactor,
                  psyData->sfbThreshold.sfbLong,
                  psyData->mdctScale,
                  psyData->mdctScalenm1);
  psyData->mdctScalenm1 = psyData->mdctScale;                           move16();

  test();
  if (sub(psyData->blockSwitchingControl.windowSequence, START_WINDOW) == 0) {
    for (i=0; i<hPsyConfLong->sfbCnt; i++) {
      psyData->sfbThresholdnm1[i] = MAX_32;                             move32();
    }
    psyData->mdctScalenm1 = 0;                                          move16();
  }

  /* apply tns mult table on cb thresholds */
  ApplyTnsMultTableToRatios(hPsyConfLong->tnsConf.tnsRatioPatchLowestCb,
                            hPsyConfLong->tnsConf.tnsStartBand,
                            tnsData->dataRaw.tnsLong.subBlockInfo,
                            psyData->sfbThreshold.sfbLong);


  /* spreaded energy */
  for (i=0; i<hPsyConfLong->sfbCnt; i++) {
    psyData->sfbSpreadedEnergy.sfbLong[i] = psyData->sfbEnergy.sfbLong[i];      move32();
  }
  SpreadingMax(hPsyConfLong->sfbCnt,
               hPsyConfLong->sfbMaskLowFactorSprEn, 
               hPsyConfLong->sfbMaskHighFactorSprEn,
               psyData->sfbSpreadedEnergy.sfbLong);

  return 0;
}


static Word16 advancePsychLongMS (PSY_DATA psyData[MAX_CHANNELS],
                                  const PSY_CONFIGURATION_LONG *hPsyConfLong)
{
  CalcBandEnergyMS(psyData[0].mdctSpectrum,
                   psyData[1].mdctSpectrum,
                   hPsyConfLong->sfbOffset,
                   hPsyConfLong->sfbActive,
                   psyData[0].sfbEnergyMS.sfbLong,
                   &psyData[0].sfbEnergySumMS.sfbLong,
                   psyData[1].sfbEnergyMS.sfbLong,
                   &psyData[1].sfbEnergySumMS.sfbLong);

  return 0;
}


/*****************************************************************************

    functionname: advancePsychShort
    description:  psychoacoustic for short blocks

*****************************************************************************/

static Word16 advancePsychShort(PSY_DATA* psyData,
                                TNS_DATA* tnsData,
                                const PSY_CONFIGURATION_SHORT *hPsyConfShort,
                                PSY_OUT_CHANNEL* psyOutChannel,
                                Word32 *pScratchTns,
                                const TNS_DATA *tnsData2,
                                const Word16 ch)
{
  Word16 w;
  Word16 normEnergyShift = shl(add(psyData->mdctScale,1), 1); /* in reference code, mdct spectrum must be multipied with 2, so +1 */
  Word32 clipEnergy = L_shr(hPsyConfShort->clipEnergy, normEnergyShift);
  Word16 wOffset = 0;                                                           move16();

  for(w = 0; w < TRANS_FAC; w++) {
    Word16 i;

    /* low pass */
    for(i=hPsyConfShort->lowpassLine; i<FRAME_LEN_SHORT; i++){
      psyData->mdctSpectrum[i+wOffset] = 0;                                     move32();
    }

    /* Calc sfb-bandwise mdct-energies for left and right channel */
    CalcBandEnergy( psyData->mdctSpectrum+wOffset,
                    hPsyConfShort->sfbOffset,
                    hPsyConfShort->sfbActive,
                    psyData->sfbEnergy.sfbShort[w],
                    &psyData->sfbEnergySum.sfbShort[w]);

    /*
       TNS
    */
    TnsDetect(tnsData,
              hPsyConfShort->tnsConf,
              pScratchTns,
              (const Word16*)hPsyConfShort->sfbOffset,
              psyData->mdctSpectrum+wOffset,
              w,
              psyData->blockSwitchingControl.windowSequence,
              psyData->sfbEnergy.sfbShort[w]);

    /*  TnsSync */
    if (sub(ch,1) == 0) {
      TnsSync(tnsData,
              tnsData2,
              hPsyConfShort->tnsConf,
              w,
              psyData->blockSwitchingControl.windowSequence);
    }

    TnsEncode(&psyOutChannel->tnsInfo,
              tnsData,
              hPsyConfShort->sfbCnt,
              hPsyConfShort->tnsConf,
              hPsyConfShort->lowpassLine,
              psyData->mdctSpectrum+wOffset,
              w,
              psyData->blockSwitchingControl.windowSequence);

    /* first part of threshold calculation */
    for (i=0; i<hPsyConfShort->sfbCnt; i++) {
      psyData->sfbThreshold.sfbShort[w][i] = fixmul_32x16b(psyData->sfbEnergy.sfbShort[w][i], hPsyConfShort->ratio);
      psyData->sfbThreshold.sfbShort[w][i] = L_min(psyData->sfbThreshold.sfbShort[w][i], clipEnergy);
    }

    /* Calc sfb-bandwise mdct-energies for left and right channel again */
    test();
    if (tnsData->dataRaw.tnsShort.subBlockInfo[w].tnsActive != 0) {
      Word16 tnsStartBand = hPsyConfShort->tnsConf.tnsStartBand;                           move16();
      CalcBandEnergy( psyData->mdctSpectrum+wOffset,
                      hPsyConfShort->sfbOffset+tnsStartBand,
                      sub(hPsyConfShort->sfbActive, tnsStartBand),
                      psyData->sfbEnergy.sfbShort[w]+tnsStartBand,
                      &psyData->sfbEnergySum.sfbShort[w]);
      for (i=0; i<tnsStartBand; i++)
        psyData->sfbEnergySum.sfbShort[w] = L_add(psyData->sfbEnergySum.sfbShort[w],
                                                  psyData->sfbEnergy.sfbShort[w][i]);
    }

    /* spreading */
    SpreadingMax(hPsyConfShort->sfbCnt,
                 hPsyConfShort->sfbMaskLowFactor,
                 hPsyConfShort->sfbMaskHighFactor,
                 psyData->sfbThreshold.sfbShort[w]);


    /* threshold in quiet */
    for (i=0; i<hPsyConfShort->sfbCnt; i++)
      psyData->sfbThreshold.sfbShort[w][i] = L_max(psyData->sfbThreshold.sfbShort[w][i],
                                                   L_shr(hPsyConfShort->sfbThresholdQuiet[i], normEnergyShift));

    /* preecho */
    test();
    PreEchoControl( psyData->sfbThresholdnm1,
                    hPsyConfShort->sfbCnt,
                    hPsyConfShort->maxAllowedIncreaseFactor,
                    hPsyConfShort->minRemainingThresholdFactor,
                    psyData->sfbThreshold.sfbShort[w],
                    psyData->mdctScale,
                    w==0 ? psyData->mdctScalenm1 : psyData->mdctScale);

    /* apply tns mult table on cb thresholds */
    ApplyTnsMultTableToRatios( hPsyConfShort->tnsConf.tnsRatioPatchLowestCb,
                               hPsyConfShort->tnsConf.tnsStartBand,
                               tnsData->dataRaw.tnsShort.subBlockInfo[w],
                               psyData->sfbThreshold.sfbShort[w]);

    /* spreaded energy */
    for (i=0; i<hPsyConfShort->sfbCnt; i++) {
      psyData->sfbSpreadedEnergy.sfbShort[w][i] = psyData->sfbEnergy.sfbShort[w][i];    move32();
    }
    SpreadingMax(hPsyConfShort->sfbCnt,
                 hPsyConfShort->sfbMaskLowFactorSprEn, 
                 hPsyConfShort->sfbMaskHighFactorSprEn,
                 psyData->sfbSpreadedEnergy.sfbShort[w]);

    wOffset = add(wOffset, FRAME_LEN_SHORT);
  } /* for TRANS_FAC */

  psyData->mdctScalenm1 = psyData->mdctScale;             move16();

  return 0;
}


static Word16 advancePsychShortMS (PSY_DATA psyData[MAX_CHANNELS],
                                   const PSY_CONFIGURATION_SHORT *hPsyConfShort)
{
  Word16 w, wOffset;
  wOffset = 0;                                  move16();
  for(w=0; w<TRANS_FAC; w++) {
    CalcBandEnergyMS(psyData[0].mdctSpectrum+wOffset,
                     psyData[1].mdctSpectrum+wOffset,
                     hPsyConfShort->sfbOffset,
                     hPsyConfShort->sfbActive,
                     psyData[0].sfbEnergyMS.sfbShort[w],
                     &psyData[0].sfbEnergySumMS.sfbShort[w],
                     psyData[1].sfbEnergyMS.sfbShort[w],
                     &psyData[1].sfbEnergySumMS.sfbShort[w]);
    wOffset = add(wOffset, FRAME_LEN_SHORT);
  }

  return 0;
}
