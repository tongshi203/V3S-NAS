/*
   Quantizing & coding
 */
#include <stdlib.h>
#include <string.h>

#include "qc_main.h"
#include "quantize.h"
#include "interface.h"
#include "adj_thr.h"
#include "sf_estim.h"
#include "stat_bits.h"
#include "bit_cnt.h"
#include "dyn_bits.h"
#include "channel_map.h"
#include "aac_ram.h"
#include "count.h"


typedef enum{
  FRAME_LEN_BYTES_MODULO =  1,
  FRAME_LEN_BYTES_INT    =  2
}FRAME_LEN_RESULT_MODE;

static const Word16 maxFillElemBits = 7 + 270*8;

/* forward declarations */

static Word16 calcMaxValueInSfb(Word16 sfbCnt,
                                Word16 maxSfbPerGroup,
                                Word16 sfbPerGroup,
                                Word16 sfbOffset[MAX_GROUPED_SFB],
                                Word16 quantSpectrum[FRAME_LEN_LONG],
                                UWord16 maxValue[MAX_GROUPED_SFB]);


/*****************************************************************************

    functionname: calcFrameLen
    description:
    returns:
    input:
    output:

*****************************************************************************/
static Word16 calcFrameLen(Word32 bitRate,
                           Word32 sampleRate,
                           FRAME_LEN_RESULT_MODE mode)
{

  Word32 result;
  Word32 quot;

  result = ffr_Integer_Mult(L_shr(FRAME_LEN_LONG,3), bitRate);
  quot = ffr_divideWord32(result, sampleRate);

  test();
  if (sub(mode, FRAME_LEN_BYTES_MODULO) == 0) {
    result = L_sub(result, ffr_Integer_Mult(quot, sampleRate));
  }
  else { /* FRAME_LEN_BYTES_INT */
    result = quot;                                      move32();
  }

  return extract_l(result);
}

/*****************************************************************************

    functionname:framePadding
    description: Calculates if padding is needed for actual frame
    returns:
    input:
    output:

*****************************************************************************/
static Word16 framePadding(Word32 bitRate,
                           Word32 sampleRate,
                           Word32 *paddingRest)
{
  Word16 paddingOn;
  Word16 difference;

  paddingOn = 0;                                                move16();

  difference = calcFrameLen( bitRate,
                             sampleRate,
                             FRAME_LEN_BYTES_MODULO );
  *paddingRest = L_sub(*paddingRest, difference);

  test();
  if (*paddingRest <= 0 ) {
    paddingOn = 1;                                              move16();
    *paddingRest = L_add(*paddingRest, sampleRate);
  }

  return paddingOn;
}


/*********************************************************************************

         functionname: QCOutNew
         description:
         return:

**********************************************************************************/

Word16 QCOutNew(QC_OUT *hQC, Word16 nChannels)
{
  Word16 error;
  Word16 i, j;

  error = 0;                                                            move16();

  for (i=0; i<nChannels; i++) {
    hQC->qcChannel[i].quantSpec = &quantSpec[i*FRAME_LEN_LONG];
    
    hQC->qcChannel[i].maxValueInSfb = &maxValueInSfb[i*MAX_GROUPED_SFB];
    
    hQC->qcChannel[i].scf = &scf[i*MAX_GROUPED_SFB];
  }
 
  test();
  if (error) {
    QCOutDelete(hQC);
    hQC = 0;
  }
  return (hQC == 0);
}


/*********************************************************************************

         functionname: QCOutDelete
         description:
         return:

**********************************************************************************/
void QCOutDelete(QC_OUT* hQC)
{
  /* 
     nothing to do
  */
 
  hQC=NULL;
}

/*********************************************************************************

         functionname: QCNew
         description:
         return:

**********************************************************************************/
Word16 QCNew(QC_STATE *hQC)
{
  
  memset(hQC,0,sizeof(QC_STATE));

  return (0);
}

/*********************************************************************************

         functionname: QCDelete
         description:
         return:

**********************************************************************************/
void QCDelete(QC_STATE *hQC)
{
 
  /* 
     nothing to do
  */
  hQC=NULL;
}

/*********************************************************************************

         functionname: QCInit
         description:
         return:

**********************************************************************************/
Word16 QCInit(QC_STATE *hQC,
              struct QC_INIT *init)
{
  hQC->nChannels       = init->elInfo->nChannelsInEl;                   move16();
  hQC->maxBitsTot      = init->maxBits;                                 move16();
  hQC->bitResTot       = sub(init->bitRes, init->averageBits);
  hQC->averageBitsTot  = init->averageBits;                             move16();
  hQC->maxBitFac       = init->maxBitFac;                               move16();

  hQC->padding.paddingRest = init->padding.paddingRest;                 move32();

