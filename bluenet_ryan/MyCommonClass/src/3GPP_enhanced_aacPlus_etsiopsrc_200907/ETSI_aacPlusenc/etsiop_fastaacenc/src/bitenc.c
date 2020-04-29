/*
   Bitstream encoder
 */
#include <stdio.h>
#include <stdlib.h>

#include "bitenc.h"
#include "bit_cnt.h"
#include "dyn_bits.h"
#include "qc_data.h"
#include "interface.h"
#include "count.h"


#define GetBitsAvail(hB) (hB->cntBits)

static const  Word16 globalGainOffset = 100;
static const  Word16 icsReservedBit   = 0;


/*****************************************************************************

    functionname: encodeSpectralData
    description:  encode spectral data
    returns:      none
    input:
    output:

*****************************************************************************/
static Word32 encodeSpectralData(Word16             *sfbOffset,
                                 SECTION_DATA       *sectionData,
                                 Word16             *quantSpectrum,
                                 HANDLE_BIT_BUF      hBitStream)
{
  Word16 i,sfb;
  Word16 dbgVal;

  dbgVal = GetBitsAvail(hBitStream);                                    move16();

  for(i=0; i<sectionData->noOfSections; i++) {
    /*
       huffencode spectral data for this section
    */
    for(sfb=sectionData->sectionInfo[i].sfbStart;
        sfb<sectionData->sectionInfo[i].sfbStart+sectionData->sectionInfo[i].sfbCnt;
        sfb++) {
      codeValues(quantSpectrum+sfbOffset[sfb],
                 sub(sfbOffset[sfb+1],sfbOffset[sfb]),
                 sectionData->sectionInfo[i].codeBook,
                 hBitStream);
    }
  }

  return(sub(GetBitsAvail(hBitStream),dbgVal));
}

/*****************************************************************************

    functionname:encodeGlobalGain
    description: encodes Global Gain (common scale factor)
    returns:     none
    input:
    output:

*****************************************************************************/
static void encodeGlobalGain(Word16 globalGain,
                             Word16 logNorm,
                             Word16 scalefac,
                             HANDLE_BIT_BUF hBitStream)
{
  WriteBits(hBitStream, add(sub(globalGain,scalefac),sub(globalGainOffset,shl(logNorm,2))), 8);
}


/*****************************************************************************

    functionname:encodeIcsInfo
    description: encodes Ics Info
    returns:     none
    input:
    output:

*****************************************************************************/

static void encodeIcsInfo(Word16 blockType,
                          Word16 windowShape,
                          Word16 groupingMask,
                          SECTION_DATA *sectionData,
                          HANDLE_BIT_BUF  hBitStream)
{
  WriteBits(hBitStream,icsReservedBit,1);
  WriteBits(hBitStream,blockType,2);
  WriteBits(hBitStream,windowShape,1);

  test();
  switch(blockType){
    case LONG_WINDOW:
    case START_WINDOW:
    case STOP_WINDOW:
      WriteBits(hBitStream,sectionData->maxSfbPerGroup,6);

      /* No predictor data present */
      WriteBits(hBitStream, 0, 1);
      break;

    case SHORT_WINDOW:
      WriteBits(hBitStream,sectionData->maxSfbPerGroup,4);

      /*
      Write grouping bits
      */
      WriteBits(hBitStream,groupingMask,TRANS_FAC-1);
      break;
  }
}

/*****************************************************************************

    functionname: encodeSectionData
    description:  encode section data (common Huffman codebooks for adjacent
                  SFB's)
    returns:      none
    input:
    output:

*****************************************************************************/
static Word32 encodeSectionData(SECTION_DATA *sectionData,
                                HANDLE_BIT_BUF hBitStream)
{
  Word16 sectEscapeVal=0,sectLenBits=0;
  Word16 sectLen;
  Word16 i;
  Word16 dbgVal=GetBitsAvail(hBitStream);
  move16(); move16(); move16();

  test();
  switch(sectionData->blockType)
  {
    case LONG_WINDOW:
    case START_WINDOW:
    case STOP_WINDOW:
      sectEscapeVal = SECT_ESC_VAL_LONG;                move16();
      sectLenBits   = SECT_BITS_LONG;                   move16();
      break;

    case SHORT_WINDOW:
      sectEscapeVal = SECT_ESC_VAL_SHORT;               move16();
      sectLenBits   = SECT_BITS_SHORT;                  move16();
      break;
  }

  for(i=0;i<sectionData->noOfSections;i++) {
    WriteBits(hBitStream,sectionData->sectionInfo[i].codeBook,4);
    sectLen = sectionData->sectionInfo[i].sfbCnt;       move16();

    while(sub(sectLen, sectEscapeVal) >= 0) {
      test();
      WriteBits(hBitStream,sectEscapeVal,sectLenBits);
      sectLen = sub(sectLen, sectEscapeVal);
    }
    WriteBits(hBitStream,sectLen,sectLenBits);
  }
  return(sub(GetBitsAvail(hBitStream),dbgVal));
}

