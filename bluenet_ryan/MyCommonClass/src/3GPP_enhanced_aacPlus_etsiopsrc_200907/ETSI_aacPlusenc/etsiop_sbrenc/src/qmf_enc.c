/*
   QMF analysis
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "qmf_enc.h"
#include "sbr_rom.h"
#include "sbr_ram.h"
#include "sbr_def.h"
#include "aac_ram.h"
#include "aac_rom.h"
#include "count.h"



/*!
 *
 *  \brief Calculates max possible scale factor for input vector
 *
 *  \return Maximum scale factor
 *
 */
static Word16 getScalefactor(
                             const Word16 *vector, /*!< pointer to input vector */ 
                             Word16 len,           /*!< length of input vector */
                             Word16 stride         /*!< stride of input vector */
                             )
{
  Word16 maxSfc;
  Word16 i;
  Word16 tmp;
  Word16 max = 0;                             move16();

  for(i=0;i<len;i++){
    tmp = abs_s(vector[i*stride]);
    max |= tmp; logic16();
  }

  test(); move16();
  maxSfc = max ? norm_s(max) : 15;

  return(maxSfc);
}


/*!
 *
 *  \brief Calculates max possible scale factor for input vector
 *
 *  \return Maximum scale factor
 *
 */
static Word16 getScalefactor_Blockwise(
                                       const Word16 *vector, /*!< pointer to input vector */ 
                                       Word16 stride,        /*!< stride of input vector */
                                       Word16 Blocklen       /*!< Block length  */
                                       )
{
  Word16 maxSfc;
  Word16 i,j;
  Word16 tmp;
  Word16 max = 0;                                    move16();

  for(j=0;j<64;j++) {
    for(i=0;i<(Blocklen-1);i++) {
      tmp = abs_s(vector[Blocklen*j+i*stride]);
      max |= tmp; logic16();
    }    
  }

  test(); move16();
  maxSfc = max ? norm_s(max) : 15;

  return(maxSfc);
}


/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *
 *  \return 
 *
 */
static void scaleValues(Word16 *vector,    /*!< Vector */  
                        Word16 len,        /*!< Length */
                        Word16 scalefactor /*!< Scalefactor */ 
                        )
{
  Word16 i;

  for ( i = 0; i < len; i++ ) {
    vector[i] = shl(vector[i], scalefactor); move16();
  }
}



/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *  \Scales input vector in Block of size 10 and skips every 9 element of each Block 
 *  \return 
 *
 */
static void scaleValues_Blockwise(Word16 *vector,    /*!< Vector */  
                                  Word16 scalefactor /*!< Scalefactor */ 
                        )
{
  Word32 i,j;
  
  for (j=0;j<64;j++) {
    for ( i = 0; i < 9; i++ ) {
      vector[i+10*j] = shl(vector[i+10*j], scalefactor); move16();
    }
  }
}


/*!
 
  \brief  Performs cosine modulation of the time domain data of timeIn
          and stores the result in subband

  
  \return none
 
*/


static void
cosMod16 (
          Word16 *subband,                   /*!< Pointer to subband */ 
          HANDLE_SBR_QMF_FILTER_BANK qmfBank /*!< Handle of qmfBank  */
          )
{
  Word16 i;
  Word16 wim, wre, re1, im1, re2, im2;
  Word32 accu1,accu2;

  for (i = 0; i < 16; i++) {
    re1 = subband[2 * i];
    im2 = subband[2 * i + 1];
    re2 = subband[2 * 32 - 2 - 2 * i];
    im1 = subband[2 * 32 - 1 - 2 * i];

    wim = qmfBank->sin_twiddle[i];
    wre = qmfBank->cos_twiddle[i];

    accu1 = L_mac( L_mult(im1, wim), re1, wre );
    accu2 = L_msu( L_mult(im1, wre), re1, wim );
    
    subband[2 * i] =     shr(etsiopround(accu1), 1);                   move16();
    subband[2 * i + 1] = shr(etsiopround(accu2), 1);                   move16();

    wim = qmfBank->sin_twiddle[32 - 1 - i];
    wre = qmfBank->cos_twiddle[32 - 1 - i];

    accu1 = L_mac( L_mult(im2, wim), re2, wre );
    accu2 = L_msu( L_mult(im2, wre), re2, wim );

    subband[2 * 32 - 2 - 2 * i] = shr(etsiopround(accu1), 1);          move16();
    subband[2 * 32 - 1 - 2 * i] = shr(etsiopround(accu2), 1);          move16();
  }

  cfft16(subband, 32, -1);
  
  wim = qmfBank->alt_sin_twiddle[0];
  wre = qmfBank->alt_sin_twiddle[32];

  for (i = 0; i < 16; i++) {

    re1 = subband[2 * i];
    im1 = subband[2 * i + 1];
    re2 = subband[2 * 32 - 2 - 2 * i];
    im2 = subband[2 * 32 - 1 - 2 * i];

    accu1 = L_mac( L_mult(re1, wre), im1, wim );
    accu2 = L_msu( L_mult(re1, wim), im1, wre );

    subband[2 * i] =              etsiopround(accu1);                  move16();
    subband[2 * 32 - 1 - 2 * i] = etsiopround(accu2);                  move16();

    wim = qmfBank->alt_sin_twiddle[i + 1];
    wre = qmfBank->alt_sin_twiddle[32 - 1 - i];

    accu1 = L_mac( L_mult(re2, wim), im2, wre );
    accu2 = L_msu( L_mult(re2, wre), im2, wim );

    subband[2 * 32 - 2 - 2 * i] = etsiopround(accu1);                  move16();
    subband[2 * i + 1] =          etsiopround(accu2);                  move16();
  }
}


