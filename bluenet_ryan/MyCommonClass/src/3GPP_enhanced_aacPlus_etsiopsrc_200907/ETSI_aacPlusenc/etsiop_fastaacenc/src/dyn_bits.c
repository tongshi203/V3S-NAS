/*
   Noiseless coder module
 */
#include <stdlib.h>
#include <limits.h>
#include "aac_ram.h"
#include "dyn_bits.h"
#include "bit_cnt.h"
#include "psy_const.h"
#include "count.h"




static Word16
calcSideInfoBits(Word16 sfbCnt,
                 Word16 blockType)
{
  Word16 seg_len_bits;
  Word16 escape_val;
  Word16 sideInfoBits, tmp;

  test(); move16(); move16();
  if (sub(blockType, SHORT_WINDOW) == 0) {
    seg_len_bits = 3;
    escape_val = 7;
  }
  else {
    seg_len_bits = 5;
    escape_val = 31;
  }

  sideInfoBits = CODE_BOOK_BITS;        move16();
  tmp = sfbCnt;                         move16();

  test();
  while (tmp >= 0) {
    sideInfoBits = add(sideInfoBits, seg_len_bits);
    tmp = sub(tmp, escape_val);
  }

  return (sideInfoBits);
}



/*
  count bits using all possible tables
*/

static void
buildBitLookUp(const Word16 *quantSpectrum,
               const Word16 maxSfb,
               const Word16 *sfbOffset,
               const UWord16 *sfbMax,
               Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
               SECTION_INFO * sectionInfo)
{
  Word16 i;

  for (i=0; i<maxSfb; i++) {
    Word16 sfbWidth, maxVal;

    sectionInfo[i].sfbCnt = 1;                                  move16();
    sectionInfo[i].sfbStart = i;                                move16();
    sectionInfo[i].sectionBits = INVALID_BITCOUNT;              move16();
    sectionInfo[i].codeBook = -1;                               move16();
    sfbWidth = sub(sfbOffset[i + 1], sfbOffset[i]);             move16();
    maxVal = sfbMax[i];                                         move16();
    bitCount(quantSpectrum + sfbOffset[i], sfbWidth, maxVal, bitLookUp[i]);
  }
}

/*
  essential helper functions
*/

static Word16
findBestBook(const Word16 *bc, Word16 *book)
{
  Word16 minBits, j;
  minBits = INVALID_BITCOUNT;                                   move16();

  for (j=0; j<=CODE_BOOK_ESC_NDX; j++) {
    test();
    if (sub(bc[j], minBits) < 0) {
      minBits = bc[j];                                          move16();
      *book = j;                                                move16();
    }
  }
  return (minBits);
}

static Word16
findMinMergeBits(const Word16 *bc1, const Word16 *bc2)
{
  Word16 minBits, j, sum;
  minBits = INVALID_BITCOUNT;                                   move16();

  for (j=0; j<=CODE_BOOK_ESC_NDX; j++) {
    sum = add(bc1[j], bc2[j]);
    if (sub(sum, minBits) < 0) {
      minBits = sum;                                            move16();
    }
  }
  return (minBits);
}

static void
mergeBitLookUp(Word16 *bc1, const Word16 *bc2)
{
  Word16 j;

  for (j=0; j<=CODE_BOOK_ESC_NDX; j++) {
    bc1[j] = S_min(add(bc1[j], bc2[j]), INVALID_BITCOUNT);
  }
}

static Word16
findMaxMerge(const Word16 mergeGainLookUp[MAX_SFB_LONG],
             const SECTION_INFO *sectionInfo,
             const Word16 maxSfb, Word16 *maxNdx)
{
  Word16 i, maxMergeGain;
  maxMergeGain = 0;                                             move16();

  for (i=0; i+sectionInfo[i].sfbCnt < maxSfb; i += sectionInfo[i].sfbCnt) {
    test();
    if (sub(mergeGainLookUp[i], maxMergeGain) > 0) {
      maxMergeGain = mergeGainLookUp[i];                        move16();
      *maxNdx = i;                                              move16();
    }
  }
  return (maxMergeGain);
}



