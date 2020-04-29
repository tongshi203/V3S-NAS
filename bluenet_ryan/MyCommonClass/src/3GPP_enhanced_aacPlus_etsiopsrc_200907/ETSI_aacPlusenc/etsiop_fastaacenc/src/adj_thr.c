/*
   Threshold compensation 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "adj_thr_data.h"
#include "adj_thr.h"
#include "qc_data.h"
#include "line_pe.h"
#include "count.h"


#define  minSnrLimit    0x6666 /* 1 dB */

/* values for avoid hole flag */
enum _avoid_hole_state {
  NO_AH              =0,
  AH_INACTIVE        =1,
  AH_ACTIVE          =2
};


/* convert from bits to pe */
Word16 bits2pe(const Word16 bits) {
  return add(bits, mult(0x170a, bits));
}

/* loudness calculation (threshold to the power of redExp) */
static void calcThreshExp(Word32 thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
                          PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
                          const Word16 nChannels)
{
  Word16 ch, sfb, sfbGrp;
  for (ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup)
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        thrExp[ch][sfbGrp+sfb] = ffr_sqrt(ffr_sqrt(psyOutChan->sfbThreshold[sfbGrp+sfb],INT_BITS),INT_BITS);
      }
  }
}

/* reduce minSnr requirements for bands with relative low energies */
static void adaptMinSnr(PSY_OUT_CHANNEL     psyOutChannel[MAX_CHANNELS],
                        Word16              logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                        MINSNR_ADAPT_PARAM *msaParam,
                        const Word16        nChannels)
{
  Word16 ch, sfb, sfbOffs, nSfb;
  Word32 avgEn;
  Word16 log_avgEn = 0;
  Word32 startRatio_x_avgEn = 0;

                                                                        move16(); move32();

  for (ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL* psyOutChan = &psyOutChannel[ch];

    /* calc average energy per scalefactor band */
    avgEn = 0;                                                          move32();
    nSfb = 0;                                                           move16();
    for (sfbOffs=0; 
         sfbOffs<psyOutChan->sfbCnt;
         sfbOffs+=psyOutChan->sfbPerGroup) {
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        avgEn = L_add(avgEn, psyOutChan->sfbEnergy[sfbOffs+sfb]);
        nSfb = add(nSfb, 1);
      }
    }
    test();
    if (nSfb > 0) {
      Word32 inSfb;
      inSfb = ffr_div32_32( 1, nSfb );
      avgEn = fixmul(avgEn, inSfb);

      log_avgEn = ffr_iLog4(avgEn);
      startRatio_x_avgEn = fixmul(msaParam->startRatio, avgEn);
    }

    
    /* reduce minSnr requirement by minSnr^minSnrRed dependent on avgEn/sfbEn */
    for (sfbOffs=0; 
         sfbOffs<psyOutChan->sfbCnt;
         sfbOffs+=psyOutChan->sfbPerGroup) {
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        if (L_sub(psyOutChan->sfbEnergy[sfbOffs+sfb], startRatio_x_avgEn) < 0) {
          Word16 dbRatio, minSnrRed;
          Word32 snrRed;
          Word16 newMinSnr;
          
          dbRatio = sub(log_avgEn, logSfbEnergy[ch][sfbOffs+sfb]);
          dbRatio = add(dbRatio, shl(dbRatio,1));

          minSnrRed = sub(110, shr(add(dbRatio, shl(dbRatio,1)), 2));
          minSnrRed = S_max(minSnrRed, 20); /* 110: (0.375(redOffs)+1)*80,  
                                               3: 0.00375(redRatioFac)*80
                                               20: 0.25(maxRed) * 80 */

          snrRed = ffr_Integer_Mult16x16(minSnrRed,
                                         ffr_iLog4(L_shl(psyOutChan->sfbMinSnr[sfbOffs+sfb],16))); 
          /* 
             snrRedI si now scaled by 80 (minSnrRed) and 4 (ffr_iLog4)
          */
        
          newMinSnr = etsiopround(ffr_pow2_xy(snrRed,80*4));
         
          psyOutChan->sfbMinSnr[sfbOffs+sfb] = S_min(newMinSnr, minSnrLimit);
        }
      }
    }

  }

}



