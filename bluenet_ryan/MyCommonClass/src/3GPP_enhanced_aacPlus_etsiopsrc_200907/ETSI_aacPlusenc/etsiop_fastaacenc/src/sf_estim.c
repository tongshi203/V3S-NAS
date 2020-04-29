/*
   Scale factor estimation
 */
#ifdef WIN32
#include <memory.h>
#else
#include <stdlib.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "sf_estim.h"
#include "quantize.h"
#include "bit_cnt.h"
#include "aac_rom.h"
#include "count.h"



static const Word16 MAX_SCF_DELTA = 60;

/*
  constants reference in comments 

  C0 = 6.75f;
  C1 = -69.33295f;   -16/3*log(MAX_QUANT+0.5-logCon)/log(2) 
  C2 = 4.0f;
  C3 = 2.66666666f;

  PE_C1 = 3.0f;        log(8.0)/log(2) 
  PE_C2 = 1.3219281f;  log(2.5)/log(2) 
  PE_C3 = 0.5593573f;  1-C2/C1 

*/

#define FF_SQRT_BITS                    7
#define FF_SQRT_TABLE_SIZE              (1<<FF_SQRT_BITS - 1<<(FF_SQRT_BITS-2))

/* calculates sqrt(x)/256 */
static Word32 formfac_sqrt(Word32 x)
{
  Word32 y;
  Word16 preshift, postshift;

  test();
  if (x==0) return 0;
  preshift  = sub(ffr_norm32(x), INT_BITS-1-FF_SQRT_BITS);
  postshift = shr(preshift, 1);
  preshift  = shl(postshift, 1);
  postshift = add(postshift, 8); /* sqrt/256 */
  y = L_shl(x, preshift);        /* now 1/4 <= y < 1 */
  y = formfac_sqrttable[y-32];
  y = L_shr(y, postshift);

  return y;
}


static void
CalcFormFactorChannel(Word16 *logSfbFormFactor,
                      Word16 *sfbNRelevantLines,
                      Word16 *logSfbEnergy,
                      PSY_OUT_CHANNEL *psyOutChan)
{
  Word16 i, j;
  Word16 sfbOffs, sfb;

  for (sfbOffs=0; sfbOffs<psyOutChan->sfbCnt; sfbOffs+=psyOutChan->sfbPerGroup){
    for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
      i = sfbOffs+sfb;
      
      test();
      if (L_sub(psyOutChan->sfbEnergy[i], psyOutChan->sfbThreshold[i]) > 0) {
        Word32 accu, avgFormFactor,iSfbWidth;
        accu = sub(psyOutChan->sfbOffsets[i+1], psyOutChan->sfbOffsets[i]);
        accu = ffr_div32_32( 1, accu );
        iSfbWidth = ffr_sqrt((Word32)accu, INT_BITS);         
        accu = 0;                                                                       move32();
        /* calc sum of sqrt(spec) */
        for (j=psyOutChan->sfbOffsets[i]; j<psyOutChan->sfbOffsets[i+1]; j++) {
           accu = L_add(accu, formfac_sqrt(L_abs(psyOutChan->mdctSpectrum[j])));
        }
        logSfbFormFactor[i] = ffr_iLog4(accu);
        logSfbEnergy[i] = ffr_iLog4(psyOutChan->sfbEnergy[i]);
        avgFormFactor = fixmul(ffr_sqrt(psyOutChan->sfbEnergy[i],INT_BITS), iSfbWidth);
        avgFormFactor = L_shr(ffr_sqrt((Word32)avgFormFactor,INT_BITS), 8);
        /* result is multiplied by 4 */
        sfbNRelevantLines[i] = extract_l(ffr_divideWord32(accu, L_shr(avgFormFactor,2)));
      }
      else {
        /* set number of lines to zero */
        sfbNRelevantLines[i] = 0;                                                       move16();
      }
    }
  }

}