static Word16
CalcMergeGain(const SECTION_INFO *sectionInfo,
              Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
              const Word16 *sideInfoTab,
              const Word16 ndx1,
              const Word16 ndx2)
{
  Word16 SplitBits;
  Word16 MergeBits;
  Word16 MergeGain;

  /*
    Bit amount for splitted sections
  */
  SplitBits = add(sectionInfo[ndx1].sectionBits, sectionInfo[ndx2].sectionBits);

  MergeBits = add(sideInfoTab[add(sectionInfo[ndx1].sfbCnt, sectionInfo[ndx2].sfbCnt)],
                  findMinMergeBits(bitLookUp[ndx1], bitLookUp[ndx2]));
  MergeGain = sub(SplitBits, MergeBits);

  return (MergeGain);
}

/*
  sectioning Stage 0:find minimum codbooks
*/

static void
gmStage0(SECTION_INFO * sectionInfo,
         Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
         const Word16 maxSfb)
{
  Word16 i;

  for (i=0; i<maxSfb; i++) {
    /* Side-Info bits will be calculated in Stage 1!  */
    test();
    if (sub(sectionInfo[i].sectionBits, INVALID_BITCOUNT) == 0) {
      sectionInfo[i].sectionBits = findBestBook(bitLookUp[i], &(sectionInfo[i].codeBook));
    }
  }
}

/*
  sectioning Stage 1:merge all connected regions with the same code book and
  calculate side info
*/

static void
gmStage1(SECTION_INFO * sectionInfo,
         Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
         const Word16 maxSfb,
         const Word16 *sideInfoTab)
{
  Word16 mergeStart, mergeEnd;
  mergeStart = 0;                                                       move16();

  do {

    for (mergeEnd=mergeStart+1; mergeEnd<maxSfb; mergeEnd++) {
      test();
      if (sub(sectionInfo[mergeStart].codeBook, sectionInfo[mergeEnd].codeBook) != 0)
        break;
      sectionInfo[mergeStart].sfbCnt = add(sectionInfo[mergeStart].sfbCnt, 1);
      sectionInfo[mergeStart].sectionBits = add(sectionInfo[mergeStart].sectionBits, sectionInfo[mergeEnd].sectionBits);

      mergeBitLookUp(bitLookUp[mergeStart], bitLookUp[mergeEnd]);
    }

    sectionInfo[mergeStart].sectionBits = add(sectionInfo[mergeStart].sectionBits, sideInfoTab[sectionInfo[mergeStart].sfbCnt]);
    sectionInfo[sub(mergeEnd, 1)].sfbStart = sectionInfo[mergeStart].sfbStart;      /* speed up prev search */ move16();

    mergeStart = mergeEnd;                                              move16();

    test();
  } while (sub(mergeStart, maxSfb) < 0);
}

/*
  sectioning Stage 2:greedy merge algorithm, merge connected sections with
  maximum bit gain until no more gain is possible
*/
static void
gmStage2(SECTION_INFO *sectionInfo,
         Word16 mergeGainLookUp[MAX_SFB_LONG],
         Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
         const Word16 maxSfb,
         const Word16 *sideInfoTab)
{
  Word16 i;

  for (i=0; i+sectionInfo[i].sfbCnt<maxSfb; i+=sectionInfo[i].sfbCnt) {
    mergeGainLookUp[i] = CalcMergeGain(sectionInfo,
                                       bitLookUp,
                                       sideInfoTab,
                                       i,
                                       add(i, sectionInfo[i].sfbCnt));
  }

  while (TRUE) {
    Word16 maxMergeGain, maxNdx, maxNdxNext, maxNdxLast;

    maxMergeGain = findMaxMerge(mergeGainLookUp, sectionInfo, maxSfb, &maxNdx);

    test();
    if (maxMergeGain <= 0)
      break;


    maxNdxNext = add(maxNdx, sectionInfo[maxNdx].sfbCnt);

    sectionInfo[maxNdx].sfbCnt = add(sectionInfo[maxNdx].sfbCnt, sectionInfo[maxNdxNext].sfbCnt);
    sectionInfo[maxNdx].sectionBits = add(sectionInfo[maxNdx].sectionBits,
                                          sub(sectionInfo[maxNdxNext].sectionBits, maxMergeGain));


    mergeBitLookUp(bitLookUp[maxNdx], bitLookUp[maxNdxNext]);

    test();
    if (maxNdx != 0) {
      maxNdxLast = sectionInfo[sub(maxNdx, 1)].sfbStart;
      mergeGainLookUp[maxNdxLast] = CalcMergeGain(sectionInfo,
                                                  bitLookUp,
                                                  sideInfoTab,
                                                  maxNdxLast,
                                                  maxNdx);
    }
    maxNdxNext = add(maxNdx, sectionInfo[maxNdx].sfbCnt);

    sectionInfo[sub(maxNdxNext, 1)].sfbStart = sectionInfo[maxNdx].sfbStart;            move16();

    test();
    if (sub(maxNdxNext, maxSfb) < 0) {
      mergeGainLookUp[maxNdx] = CalcMergeGain(sectionInfo,
                                              bitLookUp,
                                              sideInfoTab,
                                              maxNdx,
                                              maxNdxNext);
    }
  }
}