/* determine bands where avoid hole is not necessary resp. possible */
static void initAvoidHoleFlag(Word16 ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                              PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                              PSY_OUT_ELEMENT* psyOutElement,
                              const Word16 nChannels,
                              AH_PARAM *ahParam)
{
  Word16 ch, sfb, sfbGrp;
  Word16 threshold;


  for (ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    test();
    if (sub(psyOutChan->windowSequence, SHORT_WINDOW) != 0) {
      for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
        for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
          psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb] = L_shr(psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb], 1);
        }
      }
    }
    else {
      for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
        for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
          psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb] = L_add(L_shr(psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb], 1),
                                                            L_shr(psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb], 3));
        }
      }
    }
  }


  /* increase minSnr for local peaks, decrease it for valleys */
  test();
  if (ahParam->modifyMinSnr) {
    for(ch=0; ch<nChannels; ch++) {
      PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
      test(); move16();
      if (sub(psyOutChan->windowSequence, SHORT_WINDOW) != 0)
        threshold = 0x2873;
      else
        threshold = 0x4000;

      for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
        for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
          Word32 sfbEn, sfbEnm1, sfbEnp1, avgEn;
          test(); move32();
          if (sfb > 0)
            sfbEnm1 = psyOutChan->sfbEnergy[sfbGrp+sfb-1];
          else
            sfbEnm1 = psyOutChan->sfbEnergy[sfbGrp];
          test(); move32();
          if (sub(sfb, sub(psyOutChan->maxSfbPerGroup,1)) < 0)
            sfbEnp1 = psyOutChan->sfbEnergy[sfbGrp+sfb+1];
          else
            sfbEnp1 = psyOutChan->sfbEnergy[sfbGrp+sfb];
          avgEn = L_shr(L_add(sfbEnm1, sfbEnp1), 1);
          sfbEn = psyOutChan->sfbEnergy[sfbGrp+sfb];                            move32();
          test(); test();
          if (L_sub(sfbEn, avgEn) > 0 && avgEn > 0) {
            Word16 tmpMinSnr;
            tmpMinSnr = etsiopround(ffr_div32_32( fixmul_32x16b(avgEn, 0x6666), sfbEn ));
            tmpMinSnr = S_max(tmpMinSnr, 0x2873);                  
            tmpMinSnr = S_max(tmpMinSnr,threshold);
            psyOutChan->sfbMinSnr[sfbGrp+sfb] = 
              S_min(psyOutChan->sfbMinSnr[sfbGrp+sfb], tmpMinSnr);
          }
          /* valley ? */
          test(); test();
          if ((L_sub(sfbEn, L_shr(avgEn,1)) < 0) && (sfbEn > 0)) {
            Word16 tmpMinSnr;
            Word32 minSnrEn = L_shr(fixmul_32x16b(avgEn, psyOutChan->sfbMinSnr[sfbGrp+sfb]), 1);
                 
            test();
            if(L_sub(minSnrEn, sfbEn) < 0) {
              tmpMinSnr = etsiopround(ffr_div32_32( minSnrEn, sfbEn ));
            }
            else {
              tmpMinSnr = MAX_16;                                            move16();
            }
            tmpMinSnr = S_min(0x6666, tmpMinSnr);

            psyOutChan->sfbMinSnr[sfbGrp+sfb] =
              shl(S_min(shr(tmpMinSnr, 2), mult(psyOutChan->sfbMinSnr[sfbGrp+sfb], 0x651f)), 2);
          }
        }
      }
    }
  }

  /* stereo: adapt the minimum requirements sfbMinSnr of mid and
     side channels */
  test();
  if (sub(nChannels, 2) == 0) {
    PSY_OUT_CHANNEL *psyOutChanM = &psyOutChannel[0];
    PSY_OUT_CHANNEL *psyOutChanS = &psyOutChannel[1];
    for (sfb=0; sfb<psyOutChanM->sfbCnt; sfb++) {
      if (psyOutElement->toolsInfo.msMask[sfb]) {
        Word32 sfbEnM = psyOutChanM->sfbEnergy[sfb];
        Word32 sfbEnS = psyOutChanS->sfbEnergy[sfb];
        Word32 maxSfbEn = L_max(sfbEnM, sfbEnS);
        Word32 maxThr = L_shr(fixmul_32x16b(maxSfbEn, psyOutChanM->sfbMinSnr[sfb]), 2);

        
        test();
        if(L_sub(maxThr, sfbEnM) >= 0) {
          psyOutChanM->sfbMinSnr[sfb] = MAX_16;                                         move32();
        }
        else {
          psyOutChanM->sfbMinSnr[sfb] = S_min(S_max(psyOutChanM->sfbMinSnr[sfb], etsiopround(ffr_div32_32(maxThr, sfbEnM ))), 0x6666);
        }
        test();
        if(L_sub(maxThr, sfbEnS) >= 0) {
          psyOutChanS->sfbMinSnr[sfb] = MAX_16;
        }
        else {
          psyOutChanS->sfbMinSnr[sfb] = S_min(S_max(psyOutChanS->sfbMinSnr[sfb], etsiopround(ffr_div32_32(maxThr, sfbEnS ))), 0x6666);
        }

        test();
        if (L_sub(sfbEnM, psyOutChanM->sfbSpreadedEnergy[sfb]) > 0)
          psyOutChanS->sfbSpreadedEnergy[sfb] = fixmul_32x16b(sfbEnS, 0x7333);
        test();
        if (L_sub(sfbEnS, psyOutChanS->sfbSpreadedEnergy[sfb]) > 0)
          psyOutChanM->sfbSpreadedEnergy[sfb] = fixmul_32x16b(sfbEnM, 0x7333);
      }
    }
  }


  /* init ahFlag (0: no ah necessary, 1: ah possible, 2: ah active */
  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        test(); test(); test(); move16();
        if (L_sub(psyOutChan->sfbSpreadedEnergy[sfbGrp+sfb], psyOutChan->sfbEnergy[sfbGrp+sfb]) > 0 ||
            L_sub(psyOutChan->sfbEnergy[sfbGrp+sfb], psyOutChan->sfbThreshold[sfbGrp+sfb]) <= 0 ||
            L_sub(psyOutChan->sfbMinSnr[sfbGrp+sfb], MAX_16) == 0) {
          ahFlag[ch][sfbGrp+sfb] = NO_AH;
        }
        else {
          ahFlag[ch][sfbGrp+sfb] = AH_INACTIVE;
        }
      }
      for (sfb=psyOutChan->maxSfbPerGroup; sfb<psyOutChan->sfbPerGroup; sfb++) {
        ahFlag[ch][sfbGrp+sfb] = NO_AH;                                                         move16();
      }
    }
  }

}


/* sum the pe data only for bands where avoid hole is inactive */
static void calcPeNoAH(Word16          *pe,
                       Word16          *constPart,
                       Word16          *nActiveLines,
                       PE_DATA         *peData,
                       Word16           ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                       PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                       const Word16     nChannels)
{
  Word16 ch, sfb, sfbGrp;

  *pe = 0;                                                      move16();
  *constPart = 0;                                               move16();
  *nActiveLines = 0;                                            move16();
  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    PE_CHANNEL_DATA *peChanData = &peData->peChannelData[ch];
    for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        test();
        if (sub(ahFlag[ch][sfbGrp+sfb], AH_ACTIVE) < 0) {
          *pe = add(*pe, peChanData->sfbPe[sfbGrp+sfb]);
          *constPart = add(*constPart, peChanData->sfbConstPart[sfbGrp+sfb]);
          *nActiveLines = add(*nActiveLines, peChanData->sfbNActiveLines[sfbGrp+sfb]);
        }
      }
    }
  }
}



