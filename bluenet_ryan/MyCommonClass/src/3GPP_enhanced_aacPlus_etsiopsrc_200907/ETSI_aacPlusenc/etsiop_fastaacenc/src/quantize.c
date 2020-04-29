/*
   Quantization
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "quantize.h"
#include "ffr.h"
#include "aac_rom.h"
#include "count.h"

#define MANT_DIGITS 9
#define MANT_SIZE   (1<<MANT_DIGITS)

static const Word32 k = 0x33e425af; /* final rounding constant */

/*! calculate $x^{\frac{3}{4}}, for 0.5 < x < 1.0$.  */
static Word32 pow34(Word32 x)
{
  /* index table using MANT_DIGITS bits, but mask out the sign bit and the MSB
     which is always one */
  logic32();
  return mTab_3_4[L_shr(x, (INT_BITS-2-MANT_DIGITS)) & (MANT_SIZE-1)];
}


static Word16 quantizeSingleLine(const Word16 gain, const Word32 absSpectrum)
{
  Word16 e, minusFinalExp, finalShift;
  Word32 x;
  Word16 qua = 0;                       move16();

  test();
  if (absSpectrum) {
    e = ffr_norm32(absSpectrum);
    x = pow34(L_shl(absSpectrum, e));

    /* calculate the final fractional exponent times 16 (was 3*(4*e + gain) + (INT_BITS-1)*16) */
    minusFinalExp = add(shl(e,2), gain);
    minusFinalExp = add(shl(minusFinalExp,1), minusFinalExp);
    minusFinalExp = add(minusFinalExp, shl(INT_BITS-1,4));

    /* separate the exponent into a shift, and a multiply */
    finalShift = shr(minusFinalExp, 4) ;

    test();
    if (sub(finalShift, INT_BITS) < 0) {
      x = L_shr(fixmul_32x16b(x, pow2tominusNover16[minusFinalExp & 15]), 1);

      x = L_add(x, L_shr(k, sub(INT_BITS,finalShift)));

      /* shift and quantize */
      qua = extract_l(L_shr(x, sub(finalShift,1)));
    }
  }

  return qua;
}

/*****************************************************************************

    functionname:quantizeLines 
    description: quantizes spectrum lines  
                 quaSpectrum = mdctSpectrum^3/4*2^(-(3/16)*gain)    
    input: global gain, number of lines to process, spectral data         
    output: quantized spectrum

*****************************************************************************/
static void quantizeLines(const Word16 gain,
                          const Word16 noOfLines,
                          const Word32 *mdctSpectrum,
                          Word16 *quaSpectrum)
{
  Word16 line;
  Word16 m = gain&3;
  Word16 g = add(shr(gain,2), 4);
  logic16(); /* gain&3 */

  for (line=0; line<noOfLines; line++) {
    Word16 qua;
    qua = 0;                                                    move16();
    test();
    if (mdctSpectrum[line]) {
      Word32 sa;
      Word16 saShft;

      sa = L_abs(mdctSpectrum[line]);
      saShft = etsiopround(L_shr(sa, g));

      test();
      if (sub(saShft, quantBorders[0][m]) > 0) {
        test();
        if (sub(saShft, quantBorders[1][m]) < 0) {
          test(); move16();
          qua = mdctSpectrum[line]>0 ? 1 : -1;
        }
        else {
          test();
          if (sub(saShft, quantBorders[2][m]) < 0) {
            test(); move16();
            qua = mdctSpectrum[line]>0 ? 2 : -2;
          }
          else {
            test();
            if (sub(saShft, quantBorders[3][m]) < 0) {
              test(); move16();
              qua = mdctSpectrum[line]>0 ? 3 : -3;
            }
            else {
              qua = quantizeSingleLine(gain, sa);
              /* adjust the sign. Since 0 < qua < 1, this cannot overflow. */
              test();
              if (mdctSpectrum[line] < 0)
                qua = negate(qua);
            }
          }
        }
      }
    }

    quaSpectrum[line] = qua ;                                   move16();
  }

}