/*****************************************************************************

    functionname: encodeScaleFactorData
    description:  encode DPCM coded scale factors
    returns:      none
    input:
    output:

*****************************************************************************/
static Word32 encodeScaleFactorData(UWord16        *maxValueInSfb,
                                    SECTION_DATA   *sectionData,
                                    Word16         *scalefac,
                                    HANDLE_BIT_BUF  hBitStream)
{
  Word16 i,j,lastValScf,deltaScf;
  Word16 dbgVal = GetBitsAvail(hBitStream);
  move16();

  lastValScf=scalefac[sectionData->firstScf];                   move16();

  for(i=0;i<sectionData->noOfSections;i++){
    test();
    if (sub(sectionData->sectionInfo[i].codeBook, CODE_BOOK_ZERO_NO) != 0){
      for (j=sectionData->sectionInfo[i].sfbStart;
           j<add(sectionData->sectionInfo[i].sfbStart,sectionData->sectionInfo[i].sfbCnt);
           j++){
        test();
        if(maxValueInSfb[j] == 0) {
          deltaScf = 0;                                         move16();
        }
        else {
          deltaScf = sub(lastValScf, scalefac[j]);
          lastValScf = scalefac[j];                             move16();
        }
        test();
        if(codeScalefactorDelta(deltaScf,hBitStream)){
          return(1);
        }
      }
    }

  }
  return(sub(GetBitsAvail(hBitStream),dbgVal));
}

/*****************************************************************************

    functionname:encodeMsInfo
    description: encodes MS-Stereo Info
    returns:     none
    input:
    output:

*****************************************************************************/
static void encodeMSInfo(Word16          sfbCnt,
                         Word16          grpSfb,
                         Word16          maxSfb,
                         Word16          msDigest,
                         Word16         *jsFlags,
                         HANDLE_BIT_BUF  hBitStream)
{
  Word16 sfb, sfbOff;

  test();
  switch(msDigest)
  {
    case MS_NONE:
      WriteBits(hBitStream,SI_MS_MASK_NONE,2);
      break;

    case MS_ALL:
      WriteBits(hBitStream,SI_MS_MASK_ALL,2);
      break;

    case MS_SOME:
      WriteBits(hBitStream,SI_MS_MASK_SOME,2);
      for(sfbOff = 0; sfbOff < sfbCnt; sfbOff+=grpSfb) {
        for(sfb=0; sfb<maxSfb; sfb++) {
          test(); logic16();
          if(jsFlags[sfbOff+sfb] & MS_ON) {
            WriteBits(hBitStream,1,1);
          }
          else{
            WriteBits(hBitStream,0,1);
          }
        }
      }
      break;
  }

}

