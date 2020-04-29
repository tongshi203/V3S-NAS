/*
   Temporal noise shaping
 */
#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include "aac_rom.h"
#include "psy_const.h"
#include "tns.h"
#include "tns_param.h"
#include "psy_configuration.h"
#include "tns_func.h"

#include "count.h"

#define TNS_MODIFY_BEGIN         2600  /* Hz */
#define RATIO_PATCH_LOWER_BORDER 380   /* Hz */

static const Word32 TNS_PARCOR_THRESH = 0x0ccccccd;


static void CalcGaussWindow(Word32 *win, const Word16 winSize, const Word32 samplingRate,
                            const Word16 blockType, const Word32 timeResolution );

static void CalcWeightedSpectrum(const Word32 spectrum[],
                                 Word16 weightedSpectrum[],
                                 Word32* sfbEnergy,
                                 const Word16* sfbOffset, Word16 lpcStartLine,
                                 Word16 lpcStopLine, Word16 lpcStartBand,Word16 lpcStopBand,
                                 Word32 *pWork32);



static void AutoCorrelation(const Word16 input[], Word32 corr[],
                            Word16 samples, Word16 corrCoeff);
static Word16 AutoToParcor(Word32 workBuffer[], Word32 reflCoeff[], Word16 numOfCoeff);

static Word16 CalcTnsFilter(const Word16* signal, const Word32 window[], Word16 numOfLines,
                                              Word16 tnsOrder, Word32 parcor[]);


static void Parcor2Index(const Word32 parcor[], Word16 index[], Word16 order,
                         Word16 bitsPerCoeff);

static void Index2Parcor(const Word16 index[], Word32 parcor[], Word16 order,
                         Word16 bitsPerCoeff);



static void AnalysisFilterLattice(const Word32 signal[], Word16 numOfLines,
                                  const Word32 parCoeff[], Word16 order,
                                  Word32 output[]);



/*!
  \brief  Retrieve index of nearest band border

  \return index
*/
static Word16 FreqToBandWithRounding(Word32 freq,                   /*!< frequency in Hertz */
                                     Word32 fs,                     /*!< Sampling frequency in Hertz */
                                     Word16 numOfBands,             /*!< total number of bands */
                                     const Word16 *bandStartOffset) /*!< table of band borders */
{
  Word16 lineNumber, band;
  Word16 temp;

  /*  assert(freq >= 0);  */

  lineNumber = shr(add(extract_l(fixmul(shl(bandStartOffset[numOfBands],2),ffr_div32_32(freq,fs))),1),1);

  /* freq > fs/2 */
  temp = sub( lineNumber, bandStartOffset[numOfBands] );                                           test();
  if (temp >= 0)
    return numOfBands;

  /* find band the line number lies in */
  for (band=0; band<numOfBands; band++) {
    temp = sub( bandStartOffset[add(band,1)], lineNumber );                                          test();
    if (temp > 0) break;
  }

  temp = sub(lineNumber,bandStartOffset[band]);
  temp = sub(temp,sub(bandStartOffset[add(band,1)], lineNumber));                                  test();
  if ( temp > 0 )
  {
    band = add(band,1);
  }

  return(band);
}


