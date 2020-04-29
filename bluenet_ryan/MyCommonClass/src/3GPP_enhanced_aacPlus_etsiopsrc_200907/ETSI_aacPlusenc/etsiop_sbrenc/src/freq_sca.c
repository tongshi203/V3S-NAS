/*
   frequency scale
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "freq_sca.h"
#include "sbr_misc.h"

#include "count.h"


/*  StartFreq */
static Word16 getStartFreq(const Word32 fs, const Word16 start_freq);

/* StopFreq */
static Word16 getStopFreq(const Word32 fs, const Word16 stop_freq);

static Word16  numberOfBands(Word16 b_p_o, Word16 start, Word16 stop, Word16 warpFlag);
static void CalcBands(Word16 * diff, Word16 start , Word16 stop , Word16 num_bands);
static Word16  modifyBands(Word16 max_band, Word16 * diff, Word16 length);
static void cumSum(Word16 start_value, Word16* diff, Word16 length, UWord16  *start_adress);



/*******************************************************************************
 Functionname:  getSbrStartFreqRAW
 *******************************************************************************
 Description: 
 
 Arguments:   
 
 Return:      
 *******************************************************************************/

Word16
getSbrStartFreqRAW (Word16 startFreq, Word16 QMFbands, Word32 fs)
{
  Word32 result;

  test(); test(); sub(1, 1);
  if ( startFreq < 0 || startFreq > 15)
    return -1;

  /* Update startFreq struct */
  result = getStartFreq(fs, startFreq);

  result = L_shr(L_add(ffr_Integer_Div(ffr_Integer_Mult(result, fs), QMFbands), 1), 1);

  return (extract_l(result));        
  
} /* End getSbrStartFreqRAW */


/*******************************************************************************
 Functionname:  getSbrStopFreq
 *******************************************************************************
 Description: 
 
 Arguments:   
 
 Return:      
 *******************************************************************************/
Word16 getSbrStopFreqRAW  (Word16 stopFreq, Word16 QMFbands, Word32 fs)
{
  Word32 result;
  
  test(); test(); sub(1, 1);
  if ( stopFreq < 0 || stopFreq > 13)
    return -1;
  
  
  /* Uppdate stopFreq struct */
  result = getStopFreq( fs, stopFreq);

  result = L_shr(L_add(ffr_Integer_Div(ffr_Integer_Mult(result, fs), QMFbands), 1), 1);

  return (extract_l(result));        
} /* End getSbrStopFreq */


/*******************************************************************************
 Functionname:  getStartFreq
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
static Word16
getStartFreq(const Word32 fs, const Word16 start_freq)
{
  Word16 k0_min;
  
  test();
  switch(fs){
  case 32000: k0_min = 16; move16();
    break;
  case 44100: k0_min = 12; move16();
    break;
  case 48000: k0_min = 11; move16();
    break;
  default:
    k0_min=11;  move16();/* illegal fs */

  }

  test();
  switch (fs) {

  case 32000:
    {
      Word16 v_offset[]= {-6, -4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16};
      return (add(k0_min, v_offset[start_freq]));
    }
    break;
 
  case 44100:
  case 48000:
    {
      Word16 v_offset[]= {-4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20};
      return (add(k0_min, v_offset[start_freq]));
    }
    break;
  
  default:
    {
      Word16 v_offset[]= {0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24, 28, 33};
      return (add(k0_min, v_offset[start_freq]));
    }

  }

}/* End getStartFreq */