/*****************************************************************************

    functionname: encodeTnsData
    description:  encode TNS data (filter order, coeffs, ..)
    returns:      none
    input:
    output:

*****************************************************************************/
static void encodeTnsData(TNS_INFO tnsInfo,
                          Word16 blockType,
                          HANDLE_BIT_BUF hBitStream) {
  Word16 i,k;
  Flag tnsPresent;
  Word16 numOfWindows;
  Word16 coefBits;
  Flag isShort;

  test(); move16(); move16();
  if (sub(blockType,2) == 0) {
    isShort = 1;
    numOfWindows = TRANS_FAC;
  }
  else {
    isShort = 0;
    numOfWindows = 1;
  }

  tnsPresent=0;                                                 move16();
  for (i=0; i<numOfWindows; i++) {
    test();
    if (tnsInfo.tnsActive[i]) {
      tnsPresent=1;                                             move16();
    }
  }
  test();
  if (tnsPresent==0) {
    WriteBits(hBitStream,0,1);
  }
  else{ /* there is data to be written*/
    WriteBits(hBitStream,1,1); /*data_present */
    for (i=0; i<numOfWindows; i++) {
      test();
      WriteBits(hBitStream,tnsInfo.tnsActive[i],(isShort?1:2));
      test();
      if (tnsInfo.tnsActive[i]) {
        test();
        WriteBits(hBitStream,(sub(tnsInfo.coefRes[i],4)==0?1:0),1);
        test();
        WriteBits(hBitStream,tnsInfo.length[i],(isShort?4:6));
        test();
        WriteBits(hBitStream,tnsInfo.order[i],(isShort?3:5));
        test();
        if (tnsInfo.order[i]){
          WriteBits(hBitStream, FILTER_DIRECTION, 1);
          test();
          if(sub(tnsInfo.coefRes[i], 4) == 0) {
            coefBits = 3;                                               move16();
            for(k=0; k<tnsInfo.order[i]; k++) {
              test(); test();
              if (sub(tnsInfo.coef[i*TNS_MAX_ORDER_SHORT+k], 3) > 0 ||
                  sub(tnsInfo.coef[i*TNS_MAX_ORDER_SHORT+k], -4) < 0) {
                coefBits = 4;                                           move16();
                break;
              }
            }
          }
          else {
            coefBits = 2;                                               move16();
            for(k=0; k<tnsInfo.order[i]; k++) {
              test(); test();
              if (sub(tnsInfo.coef[i*TNS_MAX_ORDER_SHORT+k], 1) > 0 ||
                  sub(tnsInfo.coef[i*TNS_MAX_ORDER_SHORT+k], -2) < 0) {
                coefBits = 3;                                           move16();
                break;
              }
            }
          }
          WriteBits(hBitStream,sub(tnsInfo.coefRes[i], coefBits),1); /*coef_compres*/
          for (k=0; k<tnsInfo.order[i]; k++ ) {
            static const Word16 rmask[] = {0,1,3,7,15};
            logic16();
            WriteBits(hBitStream,tnsInfo.coef[i*TNS_MAX_ORDER_SHORT+k] & rmask[coefBits],coefBits);
          }
        }
      }
    }
  }

}

/*****************************************************************************

    functionname: encodeGainControlData
    description:  unsupported
    returns:      none
    input:
    output:

*****************************************************************************/
static void encodeGainControlData(HANDLE_BIT_BUF hBitStream)
{
  WriteBits(hBitStream,0,1);
}

/*****************************************************************************

    functionname: encodePulseData
    description:  not supported yet (dummy)
    returns:      none
    input:
    output:

*****************************************************************************/
static void encodePulseData(HANDLE_BIT_BUF hBitStream)
{
  WriteBits(hBitStream,0,1);
}


/*****************************************************************************

    functionname: WriteIndividualChannelStream
    description:  management of write process of individual channel stream
    returns:      none
    input:
    output:

*****************************************************************************/
static void
writeIndividualChannelStream(Flag   commonWindow,
                             Word16 mdctScale,
                             Word16 windowShape,
                             Word16 groupingMask,
                             Word16 *sfbOffset,
                             Word16 scf[],
                             UWord16 *maxValueInSfb,
                             Word16 globalGain,
                             Word16 quantSpec[],
                             SECTION_DATA *sectionData,
                             HANDLE_BIT_BUF hBitStream,
                             TNS_INFO tnsInfo)
{
  Word16 logNorm;

  logNorm = sub(LOG_NORM_PCM, add(mdctScale,1));

  encodeGlobalGain(globalGain, logNorm,scf[sectionData->firstScf], hBitStream);

  test();
  if(!commonWindow) {
    encodeIcsInfo(sectionData->blockType, windowShape, groupingMask, sectionData, hBitStream);
  }

  encodeSectionData(sectionData, hBitStream);

  encodeScaleFactorData(maxValueInSfb,
                        sectionData,
                        scf,
                        hBitStream);

  encodePulseData(hBitStream);

  encodeTnsData(tnsInfo, sectionData->blockType, hBitStream);

  encodeGainControlData(hBitStream);

  encodeSpectralData(sfbOffset,
                     sectionData,
                     quantSpec,
                     hBitStream);

}

