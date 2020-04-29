/*
   Short block grouping
 */
#include "psy_const.h"
#include "interface.h"
#include "count.h"
#include "grp_data.h"


/*
* this routine does not work in-place
*/
void
groupShortData(Word32        *mdctSpectrum,
               Word32        *tmpSpectrum,
               SFB_THRESHOLD *sfbThreshold,
               SFB_ENERGY    *sfbEnergy,
               SFB_ENERGY    *sfbEnergyMS,
               SFB_ENERGY    *sfbSpreadedEnergy,
               const Word16   sfbCnt,
               const Word16  *sfbOffset,
               const Word16  *sfbMinSnr,
               Word16        *groupedSfbOffset,
               Word16        *maxSfbPerGroup,
               Word16        *groupedSfbMinSnr,
               const Word16   noOfGroups,
               const Word16  *groupLen)
{
  Word16 i, j;
  Word16 line;
  Word16 sfb;
  Word16 grp;
  Word16 wnd;
  Word16 offset;
  Word16 highestSfb;

  /* for short: regroup and  */
  /* cumulate energies und thresholds group-wise . */
  
  /* calculate sfbCnt */
  highestSfb = 0;                                       move16();
  for (wnd=0; wnd<TRANS_FAC; wnd++) {
    for (sfb=sub(sfbCnt,1); sfb>=highestSfb; sfb--) {
      for (line=sub(sfbOffset[add(sfb,1)],1); line>=sfbOffset[sfb]; line--) {
        test();
        if (mdctSpectrum[wnd*FRAME_LEN_SHORT+line] != 0) break; 
      }
      test();
      if (sub(line, sfbOffset[sfb]) >= 0) break;
    }
    highestSfb = S_max(highestSfb, sfb);
  }
  test();
  if (highestSfb < 0) {
    highestSfb = 0;                                     move16();
  }
  *maxSfbPerGroup = add(highestSfb, 1);

  /* calculate sfbOffset */
  i = 0;                                                move16();
  offset = 0;                                           move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      groupedSfbOffset[i] = add(offset, extract_l(ffr_Integer_Mult16x16(sfbOffset[sfb], groupLen[grp])));
      i = add(i, 1);
    }
    offset = add(offset, extract_l(ffr_Integer_Mult16x16(groupLen[grp], FRAME_LEN_SHORT)));
  }
  groupedSfbOffset[i] = FRAME_LEN_LONG;                 move16();
  i = add(i, 1);

  /* calculate minSnr */
  i = 0;                                                move16();
  offset = 0;                                           move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      groupedSfbMinSnr[i] = sfbMinSnr[sfb];             move16();
      i = add(i, 1);
    }
    offset = add(offset, extract_l(ffr_Integer_Mult16x16(groupLen[grp], FRAME_LEN_SHORT)));
  }


  /* sum up sfbThresholds */
  wnd = 0;                                                      move16();
  i = 0;                                                        move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      Word32 thresh = sfbThreshold->sfbShort[wnd][sfb];         move32();
      for (j=1; j<groupLen[grp]; j++) {
        thresh = L_add(thresh, sfbThreshold->sfbShort[wnd+j][sfb]);
      }
      sfbThreshold->sfbLong[i] = thresh;                        move32();
      i = add(i, 1);
    }
    wnd = add(wnd, groupLen[grp]);
  }

  /* sum up sfbEnergies left/right */
  wnd = 0;                                                      move16();
  i = 0;                                                        move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      Word32 energy = sfbEnergy->sfbShort[wnd][sfb];            move32();
      for (j=1; j<groupLen[grp]; j++) {
        energy = L_add(energy, sfbEnergy->sfbShort[wnd+j][sfb]);
      }
      sfbEnergy->sfbLong[i] = energy;                           move32();
      i = add(i, 1);
    }
    wnd = add(wnd, groupLen[grp]);
  }

  /* sum up sfbEnergies mid/side */
  wnd = 0;                                                      move16();
  i = 0;                                                        move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      Word32 energy = sfbEnergyMS->sfbShort[wnd][sfb];          move32();
      for (j=1; j<groupLen[grp]; j++) {
        energy = L_add(energy, sfbEnergyMS->sfbShort[wnd+j][sfb]);
      }
      sfbEnergyMS->sfbLong[i] = energy;                         move32();
      i = add(i, 1);
    }
    wnd = add(wnd, groupLen[grp]);
  }

  /* sum up sfbSpreadedEnergies */
  wnd = 0;                                                      move16();
  i = 0;                                                        move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      Word32 energy = sfbSpreadedEnergy->sfbShort[wnd][sfb];    move32();
      for (j=1; j<groupLen[grp]; j++) {
        energy = L_add(energy, sfbSpreadedEnergy->sfbShort[wnd+j][sfb]);
      }
      sfbSpreadedEnergy->sfbLong[i] = energy;                   move32();
      i = add(i, 1);
    }
    wnd = add(wnd, groupLen[grp]);
  }

  /* re-group spectrum */
  wnd = 0;                                                      move16();
  i = 0;                                                        move16();
  for (grp = 0; grp < noOfGroups; grp++) {
    for (sfb = 0; sfb < sfbCnt; sfb++) {
      for (j = 0; j < groupLen[grp]; j++) {
        Word16 lineOffset = extract_l(ffr_Integer_Mult16x16(FRAME_LEN_SHORT, add(wnd,j)));
        for (line = add(lineOffset,sfbOffset[sfb]); line < add(lineOffset,sfbOffset[sfb+1]); line++) {
          tmpSpectrum[i] = mdctSpectrum[line];                  move32();
          i = add(i, 1);
        }
      }
    }
    wnd = add(wnd, groupLen[grp]);
  }

  for(i=0;i<FRAME_LEN_LONG;i++) {
    mdctSpectrum[i] = tmpSpectrum[i];                           move32();
  }
}