/*!
  \brief  Fill TNS_CONFIG structure with sensible content for long blocks
*/
Word16 InitTnsConfigurationLong(Word32 bitRate,          /*!< bitrate */
                                Word32 sampleRate,          /*!< Sampling frequency */
                                Word16 channels,            /*!< number of channels */
                                TNS_CONFIG *tC,             /*!< TNS Config struct (modified) */
                                PSY_CONFIGURATION_LONG *pC, /*!< psy config struct */
                                Word16 active)              /*!< tns active flag */
{

  Word32 bitratePerChannel;
  tC->maxOrder     = TNS_MAX_ORDER;                                           
  tC->tnsStartFreq = 1275;
  tC->coefRes      = 4;                                                                            move16(); move32(); move16();
  
  /* to avoid integer division */
  assert( channels <= 2 );                                                                         test();
  if ( sub(channels,2) == 0 ) { 
    bitratePerChannel = L_shr(bitRate,1);    
  }
  else {
    bitratePerChannel = bitRate;                                                                   move32();
  }

  GetTnsParam(&tC->confTab, bitratePerChannel, channels, LONG_WINDOW);

  CalcGaussWindow(tC->acfWindow,
                  add(tC->maxOrder,1),
                  sampleRate,
                  LONG_WINDOW,
                  tC->confTab.tnsTimeResolution);

  GetTnsMaxBands(sampleRate, LONG_WINDOW, &tC->tnsMaxSfb);

  tC->tnsActive = active;                                                                          move16();

  /* now calc band and line borders */

  tC->tnsStopBand = S_min(pC->sfbCnt, tC->tnsMaxSfb);
  tC->tnsStopLine = pC->sfbOffset[tC->tnsStopBand];                                                move16();

  tC->tnsStartBand = FreqToBandWithRounding(tC->tnsStartFreq, sampleRate,
                                            pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->tnsModifyBeginCb = FreqToBandWithRounding(TNS_MODIFY_BEGIN,
                                                sampleRate,
                                                pC->sfbCnt,
                                                (const Word16*)pC->sfbOffset);

  tC->tnsRatioPatchLowestCb = FreqToBandWithRounding(RATIO_PATCH_LOWER_BORDER,
                                                     sampleRate,
                                                     pC->sfbCnt,
                                                     (const Word16*)pC->sfbOffset);


  tC->tnsStartLine = pC->sfbOffset[tC->tnsStartBand];                                                                                                            move16();

  tC->lpcStopBand=FreqToBandWithRounding(tC->confTab.lpcStopFreq, sampleRate,
                                         pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->lpcStopBand = S_min(tC->lpcStopBand, pC->sfbActive);

  tC->lpcStopLine = pC->sfbOffset[tC->lpcStopBand];                                                move16();

  tC->lpcStartBand = FreqToBandWithRounding(tC->confTab.lpcStartFreq, sampleRate,
                                            pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->lpcStartLine = pC->sfbOffset[tC->lpcStartBand];                                              move16();

  tC->threshold = tC->confTab.threshOn;                                                            move16();


  return(0);
}


/*!
  \brief  Fill TNS_CONFIG structure with sensible content for short blocks
*/
Word16 InitTnsConfigurationShort(Word32 bitRate,              /*!< bitrate */
                                 Word32 sampleRate,           /*!< Sampling frequency */
                                 Word16 channels,             /*!< number of channels */
                                 TNS_CONFIG *tC,              /*!< TNS Config struct (modified) */
                                 PSY_CONFIGURATION_SHORT *pC, /*!< psy config struct */
                                 Word16 active)               /*!< tns active flag */
{
  Word32 bitratePerChannel;
  tC->maxOrder     = TNS_MAX_ORDER_SHORT;
  tC->tnsStartFreq = 2750;
  tC->coefRes      = 3;                                                                            move16(); move32(); move16();
  
  /* to avoid integer division */
  assert( channels <= 2 );                                                                         test();
  if ( sub(channels,2) == 0 ) {
    bitratePerChannel = L_shr(bitRate,1);    
  }
  else {
    bitratePerChannel = bitRate;                                                                   move32();
  }

  GetTnsParam(&tC->confTab, bitratePerChannel, channels, SHORT_WINDOW);

  CalcGaussWindow(tC->acfWindow,
                  add(tC->maxOrder,1),
                  sampleRate,
                  SHORT_WINDOW,
                  tC->confTab.tnsTimeResolution);

  GetTnsMaxBands(sampleRate, SHORT_WINDOW, &tC->tnsMaxSfb);

  tC->tnsActive = active;                                                                          move16();

  /* now calc band and line borders */

  tC->tnsStopBand = S_min(pC->sfbCnt, tC->tnsMaxSfb);
  tC->tnsStopLine = pC->sfbOffset[tC->tnsStopBand];                                                move16();

  tC->tnsStartBand=FreqToBandWithRounding(tC->tnsStartFreq, sampleRate,
                                          pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->tnsModifyBeginCb = FreqToBandWithRounding(TNS_MODIFY_BEGIN,
                                                sampleRate,
                                                pC->sfbCnt,
                                                (const Word16*)pC->sfbOffset);

  tC->tnsRatioPatchLowestCb = FreqToBandWithRounding(RATIO_PATCH_LOWER_BORDER,
                                                     sampleRate,
                                                     pC->sfbCnt,
                                                     (const Word16*)pC->sfbOffset);


  tC->tnsStartLine = pC->sfbOffset[tC->tnsStartBand];                                              move16();

  tC->lpcStopBand = FreqToBandWithRounding(tC->confTab.lpcStopFreq, sampleRate,
                                           pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->lpcStopBand = S_min(tC->lpcStopBand, pC->sfbActive);

  tC->lpcStopLine = pC->sfbOffset[tC->lpcStopBand];                                                move16();

  tC->lpcStartBand = FreqToBandWithRounding(tC->confTab.lpcStartFreq, sampleRate,
                                            pC->sfbCnt, (const Word16*)pC->sfbOffset);

  tC->lpcStartLine = pC->sfbOffset[tC->lpcStartBand];                                              move16();

  tC->threshold = tC->confTab.threshOn;                                                            move16();

  return(0);
}


/*!
  \brief   Calculate TNS filter and decide on TNS usage 
  \return  zero
*/
Word32 TnsDetect(TNS_DATA* tnsData,        /*!< tns data structure (modified) */
                 TNS_CONFIG tC,            /*!< tns config structure */
                 Word32* pScratchTns,      /*!< pointer to scratch space */
                 const Word16 sfbOffset[], /*!< scalefactor size and table */
                 Word32* spectrum,         /*!< spectral data */
                 Word16 subBlockNumber,    /*!< subblock num */
                 Word16 blockType,         /*!< blocktype (long or short) */
                 Word32 * sfbEnergy)       /*!< sfb-wise energy */
{

  Word16  predictionGain;
  Word16  temp;
  Word32* pWork32 = &pScratchTns[mult(subBlockNumber,FRAME_LEN_SHORT)];
  Word16* pWeightedSpectrum = (Word16 *)&pScratchTns[mult(subBlockNumber,FRAME_LEN_SHORT)];

                                                                                                   test();
  if (tC.tnsActive) {
    CalcWeightedSpectrum(spectrum,
                         pWeightedSpectrum,
                         sfbEnergy,
                         sfbOffset,
                         tC.lpcStartLine,
                         tC.lpcStopLine,
                         tC.lpcStartBand,
                         tC.lpcStopBand,
                         pWork32);

    temp = sub( blockType, SHORT_WINDOW );                                                         test();
    if ( temp != 0 ) {
        predictionGain = CalcTnsFilter( &pWeightedSpectrum[tC.lpcStartLine],
                                        tC.acfWindow,
                                        sub(tC.lpcStopLine,tC.lpcStartLine),
                                        tC.maxOrder,
                                        tnsData->dataRaw.tnsLong.subBlockInfo.parcor);


        temp = sub( predictionGain, tC.threshold );                                                test(); 
        if ( temp > 0 ) {
          tnsData->dataRaw.tnsLong.subBlockInfo.tnsActive = 1;                                     move16();
        }
        else {
          tnsData->dataRaw.tnsLong.subBlockInfo.tnsActive = 0;                                     move16();
        }

        tnsData->dataRaw.tnsLong.subBlockInfo.predictionGain = predictionGain;                     move16();
    }
    else{

        predictionGain = CalcTnsFilter( &pWeightedSpectrum[tC.lpcStartLine],
                                        tC.acfWindow,
                                        sub(tC.lpcStopLine, tC.lpcStartLine),
                                        tC.maxOrder,
                                        tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor);

        temp = sub( predictionGain, tC.threshold );                                                test();
        if ( temp > 0 ) {
          tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].tnsActive = 1;                    move16();
        }
        else {
          tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].tnsActive = 0;                    move16();
        }

        tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].predictionGain = predictionGain;    move16();
    }

  }
  else{

    temp = sub( blockType, SHORT_WINDOW );                                                         test();
    if ( temp != 0 ) {
        tnsData->dataRaw.tnsLong.subBlockInfo.tnsActive = 0;                                       move16();
        tnsData->dataRaw.tnsLong.subBlockInfo.predictionGain = 0;                                  move16();
    }
    else {
        tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].tnsActive = 0;                      move16();
        tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].predictionGain = 0;                 move16();
    }
  }

  return(0);
}


