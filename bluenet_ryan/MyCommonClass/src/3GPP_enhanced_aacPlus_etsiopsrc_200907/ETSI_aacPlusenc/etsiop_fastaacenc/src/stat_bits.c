/*
   Static bit counter
 */
#include "stat_bits.h"
#include "bitenc.h"
#include "tns.h"
#include "count.h"


typedef enum {
  SI_ID_BITS                =(3),
  SI_FILL_COUNT_BITS        =(4),
  SI_FILL_ESC_COUNT_BITS    =(8),
  SI_FILL_EXTENTION_BITS    =(4),
  SI_FILL_NIBBLE_BITS       =(4),
  SI_SCE_BITS               =(4),
  SI_CPE_BITS               =(5),
  SI_CPE_MS_MASK_BITS       =(2) ,
  SI_ICS_INFO_BITS_LONG     =(1+2+1+6+1),
  SI_ICS_INFO_BITS_SHORT    =(1+2+1+4+7),
  SI_ICS_BITS               =(8+1+1+1)
} SI_BITS;



static Word16 countMsMaskBits(Word16   sfbCnt,
                              Word16   sfbPerGroup,
                              Word16   maxSfbPerGroup,
                              struct TOOLSINFO *toolsInfo)
{
  Word16 msBits, sfbOff, sfb;
  msBits = 0;                                           move16();

  test();
  switch(toolsInfo->msDigest) {
    case MS_NONE:
    case MS_ALL:
      break;

    case MS_SOME:
      for(sfbOff=0; sfbOff<sfbCnt; sfbOff+=sfbPerGroup)
        for(sfb=0; sfb<maxSfbPerGroup; sfb++)
          msBits = add(msBits, 1);
      break;
  }
  return(msBits);
}


static Word16 tnsCount(TNS_INFO *tnsInfo, Word16 blockType)
{

  Word16 i, k;
  Flag tnsPresent;
  Word16 numOfWindows;
  Word16 count;
  Word16 coefBits;

  count = 0;                                            move16();

  test(); move16();
  if (sub(blockType,2) == 0)
    numOfWindows = 8;
  else
    numOfWindows = 1;
  tnsPresent = 0;                                       move16();

  for (i=0; i<numOfWindows; i++) {
    test();
    if (tnsInfo->tnsActive[i]!=0) {
      tnsPresent = 1;                                   move16();
    }
  }
  test();
  if (tnsPresent) {
    /* there is data to be written*/
    /*count += 1; */
    for (i=0; i<numOfWindows; i++) {
      test();
      if (sub(blockType,2) == 0)
        count = add(count, 1);
      else
        count = add(count, 2);
      test();
      if (tnsInfo->tnsActive[i]) {
        count = add(count, 1);
        test();
        if (sub(blockType,2) == 0) {
          count = add(count, 4);
          count = add(count, 3);
        }
        else {
          count = add(count, 6);
          count = add(count, 5);
        }
        test();
        if (tnsInfo->order[i]) {
          count = add(count, 1); /*direction*/
          count = add(count, 1); /*coef_compression */
          test();
          if (sub(tnsInfo->coefRes[i], 4) == 0) {
            coefBits = 3;                                       move16();
            for(k=0; k<tnsInfo->order[i]; k++) {
              test(); test();
              if (sub(tnsInfo->coef[i*TNS_MAX_ORDER_SHORT+k], 3) > 0 ||
                  sub(tnsInfo->coef[i*TNS_MAX_ORDER_SHORT+k], -4) < 0) {
                coefBits = 4;                                   move16();
                break;
              }
            }
          }
          else {
            coefBits = 2;                                       move16();
            for(k=0; k<tnsInfo->order[i]; k++) {
              test(); test();
              if (sub(tnsInfo->coef[i*TNS_MAX_ORDER_SHORT+k], 1) > 0 ||
                  sub(tnsInfo->coef[i*TNS_MAX_ORDER_SHORT+k], -2) < 0) {
                coefBits = 3;                                   move16();
                break;
              }
            }
          }
          for (k=0; k<tnsInfo->order[i]; k++ ) {
            count = add(count, coefBits);
          }
        }
      }
    }
  }
  return(count);
}


static Word16 countTnsBits(TNS_INFO *tnsInfo,Word16 blockType)
{
  return(tnsCount(tnsInfo, blockType));
}





Word16 countStaticBitdemand(PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
                            PSY_OUT_ELEMENT *psyOutElement,
                            Word16 channels)
{
  Word16 statBits;
  Word16 ch;
  
  statBits = 0;                                                 move16();

  test();
  switch (channels) {
    case 1:
      statBits = add(statBits, SI_ID_BITS+SI_SCE_BITS+SI_ICS_BITS);
      statBits = add(statBits, countTnsBits(&(psyOutChannel[0].tnsInfo),
                                            psyOutChannel[0].windowSequence));
      test();
      switch(psyOutChannel[0].windowSequence){
        case LONG_WINDOW:
        case START_WINDOW:
        case STOP_WINDOW:
          statBits = add(statBits, SI_ICS_INFO_BITS_LONG);
          break;
        case SHORT_WINDOW:
          statBits = add(statBits, SI_ICS_INFO_BITS_SHORT);
          break;
      }
      break;
    case 2:
      statBits = add(statBits, SI_ID_BITS+SI_CPE_BITS+2*SI_ICS_BITS);

      statBits = add(statBits, SI_CPE_MS_MASK_BITS);
      statBits = add(statBits, countMsMaskBits(psyOutChannel[0].sfbCnt,
                                               psyOutChannel[0].sfbPerGroup,
                                               psyOutChannel[0].maxSfbPerGroup,
                                               &psyOutElement->toolsInfo));
      test();
      switch (psyOutChannel[0].windowSequence) {
        case LONG_WINDOW:
        case START_WINDOW:
        case STOP_WINDOW:
          statBits = add(statBits, SI_ICS_INFO_BITS_LONG);
          break;
        case SHORT_WINDOW:
          statBits = add(statBits, SI_ICS_INFO_BITS_SHORT);
          break;
      }
      for(ch=0; ch<2; ch++)
        statBits = add(statBits, countTnsBits(&(psyOutChannel[ch].tnsInfo),
                                              psyOutChannel[ch].windowSequence));
      break;
  }

  return(statBits);
}