/* apply reduction formula */
static void reduceThresholds(PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                             Word16           ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                             Word32           thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
                             const Word16     nChannels,
                             const Word32     redVal)
{
  Word16 ch, sfb, sfbGrp;
  Word32 sfbEn, sfbThr, sfbThrReduced;


  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    for(sfbGrp=0; sfbGrp<psyOutChan->sfbCnt; sfbGrp+=psyOutChan->sfbPerGroup) {
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        sfbEn  = psyOutChan->sfbEnergy[sfbGrp+sfb];                                     move32();
        sfbThr = psyOutChan->sfbThreshold[sfbGrp+sfb];                                  move32();
        test();
        if (L_sub(sfbEn, sfbThr) > 0) {
          /* threshold reduction formula */
          Word32 tmp = L_add(thrExp[ch][sfbGrp+sfb], redVal);
          tmp = fixmul(tmp, tmp);
          sfbThrReduced = fixmul(tmp, tmp);
          /* avoid holes */
          tmp = fixmul_32x16b(sfbEn, psyOutChan->sfbMinSnr[sfbGrp+sfb]);
          test(); test();
          if ((L_sub(sfbThrReduced, tmp) > 0) && 
              (sub(ahFlag[ch][sfbGrp+sfb], NO_AH) != 0)){
            sfbThrReduced = L_max(tmp, sfbThr);
            ahFlag[ch][sfbGrp+sfb] = AH_ACTIVE;                                         move16();
          }
          psyOutChan->sfbThreshold[sfbGrp+sfb] = sfbThrReduced;                         move32();
        }
      }
    }
  }
}




/* if pe difference deltaPe between desired pe and real pe is small enough,
   the difference can be distributed among the scale factor bands. */
static void correctThresh(PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                          Word16           ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                          PE_DATA         *peData,
                          Word32           thrExp[MAX_CHANNELS][MAX_GROUPED_SFB],
                          const Word32     redVal,
                          const Word16     nChannels,
                          const Word32     deltaPe)
{
  Word16 ch, sfb, sfbGrp;
  PSY_OUT_CHANNEL *psyOutChan;
  PE_CHANNEL_DATA *peChanData;
  Word32 deltaSfbPe;
  Word32 sfbPeFactors[MAX_CHANNELS][MAX_GROUPED_SFB], normFactor;
  Word32 sfbEn, sfbThr;
  Word32 sfbThrReduced;

  /* for each sfb calc relative factors for pe changes */
  normFactor = 1;                                                                       move32();
  for(ch=0; ch<nChannels; ch++) {
    psyOutChan = &psyOutChannel[ch];
    peChanData = &peData->peChannelData[ch];
    for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        Word32 redThrExp = L_add(thrExp[ch][sfbGrp+sfb], redVal);
        test(); test(); test();
        if (((sub(ahFlag[ch][sfbGrp+sfb], AH_ACTIVE) < 0) || (deltaPe > 0)) && (redThrExp > 0) ) {
            
          sfbPeFactors[ch][sfbGrp+sfb] = ffr_Integer_Mult16x16(peChanData->sfbNActiveLines[sfbGrp+sfb],
                                                               ffr_Integer_Div(0x7fffffff, redThrExp));
          normFactor = L_add(normFactor, sfbPeFactors[ch][sfbGrp+sfb]);
        }
        else {
          sfbPeFactors[ch][sfbGrp+sfb] = 0;                                             move32();
        }
      }
    }
  }

 
  /* calculate new thresholds */
  for(ch=0; ch<nChannels; ch++) {
    psyOutChan = &psyOutChannel[ch];
    peChanData = &peData->peChannelData[ch];
    for(sfbGrp = 0;sfbGrp < psyOutChan->sfbCnt;sfbGrp+= psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        /* pe difference for this sfb */
        deltaSfbPe = ffr_Integer_Mult(sfbPeFactors[ch][sfbGrp+sfb], deltaPe);
        
        test();
        if (peChanData->sfbNActiveLines[sfbGrp+sfb] > 0) {
          /* new threshold */
          Word32 thrFactor;
          sfbEn  = psyOutChan->sfbEnergy[sfbGrp+sfb];
          sfbThr = psyOutChan->sfbThreshold[sfbGrp+sfb];

          test();
          if(deltaSfbPe >= 0){
            /*
              reduce threshold
            */
            thrFactor = ffr_pow2_xy(L_negate(deltaSfbPe), ffr_Integer_Mult(normFactor,peChanData->sfbNActiveLines[sfbGrp+sfb]));
              
            sfbThrReduced = fixmul_32x16b(sfbThr, etsiopround(thrFactor));
          }
          else {
            /*
              increase threshold
            */
            thrFactor = ffr_pow2_xy(deltaSfbPe, ffr_Integer_Mult(normFactor, peChanData->sfbNActiveLines[sfbGrp+sfb]));
              
            test();
            if(L_sub(thrFactor, sfbThr) > 0) {
              sfbThrReduced = ffr_div32_32( sfbThr, thrFactor );
            }
            else {
              sfbThrReduced = MAX_32;                                                                           move32();
            }

          }
            
          /* avoid hole */
          sfbEn = fixmul_32x16b(sfbEn, psyOutChan->sfbMinSnr[sfbGrp+sfb]);
          test(); test();
          if ((L_sub(sfbThrReduced, sfbEn) > 0) &&
              (sub(ahFlag[ch][sfbGrp+sfb], AH_INACTIVE) == 0)) {
            sfbThrReduced = L_max(sfbEn, sfbThr);
            ahFlag[ch][sfbGrp+sfb] = AH_ACTIVE;                                                                 move16();
          }

          psyOutChan->sfbThreshold[sfbGrp+sfb] = sfbThrReduced;                                                 move32();
        }
      }
    }
  }

}