/*****************************************************************************
    functionname: TnsSync
    description:

*****************************************************************************/
void TnsSync(TNS_DATA *tnsDataDest,
             const TNS_DATA *tnsDataSrc,
             const TNS_CONFIG tC,
             const Word16 subBlockNumber,
             const Word16 blockType)
{
   TNS_SUBBLOCK_INFO *sbInfoDest;
   const TNS_SUBBLOCK_INFO *sbInfoSrc;
   Word16 i, temp;

   temp = sub( blockType, SHORT_WINDOW );                                                          test();
   if ( temp != 0 ) {
      sbInfoDest = &tnsDataDest->dataRaw.tnsLong.subBlockInfo;                                     move32();
      sbInfoSrc  = &tnsDataSrc->dataRaw.tnsLong.subBlockInfo;                                      move32();
   }
   else {
      sbInfoDest = &tnsDataDest->dataRaw.tnsShort.subBlockInfo[subBlockNumber];                    move32();
      sbInfoSrc  = &tnsDataSrc->dataRaw.tnsShort.subBlockInfo[subBlockNumber];                     move32();
   }

   if (100*abs(sbInfoDest->predictionGain - sbInfoSrc->predictionGain) <
       (3 * sbInfoDest->predictionGain)) {
      sbInfoDest->tnsActive = sbInfoSrc->tnsActive;                                                move16();
      for ( i=0; i< tC.maxOrder; i++) {
        sbInfoDest->parcor[i] = sbInfoSrc->parcor[i];                                              move32();
      }
   }
}