/*******************************************************************************
 Functionname:  getStopFreq
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
static Word16
getStopFreq(const Word32 fs, const Word16 stop_freq)
{
  Word32 i;
  Word16 result;
  Word16 *v_stop_freq = NULL;
  Word16 k1_min;
  Word16 v_dstop[13];


  Word16 v_stop_freq_32[14] = {32,34,36,38,40,42,44,46,49,52,55,58,61,64};
  Word16 v_stop_freq_44[14] = {23,25,27,29,32,34,37,40,43,47,51,55,59,64};
  Word16 v_stop_freq_48[14] = {21,23,25,27,30,32,35,38,42,45,49,54,59,64};

  test();
  switch(fs){
  case 32000: k1_min = 32;                 move16();
              v_stop_freq =v_stop_freq_32; move32(); 
    break;
  case 44100: k1_min = 23;                 move16();
              v_stop_freq =v_stop_freq_44; move32(); 

    break;
  case 48000: k1_min = 21;                 move16();
              v_stop_freq =v_stop_freq_48; move32(); 

    break;
  default:
    k1_min = 21; /* illegal fs  */         move16();
  }

  
  
  
  
  /* Ensure increasing bandwidth */
  for(i = 0; i <= 12; i++) {
    v_dstop[i] = sub(v_stop_freq[i+1], v_stop_freq[i]);                                 move16();
  }

  Shellsort_short(v_dstop, 13); /* Sort bandwidth changes */
  
  result = k1_min;                                                                      move16();
  for(i = 0; i < stop_freq; i++) {
    result = add(result, v_dstop[i]);
  }
  
  return(result);
  
}/* End getStopFreq */


/*******************************************************************************
 Functionname:  FindStartAndStopBand
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
Word16      
FindStartAndStopBand(const Word32 samplingFreq,
                     const Word16 noChannels,
                     const Word16 startFreq,
                     const Word16 stopFreq,
                     const SR_MODE sampleRateMode,
                     Word16 *k0,
                     Word16 *k2)
{
  Word16 k0k2diff;
  /* Update startFreq struct */
  *k0 = getStartFreq(samplingFreq, startFreq);                                                       move16(); 

  /* Test if start freq is outside corecoder range */
  test(); test(); sub(1, 1); L_sub(1, 1);
  if( ( sampleRateMode == 1 ) &&
      ( samplingFreq*noChannels  <
        L_shl(ffr_Integer_Mult(*k0, samplingFreq), 1) ) ) {
    return (1); /* raise the cross-over frequency and/or lower the number
                   of target bands per octave (or lower the sampling frequency) */
  }
  
  /*Update stopFreq struct */
  test(); test(); sub(1, 1);
  if ( stopFreq < 14 ) { 
    *k2 = getStopFreq(samplingFreq, stopFreq);                                                       move16();
  } else if( stopFreq == 14 ) {
    *k2 = add(*k0, *k0);                                                                             move16();
  } else {
    *k2 = add(add(*k0, *k0), *k0);                                                                   move16();
  }
  
  /* limit to Nyqvist */
  test(); sub(1, 1);
  if (*k2 > noChannels) {
    *k2 = noChannels;                                                                                move16();
  }

  /* Test for invalid  k0 k2 combinations */
  k0k2diff = sub(*k2, *k0);
  test(); sub(1, 1);
  if (k0k2diff > MAX_FREQ_COEFFS)
    return (1);/*Number of bands exceeds valid range of MAX_FREQ_COEFFS */

  test();
  if (k0k2diff < 0)
    return (1);/* Number of bands is negative */

  return(0);
}