  hQC->globStatBits    = 3;                          /* for ID_END */   move16();

  InitElementBits(&hQC->elementBits,
                  *init->elInfo,
                  init->bitrate,
                  init->averageBits,
                  hQC->globStatBits);

  AdjThrInit(&hQC->adjThr,
             init->meanPe,
             hQC->elementBits.chBitrate);

  BCInit();

  return 0;
}


/*********************************************************************************

         functionname: QCMain
         description:
         return:

**********************************************************************************/
Word16 QCMain(QC_STATE* hQC,
              Word16 nChannels,
              ELEMENT_BITS* elBits,
              ATS_ELEMENT* adjThrStateElement,
              PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],  /* may be modified in-place */
              PSY_OUT_ELEMENT* psyOutElement,
              QC_OUT_CHANNEL  qcOutChannel[MAX_CHANNELS],    /* out                      */
              QC_OUT_ELEMENT* qcOutElement,
              Word16 ancillaryDataBytes)      
{
  Word16 ch;
  Word16 logSfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB];
  Word16 sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB];
  Word16 logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB];
  Word16 maxChDynBits[MAX_CHANNELS];
  Word16 chBitDistribution[MAX_CHANNELS];  


  test();
  if (elBits->bitResLevel < 0) {
    return -1;
  }
  test();
  if (sub(elBits->bitResLevel, elBits->maxBitResBits) > 0) {
    return -1;
  }

  qcOutElement->staticBitsUsed = countStaticBitdemand(psyOutChannel,
                                                      psyOutElement,
                                                      nChannels);

  test();
  if (ancillaryDataBytes) {
    qcOutElement->ancBitsUsed = add(7, shl(ancillaryDataBytes,3));
    test();
    if (sub(ancillaryDataBytes, 15) >= 0)
      qcOutElement->ancBitsUsed = add(qcOutElement->ancBitsUsed, 8);
  }
  else {
    qcOutElement->ancBitsUsed = 0; 
  }

  CalcFormFactor(logSfbFormFactor, sfbNRelevantLines, logSfbEnergy, psyOutChannel, nChannels);

  AdjustThresholds(&hQC->adjThr,
                   adjThrStateElement,
                   psyOutChannel,
                   psyOutElement,
                   chBitDistribution,
                   logSfbEnergy,
                   sfbNRelevantLines,
                   nChannels,
                   qcOutElement,
                   sub(elBits->averageBits, add(qcOutElement->staticBitsUsed, qcOutElement->ancBitsUsed)),
                   elBits->bitResLevel,
                   elBits->maxBits,
                   hQC->maxBitFac,
                   add(qcOutElement->staticBitsUsed, qcOutElement->ancBitsUsed));


  EstimateScaleFactors(psyOutChannel,
                       qcOutChannel,
                       logSfbEnergy,
                       logSfbFormFactor,
                       sfbNRelevantLines,
                       nChannels);


  /* condition to prevent empty bitreservoir */
  for (ch = 0; ch < nChannels; ch++) {
    Word16 maxDynBits;
    maxDynBits = sub(add(elBits->averageBits, elBits->bitResLevel), 7); /* -7 bec. of align bits */
    maxDynBits = sub(maxDynBits, add(qcOutElement->staticBitsUsed, qcOutElement->ancBitsUsed));
    maxChDynBits[ch] = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(chBitDistribution[ch], maxDynBits), 1000));
  }

  qcOutElement->dynBitsUsed = 0;                                        move16();
  for (ch = 0; ch < nChannels; ch++) {
    Word16 chDynBits;
    Flag   constraintsFulfilled;
    Word16 iter;
    iter = 0;                                                         move16();
    do {
      constraintsFulfilled = 1;                                       move16();

      QuantizeSpectrum(psyOutChannel[ch].sfbCnt,
                       psyOutChannel[ch].maxSfbPerGroup,
                       psyOutChannel[ch].sfbPerGroup,
                       psyOutChannel[ch].sfbOffsets,
                       psyOutChannel[ch].mdctSpectrum,
                       qcOutChannel[ch].globalGain,
                       qcOutChannel[ch].scf,
                       qcOutChannel[ch].quantSpec);

      test();
      if (sub(calcMaxValueInSfb(psyOutChannel[ch].sfbCnt,
                                psyOutChannel[ch].maxSfbPerGroup,
                                psyOutChannel[ch].sfbPerGroup,
                                psyOutChannel[ch].sfbOffsets,
                                qcOutChannel[ch].quantSpec,
                                qcOutChannel[ch].maxValueInSfb), MAX_QUANT) > 0) {
        constraintsFulfilled = 0;                                       move16();
      }

      chDynBits = dynBitCount(qcOutChannel[ch].quantSpec,
                              qcOutChannel[ch].maxValueInSfb,
                              qcOutChannel[ch].scf,
                              psyOutChannel[ch].windowSequence,
                              psyOutChannel[ch].sfbCnt,
                              psyOutChannel[ch].maxSfbPerGroup,
                              psyOutChannel[ch].sfbPerGroup,
                              psyOutChannel[ch].sfbOffsets,
                              &qcOutChannel[ch].sectionData);

      test();
      if (sub(chDynBits, maxChDynBits[ch]) >= 0) {
        constraintsFulfilled = 0;                                       move16();
      }

      test();
      if (!constraintsFulfilled) {
        qcOutChannel[ch].globalGain = add(qcOutChannel[ch].globalGain, 1);
      }

      iter = add(iter, 1);

      test();
    } while(!constraintsFulfilled);

    qcOutElement->dynBitsUsed = add(qcOutElement->dynBitsUsed, chDynBits);

    qcOutChannel[ch].mdctScale    = psyOutChannel[ch].mdctScale;        move16();
    qcOutChannel[ch].groupingMask = psyOutChannel[ch].groupingMask;     move16();
    qcOutChannel[ch].windowShape  = psyOutChannel[ch].windowShape;      move16();
  }

  /* save dynBitsUsed for correction of bits2pe relation */
  AdjThrUpdate(adjThrStateElement, qcOutElement->dynBitsUsed);

  {
    Word16 bitResSpace = sub(elBits->maxBitResBits, elBits->bitResLevel);
    Word16 deltaBitRes = sub(elBits->averageBits, 
                             add(qcOutElement->staticBitsUsed,
                                 add(qcOutElement->dynBitsUsed, qcOutElement->ancBitsUsed)));

    qcOutElement->fillBits = S_max(0, sub(deltaBitRes, bitResSpace));
  }

  return 0; /* OK */
}