static Word16 improveScf(Word32 *spec, 
                         Word16  sfbWidth, 
                         Word32  thresh, 
                         Word16  scf,
                         Word16  minScf,
                         Word32 *dist, 
                         Word16 *minScfCalculated)
{
  Word16 cnt;
  Word32 sfbDist;
  Word16 scfBest;
  Word32 thresh125 = L_add(thresh, L_shr(thresh,2));

  scfBest = scf;                                                       move16();
   
  /* calc real distortion */
  sfbDist = calcSfbDist(spec, sfbWidth, scf);
  *minScfCalculated = scf;                                             move16();

  test();
  if (L_sub(sfbDist, thresh125) > 0) {
    Word16 scfEstimated;
    Word32 sfbDistBest;
    scfEstimated = scf;                                               move16();
    sfbDistBest = sfbDist;                                            move32();

    cnt = 0;                                                          move16();
    while ((L_sub(sfbDist, thresh125)) > 0 && (cnt < 3)) {
      test(); test();
      scf = add(scf, 1);
      sfbDist = calcSfbDist(spec, sfbWidth, scf);
      test();
      if (L_sub(sfbDist, sfbDistBest) < 0) {
        scfBest = scf;                                              move16();
        sfbDistBest = sfbDist;                                      move32();
      }
      cnt = add(cnt, 1);
    }
    cnt = 0;                                                          move16();
    scf = scfEstimated;                                               move16();
    sfbDist = sfbDistBest;                                            move32();
    while ((L_sub(sfbDist, thresh125) > 0) && (cnt < 1) && (sub(scf,minScf) > 0)) {
      test(); test(); test();
      scf = sub(scf, 1);
      sfbDist = calcSfbDist(spec, sfbWidth, scf);
      test();
      if (L_sub(sfbDist, sfbDistBest) < 0) {
        scfBest = scf;                                              move16();
        sfbDistBest = sfbDist;                                      move32();
      }
      *minScfCalculated = scf;                                       move16();
      cnt = add(cnt, 1);
    }
    *dist = sfbDistBest;                                              move32();
  }
  else {
    Word32 sfbDistBest; 
    Word32 sfbDistAllowed;
    Word32 thresh08 = fixmul(0x66666666, thresh);
    sfbDistBest = sfbDist;                                            move32();
    test(); move32();
    if (L_sub(sfbDist, thresh08) < 0)
      sfbDistAllowed = sfbDist;
    else
      sfbDistAllowed = thresh08;
    for (cnt=0; cnt<3; cnt++) {
      scf = add(scf, 1);
      sfbDist = calcSfbDist(spec, sfbWidth, scf);
      test();
      if (L_sub(fixmul(0x66666666,sfbDist), sfbDistAllowed) < 0) {
        *minScfCalculated = add(scfBest, 1);
        scfBest = scf;                                              move16();
        sfbDistBest = sfbDist;                                      move32();
      }
    }
    *dist = sfbDistBest;                                              move32();
  }

  /* return best scalefactor */
  return scfBest;
}

static Word16 countSingleScfBits(Word16 scf, Word16 scfLeft, Word16 scfRight)
{
  Word16 scfBits;

  scfBits = add(bitCountScalefactorDelta(sub(scfLeft, scf)),
                bitCountScalefactorDelta(sub(scf, scfRight)));

  return scfBits;
}

static Word16 calcSingleSpecPe(Word16 scf, Word16 sfbConstPePart, Word16 nLines)
{
  Word16 specPe;
  Word16 ldRatio;
  Word16 scf3;

  ldRatio = shl(sfbConstPePart,3); /*  (sfbConstPePart -0.375*scf)*8 */
  scf3 = add(add(scf, scf), scf);
  ldRatio = sub(ldRatio, scf3);
  
  test();
  if (sub(ldRatio, 8*3) < 0) {
    /* 21 : 2*8*PE_C2, 2*PE_C3 ~ 1*/ 
    ldRatio = shr(add(ldRatio, 21), 1);
  }
  specPe = extract_l(L_mult(nLines, ldRatio));
  specPe = mult(specPe, 0x059a);

  return specPe;
}



