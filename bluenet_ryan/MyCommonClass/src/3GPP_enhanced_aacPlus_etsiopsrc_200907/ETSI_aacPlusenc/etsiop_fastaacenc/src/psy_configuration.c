/*
   Psychoaccoustic configuration
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "psy_configuration.h"
#include "adj_thr.h"
#include "aac_rom.h"
#include "count.h"


#define BARC_SCALE 100 /* integer barc values are scaled with 100 */


typedef struct{
  Word32 sampleRate;
  const UWord8 *paramLong;
  const UWord8 *paramShort;
}SFB_INFO_TAB;

static const Word16 ABS_LEV = 20;
static const Word16 BARC_THR_QUIET[] = {15, 10,  7,  2,  0,  0,  0,  0,  0,  0,
                                         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                         3,  5, 10, 20, 30};



static SFB_INFO_TAB sfbInfoTab[] ={
  {16000, sfb_16000_long_1024, sfb_16000_short_128},
  {22050, sfb_22050_long_1024, sfb_22050_short_128},
  {24000, sfb_24000_long_1024, sfb_24000_short_128},
};




static const Word16 max_bark = 24; /* maximum bark-value */
static const Word16 maskLow  = 30; /* in 1dB/bark */
static const Word16 maskHigh = 15; /* in 1*dB/bark */
static const Word16 c_ratio  = 0x0029; /* pow(10.0f, -(29.0f/10.0f)) */

static const Word16 maskLowSprEnLong = 30;       /* in 1dB/bark */
static const Word16 maskHighSprEnLong = 20;      /* in 1dB/bark */
static const Word16 maskHighSprEnLongLowBr = 15; /* in 1dB/bark */
static const Word16 maskLowSprEnShort = 20;      /* in 1dB/bark */
static const Word16 maskHighSprEnShort = 15;     /* in 1dB/bark */


static Word16 initSfbTable(Word32 sampleRate, Word16 blockType, Word16 *sfbOffset, Word16 *sfbCnt)
{
  const UWord8 *sfbParam = 0;
  UWord16 i;
  Word16 specStartOffset, specLines = 0;

  /*
    select table
  */
  for(i=0; i<sizeof(sfbInfoTab)/sizeof(SFB_INFO_TAB); i++){
    test();
    if(L_sub(sfbInfoTab[i].sampleRate, sampleRate) == 0){
      switch(blockType){
        case LONG_WINDOW:
        case START_WINDOW:
        case STOP_WINDOW:
          sfbParam = sfbInfoTab[i].paramLong;
          specLines = FRAME_LEN_LONG;                   move16();
          break;
        case SHORT_WINDOW:
          sfbParam = sfbInfoTab[i].paramShort;
          specLines = FRAME_LEN_SHORT;                  move16();
          break;
      }
      break;
    }
  }
  test();
  if(sfbParam==0)
    return 1;

  /*
    calc sfb offsets
  */
  *sfbCnt = 0;                                          move16();
  specStartOffset = 0;                                  move16();
  
  do {
    sfbOffset[*sfbCnt] = specStartOffset;               move16();
    specStartOffset = add(specStartOffset, sfbParam[*sfbCnt]);
    *sfbCnt = add(*sfbCnt, 1);
    test();
  } while(sub(specStartOffset, specLines) < 0);
  
  sfbOffset[*sfbCnt] = specStartOffset;                 move16();

  return 0;
}


/**************************************************************************/
/*!
  \brief  calculates 1000*atan(x/1000)
  based on atan approx for x > 0

  \return    1000*atan(x/1000)

*/
/**************************************************************************/

static Word16 atan_1000(Word32 val) 
{
  Word32 y;

  test();
  if(L_sub(val, 1000) < 0) {
    y = extract_l(ffr_divideWord32(ffr_Integer_Mult(1000,val),
                                   L_add(1000, ffr_divideWord32(ffr_Integer_Mult(val, val), 3560))));
  }
  else {
    y = L_sub(1571, ffr_divideWord32(ffr_Integer_Mult(1000, val),
                                     L_add(281, ffr_divideWord32(ffr_Integer_Mult(val, val), 1000))));
  }

  return extract_l(y);
}



