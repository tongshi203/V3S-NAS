/*
   stereo pre-processing
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ffr.h"
#include "stprepro.h"
#include "count.h"

/***************************************************************************/
/*!
 
\brief  create and initialize a handle for stereo preprocessing
 
\return an error state
 
****************************************************************************/
Word16 InitStereoPreProcessing(HANDLE_STEREO_PREPRO hStPrePro, /*! handle (modified) */
                               Word16 nChannels,    /*! number of channels */ 
                               Word32 bitRate,      /*! the bit rate */
                               Word32 sampleRate,   /*! the sample rate */
                               Word16 usedScfRatio  /*! the amount of scalefactors used (0-100) */
                               )
{
  Word16 bpf = extract_l(ffr_divideWord32(L_shl(bitRate, 10), sampleRate));
  Word16 tmp;
  
  memset(hStPrePro,0,sizeof(struct STEREO_PREPRO));
  
  test();
  if (sub(nChannels, 2) == 0) {

    (hStPrePro)->stereoAttenuationFlag = 1;                                     move16();

    (hStPrePro)->normPeFac = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(230*100, usedScfRatio), bpf));
     
    (hStPrePro)->ImpactFactor = extract_l(ffr_divideWord32(400000, L_sub(bitRate,
      ffr_divideWord32(ffr_Integer_Mult(sampleRate,sampleRate),72000))));
    test();
    if (sub(hStPrePro->ImpactFactor, 1) < 0) {
      (hStPrePro)->ImpactFactor = 1;                                            move16();
    }

    (hStPrePro)->stereoAttenuationInc = extract_l(ffr_divideWord32(100*22050*400, ffr_Integer_Mult(sampleRate, bpf)));
    (hStPrePro)->stereoAttenuationDec = extract_l(ffr_divideWord32(100*22050*200, ffr_Integer_Mult(sampleRate, bpf)));

    (hStPrePro)->ConstAtt     = 0;                                              move16();
    (hStPrePro)->stereoAttMax = 1200;      /* 12 db */                          move16();

    /* energy ratio thresholds (dB) */
    (hStPrePro)->SMMin = 0;                                                     move16();
    (hStPrePro)->SMMax = 15;                                                    move16();
    (hStPrePro)->LRMin = 10;                                                    move16();
    (hStPrePro)->LRMax = 30;                                                    move16();

    /* pe thresholds */
    (hStPrePro)->PeCrit =  1200;                                                move16();
    (hStPrePro)->PeMin  =  700;                                                 move16();
    (hStPrePro)->PeImpactMax = ffr_Integer_Mult16x16(100, sub(hStPrePro->PeCrit, hStPrePro->PeMin));

    /* init start values */
    (hStPrePro)->avrgFreqEnergyL  = 0;                                          move32();
    (hStPrePro)->avrgFreqEnergyR  = 0;                                          move32();
    (hStPrePro)->avrgFreqEnergyS  = 0;                                          move32();
    (hStPrePro)->avrgFreqEnergyM  = 0;                                          move32();
    (hStPrePro)->smoothedPeSumSum = 7000; /* typical start value */             move16();
    (hStPrePro)->avgStoM          = -10;  /* typical start value */             move16();
    (hStPrePro)->lastLtoR         = 0;                                          move16();
    (hStPrePro)->lastNrgLR        = 0;                                          move32();
    (hStPrePro)->stereoAttFac     = 0;                                          move32();

    tmp = sub(100, ffr_Short_Div(bpf, 26));
    tmp = S_max(tmp, 0);
    
    (hStPrePro)->stereoAttenuation = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(tmp, (hStPrePro)->stereoAttMax), 100));
  }

  return 0;
}