/*!
  \brief    do TNS filtering
  \return   zero
*/
Word16 TnsEncode(TNS_INFO* tnsInfo,     /*!< tns info structure (modified) */
                 TNS_DATA* tnsData,     /*!< tns data structure (modified) */
                 Word16 numOfSfb,       /*!< number of scale factor bands */
                 TNS_CONFIG tC,         /*!< tns config structure */
                 Word16 lowPassLine,    /*!< lowpass line */
                 Word32* spectrum,      /*!< spectral data (modified) */
                 Word16 subBlockNumber, /*!< subblock num */
                 Word16 blockType)      /*!< blocktype (long or short) */
{
  Word16 i;
  Word16 temp_s;
  Word32 temp;

  temp_s = sub(blockType,SHORT_WINDOW);                                                            test();
  if ( temp_s != 0) {                                                                              test();
    if (tnsData->dataRaw.tnsLong.subBlockInfo.tnsActive == 0) {
      tnsInfo->tnsActive[subBlockNumber] = 0;                                                      move16();
      return(0);
    }
    else {

      Parcor2Index(tnsData->dataRaw.tnsLong.subBlockInfo.parcor,
                   tnsInfo->coef,
                   tC.maxOrder,
                   tC.coefRes);

      Index2Parcor(tnsInfo->coef,
                   tnsData->dataRaw.tnsLong.subBlockInfo.parcor,
                   tC.maxOrder,
                   tC.coefRes);

      for (i=sub(tC.maxOrder,1); i>=0; i--)  {
        temp = L_sub( tnsData->dataRaw.tnsLong.subBlockInfo.parcor[i], TNS_PARCOR_THRESH );        test();
        if ( temp > 0 )
          break;
        temp = L_add( tnsData->dataRaw.tnsLong.subBlockInfo.parcor[i], TNS_PARCOR_THRESH );        test();
        if ( temp < 0 )
          break;
      }
      tnsInfo->order[subBlockNumber] = add(i,1);                                                   move16();


      tnsInfo->tnsActive[subBlockNumber] = 1;                                                      move16();
      for (i=add(subBlockNumber,1); i<TRANS_FAC; i++) {
        tnsInfo->tnsActive[i] = 0;                                                                 move16();
      }
      tnsInfo->coefRes[subBlockNumber] = tC.coefRes;                                               move16();
      tnsInfo->length[subBlockNumber] = sub(numOfSfb,tC.tnsStartBand);                             move16();   


      AnalysisFilterLattice(&(spectrum[tC.tnsStartLine]),
                            sub(S_min(tC.tnsStopLine,lowPassLine),tC.tnsStartLine),
                            tnsData->dataRaw.tnsLong.subBlockInfo.parcor,
                            tnsInfo->order[subBlockNumber],
                            &(spectrum[tC.tnsStartLine]));

    }
  }     /* if (blockType!=SHORT_WINDOW) */
  else /*short block*/ {                                                                           test();
    if (tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].tnsActive == 0) {
      tnsInfo->tnsActive[subBlockNumber] = 0;                                                      move16();
      return(0);
    }
    else {

      Parcor2Index(tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor,
                   &tnsInfo->coef[subBlockNumber*TNS_MAX_ORDER_SHORT],
                   tC.maxOrder,
                   tC.coefRes);

      Index2Parcor(&tnsInfo->coef[subBlockNumber*TNS_MAX_ORDER_SHORT],
                   tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor,
                   tC.maxOrder,
                   tC.coefRes);
      for (i=sub(tC.maxOrder,1); i>=0; i--)  {
        temp = L_sub( tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor[i], TNS_PARCOR_THRESH );   test();
         if ( temp > 0 )
          break;

        temp = L_add( tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor[i], TNS_PARCOR_THRESH );   test();
        if ( temp < 0 )
          break;
      }
      tnsInfo->order[subBlockNumber] = add(i,1);                                                   move16();

      tnsInfo->tnsActive[subBlockNumber] = 1;                                                      move16();
      tnsInfo->coefRes[subBlockNumber] = tC.coefRes;                                               move16();
      tnsInfo->length[subBlockNumber] = sub(numOfSfb, tC.tnsStartBand);                            move16();


      AnalysisFilterLattice(&(spectrum[tC.tnsStartLine]), sub(tC.tnsStopLine,tC.tnsStartLine),
                 tnsData->dataRaw.tnsShort.subBlockInfo[subBlockNumber].parcor,
                 tnsInfo->order[subBlockNumber],
                 &(spectrum[tC.tnsStartLine]));

    }
  }

  return(0);
}


