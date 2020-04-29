/*
   channel mapping functionality
 */
#include <string.h>
#include "channel_map.h"
#include "bitenc.h"
#include "psy_const.h"
#include "qc_data.h"
#include "count.h"

static const Word16 maxChannelBits = 6144;

static Word16 initElement(ELEMENT_INFO* elInfo, ELEMENT_TYPE elType)
{
  Word16 error=0;                                       move16();

  elInfo->elType=elType;                                move16();

  test();
  switch(elInfo->elType) {

    case ID_SCE:
      elInfo->nChannelsInEl=1;                          move16();

      elInfo->ChannelIndex[0]=0;                        move16();

      elInfo->instanceTag=0;                            move16();
      break;

    case ID_CPE:

      elInfo->nChannelsInEl=2;                          move16();

      elInfo->ChannelIndex[0]=0;                        move16();
      elInfo->ChannelIndex[1]=1;                        move16();

      elInfo->instanceTag=0;                            move16();
      break;

    default:
      error=1;                                          move16();
  }

  return error;
}


Word16 InitElementInfo (Word16 nChannels, ELEMENT_INFO* elInfo)
{
  Word16 error;
  error = 0;                                            move16();

  test();
  switch(nChannels) {

    case 1: 
      initElement(elInfo, ID_SCE);
      break;

    case 2:
      initElement(elInfo, ID_CPE);
      break;

    default:
      error=1;                                          move16();
  }

  return error;
}


Word16 InitElementBits(ELEMENT_BITS *elementBits,
                       ELEMENT_INFO elInfo,
                       Word32 bitrateTot,
                       Word16 averageBitsTot,
                       Word16 staticBitsTot)
{
  Word16 error;
  error = 0;                                            move16();

  test();
  switch(elInfo.nChannelsInEl) {
    case 1:
      elementBits->chBitrate = bitrateTot;                            move32();
      elementBits->averageBits = sub(averageBitsTot, staticBitsTot);
      elementBits->maxBits = maxChannelBits;                          move16();

      elementBits->maxBitResBits = sub(maxChannelBits, averageBitsTot);
      elementBits->maxBitResBits = sub(elementBits->maxBitResBits, elementBits->maxBitResBits & 7);       logic16();
      elementBits->bitResLevel = elementBits->maxBitResBits;          move16();
      elementBits->relativeBits  = 0x4000; /* 1.0f/2 */               move16();
      break;

    case 2:
      elementBits->chBitrate   = L_shr(bitrateTot, 1);
      elementBits->averageBits = sub(averageBitsTot, staticBitsTot);
      elementBits->maxBits     = shl(maxChannelBits, 1);

      elementBits->maxBitResBits = sub(shl(maxChannelBits,1), averageBitsTot);
      elementBits->maxBitResBits = sub(elementBits->maxBitResBits, elementBits->maxBitResBits & 7);         logic16();
      elementBits->bitResLevel = elementBits->maxBitResBits;          move16();
      elementBits->relativeBits = 0x4000; /* 1.0f/2 */                move16();
      break;

    default:
      error = 1;                                                      move16();
  }
  return error;
}