static Word16 countScfBitsDiff(Word16 *scfOld, Word16 *scfNew, 
                               Word16 sfbCnt, Word16 startSfb, Word16 stopSfb)
{
  Word16 scfBitsDiff;
  Word16 sfb, sfbLast;
  Word16 sfbPrev, sfbNext;

  scfBitsDiff = 0;                                                      move16();
  sfb = 0;                                                              move16();

  /* search for first relevant sfb */
  sfbLast = startSfb;                                                   move16();
  while (sub(sfbLast, stopSfb) < 0 && sub(scfOld[sfbLast], SHRT_MIN) == 0) {
    test(); test();
    sfbLast = add(sfbLast, 1);
  }
  /* search for previous relevant sfb and count diff */
  sfbPrev = sub(startSfb, 1);
  while ((sfbPrev>=0) && sub(scfOld[sfbPrev], SHRT_MIN) == 0) {
    test(); test();
    sfbPrev = sub(sfbPrev, 1);
  }
  test();
  if (sfbPrev>=0) {
    scfBitsDiff = add(scfBitsDiff,
                      sub(bitCountScalefactorDelta(sub(scfNew[sfbPrev], scfNew[sfbLast])),
                          bitCountScalefactorDelta(sub(scfOld[sfbPrev], scfOld[sfbLast]))));
  }
  /* now loop through all sfbs and count diffs of relevant sfbs */
  for (sfb=sfbLast+1; sfb<stopSfb; sfb++) {
    test();
    if (sub(scfOld[sfb], SHRT_MIN) != 0) {
      scfBitsDiff = add(scfBitsDiff,
                        sub(bitCountScalefactorDelta(sub(scfNew[sfbLast], scfNew[sfb])),
                            bitCountScalefactorDelta(sub(scfOld[sfbLast], scfOld[sfb]))));
      sfbLast = sfb;                                                    move16();
    }
  }
  /* search for next relevant sfb and count diff */
  sfbNext = stopSfb;                                                    move16();
  while (sub(sfbNext, sfbCnt) < 0 && sub(scfOld[sfbNext], SHRT_MIN) == 0) {
    test(); test();
    sfbNext = add(sfbNext, 1);
  }
  test();
  if (sub(sfbNext, sfbCnt) < 0)
    scfBitsDiff = add(scfBitsDiff,
                      sub(bitCountScalefactorDelta(sub(scfNew[sfbLast], scfNew[sfbNext])),
                      bitCountScalefactorDelta(sub(scfOld[sfbLast], scfOld[sfbNext]))));

  return scfBitsDiff;
}

static Word16 calcSpecPeDiff(Word16 *scfOld,
                             Word16 *scfNew,
                             Word16 *sfbConstPePart,
                             Word16 *logSfbEnergy,
                             Word16 *logSfbFormFactor,
                             Word16 *sfbNRelevantLines,
                             Word16 startSfb, 
                             Word16 stopSfb)
{
  Word16 specPeDiff;
  Word16 sfb;

  specPeDiff = 0;                                                       move16();

  /* loop through all sfbs and count pe difference */
  for (sfb=startSfb; sfb<stopSfb; sfb++) {

    test();
    if (sub(scfOld[sfb],SHRT_MIN) != 0) {
      Word16 ldRatioOld, ldRatioNew;
      Word16 scf3;

      test();
      if (sub(sfbConstPePart[sfb], MIN_16) == 0) {
        sfbConstPePart[sfb] = shr(add(sub(logSfbEnergy[sfb], 
                                          logSfbFormFactor[sfb]), 11-8*4+3), 2);
      }


      ldRatioOld = shl(sfbConstPePart[sfb], 3);
      scf3 = add(add(scfOld[sfb], scfOld[sfb]), scfOld[sfb]);
      ldRatioOld = sub(ldRatioOld, scf3);
      ldRatioNew = shl(sfbConstPePart[sfb], 3);
      scf3 = add(add(scfNew[sfb], scfNew[sfb]), scfNew[sfb]);
      ldRatioNew = sub(ldRatioNew, scf3);

      test();
      if (sub(ldRatioOld, 8*3) < 0) {
        /* 21 : 2*8*PE_C2, 2*PE_C3 ~ 1*/
        ldRatioOld = shr(add(ldRatioOld, 21), 1);
      }

      test();
      if (sub(ldRatioNew, 8*3) < 0) {
        /* 21 : 2*8*PE_C2, 2*PE_C3 ~ 1*/
        ldRatioNew = shr(add(ldRatioNew, 21), 1);
      }

      specPeDiff = add(specPeDiff,
                       extract_l(L_mult(sfbNRelevantLines[sfb], sub(ldRatioNew, ldRatioOld))));
    }
  }

  specPeDiff = mult(specPeDiff, 0x059a);

  return specPeDiff;
}