/*!
  \brief  Iterative power function

  Calculates pow(2.0,x-1.0*(scale+1)) with INT_BITS bit precision
  using modified cordic algorithm
*/
static Word32 m_pow2_cordic(Word32 x, Word16 scale)
{
  Word16 k;

  Word32 accu_y = 0x40000000;                                                                    move32();
  accu_y = L_shr(accu_y,scale);

  for(k=1; k<INT_BITS; k++) {
    const Word32 z = m_log2_table[k];                                                            move32();

    while(L_sub(x,z) >= 0) {
      test();
      x = L_sub(x, z);
      accu_y = L_add(accu_y, L_shr(accu_y,k));
    }
  }
  return(accu_y);
}

/* 0.25*PI/sqrt(log2)*1024/1000*/
static const Word32 c0=0x7ba5e2fa;


/*!
  \brief  Calculates Gauss window

  Calculates Gauss window for acf windowing depending on desired
  temporal resolution, transform size and sampling rate
*/
static void CalcGaussWindow(Word32  *win,                /*!< Window coefficients (right half) */
                            const Word16 winSize,        /*!< Window size (number of output values) */
                            const Word32 samplingRate,   /*!< Sampling frequency */
                            const Word16 blockType,      /*!< blocktype (long or short) */
                            const Word32 timeResolution) /*!< time resolution (ms) */
{
  Word16 i;
  Word16 akexp;
  Word16 scale;
  Word16 temp;
  
  Word32 accuGaussExp;
  Word32 t1,t2;
  Word32 accuOne;
  Word32 accugsStart;

  accuGaussExp = fixmul(samplingRate, timeResolution); 

  akexp = ffr_norm32(accuGaussExp);         
  accuGaussExp = L_shl( accuGaussExp, akexp );
  t1 = accuGaussExp;                                                                               move32();
  accuGaussExp = fixmul(t1, c0);   
  t2 = accuGaussExp;                                                                               move32();
  accuGaussExp = L_negate(fixmul(t2, t2));

  temp = sub( blockType, SHORT_WINDOW );                                                           test();
  if (temp != 0)
    /* akexp =  (akexp+8+10-(INT_BITS-1))*2+1; */
    akexp = add(shl(sub(add(akexp,18),INT_BITS-1),1),1);
  else
    /* akexp =  (akexp+8+7-(INT_BITS-1))*2+1; */
    akexp = add(shl(sub(add(akexp,15),INT_BITS-1),1),1);

  test();
  if(akexp > 0)
    accuGaussExp = L_shr(accuGaussExp,akexp);
  else
    accuGaussExp = L_shr(accuGaussExp,-akexp);

  accuOne = 0x80000000;                                                                            move32();
  accugsStart = L_sub(L_shr(accuGaussExp,2), accuOne);
  scale = 0;                                                                                       move16();

  for(i=0; i<winSize; i++) {
    Word16 j;

    for(j=0;j<i;j++){
      accugsStart = L_add(accugsStart, L_shl(accuGaussExp,1));
      while ( accugsStart < 0 ) {                                                                  test();
        accugsStart = L_sub(accugsStart, accuOne);
        scale = add(scale,1);
      }
    }
    win[i] = m_pow2_cordic(accugsStart,scale);                                                     move32();
  }

}