/*********************************************************************************

         functionname: calcMaxValueInSfb
         description:
         return:

**********************************************************************************/

static Word16 calcMaxValueInSfb(Word16 sfbCnt,
                                Word16 maxSfbPerGroup,
                                Word16 sfbPerGroup,
                                Word16 sfbOffset[MAX_GROUPED_SFB],
                                Word16 quantSpectrum[FRAME_LEN_LONG],
                                UWord16 maxValue[MAX_GROUPED_SFB])
{
  Word16 sfbOffs, sfb;
  Word16 maxValueAll;

  maxValueAll = 0;                                      move16();

  for(sfbOffs=0;sfbOffs<sfbCnt;sfbOffs+=sfbPerGroup) {
    for (sfb = 0; sfb < maxSfbPerGroup; sfb++) {
      Word16 line;
      Word16 maxThisSfb;
      maxThisSfb = 0;                                   move16();

      for (line = sfbOffset[sfbOffs+sfb]; line < sfbOffset[sfbOffs+sfb+1]; line++) {
        Word16 absVal;
        absVal = abs_s(quantSpectrum[line]);
        maxThisSfb = S_max(maxThisSfb, absVal);
      }

      maxValue[sfbOffs+sfb] = maxThisSfb;               move16();
      maxValueAll = S_max(maxValueAll, maxThisSfb);
    }
  }
  return maxValueAll;
}


/*********************************************************************************

         functionname: updateBitres
         description:
         return:

**********************************************************************************/
void updateBitres(QC_STATE* qcKernel,
                  QC_OUT*   qcOut)
                  
{
  ELEMENT_BITS *elBits;
 
  qcKernel->bitResTot = 0;                              move16();

  elBits = &qcKernel->elementBits;

  test();
  if (elBits->averageBits > 0) {
    /* constant bitrate */
    Word16 bitsUsed;
    bitsUsed = add(add(qcOut->qcElement.staticBitsUsed, qcOut->qcElement.dynBitsUsed),
                   add(qcOut->qcElement.ancBitsUsed   , qcOut->qcElement.fillBits));
    elBits->bitResLevel = add(elBits->bitResLevel, sub(elBits->averageBits, bitsUsed));
    qcKernel->bitResTot = add(qcKernel->bitResTot, elBits->bitResLevel);
  }
  else {
    /* variable bitrate */
    elBits->bitResLevel = elBits->maxBits;          move16();
    qcKernel->bitResTot = qcKernel->maxBitsTot;     move16();
  }
}