static void assimilateSingleScf(PSY_OUT_CHANNEL *psyOutChan,
                                Word16 *scf, 
                                Word16 *minScf,
                                Word32 *sfbDist, 
                                Word16 *sfbConstPePart,
                                Word16 *logSfbEnergy,
                                Word16 *logSfbFormFactor,
                                Word16 *sfbNRelevantLines,
                                Word16 *minScfCalculated,
                                Flag    restartOnSuccess)
{
  Word16 sfbLast, sfbAct, sfbNext;
  Word16 scfAct, *scfLast, *scfNext, scfMin;
  Word16 sfbPeOld, sfbPeNew;
  Word32 sfbDistNew;
  Word16 j;
  Flag   success;
  Word16 deltaPe, deltaPeNew, deltaPeTmp;
  Word16 prevScfLast[MAX_GROUPED_SFB], prevScfNext[MAX_GROUPED_SFB];
  Word16 deltaPeLast[MAX_GROUPED_SFB];
  Flag   updateMinScfCalculated;

  success = 0;                                                                  move16();
  deltaPe = 0;                                                                  move16();

  for(j=0;j<psyOutChan->sfbCnt;j++){
    prevScfLast[j] = MAX_16;                                                    move16();
    prevScfNext[j] = MAX_16;                                                    move16();
    deltaPeLast[j] = MAX_16;                                                    move16();
  }

    
  sfbLast = -1;                                                                 move16();
  sfbAct = -1;                                                                  move16();
  sfbNext = -1;                                                                 move16();
  scfLast = 0;
  scfNext = 0;
  scfMin = MAX_16;                                                              move16();
  do {
    /* search for new relevant sfb */
    sfbNext = add(sfbNext, 1);
    while (sub(sfbNext,psyOutChan->sfbCnt)<0 && sub(scf[sfbNext],MIN_16)==0) {
      test(); test();
      sfbNext = add(sfbNext, 1);
    }
    test(); test(); test();
    if ((sfbLast>=0) && (sfbAct>=0) && sub(sfbNext,psyOutChan->sfbCnt)<0) {
      /* relevant scfs to the left and to the right */
      scfAct  = scf[sfbAct];                                                    move16();
      scfLast = scf + sfbLast;
      scfNext = scf + sfbNext;
      scfMin  = S_min(*scfLast, *scfNext);
    }
    else {
      test(); test(); test();
      if (sub(sfbLast,-1)==0 && (sfbAct>=0) && sub(sfbNext,psyOutChan->sfbCnt)<0) {
        /* first relevant scf */
        scfAct  = scf[sfbAct];                                                  move16();
        scfLast = &scfAct;
        scfNext = scf + sfbNext;
        scfMin  = *scfNext;                                                     move16();
      }
      else {
        test(); test(); test();
        if ((sfbLast>=0) && (sfbAct>=0) && sub(sfbNext,psyOutChan->sfbCnt)==0) {
          /* last relevant scf */
          scfAct  = scf[sfbAct];                                                move16();
          scfLast = scf + sfbLast;
          scfNext = &scfAct;
          scfMin  = *scfLast;                                                   move16();
        }
      }
    }
    test();
    if (sfbAct>=0)
      scfMin = S_max(scfMin, minScf[sfbAct]);

    test(); test(); test(); test(); test(); test(); test();
    if ((sfbAct >= 0) && 
        (sfbLast>=0 || sub(sfbNext,psyOutChan->sfbCnt)<0) && 
        sub(scfAct,scfMin) > 0 && 
        (sub(*scfLast,prevScfLast[sfbAct])!=0 || 
         sub(*scfNext,prevScfNext[sfbAct])!=0 || 
         sub(deltaPe,deltaPeLast[sfbAct])<0)) {
      success = 0;                                                              move16();

      /* estimate required bits for actual scf */
      test();
      if (sub(sfbConstPePart[sfbAct], MIN_16) == 0) {
        sfbConstPePart[sfbAct] = add(sub(logSfbEnergy[sfbAct],
                                         logSfbFormFactor[sfbAct]), 11-8*4);
        test();
        if (sfbConstPePart[sfbAct] < 0)
          sfbConstPePart[sfbAct] = add(sfbConstPePart[sfbAct], 3);
        sfbConstPePart[sfbAct] = shr(sfbConstPePart[sfbAct], 2);
      }
                                  
      sfbPeOld = add(calcSingleSpecPe(scfAct, sfbConstPePart[sfbAct], 
                                      sfbNRelevantLines[sfbAct]),
                     countSingleScfBits(scfAct, *scfLast, *scfNext));
      deltaPeNew = deltaPe;                                                     move16();
      updateMinScfCalculated = 1;                                               move16();
      do {
        scfAct = sub(scfAct, 1);
        /* check only if the same check was not done before */
        test();
        if (sub(scfAct, minScfCalculated[sfbAct]) < 0) {
          sfbPeNew = add(calcSingleSpecPe(scfAct, sfbConstPePart[sfbAct], 
                                         sfbNRelevantLines[sfbAct]),
                         countSingleScfBits(scfAct, *scfLast, *scfNext));
          /* use new scf if no increase in pe and 
             quantization error is smaller */
          deltaPeTmp = add(deltaPe, sub(sfbPeNew, sfbPeOld));
          test();
          if (sub(deltaPeTmp, 10) < 0) {
            sfbDistNew = calcSfbDist(psyOutChan->mdctSpectrum+
                                       psyOutChan->sfbOffsets[sfbAct],
                                     sub(psyOutChan->sfbOffsets[sfbAct+1],
                                         psyOutChan->sfbOffsets[sfbAct]),
                                     scfAct);
            if (L_sub(sfbDistNew, sfbDist[sfbAct]) < 0) {
              /* success, replace scf by new one */
              scf[sfbAct] = scfAct;                                     move16();
              sfbDist[sfbAct] = sfbDistNew;                             move32();
              deltaPeNew = deltaPeTmp;                                  move16();
              success = 1;                                              move16();
            }
            /* mark as already checked */
            test();
            if (updateMinScfCalculated) {
              minScfCalculated[sfbAct] = scfAct;                        move16();
            }
          }
          else {
            updateMinScfCalculated = 0;                                 move16();
          }
        }
        test();
      } while (sub(scfAct, scfMin) > 0);
      deltaPe = deltaPeNew;                                             move16();
      /* save parameters to avoid multiple computations of the same sfb */
      prevScfLast[sfbAct] = *scfLast;                                   move16();
      prevScfNext[sfbAct] = *scfNext;                                   move16();
      deltaPeLast[sfbAct] = deltaPe;                                    move16();
    }
    test(); test();
    if (success && restartOnSuccess) {
      /* start again at first sfb */
      sfbLast = -1;                                                     move16();
      sfbAct  = -1;                                                     move16();
      sfbNext = -1;                                                     move16();
      scfLast = 0;
      scfNext = 0;
      scfMin  = MAX_16;                                                 move16();
      success = 0;                                                      move16();
    }
    else {
      /* shift sfbs for next band */
      sfbLast = sfbAct;                                                 move16();
      sfbAct  = sfbNext;                                                move16();
    }
    test();
  } while (sub(sfbNext, psyOutChan->sfbCnt) < 0);

}