/*!
  \brief  Calculate weighted spectrum for LPC calculation
*/
static void CalcWeightedSpectrum(const Word32  spectrum[],         /*!< input spectrum */
                                 Word16        weightedSpectrum[],
                                 Word32       *sfbEnergy,          /*!< sfb energies */
                                 const Word16 *sfbOffset,
                                 Word16        lpcStartLine,
                                 Word16        lpcStopLine,
                                 Word16        lpcStartBand,
                                 Word16        lpcStopBand,
                                 Word32       *pWork32)
{
    #define INT_BITS_SCAL 1<<(INT_BITS/2)

    Word16 i, sfb;
    Word16 maxShift;
    Word16 tmp_s, tmp2_s;
    Word32 tmp, tmp2;
    Word32 maxWS;
    Word32 tnsSfbMean[MAX_SFB];    /* length [lpcStopBand-lpcStartBand] should be sufficient here */

    maxWS = 0;                                                                                  move32();
  
    /* calc 1.0*2^-INT_BITS/2/sqrt(en) */
    for( sfb = lpcStartBand; sfb < lpcStopBand; sfb++) {

      tmp2 = L_sub( sfbEnergy[sfb], 2 );                                                           test();
      if( tmp2 > 0) {
        tmp = ffr_sqrt(sfbEnergy[sfb], INT_BITS);
        tmp = ffr_div32_32( INT_BITS_SCAL, tmp ); 
      }
      else {
        tmp = 0x7fffffff;                                                                          move32();
      } 
      tnsSfbMean[sfb] = tmp;                                                                       move32();
    }

    /* spread normalized values from sfbs to lines */
    sfb = lpcStartBand;                                                                            move16();
    tmp = tnsSfbMean[sfb];                                                                         move32();
    for ( i=lpcStartLine; i<lpcStopLine; i++){
      tmp_s = sub( sfbOffset[add(sfb,1)], i );                                                     test();
      if ( tmp_s == 0 ) {
        sfb = add(sfb,1);
        tmp2_s = sub(add(sfb,1),lpcStopBand);                                                      test();
        if (tmp2_s <= 0) {
          tmp = tnsSfbMean[sfb];                                                                   move32();
        }
      }
      pWork32[i] = tmp;                                                                   move32();
    }
    /*filter down*/
    for (i=sub(lpcStopLine,2); i>=lpcStartLine; i--){
        pWork32[i] = L_add(L_shr(pWork32[i],1), L_shr(pWork32[add(i,1)],1));
    }
    /* filter up */
    for (i=add(lpcStartLine,1); i<lpcStopLine; i++){
       pWork32[i] = L_add(L_shr(pWork32[i],1), L_shr(pWork32[sub(i,1)],1));
    }

    /* weight and normalize */
    for (i=lpcStartLine; i<lpcStopLine; i++){
      pWork32[i] = fixmul(pWork32[i], spectrum[i]);                              move32();
      maxWS |= L_abs(pWork32[i]);                                                         logic32();
    }
    maxShift = ffr_norm32(maxWS);
    for (i=lpcStartLine; i<lpcStopLine; i++){
      weightedSpectrum[i] = extract_h(L_shl(pWork32[i],maxShift));
    }
}




/*****************************************************************************

    functionname: CalcTnsFilter
    description:  LPC calculation for one TNS filter
    returns:      prediction gain
    input:        signal spectrum, acf window, no. of spectral lines,
                  max. TNS order, ptr. to reflection ocefficients
    output:       reflection coefficients

*****************************************************************************/
/*(half) window size must be larger than tnsOrder !!*/
static Word16 CalcTnsFilter(const Word16 *signal,
                            const Word32 window[],
                            Word16 numOfLines,
                            Word16 tnsOrder,
                            Word32 parcor[])
{
  Word32 parcorWorkBuffer[2*TNS_MAX_ORDER+1];
  Word16 predictionGain;
  Word16 i;
  Word16 tnsOrderPlus1 = add(tnsOrder, 1);

  assert(tnsOrder <= TNS_MAX_ORDER);                                                               /* remove asserts later? (btg) */

  for(i=0;i<tnsOrder;i++) {
    parcor[i] = 0;                              move32();
  }

  AutoCorrelation(signal, parcorWorkBuffer, numOfLines, tnsOrderPlus1);

  /* early return if signal is very low: signal prediction off, with zero parcor coeffs */
  if (parcorWorkBuffer[0] == 0)
    return 0;

                                                                                                   test();
  if(window) {
    for(i=0; i<tnsOrderPlus1; i++) {
      parcorWorkBuffer[i] = fixmul(parcorWorkBuffer[i], window[i]);
    }
  }

  predictionGain = AutoToParcor(parcorWorkBuffer, parcor, tnsOrder);

  return(predictionGain);
}