/*
  count bits used by the noiseless coder
*/
static void
noiselessCounter(SECTION_DATA *sectionData,
                 Word16 mergeGainLookUp[MAX_SFB_LONG],
                 Word16 bitLookUp[MAX_SFB_LONG][CODE_BOOK_ESC_NDX + 1],
                 const Word16 *quantSpectrum,
                 const UWord16 *maxValueInSfb,
                 const Word16 *sfbOffset,
                 const Word32 blockType)
{
  Word16 grpNdx, i;
  Word16 *sideInfoTab = NULL;
  SECTION_INFO *sectionInfo;

  /*
    use appropriate side info table
  */

  test();
  switch (blockType)
  {
    case LONG_WINDOW:
    case START_WINDOW:
    case STOP_WINDOW:
      sideInfoTab = sideInfoTabLong;
      break;
    case SHORT_WINDOW:
      sideInfoTab = sideInfoTabShort;
      break;
  }


  sectionData->noOfSections = 0;                                        move16();
  sectionData->huffmanBits = 0;                                         move16();
  sectionData->sideInfoBits = 0;                                        move16();

  test();
  if (sectionData->maxSfbPerGroup == 0)
    return;

  /*
    loop trough groups
  */
  for (grpNdx=0; grpNdx<sectionData->sfbCnt; grpNdx+=sectionData->sfbPerGroup) {

    sectionInfo = sectionData->sectionInfo + sectionData->noOfSections;

    buildBitLookUp(quantSpectrum,
                   sectionData->maxSfbPerGroup,
                   sfbOffset + grpNdx,
                   maxValueInSfb + grpNdx,
                   bitLookUp,
                   sectionInfo);

    /*
       0.Stage
    */
    gmStage0(sectionInfo, bitLookUp, sectionData->maxSfbPerGroup);

    /*
       1.Stage
    */
    gmStage1(sectionInfo, bitLookUp, sectionData->maxSfbPerGroup, sideInfoTab);


    /*
       2.Stage
    */
    gmStage2(sectionInfo,
             mergeGainLookUp,
             bitLookUp,
             sectionData->maxSfbPerGroup,
             sideInfoTab);


    /*
       compress output, calculate total huff and side bits
    */
    for (i=0; i<sectionData->maxSfbPerGroup; i+=sectionInfo[i].sfbCnt) {
      findBestBook(bitLookUp[i], &(sectionInfo[i].codeBook));
      sectionInfo[i].sfbStart = add(sectionInfo[i].sfbStart, grpNdx);

      sectionData->huffmanBits = add(sectionData->huffmanBits,
                                     sub(sectionInfo[i].sectionBits, sideInfoTab[sectionInfo[i].sfbCnt]));
      sectionData->sideInfoBits = add(sectionData->sideInfoBits, sideInfoTab[sectionInfo[i].sfbCnt]);
      sectionData->sectionInfo[sectionData->noOfSections] = sectionInfo[i];             memop16(4);
      sectionData->noOfSections = add(sectionData->noOfSections, 1);
    }
  }
}