/*!
 
  \brief  Performs sine modulation of the time domain data of timeIn
          and stores the result in subband
 
  \return none
 
*/
static void
sinMod16 (
          Word16 *subband,                   /*!< Pointer to subband */ 
          HANDLE_SBR_QMF_FILTER_BANK qmfBank /*!< Handle of qmfBank  */
          )
{
  Word32 i;
  Word16 wre, wim,re1, im1, re2, im2;
  Word32 accu1,accu2;

  for (i = 0; i < 16; i++) {
    re1 = subband[2 * i];
    im2 = subband[2 * i + 1];
    re2 = subband[2 * 32 - 2 - 2 * i];
    im1 = subband[2 * 32 - 1 - 2 * i];

    wre = qmfBank->sin_twiddle[i];
    wim = qmfBank->cos_twiddle[i];

    accu1 = L_mac( L_mult(im1, wim), re1, wre );
    accu2 = L_msu( L_mult(im1, wre), re1, wim );

    subband[2 * i + 1] = shr(etsiopround(accu1), 1);                   move16();
    subband[2 * i] =     shr(etsiopround(accu2), 1);                   move16();

    wre = qmfBank->sin_twiddle[32 - 1 - i];
    wim = qmfBank->cos_twiddle[32 - 1 - i];

    accu1 = L_mac( L_mult(im2, wim), re2, wre );
    accu2 = L_msu( L_mult(im2, wre), re2, wim );

    subband[2 * 32 - 1 - 2 * i] = shr(etsiopround(accu1), 1);          move16();
    subband[2 * 32 - 2 - 2 * i] = shr(etsiopround(accu2), 1);          move16();
  }

  cfft16(subband, 32,-1);                              
  
  wim = qmfBank->alt_sin_twiddle[0];
  wre = qmfBank->alt_sin_twiddle[32];

  for (i = 0; i < 16; i++) {

    re1 = subband[2 * i];
    im1 = subband[2 * i + 1];
    re2 = subband[2 * 32 - 2 - 2 * i];
    im2 = subband[2 * 32 - 1 - 2 * i];

    accu1 = L_mac( L_mult(re1, wre), im1, wim );
    accu2 = L_msu( L_mult(re1, wim), im1, wre );

    subband[2 * 32 - 1 - 2 * i] = negate(etsiopround(accu1));          move16();
    subband[2 * i] =              negate(etsiopround(accu2));          move16();

    wim = qmfBank->alt_sin_twiddle[i + 1];
    wre = qmfBank->alt_sin_twiddle[32 - 1 - i];

    accu1 = L_mac( L_mult(re2, wim), im2, wre );
    accu2 = L_msu( L_mult(re2, wre), im2, wim );

    subband[2 * i + 1] =          negate(etsiopround(accu1));          move16();
    subband[2 * 32 - 2 - 2 * i] = negate(etsiopround(accu2));          move16();
  }

}


/*!
 
  \brief  Performs complex-valued forward modulation of the time domain
          data of timeIn and stores the real part of the subband
          samples in rSubband, and the imaginary part in iSubband
 
  \return none
 
*/
static void
forwardModulation (
                   const Word16 *timeIn,              /*!< Time Signal */
                   Word16 *rSubband,                  /*!< Real Output */
                   Word16 *iSubband,                  /*!< Imaginary Output */
                   HANDLE_SBR_QMF_FILTER_BANK qmfBank /*!< Handle of qmfBank  */
                   )
{
  Word16 k;
  Word16 tmp1, tmp2;

  for (k = 0; k < 64; k++) {
    tmp1 = shr(timeIn[k], 1);
    tmp2 = shr(timeIn[128 - 1 - k], 1); 
    rSubband[k] = sub(tmp1, tmp2);                    move16();
    iSubband[k] = add(tmp1, tmp2);                    move16();
  }

  cosMod16 (rSubband, qmfBank);
  sinMod16 (iSubband, qmfBank);

}


