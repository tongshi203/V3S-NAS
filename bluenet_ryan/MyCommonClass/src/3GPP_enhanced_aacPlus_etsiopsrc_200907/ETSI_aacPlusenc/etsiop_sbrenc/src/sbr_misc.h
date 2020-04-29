/*
   Sbr miscellaneous helper functions prototypes
 */

#ifndef _SBR_MISC_H
#define _SBR_MISC_H

#include "ffr.h"
#include "ffr.h"
#include "intrinsics.h"

/* Sorting routines */
void  Shellsort_fract (Word32 *in, Word32 n);
void  Shellsort_int   (Word32 *in, Word32 n);
void Shellsort_short (Word16 *in, Word32 n);

/* arithmethic helper routines */
Word32 mulScaleDiv(Word32 nom,Word32 scale,Word32 denom);

void AddLeft (Word16 *vector, Word16 *length_vector, Word16 value);
void AddRight (Word16 *vector, Word16 *length_vector, Word16 value);
void AddVecLeft (Word16 *dst, Word16 *length_dst, Word16 *src, Word16 length_src);
void AddVecRight (Word16 *dst, Word16 *length_vector_dst, Word16 *src, Word16 length_src);
#endif