/* if the desired pe can not be reached, reduce pe by reducing minSnr */
static void reduceMinSnr(PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS], 
                         PE_DATA         *peData, 
                         Word16           ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                         const Word16     nChannels,
                         const Word16     desiredPe)
{
  Word16 ch, sfb, sfbSubWin;
  Word16 deltaPe;

  /* start at highest freq down to 0 */
  sfbSubWin = psyOutChannel[0].maxSfbPerGroup;                                                move16();
  while (sub(peData->pe, desiredPe) > 0 && sfbSubWin > 0) {
    test(); test();
    sfbSubWin = sub(sfbSubWin, 1);
    /* loop over all subwindows */
    for (sfb=sfbSubWin; sfb<psyOutChannel[0].sfbCnt;
        sfb+=psyOutChannel[0].sfbPerGroup) {
      /* loop over all channels */
      for (ch=0; ch<nChannels; ch++) {
        test(); test();
        if (sub(ahFlag[ch][sfb], NO_AH) != 0 &&
            sub(psyOutChannel[ch].sfbMinSnr[sfb], minSnrLimit) < 0) {
          psyOutChannel[ch].sfbMinSnr[sfb] = minSnrLimit;                                     move16();
          psyOutChannel[ch].sfbThreshold[sfb] =
            fixmul_32x16b(psyOutChannel[ch].sfbEnergy[sfb], psyOutChannel[ch].sfbMinSnr[sfb]);

          /* calc new pe */
          deltaPe = sub(shr(add(peData->peChannelData[ch].sfbNLines4[sfb], shr(peData->peChannelData[ch].sfbNLines4[sfb],1)),2),
              peData->peChannelData[ch].sfbPe[sfb]);
          peData->pe = add(peData->pe, deltaPe);
          peData->peChannelData[ch].pe = add(peData->peChannelData[ch].pe, deltaPe);
        }
      }
      /* stop if enough has been saved */
      test();
      if (sub(peData->pe, desiredPe) <= 0)
        break;
    }
  }

}


/* if the desired pe can not be reached, some more scalefactor bands have to be 
   quantized to zero */
static void allowMoreHoles(PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS], 
                           PSY_OUT_ELEMENT *psyOutElement,
                           PE_DATA         *peData, 
                           Word16           ahFlag[MAX_CHANNELS][MAX_GROUPED_SFB],
                           const AH_PARAM  *ahParam,
                           const Word16     nChannels,
                           const Word16     desiredPe)
{
  Word16 ch, sfb;
  Word16 actPe;

  actPe = peData->pe;                                                                   move16();

  /* for MS allow hole in the channel with less energy */
  test(); test();
  if (sub(nChannels,2)==0 &&
      sub(psyOutChannel[0].windowSequence,psyOutChannel[1].windowSequence)==0) {
    PSY_OUT_CHANNEL *psyOutChanL = &psyOutChannel[0];
    PSY_OUT_CHANNEL *psyOutChanR = &psyOutChannel[1];
    for (sfb=0; sfb<psyOutChanL->sfbCnt; sfb++) {
      Word32 minEn;
      test();
      if (psyOutElement->toolsInfo.msMask[sfb]) {
        /* allow hole in side channel ? */
        minEn = fixmul_32x16b(psyOutChanL->sfbEnergy[sfb], mult(0x3333, psyOutChanL->sfbMinSnr[sfb]));
        test(); test();
        if (sub(ahFlag[1][sfb], NO_AH) != 0 &&
            L_sub(minEn, psyOutChanR->sfbEnergy[sfb]) > 0) {
          ahFlag[1][sfb] = NO_AH;                                                               move16();
          psyOutChanR->sfbThreshold[sfb] = L_shl(psyOutChanR->sfbEnergy[sfb], 1);
          actPe = sub(actPe, peData->peChannelData[1].sfbPe[sfb]);
        }
        /* allow hole in mid channel ? */
        else {
        minEn = fixmul_32x16b(psyOutChanR->sfbEnergy[sfb], mult(0x3333, psyOutChanR->sfbMinSnr[sfb]));
          test(); test();
          if (sub(ahFlag[0][sfb], NO_AH) != 0 &&
              L_sub(minEn, psyOutChanL->sfbEnergy[sfb]) > 0) {
            ahFlag[0][sfb] = NO_AH;                                                             move16();
            psyOutChanL->sfbThreshold[sfb] = L_shl(psyOutChanL->sfbEnergy[sfb], 1);
            actPe = sub(actPe, peData->peChannelData[0].sfbPe[sfb]);
          }
        }
        test();
        if (sub(actPe, desiredPe) < 0)
          break;
      }
    }
  }

  /* subsequently erase bands */
  test();
  if (sub(actPe, desiredPe) > 0) {
    Word16 startSfb[2];
    Word32 avgEn, minEn;
    Word16 ahCnt;
    Word16 enIdx;
    Word16 enDiff;
    Word32 en[4];
    Word16 minSfb, maxSfb;
    Flag   done;

    /* do not go below startSfb */
    for (ch=0; ch<nChannels; ch++) {
      test(); move16();
      if (sub(psyOutChannel[ch].windowSequence, SHORT_WINDOW) != 0)
        startSfb[ch] = ahParam->startSfbL;
      else
        startSfb[ch] = ahParam->startSfbS;
    }

    avgEn = 0;                                                          move32();
    minEn = MAX_32;                                                     move32();
    ahCnt = 0;                                                          move16();
    for (ch=0; ch<nChannels; ch++) {
      PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
      for (sfb=startSfb[ch]; sfb<psyOutChan->sfbCnt; sfb++) {
        test(); test();
        if ((sub(ahFlag[ch][sfb],NO_AH) != 0) &&
            (L_sub(psyOutChan->sfbEnergy[sfb], psyOutChan->sfbThreshold[sfb]) > 0)) {
          minEn = L_min(minEn, psyOutChan->sfbEnergy[sfb]);
          avgEn = L_add(avgEn, psyOutChan->sfbEnergy[sfb]);
          ahCnt++;
        }
      }
    }
    test();
    if(ahCnt) {
      Word32 iahCnt;
      iahCnt = ffr_div32_32( 1, ahCnt );
      avgEn = fixmul(avgEn, iahCnt);
    }

    enDiff = sub(ffr_iLog4(avgEn), ffr_iLog4(minEn));
    /* calc some energy borders between minEn and avgEn */
    for (enIdx=0; enIdx<4; enIdx++) {
      Word32 enFac;
      enFac = ffr_Integer_Mult16x16(sub(6, shl(enIdx,1)), enDiff);
      en[enIdx] = fixmul(avgEn, ffr_pow2_xy(L_negate(enFac),7*4));
    }

    /* start with lowest energy border at highest sfb */
    maxSfb = sub(psyOutChannel[0].sfbCnt, 1);
    minSfb = startSfb[0];                                                               move16();
    test();
    if (sub(nChannels,2) == 0) {
      maxSfb = S_max(maxSfb, sub(psyOutChannel[1].sfbCnt, 1));
      minSfb = S_min(minSfb, startSfb[1]);
    }

    sfb = maxSfb;                                                                       move16();
    enIdx = 0;                                                                          move16();
    done = 0;                                                                           move16();
    while (!done) {
      test();
      for (ch=0; ch<nChannels; ch++) {
        PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
        test(); test();
        if (sub(sfb,startSfb[ch]) >= 0 && sub(sfb,psyOutChan->sfbCnt) < 0) {
          /* sfb energy below border ? */
          test(); test();
          if (sub(ahFlag[ch][sfb], NO_AH) != 0 && L_sub(psyOutChan->sfbEnergy[sfb], en[enIdx]) < 0){
            /* allow hole */
            ahFlag[ch][sfb] = NO_AH;                                                    move16();
            psyOutChan->sfbThreshold[sfb] = L_shl(psyOutChan->sfbEnergy[sfb], 1);
            actPe = sub(actPe, peData->peChannelData[ch].sfbPe[sfb]);
          }
          test();
          if (sub(actPe, desiredPe) < 0) {
            done = 1;                                                                   move16();
            break;
          }
        }
      }
      sfb = sub(sfb, 1);
      test();
      if (sub(sfb, minSfb) < 0) {
        /* restart with next energy border */
        sfb = maxSfb;                                                                   move16();
        enIdx = add(enIdx, 1);
        test();
        if (sub(enIdx, 4) >= 0)
          done = 1;                                                                     move16();
      }
    }
  }

}