/***************************************************************************/
/*!
 
\brief  do an appropriate attenuation on the side channel of a stereo
signal
 
\return nothing
 
****************************************************************************/
void ApplyStereoPreProcess(HANDLE_STEREO_PREPRO hStPrePro, /*!st.-preproc handle */
                           Word16               nChannels, /*! total number of channels */              
                           ELEMENT_INFO        *elemInfo,
                           Word16              *timeData,     /*! lr time data (modified) */
                           Word16               granuleLen) /*! num. samples to be processed */
{
  /* inplace operation on inData ! */
  Word16 StoM;
  Word16 LtoR, deltaLtoR, deltaNrg;
  Word16 EnImpact;
  Word16 PeNorm;
  Word32 PeImpact;
  Word16 Att, AttAimed;
  Word16 AttNorm;
  Word16 swiftfactor;
  Word16 maxInc, maxDec;
  Word16 i;
  
  Word32 fac;
  Word32 mPart, upper, div;
  Word16 lFac, rFac;
  Word16 temp16;
  Word32 temp32;
  Word16 idxL, idxR;

  fac = hStPrePro->stereoAttFac;                                                move32();

  
  test();
  if (!hStPrePro->stereoAttenuationFlag) {
    return;
  }

  /* calc L/R ratio */
  mPart = fixmul( hStPrePro->avrgFreqEnergyM, L_sub(0x7fffffff, fixmul(fac, fac)));

  lFac = etsiopround(L_add(0x40000000, L_shr(fac,1)));
  rFac = etsiopround(L_sub(0x40000000, L_shr(fac,1)));
 
  upper =  L_sub(L_add(fixmul_32x16b(hStPrePro->avrgFreqEnergyL, lFac), fixmul_32x16b(hStPrePro->avrgFreqEnergyR, rFac)), mPart);
  div   =  L_sub(L_add(fixmul_32x16b(hStPrePro->avrgFreqEnergyR, lFac), fixmul_32x16b(hStPrePro->avrgFreqEnergyL, rFac)), mPart);
  
  LtoR = sub(ffr_iLog4(upper), ffr_iLog4(div));
  LtoR = shr(abs_s(add(LtoR, shl(LtoR,1))), 2);

  /* calc delta energy to previous frame */
  deltaNrg = sub(sub(ffr_iLog4(L_add(hStPrePro->avrgFreqEnergyL, hStPrePro->avrgFreqEnergyR)),
                     ffr_iLog4(hStPrePro->lastNrgLR)), 4);
  deltaNrg = shr(abs_s(add(deltaNrg, shl(deltaNrg,1))), 2); /* 4:lastNrgLR*2 */
  
  /* Smooth S/M over time */
  StoM = sub(ffr_iLog4(hStPrePro->avrgFreqEnergyS), ffr_iLog4(hStPrePro->avrgFreqEnergyM));
  StoM = add(StoM, shl(StoM,1));
  test();
  if (StoM >= 0)
    StoM = shr(StoM,2);
  else
    StoM = negate(shr(negate(StoM),2));

  temp32 = L_add(ffr_Integer_Mult16x16(10, StoM),
                 ffr_Integer_Mult16x16(100-10, hStPrePro->avgStoM));
  hStPrePro->avgStoM = extract_l(ffr_divideWord32(L_abs(temp32), 100));
  test();
  if (temp32 < 0)
    hStPrePro->avgStoM = negate(hStPrePro->avgStoM);
  
  AttNorm  = 1;                                                                 move16();
  EnImpact = 1;                                                                 move16();
  
  /* energy influence */
  test();
  if (sub(hStPrePro->avgStoM, hStPrePro->SMMin) > 0) {
    test();
    if (sub(hStPrePro->avgStoM, hStPrePro->SMMax) > 0) {
      EnImpact = 0;                                                             move16();
    }
    else {
      EnImpact = sub(hStPrePro->SMMax, hStPrePro->avgStoM);
      AttNorm = sub(hStPrePro->SMMax, hStPrePro->SMMin);
    }
  }
  
  /* energy influence */
  test();
  if (sub(LtoR, hStPrePro->LRMin) > 0) {
    test();
    if (sub(LtoR, hStPrePro->LRMax) > 0) {
      EnImpact = 0;                                                             move16();
    }
    else {
      EnImpact = ffr_Short_Mult(EnImpact, sub(hStPrePro->LRMax, LtoR));
      AttNorm  = ffr_Short_Mult(AttNorm, sub(hStPrePro->LRMax, hStPrePro->LRMin));
    }
  }

  
  
  /* PE influence */
  PeImpact = 0;                                                                 move16();
  
  PeNorm = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(hStPrePro->smoothedPeSumSum, hStPrePro->normPeFac), 100*100));
  
  test();
  if (sub(PeNorm, hStPrePro->PeMin) > 0)  {
    PeImpact = sub(PeNorm, hStPrePro->PeMin); 
    AttNorm = ffr_Short_Mult(AttNorm, sub(hStPrePro->PeCrit, hStPrePro->PeMin));
  }
  
  if (L_sub(PeImpact, hStPrePro->PeImpactMax) > 0) {
    PeImpact = hStPrePro->PeImpactMax;                                          move32();
  }
  
  AttAimed = extract_l(ffr_divideWord32(ffr_Integer_Mult(ffr_Integer_Mult16x16(100, EnImpact),
    ffr_Integer_Mult(PeImpact, hStPrePro->ImpactFactor)), AttNorm));
  /* AttAimed is scaled by 100 */

  test();
  if (sub(AttAimed, hStPrePro->stereoAttMax) > 0) {
    AttAimed = hStPrePro->stereoAttMax;                                         move16();
  }
  
  /* only accept changes if they are large enough */
  test(); test();
  if (sub(abs_s(sub(AttAimed, hStPrePro->stereoAttenuation)), 100) < 0 && AttAimed != 0) {
    AttAimed = hStPrePro->stereoAttenuation;                                    move16();
  }
  
  Att = AttAimed;                                                               move16();


  swiftfactor = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(add(600, hStPrePro->stereoAttenuation), S_max(5, deltaNrg)),
                                 ffr_Short_Mult(5, add(10, LtoR))));
 
  deltaLtoR = S_max(3, sub(LtoR, hStPrePro->lastLtoR));
 
  maxDec = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(ffr_Short_Mult(deltaLtoR, deltaLtoR), swiftfactor), 9));


  maxDec = S_min(maxDec, 500);

  temp32 = ffr_Integer_Mult16x16(maxDec, hStPrePro->stereoAttenuationDec);
  test();
  if (temp32 >= 0)
    maxDec = extract_l(ffr_divideWord32(temp32, 100));
  else
    maxDec = negate(extract_l(ffr_divideWord32(L_negate(temp32), 100)));

  /* Avoid drop-outs by asymptotically reaching zero attenuation: */  
  temp16 = S_max(ffr_Short_Div(shl(hStPrePro->stereoAttenuation, 3), 10), 1);
  test();
  if (sub(maxDec, temp16) > 0) {
    maxDec = temp16;                                                                    move16();
  }

  deltaLtoR = S_max(3, sub(hStPrePro->lastLtoR, LtoR));

  maxInc = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(ffr_Short_Mult(deltaLtoR, deltaLtoR), swiftfactor), 9));

  maxInc = S_min(maxInc, 500);

  maxInc = extract_l(ffr_divideWord32(ffr_Integer_Mult16x16(maxInc, hStPrePro->stereoAttenuationInc), 100));
  
  test();
  if (sub(Att, add(hStPrePro->stereoAttenuation, maxInc)) > 0) {
    Att = add(hStPrePro->stereoAttenuation, maxInc);
  }

  test();
  if (sub(Att, sub(hStPrePro->stereoAttenuation, maxDec)) < 0) {
    Att = sub(hStPrePro->stereoAttenuation, maxDec);
  }

  test(); move16();
  if (hStPrePro->ConstAtt == 0) 
    hStPrePro->stereoAttenuation = Att;
  else                          
    hStPrePro->stereoAttenuation = hStPrePro->ConstAtt;
  
  /* perform attenuation of Side Channel */

  /*
    we calculate pow(10.0f, (float)0.05f*(-hStPrePro->stereoAttenuation)/100.0);
  */
  hStPrePro->stereoAttFac = ffr_pow2_xy(negate(hStPrePro->stereoAttenuation), 602); /* 602: 1.0/(0.05/100.0*log2(10)) */
  

  lFac = etsiopround(L_add(0x40000000, L_shr(hStPrePro->stereoAttFac, 1)));
  rFac = etsiopround(L_sub(0x40000000, L_shr(hStPrePro->stereoAttFac, 1)));

  idxL = elemInfo->ChannelIndex[0];
  idxR = elemInfo->ChannelIndex[1];
  for (i=0; i<granuleLen; i++) {
    Word16 tmpL = timeData[idxL];                                                       move16();
    timeData[idxL] = shl(add( shr(mult(lFac, tmpL), 1), shr(mult(rFac, timeData[idxR]), 1)), 1);
    timeData[idxR] = shl(add( shr(mult(rFac, tmpL), 1), shr(mult(lFac, timeData[idxR]), 1)), 1);
    idxL = add(idxL, nChannels);
    idxR = add(idxR, nChannels);
  }
  
  hStPrePro->lastLtoR = LtoR;                                                           move16();
  hStPrePro->lastNrgLR = L_add(L_shr(hStPrePro->avrgFreqEnergyL, 1), L_shr(hStPrePro->avrgFreqEnergyR, 1));
}