/*******************************************************************************
 Functionname:  UpdateFreqScale
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
Word32      
UpdateFreqScale(UWord16  *v_k_master, Word16 *h_num_bands,
                const Word16 k0, const Word16 k2,
                const Word16 freqScale,
                const Word16 alterScale)
     
{
  
  Word16     b_p_o = 0;        /* bands_per_octave */
  Word16     warpFlag;
  Word16     dk = 0;

  /* Internal variables */
  Word16     two_regions = 0;  /* True or False */
  Word16     k1 = 0, i;
  Word16     num_bands0;
  Word16     num_bands1;
  Word16     diff_tot[MAX_OCTAVE + MAX_SECOND_REGION];
  Word16     *diff0 = diff_tot;
  Word16     *diff1;
  Word16     k2_achived;
  Word16     k2_diff;
  Word16     incr = 0;
  
  diff1 =(Word16*) L_add((Word32)diff_tot, MAX_OCTAVE*sizeof(*diff1));
  
  /* Init */
  test(); sub(1, 1);
  if(freqScale==1) {
    b_p_o=12;           move16();
  }
  else {
    test(); sub(1, 1);
    if(freqScale==2) {
      b_p_o=10;           move16();
    }
    else {
      test(); sub(1, 1);
      if(freqScale==3) {
        b_p_o=8;            move16();
      }
    }
  }
  
  
  test();
  if(freqScale > 0) /*Bark*/
    {
      test();
      if(alterScale==0) {
        warpFlag=0;        move16();
      }
      else {
        warpFlag=1;        move16();
      }
      
      test(); sub(1, 1);
      if(shl(k2, 2) >= ffr_Short_Mult(9, k0))  /*two or more regions*/
        {
          two_regions=1;   move16();
          k1=shl(k0, 1);
          
          num_bands0=numberOfBands(b_p_o, k0, k1, 0);
          num_bands1=numberOfBands(b_p_o, k1, k2, warpFlag);
          
          CalcBands(diff0, k0, k1, num_bands0);/*CalcBands1 => diff0 */
          Shellsort_short( diff0, num_bands0);/*SortBands sort diff0 */

          test();
          if (diff0[0] == 0) /* too wide FB bands for target tuning */
          {
            return (1);
          }

          cumSum(k0, diff0, num_bands0, v_k_master); /* cumsum */
          
          CalcBands(diff1, k1, k2, num_bands1);     /* CalcBands2 => diff1 */
          Shellsort_short( diff1, num_bands1);            /* SortBands sort diff1 */
          test(); sub(1, 1);
          if(diff0[num_bands0-1] > diff1[0])        /* max(1) > min(2) */
            {
              test();
              if(modifyBands(diff0[num_bands0-1],diff1, num_bands1))
                return(1);
              
            }
          
          /* Add 2'nd region */
          cumSum(k1, diff1, num_bands1, &v_k_master[num_bands0]);
          *h_num_bands=add(num_bands0, num_bands1);     /* Output nr of bands */                   move16();
          
        }
      else /* one region */
        {
          two_regions=0;                                                                           move16();
          k1=k2;                                                                                   move16();
          
          num_bands0=numberOfBands(b_p_o, k0, k1, 0);
          CalcBands(diff0, k0, k1, num_bands0);/* CalcBands1 => diff0 */
          Shellsort_short( diff0, num_bands0);       /* SortBands sort diff0 */

          test();
          if (diff0[0] == 0) /* too wide FB bands for target tuning */
          {
            return (1);
          }

          cumSum(k0, diff0, num_bands0, v_k_master);/* cumsum */
          *h_num_bands=num_bands0;        /* Output nr of bands */                                   move16();
          
        }
    }
  else /* Linear mode */
    {
      test();
      if (alterScale==0) {
        dk = 1;                                                                                      move16();
        num_bands0 = shl(shr(sub(k2, k0), 1), 1);         /* FLOOR to get to few number of bands*/
      } else {
        dk = 2;                                                                                      move16();
        num_bands0 = shl(shr(add(shr(sub(k2, k0), 1), 1), 1), 1); /* ROUND to get closest fit */
      }
  
      k2_achived = add(k0, shl(num_bands0, 1));
      k2_diff = sub(k2, k2_achived);

      for(i=0;i<num_bands0;i++)
        diff_tot[i] = dk;                                                                            move16();

      /* If linear scale wasn't achived */
      /* and we got wide SBR are */
      test();
      if (k2_diff < 0) {
          incr = 1;                                                                                  move16();
          i = 0;                                                                                     move16();
      }
      
      /* If linear scale wasn't achived */
      /* and we got small SBR are */
      test();
      if (k2_diff > 0) {
          incr = -1;                                                                                 move16();
          i = sub(num_bands0, 1);           
      }

      /* Adjust diff vector to get sepc. SBR range */
      test();
      while (k2_diff != 0) {
        diff_tot[i] = sub(diff_tot[i], incr);
        i = add(i, incr);
        k2_diff = add(k2_diff, incr);
        test();
      }

      cumSum(k0, diff_tot, num_bands0, v_k_master);/* cumsum */
      *h_num_bands=num_bands0;        /* Output nr of bands */                                       move16();

    }

  test(); sub(1, 1);
  if (*h_num_bands < 1)
    return(1); /*To small sbr area */
  
  return (0);
}/* End UpdateFreqScale */