/*!
 *
 *  \brief        FIR filtering with 5 taps & modulate 
 *
 *  \return 
 *
 */
static void AnalysisPolyphaseFiltering16(Word16 *analysisBuffer,
                                         Word16 *filterStates,
                                         const Word16 *timeIn,
                                         const Word16 *pFilter)
{
	
  const Word16 *pFilterR = &pFilter[QMF_CHANNELS*NO_POLY];
  Word16 k;
  pFilter += NO_POLY;        L_add(1, 1);

  for (k = 0; k < QMF_CHANNELS; k++)
  {
    Word32 accu1;                           
    Word32 accu2;
    Word16 *filterS = &filterStates[k * 2 * NO_POLY];
    Word16 state0 = filterS[(2*0+0)];
    Word16 state1 = filterS[(2*0+1)];

    accu1 = L_mult(state0, (*pFilter++));
    accu2 = L_mult(state1, (*--pFilterR));
    filterS[(2*0+0)] = state1;                                       move16();
    
    state0 = filterS[(2*1+0)];
    state1 = filterS[(2*1+1)];
    accu1 = L_mac(accu1, state0, (*pFilter++));
    accu2 = L_mac(accu2, state1, (*--pFilterR));
    filterS[(2*1-1)] = state0;                                       move16();
    filterS[(2*1+0)] = state1;                                       move16();
    
    state0 = filterS[(2*2+0)];
    state1 = filterS[(2*2+1)];
    accu1 = L_mac(accu1, state0, (*pFilter++));
    accu2 = L_mac(accu2, state1, (*--pFilterR));
    filterS[(2*2-1)] = state0;                                       move16();
    filterS[(2*2+0)] = state1;                                       move16();
    
    state0 = filterS[(2*3+0)];
    state1 = filterS[(2*3+1)];
    accu1 = L_mac(accu1, state0, (*pFilter++));
    accu2 = L_mac(accu2, state1, (*--pFilterR));
    filterS[(2*3-1)] = state0;                                       move16();
    filterS[(2*3+0)] = state1;                                       move16();
    
    state0 = filterS[(2*4+0)];
    state1 = *timeIn++;
    accu1 = L_mac(accu1, state0, (*pFilter++));
    accu2 = L_mac(accu2, state1, (*--pFilterR));
    filterS[(2*4-1)] = state0;                                       move16();
    filterS[(2*4+0)] = state1;                                       move16();

    analysisBuffer[2*QMF_CHANNELS - 1 - k]   = etsiopround(accu1);   move16();
    analysisBuffer[QMF_CHANNELS -   1 - k]   = etsiopround(accu2);   move16();
  }

}


/*!
 
  \brief      Performs Dynamic spectrum scaling for all subband 
 
  \return     none
 
*/
static void AnalysisPostSpectrumScaling(Word16 *rSubband,                  /*!< Real Output */
                                        Word16 *iSubband,                  /*!< Imaginary Output */
                                        HANDLE_SBR_QMF_FILTER_BANK qmfBank /*!< Handle of qmfBank  */
                                        )
{
  Word16 i;  
  Word16 headRoom;  

  headRoom = sub(SHORT_BITS-1, qmfBank->qmfScale);

  headRoom = S_min(headRoom,getScalefactor(rSubband, QMF_CHANNELS*qmfBank->no_col,1));
  headRoom = S_min(headRoom,getScalefactor(iSubband, QMF_CHANNELS*qmfBank->no_col,1));


  for (i = 0; i < qmfBank->no_col * QMF_CHANNELS; i++) {
    rSubband[i] = shl(rSubband[i], headRoom);                                move16();
    iSubband[i] = shl(iSubband[i], headRoom);                                move16();
  }
  qmfBank->qmfScale = add(qmfBank->qmfScale, headRoom);                      move16();

}


#ifndef MONO_ONLY
/*!
 *
 * \brief Cosine modulation of the time domain data of a subband. Performed in-place
 * 
 */