/*****************************************************************************

    functionname: AutoCorrelation
    description:  calc. autocorrelation (acf)
    returns:      -
    input:        input values, no. of input values, no. of acf values
    output:       acf values

*****************************************************************************/
static void AutoCorrelation(const Word16 input[],
                            Word32       corr[],
                            Word16       samples,
                            Word16       corrCoeff) {
  Word16 i, j;
  Word32 accu;
  Word16 scf;

  scf = 10;                                                                                     move16();

  /* calc first corrCoef:  R[0] = sum { t[i] * t[i] } ; i = 0..N-1 */
  accu = 0;                                                                                     move32();
  for(j=0; j<samples; j++) {
    accu = L_add(accu, L_shr(L_mult(input[j], input[j]), scf));
  }
  corr[0] = accu;                                                                               move32();

  /* early termination if all corr coeffs are likely going to be zero */
  if(corr[0] == 0) return ;

  /* calc all other corrCoef:  R[j] = sum { t[i] * t[i+j] } ; i = 0..(N-j-1), j=1..p */
  for(i=1; i<corrCoeff; i++) {
    samples = sub(samples, 1);
    accu = 0;                                                                                   move32();
    for(j=0; j<samples; j++) {
      accu = L_add(accu, L_shr(L_mult(input[j], input[j+i]), scf));
    }
    corr[i] = accu;                                                                             move32();
  }
}    


/*****************************************************************************

    functionname: AutoToParcor
    description:  conversion autocorrelation to reflection coefficients
    returns:      prediction gain
    input:        <order+1> input values, no. of output values (=order),
                  ptr. to workbuffer (required size: 2*order)
    output:       <order> reflection coefficients

*****************************************************************************/
static Word16 AutoToParcor(Word32 workBuffer[], Word32 reflCoeff[], Word16 numOfCoeff) {

  Word16 i, j;
  Word32 *pWorkBuffer; /* temp pointer */
  Word16 predictionGain = 0;
  Word32 num, denom;
  Word32 temp;
  move16();

  num = workBuffer[0];                                                                          move32();
  temp = workBuffer[numOfCoeff];                                                                move32();

  for(i=0; i<numOfCoeff-1; i++) {
    workBuffer[add(i,numOfCoeff)] = workBuffer[add(i,1)];                                       move32();
  }
  workBuffer[add(i,numOfCoeff)] = temp;                                                         move32();                 

  for(i=0; i<numOfCoeff; i++) {
    Word32 refc;

    test();
    if (L_sub(workBuffer[0], L_abs(workBuffer[add(i,numOfCoeff)])) < 0) {
      return 0 ;
    }

    /* calculate refc = -workBuffer[numOfCoeff+i] / workBuffer[0]; -1 <= refc < 1 */
    refc = L_negate(ffr_div32_32(workBuffer[numOfCoeff + i], workBuffer[0]));

    reflCoeff[i] = refc;                                                                           move32();

    pWorkBuffer = &(workBuffer[numOfCoeff]);                                                       move32();

    for(j=i; j<numOfCoeff; j++) {
      Word32 accu1, accu2;
      accu1 = L_add(pWorkBuffer[j], fixmul(refc, workBuffer[sub(j,i)]));
      accu2 = L_add(workBuffer[sub(j,i)], fixmul(refc, pWorkBuffer[j]));
      pWorkBuffer[j] = accu1;                                                                      move32();
      workBuffer[sub(j,i)] = accu2;                                                                move32();
    }
  }

  denom = fixmul(workBuffer[0], 0x0147ae14);
  test();
  if (denom != 0) {
    Word32 temp = ffr_div32_32(1, denom);
    predictionGain = extract_l(fixmul(num, temp));
  }

  return(predictionGain);
}



static Word16 Search3(Word32 parcor)
{
  Word16 index = 0;
  Word16 i;
  Word32 temp;
  move16();

  for (i=0;i<8;i++) {
    temp = L_sub( parcor, tnsCoeff3Borders[i]);                                                    test();
    if (temp > 0)
      index=i;                                                                                     move16();
  }
  return (sub(index,4));
}