/*******************************************************************************

     functionname: scfCount
     returns     : ---
     description : count bits used by scalefactors.

********************************************************************************/
static void scfCount(const Word16 *scalefacGain,
                     const UWord16 *maxValueInSfb,
                     SECTION_DATA * sectionData)

{
  /* counter */
  Word16 i = 0; /* section counter */
  Word16 j = 0; /* sfb counter */
  Word16 k = 0; /* current section auxiliary counter */
  Word16 m = 0; /* other section auxiliary counter */
  Word16 n = 0; /* other sfb auxiliary counter */

  /* further variables */
  Word16 lastValScf     = 0;
  Word16 deltaScf       = 0;
  Flag found            = 0;
  Word16 scfSkipCounter = 0;

  move16(); move16(); move16(); move16(); move16();
  move16(); move16(); move16(); move16();

  sectionData->scalefacBits = 0;                                move16();

  test();
  if (scalefacGain == NULL) {
    return;
  }

  lastValScf = 0;                                               move16();
  sectionData->firstScf = 0;                                    move16();

  for (i=0; i<sectionData->noOfSections; i++) {
    test();
    if (sub(sectionData->sectionInfo[i].codeBook, CODE_BOOK_ZERO_NO) != 0) {
      sectionData->firstScf = sectionData->sectionInfo[i].sfbStart;     move16();
      lastValScf = scalefacGain[sectionData->firstScf];                 move16();
      break;
    }
  }

  for (i=0; i<sectionData->noOfSections; i++) {
    test(); test();
    if (sub(sectionData->sectionInfo[i].codeBook, CODE_BOOK_ZERO_NO) != 0
        && sub(sectionData->sectionInfo[i].codeBook, CODE_BOOK_PNS_NO) != 0) {
      for (j = sectionData->sectionInfo[i].sfbStart;
           j < add(sectionData->sectionInfo[i].sfbStart, sectionData->sectionInfo[i].sfbCnt);
           j++) {
        /* check if we can repeat the last value to save bits */
        test();
        if (maxValueInSfb[j] == 0) {
          found = 0;                                                    move16();
          test();
          if (scfSkipCounter == 0) {
            /* end of section */
            test();
            if (sub(j, sub(add(sectionData->sectionInfo[i].sfbStart, sectionData->sectionInfo[i].sfbCnt), 1)) == 0) {
              found = 0;                                                move16();
            }
            else {
              for (k = add(j,1); k < add(sectionData->sectionInfo[i].sfbStart, sectionData->sectionInfo[i].sfbCnt); k++) {
                test();
                if (maxValueInSfb[k] != 0) {
                  found = 1;                                            move16();
                  test();
                  if ( sub(abs(sub(scalefacGain[k], lastValScf)), CODE_BOOK_SCF_LAV) < 0) {
                    /* save bits */
                    deltaScf = 0;                                       move16();
                  }
                  else {
                    /* do not save bits */
                    deltaScf = sub(lastValScf, scalefacGain[j]);
                    lastValScf = scalefacGain[j];                       move16();
                    scfSkipCounter = 0;                                 move16();
                  }
                  break;
                }
                /* count scalefactor skip */
                scfSkipCounter = add(scfSkipCounter, 1);
              }
            }

            /* search for the next maxValueInSfb[] != 0 in all other sections */
            for (m = add(i,1); (m < sectionData->noOfSections) && (found == 0); m++) {
              test(); test();
              if (sub(sectionData->sectionInfo[m].codeBook, CODE_BOOK_ZERO_NO) != 0 &&
                  sub(sectionData->sectionInfo[m].codeBook, CODE_BOOK_PNS_NO) != 0) {
                for (n = sectionData->sectionInfo[m].sfbStart;
                     n < add(sectionData->sectionInfo[m].sfbStart, sectionData->sectionInfo[m].sfbCnt);
                     n++) {
                  test();
                  if (maxValueInSfb[n] != 0) {
                    found = 1;                                          move16();
                    test();
                    if ( sub(abs(sub(scalefacGain[n], lastValScf)), CODE_BOOK_SCF_LAV) < 0) {
                      deltaScf = 0;                                     move16();
                    }
                    else {
                      deltaScf = sub(lastValScf, scalefacGain[j]);
                      lastValScf = scalefacGain[j];                     move16();
                      scfSkipCounter = 0;                               move16();
                    }
                    break;
                  }
                  /* count scalefactor skip */
                  scfSkipCounter = add(scfSkipCounter, 1);
                }
              }
            }

            test();
            if (found == 0) {
              deltaScf = 0;                                             move16();
              scfSkipCounter = 0;                                       move16();
            }
          }
          else {
            deltaScf = 0;                                               move16();
            scfSkipCounter = sub(scfSkipCounter, 1);
          }
        }
        else {
          deltaScf = sub(lastValScf, scalefacGain[j]);
          lastValScf = scalefacGain[j];                                 move16();
        }
        sectionData->scalefacBits = add(sectionData->scalefacBits, bitCountScalefactorDelta(deltaScf));
      }
    }
  }
}


