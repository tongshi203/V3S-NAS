/*
   Sbr miscellaneous helper functions
 */
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "sbr_misc.h"
#include "ffr.h"
#include "count.h"


/***************************************************************************/
/*!
 
  \brief  calculates  nom*scale/denom with enhanced precision for
          denom > scale

  \return nom*scale/denom
 
****************************************************************************/
Word32 mulScaleDiv(Word32 nom,Word32 scale,Word32 denom)
{
  Word16 sf;
  Word32 ret;

  sf = ffr_norm32(denom);
  
  denom = L_shl(denom, sf);
  scale = L_shl(scale, sf);

  nom = fixmul(scale, nom);

  ret = ffr_div32_32(nom, denom);
  
  return(ret);
}


/* Sorting routine */
void Shellsort_short (Word16 *in, Word32 n)
{
  Word16 v;
  Word16 i, j;
  Word16 inc = 1;

  assert(abs(n) < 32768); 

  do {
    inc = add(add(inc, add(inc, inc)), 1);
    test();
  } while (inc <= n);

  do {
    inc = ffr_Short_Div(inc, 3);
    for (i = add(inc, 1); i <= n; i++) {
      v = in[i-1];
      j = i;                         move16();
      while (in[j-inc-1] > v) {
        test();
        in[j-1] = in[j-inc-1];       move16();
        j = sub(j, inc);
        test(); sub(1, 1);
        if (j <= inc)
          break;
      }
      in[j-1] = v;                   move16();
    }
    test();
  } while (inc > 1);

}


/* Sorting routine */
void Shellsort_fract (Word32 *in, Word32 n)
{
  Word32 v;
  Word16 i, j;
  Word16 inc = 1;

  assert(abs(n) < 32768); 

  do {
    inc = add(add(inc, add(inc, inc)), 1);
    test();
  } while (inc <= n);

  do {
    inc = ffr_Short_Div(inc, 3);
    for (i = add(inc, 1); i <= n; i++) {
      v = in[i-1];
      j = i;                         move16();
      while (in[j-inc-1] > v) {
        test();
        in[j-1] = in[j-inc-1];       move32();
        j = sub(j, inc);
        test(); sub(1, 1);
        if (j <= inc)
          break;
      }
      in[j-1] = v;                   move32();
    }
    test();
  } while (inc > 1);

}


/* Sorting routine */
void Shellsort_int (Word32 *in, Word32 n)
{
  Shellsort_fract(in, n);
}



/*******************************************************************************
 Functionname:  AddVecLeft
 *******************************************************************************

 Description: 

 Arguments:   Word32* dst, Word32* length_dst, Word32* src, Word32 length_src

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
void
AddVecLeft (Word16 *dst, Word16 *length_dst, Word16 *src, Word16 length_src)
{
  Word32 i;

  for (i = sub(length_src, 1); i >= 0; i--)
    AddLeft (dst, length_dst, src[i]);
}


/*******************************************************************************
 Functionname:  AddLeft
 *******************************************************************************

 Description: 

 Arguments:   Word32* vector, Word32* length_vector, Word32 value

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
void
AddLeft (Word16 *vector, Word16 *length_vector, Word16 value)
{
  Word32 i;

  for (i = *length_vector; i > 0; i--) {
    vector[i] = vector[i - 1];   move16();
  }
  vector[0] = value;             move16();
  *length_vector = add(*length_vector, 1);
}


/*******************************************************************************
 Functionname:  AddRight
 *******************************************************************************

 Description: 

 Arguments:   Word32* vector, Word32* length_vector, Word32 value

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
void
AddRight (Word16 *vector, Word16 *length_vector, Word16 value)
{
  vector[*length_vector] = value; move16();
  *length_vector = add(*length_vector, 1);
}



/*******************************************************************************
 Functionname:  AddVecRight
 *******************************************************************************

 Description: 

 Arguments:   Word32* dst, Word32* length_dst, Word32* src, Word32 length_src)

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
void
AddVecRight (Word16 *dst, Word16 *length_dst, Word16 *src, Word16 length_src)
{
  Word32 i;
  for (i = 0; i < length_src; i++)
    AddRight (dst, length_dst, src[i]);
}