static Word16 Search4(Word32 parcor)
{
  Word16 index = 0;
  Word16 i;
  Word32 temp;
  move16();

  for (i=0;i<16;i++) {
    temp = L_sub(parcor, tnsCoeff4Borders[i]);                                                     test();
    if (temp > 0)
      index=i;                                                                                     move16();
  }
  return(sub(index,8));
}



/*****************************************************************************

    functionname: Parcor2Index

*****************************************************************************/
static void Parcor2Index(const Word32 parcor[],   /*!< parcor coefficients */
                         Word16 index[],          /*!< quantized coeff indices */
                         Word16 order,            /*!< filter order */
                         Word16 bitsPerCoeff) {   /*!< quantizer resolution */
  Word16 i;
  Word16 temp;

  for(i=0; i<order; i++) {
    temp = sub(bitsPerCoeff, 3);                                                                   test();
    if (temp == 0) {
      index[i] = Search3(parcor[i]);                                                               move16();
    } 
    else {
      index[i] = Search4(parcor[i]);                                                               move16();
    }
  }
}


/*!
  \brief  Inverse quantization for reflection coefficients
*/
static void Index2Parcor(const Word16 index[],  /*!< quantized values */
                         Word32 parcor[],       /*!< ptr. to reflection coefficients (output) */
                         Word16 order,          /*!< no. of coefficients */
                         Word16 bitsPerCoeff)   /*!< quantizer resolution */
{
  Word16 i;
  Word16 temp;

  for (i=0; i<order; i++) {
    temp = sub(bitsPerCoeff,4);                                                                    test();
    if ( temp == 0 ) {
        parcor[i] = tnsCoeff4[add(index[i],8)];                                                    move32();
    }
    else {
        parcor[i] = tnsCoeff3[add(index[i],4)];                                                    move32();
    }
  }
}


/*!
  \brief  in place lattice filtering of spectral data
  \return pointer to modified data
*/
static Word32 FIRLattice(Word16 order,           /*!< filter order */
                         Word32 x,               /*!< spectral data */
                         Word32 *state_par,      /*!< filter states */
                         const Word32 *coef_par) /*!< filter coefficients */
{
   Word16 i;
   Word32 accu,tmp,tmpSave;

   x = L_shr(x, 1);
   tmpSave = x;                                                                                    move32();

   for (i=0; i<sub(order,1); i++) {

     tmp = L_add(fixmul(coef_par[i], x), state_par[i]);
     x   = L_add(fixmul(coef_par[i], state_par[i]), x);

     state_par[i] = tmpSave;                                                                       move32();
     tmpSave = tmp;                                                                                move32();
  }

  /* last stage: only need half operations */
  accu = fixmul(state_par[sub(order,1)], coef_par[sub(order,1)]);
  state_par[sub(order,1)] = tmpSave;                                                               move32();

  x = L_add(accu, x);
  x = L_shl(x, 1);

  return x;
}



/*!
  \brief   filters spectral lines with TNS filter
  \return  nothing
*/
static void AnalysisFilterLattice(const  Word32 signal[],  /*!< input spectrum */
                                  Word16 numOfLines,       /*!< no. of lines */
                                  const  Word32 parCoeff[],/*!< PARC coefficients */
                                  Word16 order,            /*!< filter order */
                                  Word32 output[])         /*!< filtered signal values */
{

  Word32 state_par[TNS_MAX_ORDER];
  Word16 j;

  for ( j=0; j<TNS_MAX_ORDER; j++ ) {
    state_par[j] = 0;                                                                              move32();
  }

  for(j=0; j<numOfLines; j++) {
    output[j] = FIRLattice(order,signal[j],state_par,parCoeff);                                    move32();
  }
}


/*!
  \brief  Change thresholds according to tns
*/
void ApplyTnsMultTableToRatios(Word16 startCb,
                               Word16 stopCb,
                               TNS_SUBBLOCK_INFO subInfo, /*!< TNS subblock info */
                               Word32 *thresholds)        /*!< thresholds (modified) */
{
  Word16 i;                                                                                        test();
  if (subInfo.tnsActive) {
    for(i=startCb; i<stopCb; i++) {
      /* thresholds[i] * 0.25 */
      thresholds[i] = L_shr(thresholds[i], 2);                                                     move32();
    }
  }
}