/*****************************************************************************

    functionname: BarcLineValue
    description:  Calculates barc value for one frequency line
    returns:      barc value of line * BARC_SCALE
    input:        number of lines in transform, index of line to check, Fs
    output:

*****************************************************************************/
static Word16 BarcLineValue(Word16 noOfLines, Word16 fftLine, Word32 samplingFreq)
{
  Word32 center_freq, temp, bvalFFTLine;

  /* center frequency of fft line */
  center_freq = ffr_divideWord32(ffr_Integer_Mult(fftLine, samplingFreq), shl(noOfLines,1));
  temp =  atan_1000(ffr_divideWord32(L_shl(center_freq,2), (3*10)));
  bvalFFTLine = ffr_divideWord32
    (L_add(ffr_Integer_Mult(26600, atan_1000(ffr_divideWord32(ffr_Integer_Mult(center_freq,76), 100))),
           ffr_Integer_Mult(7,ffr_Integer_Mult16x16(temp,temp))),
     (ffr_divideWord32(2*1000*1000, BARC_SCALE)));
  
  return extract_l(bvalFFTLine);
}



static void initThrQuiet(Word16  numPb,
                         Word16 *pbOffset,
                         Word16 *pbBarcVal,
                         Word32 *pbThresholdQuiet) {
  Word16 i;
  Word16 barcThrQuiet;

  for(i=0; i<numPb; i++) {
    Word16 bv1, bv2;

    test();
    if (i>0)
      bv1 = shr(add(pbBarcVal[i], pbBarcVal[i-1]), 1);
    else
      bv1 = shr(pbBarcVal[i], 1);

    test();
    if (sub(i, sub(numPb,1)) < 0)
      bv2 = shr(add(pbBarcVal[i], pbBarcVal[i+1]), 1);
    else {
      bv2 = pbBarcVal[i];                                       move16();
    }

    bv1 = S_min(ffr_Short_Div(bv1, BARC_SCALE), max_bark);
    bv2 = S_min(ffr_Short_Div(bv2, BARC_SCALE), max_bark);

    barcThrQuiet = S_min(BARC_THR_QUIET[bv1], BARC_THR_QUIET[bv2]);

    
    /*
      we calculate 
      pow(10.0f,(float)(barcThrQuiet - ABS_LEV)*0.1)*(float)ABS_LOW*(pbOffset[i+1] - pbOffset[i]);
    */

    pbThresholdQuiet[i] = ffr_Integer_Mult(
        ffr_pow2_xy(L_add(ffr_Integer_Mult16x16(sub(barcThrQuiet, ABS_LEV), 100), 
                          301*(14+2*LOG_NORM_PCM)), 301),
        sub(pbOffset[i+1], pbOffset[i]));
  }
}



static void initSpreading(Word16  numPb,
                          Word16 *pbBarcValue,
                          Word16 *pbMaskLoFactor,
                          Word16 *pbMaskHiFactor,
                          Word16 *pbMaskLoFactorSprEn,
                          Word16 *pbMaskHiFactorSprEn,
                          const Word32 bitrate,
                          const Word16 blockType)
{
  Word16 i;
  Word16 maskLowSprEn, maskHighSprEn;

  test();
  if (sub(blockType, SHORT_WINDOW) != 0) {
    maskLowSprEn = maskLowSprEnLong;                                    move16();
    test(); move16();
    if (L_sub(bitrate, 22000) > 0)
      maskHighSprEn = maskHighSprEnLong;
    else
      maskHighSprEn = maskHighSprEnLongLowBr;
  }
  else {
    maskLowSprEn = maskLowSprEnShort;           move16();
    maskHighSprEn = maskHighSprEnShort;         move16();
  }

  for(i=0; i<numPb; i++) {
    test();
    if (i > 0) {
      Word32 dbVal;
      Word16 dbark = sub(pbBarcValue[i], pbBarcValue[i-1]);

      /*
        we calulate pow(10.0f, -0.1*dbVal/BARC_SCALE) 
      */
      dbVal = ffr_Integer_Mult16x16(maskHigh, dbark);
      pbMaskHiFactor[i] = etsiopround(ffr_pow2_xy(L_negate(dbVal), (Word32)(10*BARC_SCALE/3.3219)));             /* 3.3219 log2(10) */
       
      dbVal = ffr_Integer_Mult16x16(maskLow, dbark);
      pbMaskLoFactor[i-1] = etsiopround(ffr_pow2_xy(L_negate(dbVal),(Word32)(10*BARC_SCALE/3.3219))); 
       
      
      dbVal = ffr_Integer_Mult16x16(maskHighSprEn, dbark);
      pbMaskHiFactorSprEn[i] =  etsiopround(ffr_pow2_xy(L_negate(dbVal),(Word32)(10*BARC_SCALE/3.3219))); 
      dbVal = ffr_Integer_Mult16x16(maskLowSprEn, dbark);
      pbMaskLoFactorSprEn[i-1] = etsiopround(ffr_pow2_xy(L_negate(dbVal),(Word32)(10*BARC_SCALE/3.3219)));
    }
    else {
      pbMaskHiFactor[i] = 0;                    move16();
      pbMaskLoFactor[numPb-1] = 0;              move16();

      pbMaskHiFactorSprEn[i] = 0;               move16();
      pbMaskLoFactorSprEn[numPb-1] = 0;         move16();
    }
  }

}