static void assimilateMultipleScf(PSY_OUT_CHANNEL *psyOutChan,
                                  Word16 *scf, 
                                  Word16 *minScf,
                                  Word32 *sfbDist, 
                                  Word16 *sfbConstPePart,
                                  Word16 *logSfbEnergy,
                                  Word16 *logSfbFormFactor,
                                  Word16 *sfbNRelevantLines)
{
  Word16 sfb, startSfb, stopSfb;
  Word16 scfTmp[MAX_GROUPED_SFB], scfMin, scfMax, scfAct;
  Flag   possibleRegionFound;
  Word32 sfbDistNew[MAX_GROUPED_SFB];
  Word16 deltaScfBits;
  Word16 deltaSpecPe;
  Word16 deltaPe, deltaPeNew;
  Word16 sfbCnt;

  deltaPe = 0;                                                          move16();
  sfbCnt = psyOutChan->sfbCnt;                                          move16();

  /* calc min and max scalfactors */
  scfMin = MAX_16;                                                      move16();
  scfMax = MIN_16;                                                      move16();
  for (sfb=0; sfb<sfbCnt; sfb++) {
    test();
    if (sub(scf[sfb],MIN_16) != 0) {
      scfMin = S_min(scfMin, scf[sfb]);
      scfMax = S_max(scfMax, scf[sfb]);
    }
  }

  test();
  if (sub(scfMax, MIN_16) != 0) {

    scfAct = scfMax;                                                    move16();

    do {
      scfAct = sub(scfAct,1);
      for (sfb=0; sfb<sfbCnt; sfb++) {
        scfTmp[sfb] = scf[sfb];                                         move16();
      }
      stopSfb = 0;                                                      move16();
      do {
        sfb = stopSfb;                                                  move16();
        test(); test(); test();
        while (sub(sfb,sfbCnt)<0 && (sub(scf[sfb],MIN_16)==0 || sub(scf[sfb],scfAct)<=0)) {
          sfb = add(sfb, 1);
        }
        startSfb = sfb;                                                 move16();
        sfb = add(sfb, 1);
        test(); test(); test();
        while (sub(sfb,sfbCnt)<0 && (sub(scf[sfb],MIN_16)==0 || sub(scf[sfb],scfAct)>0)) {
          sfb = add(sfb, 1);
        }
        stopSfb = sfb;                                                  move16();

        possibleRegionFound = 0;                                        move16();
        test();
        if (sub(startSfb,sfbCnt) < 0) {
          possibleRegionFound = 1;                                      move16();
          for (sfb=startSfb; sfb<stopSfb; sfb++) {
            test();
            if (sub(scf[sfb],MIN_16) != 0) {
              test();
              if (sub(scfAct,minScf[sfb]) < 0) {
                possibleRegionFound = 0;                                move16();
                break;
              }
            }
          }
        }

        test();
        if (possibleRegionFound) { /* region found */

          /* replace scfs in region by scfAct */
          for (sfb=startSfb; sfb<stopSfb; sfb++) {
            test();
            if (sub(scfTmp[sfb],MIN_16) != 0)
              scfTmp[sfb] = scfAct;                                     move16();
          }

          /* estimate change in bit demand for new scfs */
          deltaScfBits = countScfBitsDiff(scf,scfTmp,sfbCnt,startSfb,stopSfb);
          deltaSpecPe = calcSpecPeDiff(scf, scfTmp, sfbConstPePart,
                                       logSfbEnergy, logSfbFormFactor, sfbNRelevantLines, 
                                       startSfb, stopSfb);
          deltaPeNew = add(deltaPe, add(deltaScfBits, deltaSpecPe));

          test();
          if (sub(deltaPeNew, 10) < 0) {
            Word32 distOldSum, distNewSum;

            /* quantize and calc sum of new distortion */
            distOldSum = 0;                                                     move32();
            distNewSum = 0;                                                     move32();
            for (sfb=startSfb; sfb<stopSfb; sfb++) {
              test();
              if (sub(scfTmp[sfb],MIN_16) != 0) {
                distOldSum = L_add(distOldSum, sfbDist[sfb]);

                sfbDistNew[sfb] = calcSfbDist(psyOutChan->mdctSpectrum +
                                                psyOutChan->sfbOffsets[sfb], 
                                              sub(psyOutChan->sfbOffsets[sfb+1],
                                                  psyOutChan->sfbOffsets[sfb]),
                                              scfAct);

                test();
                if (L_sub(sfbDistNew[sfb],psyOutChan->sfbThreshold[sfb]) > 0) {
                  distNewSum = L_shl(distOldSum, 1);
                  break;
                }
                distNewSum = L_add(distNewSum, sfbDistNew[sfb]);
              }
            }
            test();
            if (L_sub(distNewSum,distOldSum) < 0) {
              deltaPe = deltaPeNew;                                             move16();
              for (sfb=startSfb; sfb<stopSfb; sfb++) {
                test();
                if (sub(scf[sfb],MIN_16) != 0) {
                  scf[sfb] = scfAct;                                            move16();
                  sfbDist[sfb] = sfbDistNew[sfb];                               move32();
                }
              }
            }

          }
        }


        test();
      } while (sub(stopSfb,sfbCnt) <= 0);

      test();
    } while (sub(scfAct,scfMin) > 0);
  }

}




