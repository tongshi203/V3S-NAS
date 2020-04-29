/*
   Remaining SBR Bit Writing Routines
 */ 

#ifndef BIT_ENV_H
#define BIT_ENV_H

#include "FFR_bitbuffer.h"

#define AAC_SI_FIL_SBR             13  /* 1101 */

#define  SI_ID_BITS_AAC             3
#define  SI_FILL_COUNT_BITS         4
#define  SI_FILL_ESC_COUNT_BITS     8
#define  SI_FILL_EXTENTION_BITS     4
#define  ID_FIL                     6


struct COMMON_DATA;

void InitSbrBitstream(struct COMMON_DATA  *hCmonData);
void AssembleSbrBitstream(struct COMMON_DATA  *hCmonData); 
                   
#endif /* #ifndef BIT_ENV_H */