static void initBarcValues(Word16  numPb,
                           Word16 *pbOffset,
                           Word16  numLines,
                           Word32  samplingFrequency,
                           Word16 *pbBval)
{
  Word16 i;
  Word16 pbBval0, pbBval1;

  pbBval0 = 0;                                          move16();

  for(i=0; i<numPb; i++){
    pbBval1 = BarcLineValue(numLines, pbOffset[i+1], samplingFrequency);
    pbBval[i] = shr(add(pbBval0,pbBval1), 1);
    pbBval0 = pbBval1;                                  move16();
  }
}



static void initMinSnr(const Word32  bitrate,
                       const Word32  samplerate,
                       const Word16  numLines,
                       const Word16 *sfbOffset,
                       const Word16 *pbBarcVal,
                       const Word16  sfbActive,
                       Word16       *sfbMinSnr)
{
  Word16 sfb;
  Word16 barcWidth;
  Word16 pePerWindow;
  Word32 pePart;
  Word32 snr;
  Word16 pbVal0, pbVal1;

  /* relative number of active barks */


  pePerWindow = bits2pe(extract_l(ffr_divideWord32(ffr_Integer_Mult(bitrate, numLines), samplerate)));

  pbVal0 = 0;                                                   move16();

  for (sfb=0; sfb<sfbActive; sfb++) {

    pbVal1 = sub(shl(pbBarcVal[sfb],1), pbVal0);
    barcWidth = sub(pbVal1, pbVal0);
    pbVal0 = pbVal1;                                            move16();

    pePart = ffr_divideWord32(
        ffr_Integer_Mult(
          ffr_Integer_Mult16x16(pePerWindow, 24),
          ffr_Integer_Mult16x16(max_bark, barcWidth)),
        ffr_Integer_Mult16x16(pbBarcVal[sfbActive-1], sub(sfbOffset[sfb+1], sfbOffset[sfb])));
   
      
    pePart = L_min(pePart, 8400); 
    pePart = L_max(pePart, 1400);

    /* we add an offset of 2^16 to the pow functions */
      
    snr = L_sub(ffr_pow2_xy(L_sub(pePart,16*1000),1000), 0x0000c000);
      
    snr = ffr_div32_32(0x00008000, snr);  
      
    /* upper limit is -1 dB */
    snr = L_min(snr, 0x66666666);
    /* lower limit is -25 dB */
    snr = L_max(snr, 0x00624dd3);
    sfbMinSnr[sfb] = etsiopround(snr);
  }

}