typedef Word16 (*lookUpTable)[CODE_BOOK_ESC_NDX + 1];


Word16
dynBitCount(const Word16  *quantSpectrum,
            const UWord16 *maxValueInSfb,
            const Word16  *scalefac,
            const Word16   blockType,
            const Word16   sfbCnt,
            const Word16   maxSfbPerGroup,
            const Word16   sfbPerGroup,
            const Word16  *sfbOffset,
            SECTION_DATA  *sectionData)
{
  sectionData->blockType      = blockType;                      move16();
  sectionData->sfbCnt         = sfbCnt;                         move16();
  sectionData->sfbPerGroup    = sfbPerGroup;                    move16();
  sectionData->noOfGroups     = ffr_Short_Div(sfbCnt, sfbPerGroup);
  sectionData->maxSfbPerGroup = maxSfbPerGroup;                 move16();

  noiselessCounter(sectionData,
                   sectionData->mergeGainLookUp,
                   (lookUpTable)sectionData->bitLookUp,
                   quantSpectrum,
                   maxValueInSfb,
                   sfbOffset,
                   blockType);

  scfCount(scalefac,
           maxValueInSfb,
           sectionData);


  return (add(add(sectionData->huffmanBits,
                  sectionData->sideInfoBits),
                  sectionData->scalefacBits));
}


Word16 RecalcSectionInfo(SECTION_DATA  *sectionData,
                         const UWord16 *maxValueInSfb,
                         const Word16  *scalefac,
                         const Word16   blockType)
{
  Word16 i;
  sectionData->huffmanBits = 0;                                 move16();
  sectionData->sideInfoBits = 0;                                move16();

  for (i=0; i<sectionData->noOfSections; i++) {
    Word16 siBits;
    sectionData->huffmanBits = add(sectionData->huffmanBits, sectionData->sectionInfo[i].sectionBits);
    test(); move16();
    if (sub(blockType, SHORT_WINDOW) != 0)
      siBits = sideInfoTabLong[sectionData->sectionInfo[i].sfbCnt];
    else
      siBits = sideInfoTabShort[sectionData->sectionInfo[i].sfbCnt];

    sectionData->sectionInfo[i].sectionBits = add(sectionData->sectionInfo[i].sectionBits, siBits);
    sectionData->sideInfoBits = add(sectionData->sideInfoBits, siBits);
  }

  scfCount(scalefac,
           maxValueInSfb,
           sectionData);

  return (sectionData->huffmanBits +
          sectionData->sideInfoBits +
          sectionData->scalefacBits);
}


Word16
BCInit(void)
{
  Word16 i;

  for (i=0; i<=MAX_SFB_LONG; i++)
    sideInfoTabLong[i] = calcSideInfoBits(i, LONG_WINDOW);
  for (i = 0; i <= MAX_SFB_SHORT; i++)
    sideInfoTabShort[i] = calcSideInfoBits(i, SHORT_WINDOW);

  return (0);
}