static void cosModEnc (Word32 *subband, 
                       HANDLE_SBR_QMF_FILTER_BANK qmfBank                       )
{
  Word32 re1, im1, re2, im2 ;
  Word16 wim, wre ;
  Word32 xr, xi ;

  Word16 i;
  const Word16 *pCos = &qmfBank->cos_twiddle[0];
  const Word16 *pSin = &qmfBank->sin_twiddle[0];

  for (i = 0; i < 8; i += 2)
  {
    re1 = subband [2*i] ;
    im2 = subband [2*i + 1] ;
    re2 = subband [2*16 - 2*i - 2] ;
    im1 = subband [2*16 - 2*i - 1] ;

    wim = pSin [i] ;
    wre = pCos [i] ;

    xi = L_add(L_shr(fixmul_32x16b(im1,wim), 1), L_shr(fixmul_32x16b(re1,wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im1,wre), 1), L_shr(fixmul_32x16b(re1,wim), 1));

    wim = pSin [16 - i - 1] ;
    wre = pCos [16 - i - 1] ;

    subband [2*i] = xi ;                                                                   move32();
    subband [2*i+1] = xr ;                                                                 move32();

    xi = L_add(L_shr(fixmul_32x16b(im2,wim), 1), L_shr(fixmul_32x16b(re2,wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im2,wre), 1), L_shr(fixmul_32x16b(re2,wim), 1));

    re1 = subband [2*i + 2] ;
    im2 = subband [2*i + 3] ;
    re2 = subband [2*16 - 2*i - 4] ;
    im1 = subband [2*16 - 2*i - 3] ;

    wim = pSin [i+1] ;
    wre = pCos [i+1] ;

    subband [2*16 - 2*i - 2] = xi ;                                                         move32();
    subband [2*16 - 2*i - 1] = xr ;                                                         move32();

    xi = L_add(L_shr(fixmul_32x16b(im1,wim), 1), L_shr(fixmul_32x16b(re1,wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im1,wre), 1), L_shr(fixmul_32x16b(re1,wim), 1));

    wim = pSin [16 - i - 2] ;
    wre = pCos [16 - i - 2] ;

    subband [2*i+2] = xi ;                                                                  move32();
    subband [2*i+3] = xr ;                                                                  move32();

    xi = L_add(L_shr(fixmul_32x16b(im2,wim), 1), L_shr(fixmul_32x16b(re2,wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im2,wre), 1), L_shr(fixmul_32x16b(re2,wim), 1));

    subband [2*16 - 2*i - 4] = xi ;                                                         move32();
    subband [2*16 - 2*i - 3] = xr ;                                                         move32();
  }


  
  cfft32(subband, 16, -1, 1);

  re2 = subband [2*16 - 2];
  im2 = subband [2*16 - 1];

  subband [0] = L_shr(subband [0], 1);                                                      move32();
  subband [2*16 - 1] = L_negate(L_shr(subband [1], 1));                                     move32();

  wim = qmfBank->alt_sin_twiddle[1] ;
  wre = qmfBank->alt_sin_twiddle[16 - 1] ;

  xi = L_add(L_shr(fixmul_32x16b(re2, wim), 1), L_shr(fixmul_32x16b(im2, wre), 1));
  xr = L_sub(L_shr(fixmul_32x16b(re2, wre), 1), L_shr(fixmul_32x16b(im2, wim), 1));

  subband [2*16 - 2] = xi ;                                                                 move32();
  subband [1] = xr ;                                                                        move32();

  for (i = 1 ; i < 8; i++)
  {
    re1 = subband [2*i];
    im1 = subband [2*i + 1];
    re2 = subband [2*16 - 2*i - 2];
    im2 = subband [2*16 - 2*i - 1];

    xi = L_add(L_shr(fixmul_32x16b(re1,wre), 1), L_shr(fixmul_32x16b(im1,wim), 1));
    xr = L_sub(L_shr(fixmul_32x16b(re1,wim), 1), L_shr(fixmul_32x16b(im1,wre), 1));

    wim = qmfBank->alt_sin_twiddle [i + 1] ;
    wre = qmfBank->alt_sin_twiddle [16 - 1 - i] ;

    im1 = L_add(L_shr(fixmul_32x16b(re2,wim), 1), L_shr(fixmul_32x16b(im2,wre), 1));
    re1 = L_sub(L_shr(fixmul_32x16b(re2,wre), 1), L_shr(fixmul_32x16b(im2,wim), 1));

    subband [2*i] = xi ;                                                                    move32();
    subband [2*i + 1] = re1 ;                                                               move32();
    subband [2*16 - 2*i - 2] = im1 ;                                                        move32();
    subband [2*16 - 2*i - 1] = xr ;                                                         move32();
  }
}

/*!
 *
 * \brief Sine modulation of the time domain data of a subband. Performed in-place
 *
 */