Word16 InitPsyConfigurationLong(Word32 bitrate,
                                Word32 samplerate,
                                Word16 bandwidth,
                                PSY_CONFIGURATION_LONG *psyConf)
{
  Word16 sfb;
  Word16 sfbBarcVal[MAX_SFB_LONG];

  /*
    init sfb table
  */
  if(initSfbTable(samplerate, LONG_WINDOW, psyConf->sfbOffset, &(psyConf->sfbCnt)))
    return(1);


  /*
    calculate barc values for each pb
  */
  initBarcValues(psyConf->sfbCnt,
                 psyConf->sfbOffset,
                 psyConf->sfbOffset[psyConf->sfbCnt],
                 samplerate,
                 sfbBarcVal);

  /*
    init thresholds in quiet
  */
  initThrQuiet(psyConf->sfbCnt,
               psyConf->sfbOffset,
               sfbBarcVal,
               psyConf->sfbThresholdQuiet);

  /*
    calculate spreading function
  */
  initSpreading(psyConf->sfbCnt,
                sfbBarcVal,
                psyConf->sfbMaskLowFactor,
                psyConf->sfbMaskHighFactor,
                psyConf->sfbMaskLowFactorSprEn,
                psyConf->sfbMaskHighFactorSprEn,
                bitrate,
                LONG_WINDOW);

  /*
    init ratio
  */
  psyConf->ratio = c_ratio;                                                     move16();

  psyConf->maxAllowedIncreaseFactor = 2;                                        move16();
  psyConf->minRemainingThresholdFactor = 0x0148;                            	move16();

  psyConf->clipEnergy = 0x77359400;                                             move32();
  psyConf->lowpassLine = extract_l(ffr_divideWord32(
        ffr_Integer_Mult16x16(shl(bandwidth,1), FRAME_LEN_LONG),
        samplerate));

  for (sfb = 0; sfb < psyConf->sfbCnt; sfb++) {
    test();
    if (sub(psyConf->sfbOffset[sfb], psyConf->lowpassLine) >= 0)
      break;
  }
  psyConf->sfbActive = sfb;                                                     move16();

  /*
    calculate minSnr
  */
  initMinSnr(bitrate,
             samplerate,
             psyConf->sfbOffset[psyConf->sfbCnt],
             psyConf->sfbOffset,
             sfbBarcVal,
             psyConf->sfbActive,
             psyConf->sfbMinSnr);


  return(0);
}


Word16 InitPsyConfigurationShort(Word32 bitrate,
                                 Word32 samplerate,
                                 Word16 bandwidth,
                                 PSY_CONFIGURATION_SHORT *psyConf) {
  Word16 sfb;
  Word16 sfbBarcVal[MAX_SFB_SHORT];

  /*
    init sfb table
  */
  if (initSfbTable(samplerate, SHORT_WINDOW, psyConf->sfbOffset, &(psyConf->sfbCnt)))
    return(1);

  /*
    calculate barc values for each pb
  */
  initBarcValues(psyConf->sfbCnt,
                 psyConf->sfbOffset,
                 psyConf->sfbOffset[psyConf->sfbCnt],
                 samplerate,
                 sfbBarcVal);

  /*
    init thresholds in quiet
  */
  initThrQuiet(psyConf->sfbCnt,
               psyConf->sfbOffset,
               sfbBarcVal,
               psyConf->sfbThresholdQuiet);

  /*
    calculate spreading function
  */
  initSpreading(psyConf->sfbCnt,
                sfbBarcVal,
                psyConf->sfbMaskLowFactor,
                psyConf->sfbMaskHighFactor,
                psyConf->sfbMaskLowFactorSprEn,
                psyConf->sfbMaskHighFactorSprEn,
                bitrate,
                SHORT_WINDOW);

  /*
    init ratio
  */
  psyConf->ratio = c_ratio;                                                     move16();

  psyConf->maxAllowedIncreaseFactor = 2;                                        move16();
  psyConf->minRemainingThresholdFactor = 0x0148;                            	move16();

  psyConf->clipEnergy = 0x01dcd650;                                             move32();

  psyConf->lowpassLine = extract_l(ffr_divideWord32(
        ffr_Integer_Mult16x16(shl(bandwidth,1), FRAME_LEN_SHORT),
        samplerate));
 
  for (sfb = 0; sfb < psyConf->sfbCnt; sfb++) {
    test();
    if (sub(psyConf->sfbOffset[sfb], psyConf->lowpassLine) >= 0)
      break;
  }
  psyConf->sfbActive = sfb;                                                     move16();

  /*
    calculate minSnr
  */
  initMinSnr(bitrate,
             samplerate,
             psyConf->sfbOffset[psyConf->sfbCnt],
             psyConf->sfbOffset,
             sfbBarcVal,
             psyConf->sfbActive,
             psyConf->sfbMinSnr);

  return(0);
}