/***************************************************************************/
/*!
 
\brief  calc attenuation parameters
 
\return nothing
 
****************************************************************************/
void UpdateStereoPreProcess(PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
                            QC_OUT_ELEMENT* qcOutElement,   /*! provides access to PE */
                            HANDLE_STEREO_PREPRO hStPrePro,
                            Word16 weightPeFac               /*! ratio of ms PE vs. lr PE  100 <> 1.0 */
                            )
{
  test();
  if (hStPrePro->stereoAttenuationFlag) { 
    
    hStPrePro->avrgFreqEnergyL = L_shr(psyOutChannel[0].sfbEnSumLR, sub(22,shl(psyOutChannel[0].mdctScale,1)));
    hStPrePro->avrgFreqEnergyR = L_shr(psyOutChannel[1].sfbEnSumLR, sub(22,shl(psyOutChannel[1].mdctScale,1)));
    hStPrePro->avrgFreqEnergyM = L_shr(psyOutChannel[0].sfbEnSumMS, sub(22,shl(psyOutChannel[0].mdctScale,1)));
    hStPrePro->avrgFreqEnergyS = L_shr(psyOutChannel[1].sfbEnSumMS, sub(22,shl(psyOutChannel[1].mdctScale,1)));

    hStPrePro->smoothedPeSumSum = 
      extract_l(ffr_divideWord32(L_add(ffr_Integer_Mult16x16(qcOutElement->pe, weightPeFac),
                                       ffr_Integer_Mult16x16(900, hStPrePro->smoothedPeSumSum)), 1000));
  }
}