static void sinModEnc (Word32 *subband, 
                       HANDLE_SBR_QMF_FILTER_BANK qmfBank                       )
{
  Word32 re1, im1, re2, im2 ;
  Word16 wre, wim ;
  Word32 xr, xi ;

  Word16 i;
  const Word16 *pCos = &qmfBank->cos_twiddle[0];
  const Word16 *pSin = &qmfBank->sin_twiddle[0];

  for (i = 0; i < 8; i += 2) 
  {
    re1 = subband [2*i] ;
    im2 = subband [2*i + 1] ;
    re2 = subband [2*16 - 2*i - 2] ;
    im1 = subband [2*16 - 2*i - 1] ;

    wre = pSin [i] ;
    wim = pCos [i] ;

    xi = L_add(L_shr(fixmul_32x16b(im1, wim), 1), L_shr(fixmul_32x16b(re1, wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im1, wre), 1), L_shr(fixmul_32x16b(re1, wim), 1));

    wre = pSin [16 - i - 1] ;
    wim = pCos [16 - i - 1] ;

    subband [2*i] = xr ;                                                                   move32();
    subband [2*i+1] = xi ;                                                                 move32();

    xi = L_add(L_shr(fixmul_32x16b(im2, wim), 1), L_shr(fixmul_32x16b(re2, wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im2, wre), 1), L_shr(fixmul_32x16b(re2, wim), 1));

    re1 = subband [2*i + 2] ;
    im2 = subband [2*i + 3] ;
    re2 = subband [2*16 - 2*i - 4] ;
    im1 = subband [2*16 - 2*i - 3] ;

    wre = pSin [i+1] ;
    wim = pCos [i+1] ;

    subband [2*16 - 2*i - 2] = xr ;                                                         move32();
    subband [2*16 - 2*i - 1] = xi ;                                                         move32();

    xi = L_add(L_shr(fixmul_32x16b(im1, wim), 1), L_shr(fixmul_32x16b(re1, wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im1, wre), 1), L_shr(fixmul_32x16b(re1, wim), 1));

    wre = pSin [16 - i - 2] ;
    wim = pCos [16 - i - 2] ;

    subband [2*i+2] = xr ;                                                                  move32();
    subband [2*i+3] = xi ;                                                                  move32();

    xi = L_add(L_shr(fixmul_32x16b(im2, wim), 1), L_shr(fixmul_32x16b(re2, wre), 1));
    xr = L_sub(L_shr(fixmul_32x16b(im2, wre), 1), L_shr(fixmul_32x16b(re2, wim), 1));

    subband [2*16 - 2*i - 4] = xr ;                                                         move32();
    subband [2*16 - 2*i - 3] = xi ;                                                         move32();
  }
  
  
  cfft32(subband, 16, -1, 1);
  
  
  re2 = subband [2*16 - 2];
  im2 = subband [2*16 - 1];

  subband [2*16 - 1] = L_negate(L_shr(subband[0], 1));                                      move32();
  subband [0] = L_shr(subband[1], 1) ;                                                      move32();

  wim = qmfBank->alt_sin_twiddle [1] ;
  wre = qmfBank->alt_sin_twiddle [16 - 1] ;

  xi = L_add(L_shr(fixmul_32x16b(re2, wim), 1), L_shr(fixmul_32x16b(im2, wre), 1));
  xr = L_sub(L_shr(fixmul_32x16b(re2, wre), 1), L_shr(fixmul_32x16b(im2, wim), 1));

  subband [1] = L_negate(xi);                                                               move32();
  subband [2*16 - 2] = L_negate(xr);                                                        move32();

  for (i = 1 ; i < 8; i++) 
  {
    re1 = subband [2*i] ;
    im1 = subband [2*i + 1] ;
    re2 = subband [2*16 - 2*i - 2] ;
    im2 = subband [2*16 - 2*i - 1] ;

    xi = L_add(L_shr(fixmul_32x16b(re1, wre), 1), L_shr(fixmul_32x16b(im1, wim), 1));
    xr = L_sub(L_shr(fixmul_32x16b(re1, wim), 1), L_shr(fixmul_32x16b(im1, wre), 1));

    wim = qmfBank->alt_sin_twiddle [i + 1] ;
    wre = qmfBank->alt_sin_twiddle [16 - 1 - i] ;

    im1 = L_add(L_shr(fixmul_32x16b(re2, wim), 1), L_shr(fixmul_32x16b(im2, wre), 1));
    re1 = L_sub(L_shr(fixmul_32x16b(re2, wre), 1), L_shr(fixmul_32x16b(im2, wim), 1));

    subband [2*i] = L_negate(xr);                                                           move32();
    subband [2*i+1] = L_negate(im1);                                                        move32();
    subband [2*16 - 2*i - 2] = L_negate(re1);                                               move32();
    subband [2*16 - 2*i - 1] = L_negate(xi);                                                move32();
  }
}


