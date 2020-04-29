/*
   Remaining SBR Bit Writing Routines
 */ 

#include <stdlib.h>
#include <string.h>
#include "sbr_main.h"
#include "env_bit.h"
#include "cmondata.h"

#include "count.h"



/*
  Inittialisation of sbr bitstream, write of dummy header 
*/
void InitSbrBitstream(HANDLE_COMMON_DATA  hCmonData)
{

  ResetBitBuf(&hCmonData->sbrBitbuf, hCmonData->sbrBitbuf.pBitBufBase, shr(hCmonData->sbrBitbuf.size, 3) /* / 8 */);

  WriteBits (&hCmonData->sbrBitbuf, 0, SI_FILL_EXTENTION_BITS);
}


/* ************************** AssembleSbrBitstream *******************************/
/**
 * @fn
 * @brief    Formats the SBR payload
 * @return   nothing
 *
 *
 */

void
AssembleSbrBitstream( HANDLE_COMMON_DATA  hCmonData)
{

  Word16 sbrLoad=0;
  struct BIT_BUF tmpWriteBitbuf;

  /* Let tmpWriteBitbuf point to the beginning of the SBR payload
     where the length info needs to be written */
  CreateBitBuffer(&tmpWriteBitbuf,
                  hCmonData->sbrBitbuf.pBitBufBase,
                  MAX_PAYLOAD_SIZE);

  /* check if SBR is present */
  test();
  if ( hCmonData==NULL )
    return;


  sbrLoad = add(hCmonData->sbrHdrBits, hCmonData->sbrDataBits);
  sbrLoad = add(sbrLoad, SI_FILL_EXTENTION_BITS);        /* signaling is done via fill elem in aac case */

  hCmonData->sbrFillBits = sub(8, (sbrLoad & 7)) & 7;                                 logic16(); logic16(); move16();
  sbrLoad = add(sbrLoad, hCmonData->sbrFillBits);

  /* 
     append fill bits
  */
  WriteBits(&hCmonData->sbrBitbuf, 0,  hCmonData->sbrFillBits );


  /* write sbr data as fill element */
  /* signal sbr extention payload  and write header */
  WriteBits (&tmpWriteBitbuf, AAC_SI_FIL_SBR, SI_FILL_EXTENTION_BITS);
   
}