/* two guesses for the reduction value and one final correction of the
   thresholds */
static void adaptThresholdsToPe(PSY_OUT_CHANNEL     psyOutChannel[MAX_CHANNELS],
                                PSY_OUT_ELEMENT    *psyOutElement,
                                Word16              logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                                PE_DATA            *peData,
                                const Word16        nChannels,
                                const Word16        desiredPe,
                                AH_PARAM           *ahParam,
                                MINSNR_ADAPT_PARAM *msaParam)
{
  Word16 noRedPe, redPe, redPeNoAH;
  Word16 constPart, constPartNoAH;
  Word16 nActiveLines, nActiveLinesNoAH;
  Word16 desiredPeNoAH;
  Word32 redVal, avgThrExp;
  Word16 iter;

  calcThreshExp(peData->thrExp, psyOutChannel, nChannels);

  adaptMinSnr(psyOutChannel, logSfbEnergy, msaParam, nChannels);

  initAvoidHoleFlag(peData->ahFlag, psyOutChannel, psyOutElement, nChannels, ahParam);

  noRedPe = peData->pe;                                                         move16();
  constPart = peData->constPart;                                                move16();
  nActiveLines = peData->nActiveLines;                                          move16();


  /* first guess of reduction value */
  avgThrExp = ffr_pow2_xy(sub(constPart,noRedPe), shl(nActiveLines,2));
  
  redVal = L_sub(ffr_pow2_xy(sub(constPart,desiredPe), shl(nActiveLines,2)), avgThrExp);

  /* reduce thresholds */
  reduceThresholds(psyOutChannel, peData->ahFlag, peData->thrExp, nChannels, redVal);

  /* pe after first guess */
  calcSfbPe(peData, psyOutChannel, nChannels);
  redPe = peData->pe;                                                           move16();

  iter = 0;                                                                     move16();
  do {
    /* pe for bands where avoid hole is inactive */
    calcPeNoAH(&redPeNoAH, &constPartNoAH, &nActiveLinesNoAH,
               peData, peData->ahFlag, psyOutChannel, nChannels);

    desiredPeNoAH = sub(desiredPe, sub(redPe, redPeNoAH));
    test();
    if (desiredPeNoAH < 0) {
      desiredPeNoAH = 0;                                                        move16();
    }

    /* second guess */
    test();
    if (nActiveLinesNoAH > 0) {

      avgThrExp = ffr_pow2_xy(sub(constPartNoAH,redPeNoAH), shl(nActiveLinesNoAH,2));
       
      redVal = L_sub(L_add(redVal, ffr_pow2_xy(sub(constPartNoAH,desiredPeNoAH), shl(nActiveLinesNoAH,2))), avgThrExp);

      /* reduce thresholds */
      reduceThresholds(psyOutChannel, peData->ahFlag, peData->thrExp, nChannels, redVal);
    }

    calcSfbPe(peData, psyOutChannel, nChannels);
    redPe = peData->pe;                                                         move16();

    iter = add(iter, 1);
    test(); test();
  } while ((L_sub(ffr_Integer_Mult16x16(20,abs_s(sub(redPe,desiredPe))), desiredPe) > 0) && (sub(iter,2) < 0));

  test();
  if (L_sub(ffr_Integer_Mult16x16(100,redPe), ffr_Integer_Mult16x16(115,desiredPe)) < 0) {
    correctThresh(psyOutChannel, peData->ahFlag, peData, peData->thrExp, redVal,
                  nChannels, desiredPe - redPe);
  }
  else {
    Word16 desiredPe105 = extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(105,desiredPe),100));
    reduceMinSnr(psyOutChannel, peData, peData->ahFlag,
                 nChannels, desiredPe105);
    allowMoreHoles(psyOutChannel, psyOutElement, peData, peData->ahFlag,
                   ahParam, nChannels, desiredPe105);
  }

}