/*!
 *
 * \brief Perform complex-valued inverse modulation of the subband
 *        samples stored in rSubband (real part) and iSubband (imaginary
 *        part) and stores the result in timeOut
 *
 */
static void
inverseModulationEnc (Word32 *qmfReal,                  /*!< Pointer to qmf real subband slot */ 
                      Word32 *qmfImag,                  /*!< Pointer to qmf imag subband slot */
                      Word16 scaleFactorLowBand,        /*!< Scalefactor for Low band */
                      HANDLE_SBR_QMF_FILTER_BANK synQmf /*!< Handle of Qmf Synthesis Bank  */
                      )
{
  Word16 i;

  /* Low band area */
  for (i = 0; i < NO_SYNTHESIS_CHANNELS ; i++) {
    qmfReal[i] = L_negate(L_shl(qmfReal[i], scaleFactorLowBand));                          move32();
    qmfImag[i] = L_negate(L_shl(qmfImag[i], scaleFactorLowBand));                          move32();
  }

  cosModEnc (qmfReal, 
             synQmf);

  sinModEnc (qmfImag, 
             synQmf);
 
  for (i = 0 ; i < NO_SYNTHESIS_CHANNELS/2 ; i++) {
    Word32 r1 = qmfReal[i] ;
    Word32 i2 = qmfImag[NO_SYNTHESIS_CHANNELS - 1 - i] ;
    Word32 r2 = qmfReal[NO_SYNTHESIS_CHANNELS - i - 1] ;
    Word32 i1 = qmfImag[i] ;

    qmfReal[i] = L_shr(L_sub(r1, i1), 1);                                                  move32();
    qmfImag[NO_SYNTHESIS_CHANNELS - 1 - i] = L_shr(L_negate(L_add(r1, i1)), 1);            move32();
    qmfReal[NO_SYNTHESIS_CHANNELS - i - 1] = L_shr(L_sub(r2, i2), 1);                      move32();
    qmfImag[i] = L_shr(L_negate(L_add(r2, i2)), 1);                                        move32();
  }
}


/*!
 *
 *
 * \brief Perform complex-valued subband synthesis of the
 *        low band and the high band and store the
 *        time domain data in timeOut
 *
 *
 */
static void
SynthesisPolyphaseFilteringEnc(Word32 *pFilterStates,     /*!< Pointer to filter states */
                               const Word16 *pFilterBase, /*!< Pointer to filter coefficients */
                               Word32 *pReal,             /*!< Low and High band slot, real */
                               Word32 *pImag,             /*!< Low and High band slot, imag */
                               Word16 *pTimeOut,          /*!< Pointer to output */
                               Word16 outScalefactor)     /*!< scalefactor to output */
{
  const Word16 *pFilter = &pFilterBase[NO_POLY];
  const Word16 *pFilterR = &pFilter[QMF_FILTER_STATE_ANA_SIZE];

  {

    Word32 accu;
    Word32 tempr,tempi;
    Word16 temprs,tempis;
    Word32 i,j;
    Word16 *pFilterStat16i;
    Word16 *pFilterStat16r;
    Word16 statei,stater;
    
    Word16 *pFilterStatess = (Word16 *)pFilterStates;  
    
    pFilterR -= NO_POLY*SBR_DOWNSAMP_FAC;                                                  L_sub(1, 1);
    
    for (j = 0; j < NO_SYNTHESIS_CHANNELS; j++) {
        
      pFilterStat16i = &pFilterStatess[j*2*NO_POLY*SBR_DOWNSAMP_FAC];                         
      pFilterStat16r = &pFilterStatess[j*2*NO_POLY*SBR_DOWNSAMP_FAC+NO_POLY*SBR_DOWNSAMP_FAC];      
      
      tempi  = L_shl(pImag[NO_SYNTHESIS_CHANNELS -1 - j], outScalefactor);
      tempis = etsiopround(tempi);
      
      tempr  = L_shl(pReal[NO_SYNTHESIS_CHANNELS -1 - j], outScalefactor);
      temprs = etsiopround(tempr);
      
      
      /* put new input samples into state buffer*/
      pFilterStat16i[NO_POLY*SBR_DOWNSAMP_FAC-1] =  tempis;                                move16();
      pFilterStat16r[NO_POLY*SBR_DOWNSAMP_FAC-1] =  temprs;                                move16();
      
      
      /* Filtering */
      accu = 0;                                                                            move32();
      for(i=0;i<NO_POLY;i++) {
        statei = pFilterStat16i[i*SBR_DOWNSAMP_FAC];	
        accu   = L_mac(accu, statei, pFilter[i]);
        stater = pFilterStat16r[i*SBR_DOWNSAMP_FAC+SBR_DOWNSAMP_FAC-1];
        accu   = L_mac(accu, stater, pFilterR[NO_POLY-1-i]);
      }

      
      /* State buffer updation */
      for(i=0;i<(NO_POLY*SBR_DOWNSAMP_FAC-1);i++) {
        statei = pFilterStat16i[i+1];
        pFilterStat16i[i] = statei;                                                        move16();
        
        stater = pFilterStat16r[i+1];
        pFilterStat16r[i] = stater;                                                        move16();
      }
      
      pFilter  += SBR_DOWNSAMP_FAC*NO_POLY;                                                L_add(1, 1);     
      pFilterR -= SBR_DOWNSAMP_FAC*NO_POLY;                                                L_sub(1, 1);        
      
      pTimeOut[NO_SYNTHESIS_CHANNELS - 1 - j] = etsiopround(accu);                         move16();       
    }
    
  }


}