static void
EstimateScaleFactorsChannel(PSY_OUT_CHANNEL *psyOutChan,
                            Word16          *scf,
                            Word16          *globalGain,
                            Word16          *logSfbEnergy,
                            Word16          *logSfbFormFactor,
                            Word16          *sfbNRelevantLines)
{
  Word16 i, j;
  Word32 thresh, energy;
  Word16 energyPart, thresholdPart;
  Word16 scfInt, minScf, maxScf, maxAllowedScf, lastSf;
  Word32 maxSpec;
  Word32 sfbDist[MAX_GROUPED_SFB];
  Word16 minSfMaxQuant[MAX_GROUPED_SFB];
  Word16 minScfCalculated[MAX_GROUPED_SFB];


  for (i=0; i<psyOutChan->sfbCnt; i++) {

    thresh = psyOutChan->sfbThreshold[i];                                       move32();
    energy = psyOutChan->sfbEnergy[i];                                          move32();

    maxSpec = 0;                                                                move32();
    /* maximum of spectrum */
    for (j=psyOutChan->sfbOffsets[i]; j<psyOutChan->sfbOffsets[i+1]; j++ ) {
      Word32 absSpec = L_abs(psyOutChan->mdctSpectrum[j]);
      maxSpec |= absSpec;                                                       logic32();
    }

    /* scfs without energy or with thresh>energy are marked with MIN_16 */
    scf[i] = MIN_16;                                                            move16();
    minSfMaxQuant[i] = MIN_16;                                                  move16();

    test(); test();
    if ((maxSpec > 0) && (L_sub(energy, thresh) > 0)) {
      
      energyPart = logSfbFormFactor[i];                                         move16();
      thresholdPart = ffr_iLog4(thresh);    
      scfInt = mult(sub(sub(thresholdPart,energyPart),20), 0x5555);

      minSfMaxQuant[i] = sub(ffr_iLog4(maxSpec), 68); /* 68  -16/3*log(MAX_QUANT+0.5-logCon)/log(2) + 1 */

      test();
      if (sub(minSfMaxQuant[i], scfInt) > 0) {
        scfInt = minSfMaxQuant[i];                                              move16();
      }

      /* find better scalefactor with analysis by synthesis */
      scfInt = improveScf(psyOutChan->mdctSpectrum+psyOutChan->sfbOffsets[i],
                          sub(psyOutChan->sfbOffsets[i+1], psyOutChan->sfbOffsets[i]),
                          thresh, scfInt, minSfMaxQuant[i], 
                          &sfbDist[i], &minScfCalculated[i]);

      scf[i] = scfInt;                                                          move16();
    }
  }

  
  {
    Word16 sfbConstPePart[MAX_GROUPED_SFB];
    for(i=0;i<psyOutChan->sfbCnt;i++) {
      sfbConstPePart[i] = MIN_16;                                               move16();
    }
     
    assimilateSingleScf(psyOutChan, scf, 
                        minSfMaxQuant, sfbDist, sfbConstPePart, logSfbEnergy,
                        logSfbFormFactor, sfbNRelevantLines, minScfCalculated, 1);

    assimilateMultipleScf(psyOutChan, scf, 
                          minSfMaxQuant, sfbDist, sfbConstPePart, logSfbEnergy,
                          logSfbFormFactor, sfbNRelevantLines);
  }

  /* get max scalefac for global gain */
  maxScf = MIN_16;                                                              move16();
  minScf = MAX_16;                                                              move16();
  for (i=0; i<psyOutChan->sfbCnt; i++) {
    test();
    if (sub(maxScf, scf[i]) < 0) {
      maxScf = scf[i];                                                          move16();
    }
    test(); test();
    if ((sub(scf[i], MIN_16) != 0) && (sub(minScf, scf[i]) > 0)) {
      minScf = scf[i];                                                          move16();
    }
  }
  /* limit scf delta */
  maxAllowedScf = add(minScf, MAX_SCF_DELTA);
  for(i=0; i<psyOutChan->sfbCnt; i++) {
    test(); test();
    if ((sub(scf[i], MIN_16) != 0) && (sub(maxAllowedScf, scf[i]) < 0)) {
      scf[i] = maxAllowedScf;                                                   move16();
    }
  }
  /* new maxScf if any scf has been limited */
  test();
  if (sub(maxAllowedScf, maxScf) < 0) {
    maxScf = maxAllowedScf;                                                     move16();
  }


  
  /* calc loop scalefactors */
  test();
  if (sub(maxScf, MIN_16) > 0) {
    *globalGain = maxScf;                                                       move16();
    lastSf = 0;                                                                 move16();

    for(i=0; i<psyOutChan->sfbCnt; i++) {
      test();
      if (sub(scf[i], MIN_16) == 0) {
        scf[i] = lastSf;                                                        move16();
        /* set band explicitely to zero */
        for (j=psyOutChan->sfbOffsets[i]; j<psyOutChan->sfbOffsets[i+1]; j++) {
          psyOutChan->mdctSpectrum[j] = 0;                                      move32();
        }
      }
      else {
        scf[i] = sub(maxScf, scf[i]);
        lastSf = scf[i];                                                        move16();
      }
    }
  }
  else{
    *globalGain = 0;                                                            move16();
    /* set spectrum explicitely to zero */
    for(i=0; i<psyOutChan->sfbCnt; i++) {
      scf[i] = 0;                                                               move16();
      for (j=psyOutChan->sfbOffsets[i]; j<psyOutChan->sfbOffsets[i+1]; j++) {
        psyOutChan->mdctSpectrum[j] = 0;                                        move32();
      }
    }
  }

}


void
CalcFormFactor(Word16 logSfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
               Word16 sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
               Word16 logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
               PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
               const Word16 nChannels)
{
  Word16 j;

  for (j=0; j<nChannels; j++) {
    CalcFormFactorChannel(logSfbFormFactor[j], sfbNRelevantLines[j], logSfbEnergy[j], &psyOutChannel[j]);
  }
}


void
EstimateScaleFactors(PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
                     QC_OUT_CHANNEL  qcOutChannel[MAX_CHANNELS],
                     Word16          logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                     Word16          logSfbFormFactor[MAX_CHANNELS][MAX_GROUPED_SFB],
                     Word16          sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
                     const Word16    nChannels)
{
  Word16 j;

  for (j=0; j<nChannels; j++) {
    EstimateScaleFactorsChannel(&psyOutChannel[j],
                                qcOutChannel[j].scf,
                                &(qcOutChannel[j].globalGain),
                                logSfbEnergy[j],
                                logSfbFormFactor[j],
                                sfbNRelevantLines[j]);
  }
}

