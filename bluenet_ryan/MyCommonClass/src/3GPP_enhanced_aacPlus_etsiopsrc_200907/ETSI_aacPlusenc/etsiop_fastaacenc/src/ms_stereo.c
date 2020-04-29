/*
   MS stereo processing
 */
#include <stdio.h>
#include "psy_const.h"
#include "ms_stereo.h"
#include "ffr.h"
#include "count.h"

/**************************************************************************/
/*!
  \brief  calculates 100*atan(x/100)
          based on atan approx for x > 0

  \return    100*atan(x/100)

*/
/**************************************************************************/

static Word32 atan_100(Word32 val) 
{
  Word32 temp, sqrval, val100, y; 

  test();
  if (val > 0) {
    val = L_min(val, 10000);
    sqrval = ffr_Integer_Mult16x16(val, val);
    val100 = ffr_Integer_Mult16x16(100, val);
    temp = L_sub(val, 100);
    test();
    if(temp < 0) {
      y = ffr_divideWord32(val100, 
                           L_add(100, ffr_divideWord32(sqrval, 356)));
    }
    else {
      y = L_sub(157, ffr_divideWord32(val100,
                                      L_add(28, ffr_divideWord32(sqrval,100)))); 
    }
  }
  else{
    val = L_max(val,-10000);
    sqrval = ffr_Integer_Mult16x16(val, val);
    val100 = ffr_Integer_Mult16x16(-100, val);
    temp = L_sub( val, -100 );
    test();
    if(temp > 0) {
      y = ffr_divideWord32(val100,
                           L_add(100, ffr_divideWord32(sqrval, 356)));
      y = L_negate(y);
    }
    else {
      y = L_add(-157, ffr_divideWord32(val100,
                                       L_add(28, ffr_divideWord32(sqrval,100)))); 
    }
  }

  return y;
}


void MsStereoProcessing(Word32       *sfbEnergyLeft,
                        Word32       *sfbEnergyRight,
                        const Word32 *sfbEnergyMid,
                        const Word32 *sfbEnergySide,
                        Word32       *mdctSpectrumLeft,
                        Word32       *mdctSpectrumRight,
                        Word32       *sfbThresholdLeft,
                        Word32       *sfbThresholdRight,
                        Word32       *sfbSpreadedEnLeft,
                        Word32       *sfbSpreadedEnRight,
                        Word16       *msDigest,
                        Word16       *msMask,
                        const Word16  sfbCnt,
                        const Word16  sfbPerGroup,
                        const Word16  maxSfbPerGroup,
                        const Word16 *sfbOffset,
                        Word16       *weightMsLrPeRatio) {
  Word32 temp;
  Word16 sfb,sfboffs, j, cnt = 0;
  Word16 msMaskTrueSomewhere = 0;
  Word16 msMaskFalseSomewhere = 0;
  Word32 sumMsLrPeRatio = 0;
                                                                                                   move32(); move32(); move32(); move32();

  for (sfb=0; sfb<sfbCnt; sfb+=sfbPerGroup) {
    for (sfboffs=0;sfboffs<maxSfbPerGroup;sfboffs++) {

      Word32 temp;
      Word32 pnlr,pnms;
      Word32 minThreshold;
      Word32 thrL, thrR, nrgL, nrgR;
      Word16 idx;

      idx = add(sfb,sfboffs);                                                                      move16();

      thrL = sfbThresholdLeft[idx];                                                                move32();
      thrR = sfbThresholdRight[idx];                                                               move32();
      nrgL = sfbEnergyLeft[idx];                                                                   move32();
      nrgR = sfbEnergyRight[idx];                                                                  move32();

      minThreshold = L_min(thrL, thrR);

      nrgL = L_add(L_max(nrgL,thrL), 1);
      nrgL = ffr_div32_32(thrL, nrgL);
      nrgR = L_add(L_max(nrgR,thrR), 1);
      nrgR = ffr_div32_32(thrR, nrgR);

      pnlr = fixmul(nrgL, nrgR);

      nrgL = sfbEnergyMid[idx];                                                                    move32();
      nrgR = sfbEnergySide[idx];                                                                   move32();

      nrgL = L_add(L_max(nrgL,minThreshold), 1);
      nrgL = ffr_div32_32(minThreshold, nrgL);

      nrgR = L_add(L_max(nrgR,minThreshold), 1);
      nrgR = ffr_div32_32(minThreshold, nrgR);

      pnms = fixmul(nrgL, nrgR);

      temp = ffr_divideWord32( L_add(pnlr,1), L_add(L_shr(pnms,8),1) );
      sumMsLrPeRatio = L_add(sumMsLrPeRatio, temp);

      cnt = add(cnt, 1);

      temp = L_sub(pnms, pnlr);                                                                    test();
      if( temp > 0 ){

        msMask[idx] = 1;                                                                           move16();
        msMaskTrueSomewhere = 1;                                                                   move16();

        for (j=sfbOffset[idx]; j<sfbOffset[idx+1]; j++) {
          Word32 left, right;
          left  = L_shr(mdctSpectrumLeft[j], 1);
          right = L_shr(mdctSpectrumRight[j], 1);
          mdctSpectrumLeft[j] = L_add( left, right );                                              move32();
          mdctSpectrumRight[j] = L_sub( left, right );                                             move32();
        }
        
        sfbThresholdLeft[idx] = minThreshold;                                                      move32();
        sfbThresholdRight[idx] = minThreshold;                                                     move32();
        sfbEnergyLeft[idx] = sfbEnergyMid[idx];                                                    move32();
        sfbEnergyRight[idx] = sfbEnergySide[idx];                                                  move32();

        sfbSpreadedEnRight[idx] = L_shr(L_min(sfbSpreadedEnLeft[idx],sfbSpreadedEnRight[idx]), 1); move32();
        sfbSpreadedEnLeft[idx] = sfbSpreadedEnRight[idx];                                          move32();
        
      }
      else {
        msMask[idx]  = 0;                                                                          move16();
        msMaskFalseSomewhere = 1;                                                                  move16();
      }
    }                                                                                              test();
    if ( msMaskTrueSomewhere ) {                                                                   test();
      if(msMaskFalseSomewhere ) {
        *msDigest = SI_MS_MASK_SOME;                                                               move16();
      } else {
        *msDigest = SI_MS_MASK_ALL;                                                                move16();
      }
    } else {
      *msDigest = SI_MS_MASK_NONE;                                                                 move16();
    }

    cnt = S_max(1,cnt);
    /*
      *weightMsLrPeRatio = 28 *  atan( 0.37f*((float)sumMsLrPeRatio/(float)cnt-6.5f) ) + 125; 
    */
    
    temp = atan_100(L_sub(ffr_divideWord32(ffr_Integer_Mult(37,sumMsLrPeRatio),shl(cnt,8)), 241));
    test();
    if (temp >= 0) {
      *weightMsLrPeRatio = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(28,temp), 100));
      *weightMsLrPeRatio = add(125, *weightMsLrPeRatio);
    }
    else {
      *weightMsLrPeRatio = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(-28,temp), 100));
      *weightMsLrPeRatio = sub(125, *weightMsLrPeRatio);
    }
  }

}