/*!
 *
 *
 * \brief Perform complex-valued subband synthesis
 *        and store the time domain data in timeOut
 *
 *
 */
void
SynthesisQmfFilteringEnc(Word32 **qmfReal,    /*!< Low and High band, real */
                         Word32 **qmfImag,    /*!< Low and High band, imag */
                         Word16 scale,        /*!< Scale of spectral input */
                         Word16 *timeOut,     /*!< Pointer to output */ 
                         HANDLE_SBR_QMF_FILTER_BANK synQmf /*!< Handle of Qmf Synthesis Bank  */
                         )
{
  Word16 i;
  Word32 *filterStates = synQmf->FilterStates;
  
  Word16 diffScalefactor;
  Word16 outScalefactor;

  /*
    scaling
  */
  synQmf->qmfStatesScale = -6;                                                             move16();

  diffScalefactor = negate(scale);

  outScalefactor = negate(sub(synQmf->qmfStatesScale, 3));
  
  for (i = 0; i < synQmf->no_col; i++){
    Word32 *imagSlot = *(qmfImag + i);

    inverseModulationEnc ( *(qmfReal + i), 
                           imagSlot, 
                           diffScalefactor, 
                           synQmf );

    SynthesisPolyphaseFilteringEnc( (Word32*)filterStates, 
                                    synQmf->p_filter,
                                    (Word32*)*(qmfReal+i),
                                    (Word32*)imagSlot,
                                    &timeOut[i*NO_SYNTHESIS_CHANNELS],
                                    sub(outScalefactor, 1)); 
  }

}


/*!
 *
 * \brief Create QMF filter bank instance
 *   
 * \return 0 if successful      
 *
 */
Word16
CreateSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf, /*!< Handle to return */
                        Word16 noCols                        /*!< Number of timeslots per frame */ 
                        )

{
  h_sbrQmf->p_filter = sbr_qmf_64_640_enc;                                      move32();
  h_sbrQmf->no_channels = NO_SYNTHESIS_CHANNELS;                                move16();
  h_sbrQmf->no_col = noCols;                                                    move16();
  
  if (h_sbrQmf->FilterStates == NULL) { /* this QMF bank hasn't been initialized before */
    h_sbrQmf->qmfStatesScale = INT_BITS-2;                                      move16();
  }
  h_sbrQmf->FilterStates = sbr_QmfStatesSynthesis;
    
  h_sbrQmf->cos_twiddle     = sbr_cos_twiddle_L32_enc;                          move32();
  h_sbrQmf->sin_twiddle     = sbr_sin_twiddle_L32_enc;                          move32();
  h_sbrQmf->alt_sin_twiddle = sbr_alt_sin_twiddle_L32_enc;                      move32();

  return 0;
}

#endif