/*****************************************************************************

    functionname: writeSingleChannelElement
    description:  write single channel element to bitstream
    returns:      none
    input:
    output:

*****************************************************************************/
static Word16 writeSingleChannelElement(Word16 instanceTag,
                                        Word16 *sfbOffset,
                                        QC_OUT_CHANNEL* qcOutChannel,
                                        HANDLE_BIT_BUF hBitStream,
                                        TNS_INFO tnsInfo)
{
  WriteBits(hBitStream,ID_SCE,3);
  WriteBits(hBitStream,instanceTag,4);
  writeIndividualChannelStream(0,
                               qcOutChannel->mdctScale,
                               qcOutChannel->windowShape,
                               qcOutChannel->groupingMask,
                               sfbOffset,
                               qcOutChannel->scf,
                               qcOutChannel->maxValueInSfb,
                               qcOutChannel->globalGain,
                               qcOutChannel->quantSpec,
                               &(qcOutChannel->sectionData),
                               hBitStream,
                               tnsInfo
                               );
  return(0);
}



/*****************************************************************************

    functionname: writeChannelPairElement
    description:
    returns:      none
    input:
    output:

*****************************************************************************/
static Word16 writeChannelPairElement(Word16 instanceTag,
                                      Word16 msDigest,
                                      Word16 msFlags[MAX_GROUPED_SFB],
                                      Word16 *sfbOffset[2],
                                      QC_OUT_CHANNEL qcOutChannel[2],
                                      HANDLE_BIT_BUF hBitStream,
                                      TNS_INFO tnsInfo[2])
{
  WriteBits(hBitStream,ID_CPE,3);
  WriteBits(hBitStream,instanceTag,4);
  WriteBits(hBitStream,1,1); /* common window */

  encodeIcsInfo(qcOutChannel[0].sectionData.blockType,
                qcOutChannel[0].windowShape,
                qcOutChannel[0].groupingMask,
                &(qcOutChannel[0].sectionData),
                hBitStream);

  encodeMSInfo(qcOutChannel[0].sectionData.sfbCnt,
               qcOutChannel[0].sectionData.sfbPerGroup,
               qcOutChannel[0].sectionData.maxSfbPerGroup,
               msDigest,
               msFlags,
               hBitStream);

  writeIndividualChannelStream(1,
                               qcOutChannel[0].mdctScale,
                               qcOutChannel[0].windowShape,
                               qcOutChannel[0].groupingMask,
                               sfbOffset[0],
                               qcOutChannel[0].scf,
                               qcOutChannel[0].maxValueInSfb,
                               qcOutChannel[0].globalGain,
                               qcOutChannel[0].quantSpec,
                               &(qcOutChannel[0].sectionData),
                               hBitStream,
                               tnsInfo[0]);

  writeIndividualChannelStream(1,
                               qcOutChannel[1].mdctScale,
                               qcOutChannel[1].windowShape,
                               qcOutChannel[1].groupingMask,
                               sfbOffset[1],
                               qcOutChannel[1].scf,
                               qcOutChannel[1].maxValueInSfb,
                               qcOutChannel[1].globalGain,
                               qcOutChannel[1].quantSpec,
                               &(qcOutChannel[1].sectionData),
                               hBitStream,
                               tnsInfo[1]);

  return(0);
}



/*****************************************************************************

    functionname: writeFillElement
    description:  write fill elements to bitstream
    returns:      none
    input:
    output:

*****************************************************************************/
static void writeFillElement( const UWord8 *ancBytes,
                              Word16 totFillBits,
                              HANDLE_BIT_BUF hBitStream)
{
  Word16 i;
  Word16 cnt,esc_count;

  /*
    Write fill Element(s):
    amount of a fill element can be 7+X*8 Bits, X element of [0..270]
  */
    
  while(sub(totFillBits,(3+4)) >= 0) {
    cnt = S_min( shr(sub(totFillBits,(3+4)),3), ((1<<4)-1));

    WriteBits(hBitStream,ID_FIL,3);
    WriteBits(hBitStream,cnt,4);

    totFillBits = sub(totFillBits, (3+4));

    test();
    if (sub(cnt,(1<<4)-1) == 0) {

      esc_count = S_min( sub(shr(totFillBits,3), ((1<<4)-1)), (1<<8)-1);
      WriteBits(hBitStream,esc_count,8);
      totFillBits = sub(totFillBits, 8);
      cnt = add(cnt, sub(esc_count,1));
    }

    for(i=0;i<cnt;i++) {
      test();
      if(ancBytes)
        WriteBits(hBitStream, *ancBytes++,8);
      else
        WriteBits(hBitStream,0,8);
      totFillBits = sub(totFillBits, 8);
    }
  }
}