/*****************************************************************************

    functionname: calcBitSave
    description:  Calculates percentage of bit save, see figure below
    returns:
    input:        parameters and bitres-fullness
    output:       percentage of bit save

*****************************************************************************/
static Word16 calcBitSave(Word16 fillLevel,
                          const Word16 clipLow,
                          const Word16 clipHigh,
                          const Word16 minBitSave,
                          const Word16 maxBitSave)
{
  Word16 bitsave;

  fillLevel = S_max(fillLevel, clipLow);
  fillLevel = S_min(fillLevel, clipHigh);

  bitsave = sub(maxBitSave, extract_l(ffr_Integer_Div(
                              ffr_Integer_Mult16x16(sub(maxBitSave,minBitSave), sub(fillLevel,clipLow)),
                              sub(clipHigh,clipLow))));

  return (bitsave);
}



/*****************************************************************************

    functionname: calcBitSpend
    description:  Calculates percentage of bit spend, see figure below
    returns:
    input:        parameters and bitres-fullness
    output:       percentage of bit spend

*****************************************************************************/
static Word16 calcBitSpend(Word16 fillLevel,
                           const Word16 clipLow,
                           const Word16 clipHigh,
                           const Word16 minBitSpend,
                           const Word16 maxBitSpend)
{
  Word16 bitspend;

  fillLevel = S_max(fillLevel, clipLow);
  fillLevel = S_min(fillLevel, clipHigh);

  bitspend = add(minBitSpend, extract_l(ffr_Integer_Div(
                                ffr_Integer_Mult16x16(sub(maxBitSpend,minBitSpend),sub(fillLevel,clipLow)),
                                sub(clipHigh,clipLow))));
                            
  return (bitspend);
}


/*****************************************************************************

    functionname: adjustPeMinMax()
    description:  adjusts peMin and peMax parameters over time
    returns:
    input:        current pe, peMin, peMax
    output:       adjusted peMin/peMax

*****************************************************************************/
static void adjustPeMinMax(const Word16 currPe,
                           Word16      *peMin,
                           Word16      *peMax)
{
  Word16 minFacHi, maxFacHi, minFacLo, maxFacLo;
  Word16 diff;
  Word16 minDiff = extract_l(ffr_Integer_Div(currPe, 6));
  minFacHi = 30;                                                        move16();
  maxFacHi = 100;                                                       move16();
  minFacLo = 14;                                                        move16();
  maxFacLo = 7;                                                         move16();

  diff = sub(currPe,*peMax) ;
  test();
  if (diff > 0) {
    *peMin = add(*peMin, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(diff, minFacHi), 100)));
    *peMax = add(*peMax, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(diff, maxFacHi), 100)));
  } else {
    diff = sub(*peMin, currPe) ;
    test();
    if (diff > 0) {
      *peMin = sub(*peMin, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(diff, minFacLo), 100)));
      *peMax = sub(*peMax, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(diff, maxFacLo), 100)));
    } else {
      *peMin = add(*peMin, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(sub(currPe, *peMin), minFacHi), 100)));
      *peMax = sub(*peMax, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(sub(*peMax, currPe), maxFacLo), 100)));
    }
  }

  test();
  if (sub(sub(*peMax,*peMin), minDiff) < 0) {
    Word16 partLo, partHi;

    partLo = S_max(0, sub(currPe, *peMin));
    partHi = S_max(0, sub(*peMax, currPe));

    *peMax = add(currPe, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(partHi, minDiff), add(partLo,partHi))));
    *peMin = sub(currPe, extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(partLo, minDiff), add(partLo,partHi))));
    *peMin = max(0, *peMin);
  }
}


