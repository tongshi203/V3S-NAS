/*
   Parametric stereo bitstream encoder
 */

#include "sbr_def.h"
#include "ps_bitenc.h"
#include "ps_enc.h"
#include "env_bit.h"
#include "sbr_ram.h"
#include "sbr_rom.h"

#include "count.h"


/*!
  \brief Compose parametric stereo bitstream data for 1 frame
  \return Number of bits written
*/
Word32 WritePsData (HANDLE_PS_ENC h_ps_e,        /*!< Parametric stereo encoder instance */
                    Word32        bHeaderActive) /*!< flag to transmit PS header data */
{
  Word32 temp, gr;
  const Word32 *aaHuffBookIidC;
  const Word16 *aaHuffBookIccC;
  const Word8 *aaHuffBookIidL;
  const Word8 *aaHuffBookIccL;
  Word32 *aaDeltaIid;
  Word32 *aaDeltaIcc;

  Word16 index, lastIndex;
  Word32 noBitsF = 0;
  Word32 noBitsT = 0;

  Word32 aaDeltaIidT[NO_IID_BINS];
  Word32 aaDeltaIccT[NO_ICC_BINS];

  Word32 aaDeltaIidF[NO_IID_BINS];
  Word32 aaDeltaIccF[NO_ICC_BINS];

  Word32 abDtFlagIid;
  Word32 abDtFlagIcc;

  Word32 bSendHeader;

  UWord32 bZeroIid = 1;
  UWord32 bZeroIcc = 1;
  UWord16 bKeepParams = 1;

  HANDLE_BIT_BUF bb = &h_ps_e->psBitBuf;

  temp = bb->cntBits;

  /* bit buffer shall be empty */
  test();
  if (temp != 0) {
    return -1;
  }

  WriteBits (bb, EXTENSION_ID_PS_CODING, SI_SBR_EXTENSION_ID_BITS);

  test();
  if (bHeaderActive) {
    bKeepParams = 0;
  }

  lastIndex = 0;

  for (gr = 0; gr < h_ps_e->iidIccBins; gr++) {
    /*
      Quantize IID value
    */
    Word32 panValue = h_ps_e->aaaIIDDataBuffer[gr];

    test(); test(); L_sub(1, 1); L_sub(1, 1);
    if (panValue >= iidQuantRight[0] &&
        panValue <= iidQuantLeft[0]) {

      index = 0;                                                                                move32();
    }
    else {
      test(); L_sub(1, 1);
      if (panValue < 0x00800000) {
        for (index = NO_IID_STEPS-1; panValue > iidQuantRight[index]; index--) {
          ;
        }
        index = sub(-1, index);
      }
      else  {
        for (index = NO_IID_STEPS-1; panValue < iidQuantLeft[index]; index--) {
          ;
        }
        index = add(index, 1);
      }
      bZeroIid = 0;                                                                             move32();
    }

    /* Delta coding in frequency direction */
    if (gr == 0) {
      aaDeltaIidF[gr] = index;                                                                  move32();
      noBitsT = 0;                                                                              move32();
      noBitsF = aBookPsIidFreqLength[index + CODE_BOOK_LAV_IID];                                L_deposit_l(1); logic16(); logic16(); /* Word8 read access */
    }
    else {
      aaDeltaIidF[gr] = L_sub(index, lastIndex);
      noBitsF = L_add(noBitsF, aBookPsIidFreqLength[aaDeltaIidF[gr] + CODE_BOOK_LAV_IID]);      L_deposit_l(1); logic16(); logic16(); /* Word8 read access */
    }
    lastIndex = index;                                                                          move32();

    /* Delta coding in time direction */
    aaDeltaIidT[gr] = L_sub(index, h_ps_e->aLastIidIndex[gr]);                                  logic16(); logic16(); /* Word8 read access */;
    h_ps_e->aLastIidIndex[gr] = (Word8)index;                                                          logic16(); logic16(); logic16(); /* Word8 write access */;
    noBitsT = L_add(noBitsT, aBookPsIidTimeLength[aaDeltaIidT[gr] + CODE_BOOK_LAV_IID]);        L_deposit_l(1); logic16(); logic16(); /* Word8 read access */

    test();
    if (aaDeltaIidT[gr] != 0) {
      bKeepParams = 0;                                                                          move32();
    }
  } /* gr */

  /* Evaluate if delta coding in time or frequency direction is cheaper
     for the IID values*/
  test(); test(); L_sub(1, 1);
  if (noBitsT < noBitsF && !bHeaderActive) {
    abDtFlagIid    = 1;                                                                         move32();
    aaDeltaIid     = aaDeltaIidT;                                                               move16();
    aaHuffBookIidC = aBookPsIidTimeCode;                                                        move16();
    aaHuffBookIidL = aBookPsIidTimeLength;                                                      move16();
  }
  else  {
    abDtFlagIid    = 0;                                                                         move32();
    aaDeltaIid     = aaDeltaIidF;                                                               move16();
    aaHuffBookIidC = aBookPsIidFreqCode;                                                        move16();
    aaHuffBookIidL = aBookPsIidFreqLength;                                                      move16();
  }


  lastIndex = 0;                                                                                move32();

  for (gr = 0; gr < h_ps_e->iidIccBins; gr++) {
    /*
      Quantize ICC value
    */
    Word32 icc = h_ps_e->aaaICCDataBuffer[gr];                                                  move32();

    for (index = 0; iccQuant[index] > icc; index++) {
      ;
    }
    test();
    if (index != 0) {
      bZeroIcc = 0;                                                                             move32();
    }

    /* Delta coding in frequency direction */
    test();
    if (gr == 0) {
      aaDeltaIccF[gr] = index;                                                                  move32();

      noBitsF = aBookPsIccFreqLength[index + CODE_BOOK_LAV_ICC];                                L_deposit_l(1); logic16(); logic16(); /* Word8 read access */;
      noBitsT = 0;                                                                              move32();
    }
    else  {
      aaDeltaIccF[gr] = L_sub(index, lastIndex);
      noBitsF = L_add(noBitsF, aBookPsIccFreqLength[aaDeltaIccF[gr] + CODE_BOOK_LAV_ICC]);      L_deposit_l(1); logic16(); logic16(); /* Word8 read access */
    }
    lastIndex = index;                                                                          move32();

    /* Delta coding in time direction */
    aaDeltaIccT[gr] = L_sub(index, h_ps_e->aLastIccIndex[gr]);                                  logic16(); logic16(); /* Word8 read access */
    h_ps_e->aLastIccIndex[gr] = (Word8)index;                                                   logic16(); logic16(); logic16(); /* Word8 write access */            
    noBitsT = L_add(noBitsT, aBookPsIccTimeLength[aaDeltaIccT[gr] + CODE_BOOK_LAV_ICC]);        L_deposit_l(1); logic16(); logic16(); /* Word8 read access */

    test();
    if (aaDeltaIccT[gr] != 0) {
      bKeepParams = 0;                                                                          move32();
    }
  } /* gr */

  /* Evaluate if delta coding in time or frequency direction is cheaper
     for the ICC values*/
  test(); test(); L_sub(1, 1);
  if (noBitsT < noBitsF && !bHeaderActive) {
    abDtFlagIcc    = 1;                                                                         move32();
    aaDeltaIcc     = aaDeltaIccT;                                                               move16();
    aaHuffBookIccC = aBookPsIccTimeCode;                                                        move16();
    aaHuffBookIccL = aBookPsIccTimeLength;                                                      move16();
  }
  else {
    abDtFlagIcc    = 0;                                                                         move32();
    aaDeltaIcc     = aaDeltaIccF;                                                               move16();
    aaHuffBookIccC = aBookPsIccFreqCode;                                                        move16();
    aaHuffBookIccL = aBookPsIccFreqLength;                                                      move16();
  }


  /* Force PS header for first frame or if an SBR header is sent */
  {
    static Word32 initheader = 0; 

    test(); test(); test(); test(); L_sub(1, 1); L_sub(1, 1);
    if (!initheader ||
        bHeaderActive ||
        h_ps_e->bPrevZeroIid != bZeroIid ||
        h_ps_e->bPrevZeroIcc != bZeroIcc) {
      initheader = 1;                                                                           move32();
      bSendHeader = 1;                                                                          move32();
    }
    else {
      bSendHeader = 0;                                                                          move32();
    }
  }

  /*
    Write PS header data
    (settings that normally do not change)
  */
  WriteBits (bb, bSendHeader, 1);

  test(); 
  if (bSendHeader) {
    WriteBits (bb, !bZeroIid, 1);           logic32();

    test(); 
    if (!bZeroIid)
      {
        test();
        WriteBits (bb, (h_ps_e->bHiFreqResIidIcc)?1:0, 3);
      }

    WriteBits (bb, !bZeroIcc, 1);           logic32();

    test();
    if (!bZeroIcc)
      {
        test();
        WriteBits (bb, (h_ps_e->bHiFreqResIidIcc)?1:0, 3);
      }

    WriteBits (bb, 0, 1);
  }

  WriteBits (bb, 0, 1);
  WriteBits (bb, sub(1, bKeepParams), 2);

  test();
  if (!bKeepParams) {

    test();
    if (!bZeroIid) {
      WriteBits (bb, abDtFlagIid, 1);

      for (gr = 0; gr < h_ps_e->iidIccBins; gr++) {

        WriteBits (bb,
                   aaHuffBookIidC[aaDeltaIid[gr] + CODE_BOOK_LAV_IID],
                   aaHuffBookIidL[aaDeltaIid[gr] + CODE_BOOK_LAV_IID]);                             logic16(); logic16(); /* Word8 read access */
      } /* gr */
    }  /* if (!bZeroIid) */
  }

  test();
  if (!bKeepParams) {
    test();
    if (!bZeroIcc) {
      WriteBits (bb, abDtFlagIcc, 1);
      for (gr = 0; gr < h_ps_e->iidIccBins; gr++) {
        WriteBits (bb,
                   aaHuffBookIccC[aaDeltaIcc[gr] + CODE_BOOK_LAV_ICC],
                   aaHuffBookIccL[aaDeltaIcc[gr] + CODE_BOOK_LAV_ICC]);                             logic16(); logic16(); /* Word8 read access */
      } /* gr */
    }  /* if (!bZeroIcc) */
  }

  h_ps_e->bPrevZeroIid = (UWord8)bZeroIid;                 move16();
  h_ps_e->bPrevZeroIcc = (UWord8)bZeroIcc;                 move16();

  return bb->cntBits;                                      move32();

} /* writePsData */