/*********************************************************************************

         functionname: FinalizeBitConsumption
         description:
         return:

**********************************************************************************/
Word16 FinalizeBitConsumption(QC_STATE *qcKernel,
                              QC_OUT* qcOut)
{
  Word16 nFullFillElem, diffBits;
  Word16 totFillBits;
  Word16 bitsUsed;

  totFillBits = 0;                                      move16();

  qcOut->totStaticBitsUsed = qcKernel->globStatBits;    move16();
  qcOut->totDynBitsUsed = 0;                            move16();
  qcOut->totAncBitsUsed = 0;                            move16();
  qcOut->totFillBits=0;                                 move16();

  qcOut->totStaticBitsUsed = add(qcOut->totStaticBitsUsed, qcOut->qcElement.staticBitsUsed);
  qcOut->totDynBitsUsed    = add(qcOut->totDynBitsUsed,    qcOut->qcElement.dynBitsUsed);
  qcOut->totAncBitsUsed    = add(qcOut->totAncBitsUsed,    qcOut->qcElement.ancBitsUsed);
  qcOut->totFillBits       = add(qcOut->totFillBits,       qcOut->qcElement.fillBits);
  test();
  if (qcOut->qcElement.fillBits) {
    totFillBits = add(totFillBits, qcOut->qcElement.fillBits);
  }

  nFullFillElem = extract_l(ffr_divideWord32(S_max(sub(qcOut->totFillBits, 1), 0), maxFillElemBits));
  qcOut->totFillBits = sub(qcOut->totFillBits, ffr_Short_Mult(nFullFillElem, maxFillElemBits));

  /* check fill elements */
  test();
  if (qcOut->totFillBits > 0) {
    /* minimum Fillelement contains 7 (TAG + byte cnt) bits */
    qcOut->totFillBits = S_max(7, qcOut->totFillBits);
    /* fill element size equals n*8 + 7 */
    qcOut->totFillBits = add(qcOut->totFillBits, ((sub(8, ((sub(qcOut->totFillBits,7)) & 0x0007))) & 0x0007));  logic16(); logic16();
  }

  qcOut->totFillBits = add(qcOut->totFillBits, ffr_Short_Mult(nFullFillElem, maxFillElemBits));

  /* now distribute extra fillbits and alignbits over channel elements */
  qcOut->alignBits = sub(7, (sub(add(add(add(qcOut->totDynBitsUsed, qcOut->totStaticBitsUsed),
                                         qcOut->totAncBitsUsed), qcOut->totFillBits), 1)) & 0x0007);            logic16();

  test(); test();
  if ( (sub(sub(add(qcOut->alignBits,qcOut->totFillBits),totFillBits),8) == 0) &&
       (sub(qcOut->totFillBits,8) > 0) )
    qcOut->totFillBits = sub(qcOut->totFillBits, 8);

  test();
  diffBits = sub(add(qcOut->alignBits, qcOut->totFillBits), totFillBits);
  test();
  if(diffBits<0) {
  }
  else {
    qcOut->qcElement.fillBits = add(qcOut->qcElement.fillBits, diffBits);
  }

  bitsUsed = add(add(qcOut->totDynBitsUsed, qcOut->totStaticBitsUsed), qcOut->totAncBitsUsed);
  bitsUsed = add(add(bitsUsed, qcOut->totFillBits), qcOut->alignBits);
  test();
  if (sub(bitsUsed, qcKernel->maxBitsTot) > 0) {
    return -1;
  }
  return 0;
}


/*********************************************************************************

         functionname: AdjustBitrate
         description:  adjusts framelength via padding on a frame to frame basis,
                       to achieve a bitrate that demands a non byte aligned
                       framelength
         return:       errorcode

**********************************************************************************/
Word16 AdjustBitrate(QC_STATE        *hQC,
                     Word32           bitRate,    /* total bitrate */
                     Word32           sampleRate) /* output sampling rate */
{
  Word16 paddingOn;
  Word16 frameLen;
  Word16 codeBits;
  Word16 codeBitsLast;

  /* Do we need a extra padding byte? */
  paddingOn = framePadding(bitRate,
                           sampleRate,
                           &hQC->padding.paddingRest);

  frameLen = paddingOn + calcFrameLen(bitRate,
                                      sampleRate,
                                      FRAME_LEN_BYTES_INT);

  frameLen = shl(frameLen, 3);
  codeBitsLast = sub(hQC->averageBitsTot, hQC->globStatBits);
  codeBits     = sub(frameLen, hQC->globStatBits);

  /* calculate bits for every channel element */
  test();
  if (sub(codeBits, codeBitsLast) != 0) {
    Word16 totalBits = 0;                                      move16();

    hQC->elementBits.averageBits = extract_l(L_shr(L_mult(hQC->elementBits.relativeBits, codeBits), 15)); /* relativeBits was scaled down by 2 */
    totalBits = add(totalBits, hQC->elementBits.averageBits);

    hQC->elementBits.averageBits = add(hQC->elementBits.averageBits, sub(codeBits, totalBits));
  }
  hQC->averageBitsTot = frameLen;                       move16();

  return 0;
}