static Word16 
numberOfBands(Word16 b_p_o, Word16 start, Word16 stop, Word16 warpFlag)
{
  Word16 numBands;
  Word32 num_bands_div128;
  Word16 bpo_div16;

  bpo_div16 = shl(b_p_o, (SHORT_BITS-1-4));

  num_bands_div128 = L_mult(ffr_getNumOctavesDiv8(start,stop), bpo_div16);

  test();
  if (warpFlag) {
    num_bands_div128 = fixmul(num_bands_div128, 0x62700000);
  }
  
  /* add scaled 1 for etsioprounding to even numbers: */
  num_bands_div128 = L_add(num_bands_div128, 0x01000000);
  /* scale back to right aligned integer and double the value: */
  numBands = shl(extract_l(L_shr(num_bands_div128, (INT_BITS - 7))), 1);

  return(numBands);
}


/*!
  \brief     Calculate frequency ratio of one SBR band

 \return    num_band-th root of k_start/k_stop
*/
static Word32 calcFactorPerBand(Word32 k_start, Word32 k_stop, Word32 num_bands)
{
  Word32 bandfactor = 0x40000000; /* Start value */
  Word32 step = 0x20000000;      /* Initial increment for factor */
  Word32 direction = 1;
  Word32 start = L_shl(k_start, (INT_BITS-8));
  Word32 stop = L_shl(k_stop, (INT_BITS-8));
  Word32 temp;
  Word32   j, i=0;

  test();
  while ( step > 0 ) {
    test();
    i++;
    temp = stop;

    /* Calculate temp^num_bands: */
    for (j=0; j<num_bands; j++)
      temp = fixmul(temp, bandfactor);

    test(); sub(1, 1);
    if (temp < start) { /* Factor too strong, make it weaker */
      test();
      if (direction == 0)
        /* Halfen step. Right shift is not done as Word32 because otherwise the
           lowest bit cannot be cleared due to etsioprounding */
        step = L_shr(step, 1);
      direction = 1;
      bandfactor = L_add(bandfactor, step);
    }
    else {  /* Factor is too weak: make it stronger */
      test(); sub(1, 1);
      if (direction == 1)
        step = L_shr(step, 1);
      direction = 0;
      bandfactor = L_sub(bandfactor, step);
    }

    test(); sub(1, 1);
    if (i>100) {
      step = 0;
    }
  }
  return(bandfactor);
}



/*!
  \brief     Calculate width of SBR bands

*/
static void
CalcBands(Word16 * diff,    /*!< Vector of widths to be calculated */
          Word16 start,     /*!< Lower end of subband range */
          Word16 stop,      /*!< Upper end of subband range */
          Word16 num_bands) /*!< Desired number of bands */
{
  Word32 i;
  Word32 previous;
  Word32 current;
  Word32 exact, temp;
  Word32 bandfactor = calcFactorPerBand(L_deposit_l(start), L_deposit_l(stop), L_deposit_l(num_bands));

  previous = stop; /* Start with highest QMF channel */
  exact = L_shl(stop, (INT_BITS-8)); /* Shift left to gain some accuracy */

  for(i=num_bands-1; i>=0; i--) {
    /* Calculate border of next lower sbr band */
    exact = fixmul(exact, bandfactor);

    /* Add scaled 0.5 for rounding:
       We use a value 127/256 instead of 0.5 to avoid some critical cases of rounding. */
    temp = L_add(exact,0x007f0000);

    /* scale back to right alinged integer: */
    current = L_shr(temp, (INT_BITS-8));

    /* Save width of band i */
    diff[i] = extract_l(L_sub(previous, current));                                                           move32();
    previous = current;
  }
}