/*****************************************************************************

    functionname: BitresCalcBitFac
    description:  calculates factor of spending bits for one frame
                    1.0 : take all frame dynpart bits
                   >1.0 : take all frame dynpart bits + bitres
                   <1.0 : put bits in bitreservoir
    returns:      BitFac*100
    input:        bitres-fullness, pe, blockType, parameter-settings
    output:

*****************************************************************************/
static Word16 bitresCalcBitFac( const Word16   bitresBits,
                                const Word16   maxBitresBits,
                                const Word16   pe,
                                const Word16   windowSequence,
                                const Word16   avgBits,
                                const Word16   maxBitFac,
                                ADJ_THR_STATE *AdjThr,
                                ATS_ELEMENT   *adjThrChan)
{
  BRES_PARAM *bresParam;
  Word16 pex;
  Word16 fillLevel;
  Word16 bitSave, bitSpend, bitresFac;

  fillLevel = extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(100, bitresBits), maxBitresBits));

  test();
  if (sub(windowSequence, SHORT_WINDOW) != 0)
    bresParam = &(AdjThr->bresParamLong);
  else
    bresParam = &(AdjThr->bresParamShort);

  pex = S_max(pe, adjThrChan->peMin);
  pex = S_min(pex,adjThrChan->peMax);

  bitSave = calcBitSave(fillLevel,
                        bresParam->clipSaveLow, bresParam->clipSaveHigh,
                        bresParam->minBitSave, bresParam->maxBitSave);

  bitSpend = calcBitSpend(fillLevel,
                          bresParam->clipSpendLow, bresParam->clipSpendHigh,
                          bresParam->minBitSpend, bresParam->maxBitSpend);

  bitresFac = add(sub(100, bitSave),extract_l(
    ffr_Integer_Div(ffr_Integer_Mult16x16(add(bitSpend, bitSave), sub(pex, adjThrChan->peMin)),
                    sub(adjThrChan->peMax, adjThrChan->peMin))));
               
  bitresFac = S_min(bitresFac,
                    add(100-30, extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(100, bitresBits), avgBits))));

  bitresFac = S_min(bitresFac, maxBitFac);

  adjustPeMinMax(pe, &adjThrChan->peMin, &adjThrChan->peMax);

  return bitresFac;
}


void AdjThrInit(ADJ_THR_STATE *hAdjThr,
                const Word32   meanPe,
                Word32         chBitrate)
{
  ATS_ELEMENT* atsElem = &hAdjThr->adjThrStateElem;
  MINSNR_ADAPT_PARAM *msaParam = &atsElem->minSnrAdaptParam;

  /* common for all elements: */
  /* parameters for bitres control */
  hAdjThr->bresParamLong.clipSaveLow   =  20;                   move16();
  hAdjThr->bresParamLong.clipSaveHigh  =  95;                   move16();
  hAdjThr->bresParamLong.minBitSave    =  -5;                   move16();
  hAdjThr->bresParamLong.maxBitSave    =  30;                   move16();
  hAdjThr->bresParamLong.clipSpendLow  =  20;                   move16();
  hAdjThr->bresParamLong.clipSpendHigh =  95;                   move16();
  hAdjThr->bresParamLong.minBitSpend   = -10;                   move16();
  hAdjThr->bresParamLong.maxBitSpend   =  40;                   move16();

  hAdjThr->bresParamShort.clipSaveLow   =  20;                  move16();
  hAdjThr->bresParamShort.clipSaveHigh  =  75;                  move16();
  hAdjThr->bresParamShort.minBitSave    =   0;                  move16();
  hAdjThr->bresParamShort.maxBitSave    =  20;                  move16();
  hAdjThr->bresParamShort.clipSpendLow  =  20;                  move16();
  hAdjThr->bresParamShort.clipSpendHigh =  75;                  move16();
  hAdjThr->bresParamShort.minBitSpend   = -5;                   move16();
  hAdjThr->bresParamShort.maxBitSpend   =  50;                  move16();

  /* specific for each element: */

  /* parameters for bitres control */
  atsElem->peMin = extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(80, meanPe), 100));
  atsElem->peMax = extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(120, meanPe), 100));

  /* additional pe offset to correct pe2bits for low bitrates */
  atsElem->peOffset = 0;                                      move16();
  test();
  if (L_sub(chBitrate, 32000)) {
    atsElem->peOffset = S_max(50, sub(100, extract_l(ffr_Integer_Div(ffr_Integer_Mult(100, chBitrate), 32000))));
  }

  /* avoid hole parameters */
  test(); move16(); move16(); move16();
  if (L_sub(chBitrate, 20000) > 0) {
    atsElem->ahParam.modifyMinSnr = TRUE;
    atsElem->ahParam.startSfbL = 15;
    atsElem->ahParam.startSfbS = 3;
  }
  else {
    atsElem->ahParam.modifyMinSnr = FALSE;
    atsElem->ahParam.startSfbL = 0;
    atsElem->ahParam.startSfbS = 0;
  }

  /* minSnr adaptation */
  /* maximum reduction of minSnr goes down to minSnr^maxRed */
  msaParam->maxRed = 0x20000000;                                      move32();
  /* start adaptation of minSnr for avgEn/sfbEn > startRatio */
  msaParam->startRatio = 0x0ccccccd;                                  move32();
  /* maximum minSnr reduction to minSnr^maxRed is reached for
     avgEn/sfbEn >= maxRatio */
  msaParam->maxRatio =  0x0020c49c;                                   move32();
  /* helper variables to interpolate minSnr reduction for
     avgEn/sfbEn between startRatio and maxRatio */

  msaParam->redRatioFac = 0xfb333333; /* -0.75/20 */                  move32();

  msaParam->redOffs = 0x30000000;  /* msaParam->redRatioFac * 10*log10(msaParam->startRatio) */ move32();

       
  /* pe correction */
  atsElem->peLast = 0;                                                move16();
  atsElem->dynBitsLast = 0;                                           move16();
  atsElem->peCorrectionFactor = 100; /* 1.0 */                        move16();

}


