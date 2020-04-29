/*
   Interface psychoaccoustic/quantizer
 */
#include "psy_const.h"
#include "interface.h"
#include "count.h"



void BuildInterface(Word32                  *groupedMdctSpectrum,
                    const Word16             mdctScale,
                    SFB_THRESHOLD           *groupedSfbThreshold,
                    SFB_ENERGY              *groupedSfbEnergy,
                    SFB_ENERGY              *groupedSfbSpreadedEnergy,
                    const SFB_ENERGY_SUM     sfbEnergySumLR,
                    const SFB_ENERGY_SUM     sfbEnergySumMS,
                    const Word16             windowSequence,
                    const Word16             windowShape,
                    const Word16             groupedSfbCnt,
                    const Word16            *groupedSfbOffset,
                    const Word16             maxSfbPerGroup,
                    const Word16            *groupedSfbMinSnr,
                    const Word16             noOfGroups,
                    const Word16            *groupLen,
                    PSY_OUT_CHANNEL         *psyOutCh)
{
  Word16 j;
  Word16 grp; 
  Word16 mask;


  /*
  copy values to psyOut
  */
  psyOutCh->maxSfbPerGroup    = maxSfbPerGroup;                            move16();
  psyOutCh->sfbCnt            = groupedSfbCnt;                             move16();
  psyOutCh->sfbPerGroup       = ffr_Short_Div(groupedSfbCnt, noOfGroups);
  psyOutCh->windowSequence    = windowSequence;                            move16();
  psyOutCh->windowShape       = windowShape;                               move16();
  psyOutCh->mdctScale         = mdctScale;                                 move16();
  psyOutCh->mdctSpectrum      = groupedMdctSpectrum;
  psyOutCh->sfbEnergy         = groupedSfbEnergy->sfbLong;
  psyOutCh->sfbThreshold      = groupedSfbThreshold->sfbLong;
  psyOutCh->sfbSpreadedEnergy = groupedSfbSpreadedEnergy->sfbLong;
  
  for(j=0; j<add(groupedSfbCnt,1); j++) {
    psyOutCh->sfbOffsets[j] = groupedSfbOffset[j];                              move16();
  }

  for(j=0;j<groupedSfbCnt; j++) {
    psyOutCh->sfbMinSnr[j] = groupedSfbMinSnr[j];                               move16();
  }
  
  /* generate grouping mask */
  mask = 0;                                                                     move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    mask = shl(mask, 1);
    for (j=1; j<groupLen[grp]; j++) {
      mask = shl(mask, 1);
      mask |= 1;                                                                logic16();
    }
  }
  psyOutCh->groupingMask = mask;                                                move16();

  test();
  if (sub(windowSequence, SHORT_WINDOW) != 0) {
    psyOutCh->sfbEnSumLR =  sfbEnergySumLR.sfbLong;                             move32();
    psyOutCh->sfbEnSumMS =  sfbEnergySumMS.sfbLong;                             move32();
  }
  else {
    Word32 i;
    Word32 accuSumMS=0;
    Word32 accuSumLR=0;                                                         move32(); move32();

    for (i=0;i< TRANS_FAC; i++) {
      accuSumLR = L_add(accuSumLR, sfbEnergySumLR.sfbShort[i]);
      accuSumMS = L_add(accuSumMS, sfbEnergySumMS.sfbShort[i]);
    }
    psyOutCh->sfbEnSumMS = accuSumMS;                                           move32();
    psyOutCh->sfbEnSumLR = accuSumLR;                                           move32();
  }
}