static void 
cumSum(Word16 start_value, Word16* diff, Word16 length,  UWord16 *start_adress)
{
  Word32 i;
  start_adress[0]=start_value;                                                                       move16();
  for(i=1;i<=length;i++) {
    start_adress[i]=add(start_adress[i-1], diff[i-1]);                                               move16();
  }
} /* End cumSum */


static Word16
modifyBands(Word16 max_band_previous, Word16 * diff, Word16 length)
{
  Word16 change=sub(max_band_previous, diff[0]);

  /* Limit the change so that the last band cannot get narrower than the first one */
  test(); sub(1, 1);
  if ( change > shr(sub(diff[length-1], diff[0]), 1) )
    change = shr(sub(diff[length-1], diff[0]), 1);

  diff[0] = add(diff[0], change);                                                                  move16();
  diff[length-1] = sub(diff[length-1], change);                                                    move16();
  Shellsort_short(diff, length);

  return(0);
}/* End modifyBands */


/*******************************************************************************
 Functionname:  UpdateHiRes
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
Word16
UpdateHiRes(UWord16 *h_hires, Word16 *num_hires,UWord16 * v_k_master, 
            Word16 num_master , Word16 *xover_band, SR_MODE drOrSr,
            Word16 noQMFChannels)
{
  Word16 i;
  Word16 divider;
  Word16 max1,max2;
  
  /* Check if we use a Dual rate => diver=2 else 1 */
  test();
  divider = (drOrSr == DUAL_RATE) ? 2 : 1;

  test(); test(); sub(1, 1); sub(1, 1);
  if( (v_k_master[*xover_band] > ((drOrSr == DUAL_RATE) ? shr(noQMFChannels, 1) : noQMFChannels) ) ||
      ( *xover_band > num_master ) )  {
      /* xover_band error, too big for this startFreq. Will be clipped */
    
    /* Calculate maximum value for xover_band */
    max1=0;
    max2=num_master;
    test(); sub(1, 1);
    if ( divider == 2 ) 
      shr(noQMFChannels, 1);
    test(); test();
    while( (v_k_master[max1+1] < noQMFChannels) &&
           ( (max1+1) < max2) )
      {
        max1++;
        test(); test(); sub(1, 1); sub(1, 1);
      }
    
    *xover_band=max1;                                                                                move16();
  }

  *num_hires = sub(num_master, *xover_band);                                                         move16();
  for(i = *xover_band; i <= num_master; i++)
    {
      h_hires[i - *xover_band] = v_k_master[i];                                                      move16();
    }
  
  return (0);
}/* End UpdateHiRes */


/*******************************************************************************
 Functionname:  UpdateLoRes
 *******************************************************************************
 Description: 
  
 Arguments:   
 
 Return:      
 *******************************************************************************/
void                    
UpdateLoRes(UWord16 * h_lores, Word16 *num_lores, UWord16 * h_hires, Word16 num_hires)
{
  Word32 i;

  test(); logic16();
  if(num_hires%2 == 0) /* if even number of hires bands */
    {
      *num_lores=shr(num_hires, 1);                                                                  move16();
      /* Use every second lores=hires[0,2,4...] */
      for(i=0;i<=*num_lores;i++)
        h_lores[i]=h_hires[i*2];                                                                     move16();
      
    }
  else            /* odd number of hires which means xover is odd */
    {
      *num_lores=shr(add(num_hires, 1), 1);                                                          move16();
      
      /* Use lores=hires[0,1,3,5 ...] */
      h_lores[0]=h_hires[0];                                                                         move16();
      for(i=1;i<=*num_lores;i++)
        {
          h_lores[i]=h_hires[i*2-1];                                                                 move16();
        }
    }
  
}/* End UpdateLoRes */