static void calcPeCorrection(Word16 *correctionFac,
                             const Word16 peAct,
                             const Word16 peLast, 
                             const Word16 bitsLast) 
{
  Word32 peAct100 = ffr_Integer_Mult16x16(100, peAct);
  Word32 peLast100 = ffr_Integer_Mult16x16(100, peLast);
  Word16 peBitsLast = bits2pe(bitsLast);
  test(); test(); test(); test(); test();
  if ((bitsLast > 0) &&
      (L_sub(peAct100, ffr_Integer_Mult16x16(150, peLast)) < 0) && 
      (L_sub(peAct100, ffr_Integer_Mult16x16(70, peLast)) > 0) &&
      (L_sub(ffr_Integer_Mult16x16(120, peBitsLast), peLast100) > 0) && 
      (L_sub(ffr_Integer_Mult16x16( 65, peBitsLast), peLast100) < 0))
    {
      Word16 newFac = extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(100, peLast), peBitsLast));
      /* dead zone */
      test();
      if (sub(newFac, 100) < 0) {
        newFac = S_min(extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(110, newFac), 100)), 100);
        newFac = S_max(newFac, 85);
      }
      else {
        newFac = S_max(extract_l(ffr_Integer_Div(ffr_Integer_Mult16x16(90, newFac), 100)), 100);
        newFac = S_min(newFac, 115);
      }
      test(); test();
      if (((sub(newFac, 100) > 0) && (sub(*correctionFac, 100) < 0)) ||
          ((sub(newFac, 100) < 0) && (sub(*correctionFac, 100) > 0))) {
        *correctionFac = 100;                                                   move16();
      }
      /* faster adaptation towards 1.0, slower in the other direction */
      test(); test(); test(); test();
      if ((sub(*correctionFac, 100) < 0 && sub(newFac, *correctionFac) < 0) ||
          (sub(*correctionFac, 100) > 0 && sub(newFac, *correctionFac) > 0))
        *correctionFac = extract_l(ffr_Integer_Div(L_add(ffr_Integer_Mult16x16(85, *correctionFac),
                                                         ffr_Integer_Mult16x16(15, newFac)), 100));
      else
        *correctionFac = extract_l(ffr_Integer_Div(L_add(ffr_Integer_Mult16x16(70, *correctionFac),
                                                         ffr_Integer_Mult16x16(30, newFac)), 100));
      *correctionFac = S_min(*correctionFac, 115);
      *correctionFac = S_max(*correctionFac, 85);
    }
  else {
    *correctionFac = 100;                                                       move16();
  }
}

void AdjustThresholds(ADJ_THR_STATE   *adjThrState,
                      ATS_ELEMENT     *AdjThrStateElement,
                      PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                      PSY_OUT_ELEMENT *psyOutElement,
                      Word16          *chBitDistribution,
                      Word16           logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                      Word16           sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
                      const Word16     nChannels,
                      QC_OUT_ELEMENT  *qcOE,
                      const Word16     avgBits,
                      const Word16     bitresBits,
                      const Word16     maxBitresBits,
                      const Word16     maxBitFac,
                      const Word16     sideInfoBits)
{
  Word16 noRedPe, grantedPe, grantedPeCorr;
  Word16 curWindowSequence;
  PE_DATA peData;
  Word16 bitFactor;
  Word16 ch;
   
  prepareSfbPe(&peData, psyOutChannel, logSfbEnergy, sfbNRelevantLines, nChannels, AdjThrStateElement->peOffset);
   
  /* pe without reduction */
  calcSfbPe(&peData, psyOutChannel, nChannels);
  noRedPe = peData.pe;                                                  move16();


  curWindowSequence = LONG_WINDOW;                                      move16();
  test();
  if (sub(nChannels,2) == 0) {
    test(); test();
    if ((sub(psyOutChannel[0].windowSequence, SHORT_WINDOW) == 0) ||
        (sub(psyOutChannel[1].windowSequence, SHORT_WINDOW) == 0)) {
      curWindowSequence = SHORT_WINDOW;                                 move16();
    }
  }
  else {
    curWindowSequence = psyOutChannel[0].windowSequence;                move16();
  }


  /* bit factor */
  bitFactor = bitresCalcBitFac(bitresBits, maxBitresBits, noRedPe+5*sideInfoBits,
                               curWindowSequence, avgBits, maxBitFac,
                               adjThrState,
                               AdjThrStateElement);

  /* desired pe */
  grantedPe = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(bitFactor,  bits2pe(avgBits)), 100));

  /* correction of pe value */
  calcPeCorrection(&(AdjThrStateElement->peCorrectionFactor), 
                   min(grantedPe, noRedPe),
                   AdjThrStateElement->peLast, 
                   AdjThrStateElement->dynBitsLast);
  grantedPeCorr = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(grantedPe,  AdjThrStateElement->peCorrectionFactor), 100));

  test(); test();
  if (sub(grantedPeCorr, noRedPe) < 0 && sub(noRedPe, peData.offset) > 0) {
    /* calc threshold necessary for desired pe */
    adaptThresholdsToPe(psyOutChannel,
                        psyOutElement,
                        logSfbEnergy,
                        &peData,
                        nChannels,
                        grantedPeCorr,
                        &AdjThrStateElement->ahParam,
                        &AdjThrStateElement->minSnrAdaptParam);
  }

  /* calculate relative distribution */
  for (ch=0; ch<nChannels; ch++) {
    Word16 peOffsDiff = sub(peData.pe, peData.offset);
    chBitDistribution[ch] = 200;                                                move16();
    test();
    if (peOffsDiff > 0) {
      Word16 temp = sub(1000, ffr_Short_Mult(nChannels, 200));
      chBitDistribution[ch] = add(chBitDistribution[ch], extract_l(ffr_divideWord32(
              ffr_Integer_Mult16x16(temp, peData.peChannelData[ch].pe),
              peOffsDiff)));
    }
  }

  /* store pe */
  qcOE->pe = noRedPe;                                                           move16();

  /* update last pe */
  AdjThrStateElement->peLast = grantedPe;                                       move16();
}


void AdjThrUpdate(ATS_ELEMENT *AdjThrStateElement,
                  const Word16 dynBitsUsed)
{
  AdjThrStateElement->dynBitsLast = dynBitsUsed;                                move16();
}