/*****************************************************************************

    functionname:iquantizeLines 
    description: iquantizes spectrum lines without sign
                 mdctSpectrum = iquaSpectrum^4/3 *2^(0.25*gain) 
    input: global gain, number of lines to process,quantized spectrum        
    output: spectral data

*****************************************************************************/
static void iquantizeLines(const Word16 gain,
                           const Word16 noOfLines,
                           const Word16 *quantSpectrum,
                           Word32 *mdctSpectrum)
{
  Word16   iquantizermod;
  Word16   iquantizershift;
  Word16   line;

  iquantizermod = gain & 3;                             logic16();
  iquantizershift = shr(gain,2);

  for (line=0; line<noOfLines; line++) {
    test();
    if( quantSpectrum[line] != 0 ) {
      Word32 accu;
      Word16 ex;
	  Word32 tabIndex;
      Word16 specExp;
      Word32 s,t;

      accu = quantSpectrum[line];

      ex = ffr_norm32(accu);
      accu = L_shl(accu, ex);
      specExp = sub(INT_BITS-1, ex);

      tabIndex = L_shr(accu, (INT_BITS-2-MANT_DIGITS)) & (~MANT_SIZE);       logic32();

      /* calculate "mantissa" ^4/3 */
      s = mTab_4_3[tabIndex];                                                   move32();

      /* get approperiate exponent multiplier for specExp^3/4 combined with scfMod */
      t = specExpMantTableComb_enc[iquantizermod][specExp];                     move32();

      /* multiply "mantissa" ^4/3 with exponent multiplier */
      accu = fixmul(s, t);

      /* get approperiate exponent shifter */
      specExp = specExpTableComb_enc[iquantizermod][specExp];                   move16();

      mdctSpectrum[line] = L_shl(accu, add(iquantizershift, specExp));
    }
    else {
      mdctSpectrum[line] = 0;                                                   move32();
    }
  }
}

/*****************************************************************************

    functionname: QuantizeSpectrum
    description: quantizes the entire spectrum
    returns:
    input: number of scalefactor bands to be quantized, ...
    output: quantized spectrum

*****************************************************************************/
void QuantizeSpectrum(Word16 sfbCnt,
                      Word16 maxSfbPerGroup,
                      Word16 sfbPerGroup,
                      Word16 *sfbOffset,
                      Word32 *mdctSpectrum,
                      Word16 globalGain,
                      Word16 *scalefactors,
                      Word16 *quantizedSpectrum)
{
  Word16 sfbOffs, sfb;

  for(sfbOffs=0;sfbOffs<sfbCnt;sfbOffs+=sfbPerGroup) {
    Word16 sfbNext ;
    for (sfb = 0; sfb < maxSfbPerGroup; sfb = sfbNext) {
      Word16 scalefactor = scalefactors[sfbOffs+sfb];                         move16();
      /* coalesce sfbs with the same scalefactor */
      for (sfbNext = sfb+1;
           sfbNext < maxSfbPerGroup && scalefactor == scalefactors[sfbOffs+sfbNext];
           sfbNext++) ;

      quantizeLines(sub(globalGain, scalefactor),
                    sub(sfbOffset[sfbOffs+sfbNext], sfbOffset[sfbOffs+sfb]),
                    mdctSpectrum + sfbOffset[sfbOffs+sfb],
                    quantizedSpectrum + sfbOffset[sfbOffs+sfb]);
    }
  }
}


/*****************************************************************************

    functionname:calcSfbDist 
    description: quantizes and requantizes lines to calculate distortion
    input:  number of lines to be quantized, ...
    output: distortion

*****************************************************************************/
Word32 calcSfbDist(const Word32 *spec,
                   Word16  sfbWidth,
                   Word16  gain)
{
  Word16 line;
  Word32 dist;
  Word16 m = gain&3;
  Word16 g = add(shr(gain,2), 4);
  logic16(); /* gain&3 */


  dist = 0;                                     move32();
  for(line=0; line<sfbWidth; line++) {
    test();
    if (spec[line]) {

      Word16 diff;
      Word32 distSingle;
      Word32 sa;
      Word16 saShft;
      sa = L_abs(spec[line]);
      saShft = etsiopround(L_shr(sa, g));

      test();
      if (sub(saShft, quantBorders[0][m]) < 0) {
        distSingle = L_shl(L_mult(saShft, saShft), shl(g,1));
      }
      else {
        test();
        if (sub(saShft, quantBorders[1][m]) < 0) {
          diff = sub(saShft, quantRecon[0][m]);
          distSingle = L_shl(L_mult(diff, diff), shl(g,1));
        }
        else {
          test();
          if (sub(saShft, quantBorders[2][m]) < 0) {
            diff = sub(saShft, quantRecon[1][m]);
            distSingle = L_shl(L_mult(diff, diff), shl(g,1));
          }
          else {
            test();
            if (sub(saShft, quantBorders[3][m]) < 0) {
              diff = sub(saShft, quantRecon[2][m]);
              distSingle = L_shl(L_mult(diff, diff), shl(g,1));
            }
            else {
              Word16 qua = quantizeSingleLine(gain, sa);
              Word32 iqval, diff32;
              /* now that we have quantized x, re-quantize it. */
              iquantizeLines(gain, 1, &qua, &iqval);
              diff32 = L_sub(sa, iqval);
              distSingle = fixmul(diff32, diff32);
            }
          }
        }
      }
      dist = L_add(dist, distSingle);
    }
  }

  return dist;
}