/*****************************************************************************

    functionname: WriteBitStream
    description:  main function of write process
    returns:
    input:
    output:

*****************************************************************************/
Word16 WriteBitstream (HANDLE_BIT_BUF hBitStream,
                       ELEMENT_INFO elInfo,
                       QC_OUT *qcOut,
                       PSY_OUT *psyOut,
                       Word16 *globUsedBits,
                       const UWord8 *ancBytes
                       ) /* returns error code */
{
  Word16 bitMarkUp;
  Word16 elementUsedBits;
  Word16 frameBits=0;
  move16();

  /*   struct bitbuffer bsWriteCopy; */

  bitMarkUp = GetBitsAvail(hBitStream);                         move16();

  *globUsedBits=0;                                              move16();

  {

    Word16 *sfbOffset[2];
    TNS_INFO tnsInfo[2];
    elementUsedBits = 0;                                        move16();

    switch (elInfo.elType) {

      case ID_SCE:      /* single channel */
        sfbOffset[0] = psyOut->psyOutChannel[elInfo.ChannelIndex[0]].sfbOffsets;
        tnsInfo[0] = psyOut->psyOutChannel[elInfo.ChannelIndex[0]].tnsInfo;

        writeSingleChannelElement(elInfo.instanceTag,
                                  sfbOffset[0],
                                  &qcOut->qcChannel[elInfo.ChannelIndex[0]],
                                  hBitStream,
                                  tnsInfo[0]);
        break;

      case ID_CPE:     /* channel pair */
        {
          Word16 msDigest;
          Word16 *msFlags = psyOut->psyOutElement.toolsInfo.msMask;
          msDigest = psyOut->psyOutElement.toolsInfo.msDigest;                       move16();
          sfbOffset[0] =
            psyOut->psyOutChannel[elInfo.ChannelIndex[0]].sfbOffsets;
          sfbOffset[1] =
            psyOut->psyOutChannel[elInfo.ChannelIndex[1]].sfbOffsets;

          tnsInfo[0]=
            psyOut->psyOutChannel[elInfo.ChannelIndex[0]].tnsInfo;
          tnsInfo[1]=
            psyOut->psyOutChannel[elInfo.ChannelIndex[1]].tnsInfo;
          writeChannelPairElement(elInfo.instanceTag,
                                  msDigest,
                                  msFlags,
                                  sfbOffset,
                                  &qcOut->qcChannel[elInfo.ChannelIndex[0]],
                                  hBitStream,
                                  tnsInfo);
        }
        break;

      default:
        return(1);

      }   /* switch */

    elementUsedBits = sub(elementUsedBits, bitMarkUp);
    bitMarkUp = GetBitsAvail(hBitStream);
    frameBits = add(frameBits, add(elementUsedBits, bitMarkUp));

  }

  /*
    ancillary data
  */
  writeFillElement(ancBytes,
                   qcOut->totAncBitsUsed, 
                   hBitStream);
  

  writeFillElement(NULL,
                   qcOut->totFillBits, 
                   hBitStream);

  WriteBits(hBitStream,ID_END,3);

  /* byte alignement */

  WriteBits(hBitStream,0, sub(8, (hBitStream->cntBits & 7)) & 7);       logic16(); logic16();
  
  *globUsedBits = sub(*globUsedBits, bitMarkUp);
  bitMarkUp = GetBitsAvail(hBitStream);                                 move16();
  *globUsedBits = add(*globUsedBits, bitMarkUp);
  frameBits = add(frameBits, *globUsedBits);

  test();
  if (sub(frameBits, add(add(qcOut->totStaticBitsUsed+qcOut->totDynBitsUsed, qcOut->totAncBitsUsed),
                      add(qcOut->totFillBits,qcOut->alignBits))) != 0) {
    return(-1);
  }
  return(0);
}