/*!
 
  \brief      Performs complex-valued subband filtering of the time domain
              data of timeIn and stores the real part of the subband
              samples in rAnalysis, and the imaginary part in iAnalysis
 
  \return     none
 
*/
void
sbrAnalysisFiltering (const Word16 *timeIn, /*!< pointer to time domain input data */
                      Word16 timeInStride,  /*!< strid of time domain input data */
                      Word16 **rAnalysis,   /*!< pointer to a matrix with at least QMFBANK_NO_CHANNELS rows and QMFBANK_NO_COL columns */
                      Word16 **iAnalysis,   /*!< pointer to a matrix with at least QMFBANK_NO_CHANNELS rows and QMFBANK_NO_COL columns */
                      HANDLE_SBR_QMF_FILTER_BANK qmfBank /*!< pointer to struct of type SBR_QMF_FILTER_BANK */
                      )
{
  Word16 i, k;
  
  Word16 timeInScale,statesScale;
  Word16 commonScale;
  Word16 syn_buffer[2*QMF_CHANNELS];
  Word16 timeIn_test[QMF_CHANNELS];
  

  test(); move16();
  timeInScale = timeIn ? getScalefactor(timeIn,2048,timeInStride):SHORT_BITS-1;
  statesScale = add(qmfBank->qmfStatesScale, getScalefactor_Blockwise(qmfBank->qmf_states_buffer, 1,10));
  commonScale = S_min(timeInScale,statesScale);
 
  
  statesScale = sub(commonScale, qmfBank->qmfStatesScale);
 
  qmfBank->qmfScale = commonScale;                                                         move16();
  qmfBank->qmfStatesScale = commonScale;                                                   move16();


  /*
    Scale states buffer
  */
  scaleValues_Blockwise(qmfBank->qmf_states_buffer,statesScale);
 

  for (i = 0; i < qmfBank->no_col; i++) {
    /*!
      insert 64 new time signal samples
    */

    for(k=0;k<64;k++)
      timeIn_test[k]= timeIn[(i*64+k)*timeInStride];
    
    /*
      Scale actual time in slot
    */
    scaleValues(timeIn_test,64,commonScale);
 
    /*!
      FIR filtering with 5 taps & modulate 
    */
    
    AnalysisPolyphaseFiltering16(syn_buffer,qmfBank->qmf_states_buffer,timeIn_test,qmfBank->p_filter);

    forwardModulation(syn_buffer,
                      *(rAnalysis + i),
                      *(iAnalysis + i),
                      qmfBank);
    
  } /* no_col */
  AnalysisPostSpectrumScaling(&rAnalysis[0][0],&iAnalysis[0][0],qmfBank);
}


/*!
 
  \brief      Calculates energy form real and imaginary part 
              of the QMF subsamples 
 
  \return     none
 
*/
void
getEnergyFromCplxQmfData(Word32 **energyValues, /*!< the result of the operation, to get original energies, shift right by 2* qmfScale */
                         Word16 **realValues,   /*!< the real part of the QMF subsamples */
                         Word16 **imagValues,   /*!< the imaginary part of the QMF subsamples */
                         Word16 numberBands,    /*!< number of QMF bands */
                         Word16 numberCols      /*!< number of QMF subsamples */
                         ) 
{
  Word16 j, k;

  for (k=0; k<numberCols/2; k++) {
    for (j=0; j<numberBands; j++) {
      Word32 tmp1, tmp2, energy;
      tmp1 = L_add( L_shr(L_mult(realValues[2*k][j], realValues[2*k][j]), 2),
                    L_shr(L_mult(imagValues[2*k][j], imagValues[2*k][j]), 2));
      tmp2 = L_add( L_shr(L_mult(realValues[2*k+1][j], realValues[2*k+1][j]), 2),
                    L_shr(L_mult(imagValues[2*k+1][j], imagValues[2*k+1][j]), 2));
      energy = L_add( tmp1, tmp2 );

      energyValues[k][j] = energy;               move32();

    }
  }
  
}


/*!
 
  \brief      Creates QMF filter bank instance 
 
  \return     1 in case of an error, otherwise 0
 
*/

Word16
createQmfBank (Word16 chan,                             /*!< Number of Instance */
               HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf      /*!< pointer to QMF filter bank instance */
              )
{
  Word16 L;

  L =  QMF_CHANNELS ;                                                                                          move16();

  h_sbrQmf->no_channels =  QMF_CHANNELS ;                                                                      move16();
  h_sbrQmf->p_filter_length = QMF_FILTER_LENGTH;                                                               move16();
  h_sbrQmf->p_filter = sbr_qmf_64_640_enc;                                                                     move32();
  h_sbrQmf->no_poly = 5;                                                                                       move16();
  h_sbrQmf->no_col = 32;                                                                                       move16();

  test();
  if (chan == 0) {
    h_sbrQmf->qmf_states_buffer = sbr_QmfStatesAnalysis;                                                         move32();;
  }
  else {
    h_sbrQmf->qmf_states_buffer = sbr_QmfStatesAnalysis+640;                                                     move32(); L_add(1, 1);
  }
  h_sbrQmf->cos_twiddle     = sbr_cos_twiddle_L64_enc;                                                           move32();
  h_sbrQmf->sin_twiddle     = sbr_sin_twiddle_L64_enc;                                                           move32();
  h_sbrQmf->alt_sin_twiddle = sbr_alt_sin_twiddle_L64_enc;                                                       move32();

  return 0;

}

void
deleteQmfBank (
               HANDLE_SBR_QMF_FILTER_BANK  h_sbrQmf /*!<pointer to QMF filter bank instance */ 
               )
{
 
  /*
    nothing to do
  */
  (void)h_sbrQmf;
}


