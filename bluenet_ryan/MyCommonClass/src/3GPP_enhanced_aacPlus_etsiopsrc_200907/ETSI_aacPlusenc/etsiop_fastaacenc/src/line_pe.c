/*
   Perceptual entropie module
 */
#include <stdio.h>
#include "ffr.h"
#include "line_pe.h"
#include "count.h"


static const Word16  C1_I = 12;    /* log(8.0)/log(2) *4         */
static const Word32  C2_I = 10830; /* log(2.5)/log(2) * 1024 * 4 * 2 */
static const Word16  C3_I = 573;   /* (1-C2/C1) *1024            */



/* constants that do not change during successive pe calculations */

void prepareSfbPe(PE_DATA *peData,
                  PSY_OUT_CHANNEL  psyOutChannel[MAX_CHANNELS],
                  Word16 logSfbEnergy[MAX_CHANNELS][MAX_GROUPED_SFB],
                  Word16 sfbNRelevantLines[MAX_CHANNELS][MAX_GROUPED_SFB],
                  const Word16 nChannels,
                  const Word16 peOffset)
{
  Word16 sfbGrp, sfb;
  Word16 ch;     

  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    PE_CHANNEL_DATA *peChanData=&peData->peChannelData[ch];
    for(sfbGrp=0;sfbGrp<psyOutChan->sfbCnt; sfbGrp+=psyOutChan->sfbPerGroup){
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
	    peChanData->sfbNLines4[sfbGrp+sfb] = sfbNRelevantLines[ch][sfbGrp+sfb];         move16();
        sfbNRelevantLines[ch][sfbGrp+sfb] = shr(sfbNRelevantLines[ch][sfbGrp+sfb],2);   move16();
	    peChanData->sfbLdEnergy[sfbGrp+sfb] = logSfbEnergy[ch][sfbGrp+sfb];             move16();
      }
    }
  }
  peData->offset = peOffset;                                                            move16();
}

/*
   constPart is sfbPe without the threshold part n*ld(thr) or n*C3*ld(thr)
*/

void calcSfbPe(PE_DATA *peData,
               PSY_OUT_CHANNEL psyOutChannel[MAX_CHANNELS],
               const Word16 nChannels)
{
  Word16 ch;
  Word16 sfbGrp, sfb;
  Word16 nLines4;
  Word16 ldThr, ldRatio;

  peData->pe = peData->offset;                                          move16();
  peData->constPart = 0;                                                move16();
  peData->nActiveLines = 0;                                             move16();
  for(ch=0; ch<nChannels; ch++) {
    PSY_OUT_CHANNEL *psyOutChan = &psyOutChannel[ch];
    PE_CHANNEL_DATA *peChanData = &peData->peChannelData[ch];
    const Word32 *sfbEnergy = psyOutChan->sfbEnergy;
    const Word32 *sfbThreshold = psyOutChan->sfbThreshold;

    peChanData->pe = 0;                                                 move16();
    peChanData->constPart = 0;                                          move16();
    peChanData->nActiveLines = 0;                                       move16();

    for(sfbGrp=0; sfbGrp<psyOutChan->sfbCnt; sfbGrp+=psyOutChan->sfbPerGroup) {
      for (sfb=0; sfb<psyOutChan->maxSfbPerGroup; sfb++) {
        Word32 nrg = sfbEnergy[sfbGrp+sfb];                             
        Word32 thres = sfbThreshold[sfbGrp+sfb];                        move32(); move32();
        test();
        if (L_sub(nrg, thres) > 0) {
          ldThr = ffr_iLog4(thres);

          ldRatio = sub(peChanData->sfbLdEnergy[sfbGrp+sfb], ldThr);

          nLines4 = peChanData->sfbNLines4[sfbGrp+sfb];                   move16();
          test();
          if (sub(ldRatio, C1_I) >= 0) {
            peChanData->sfbPe[sfbGrp+sfb] = shr(add(extract_l(L_mult(nLines4, ldRatio)), 4<<2), 3+2);
            peChanData->sfbConstPart[sfbGrp+sfb] = extract_l(L_shr(L_mult(nLines4, peChanData->sfbLdEnergy[sfbGrp+sfb]), 3+2));
          }
          else {
            peChanData->sfbPe[sfbGrp+sfb] = extract_l(L_shr( L_add( fixmul_32x16b(
                    L_shl(L_add(C2_I, L_mult(C3_I, ldRatio)), 4),
                    nLines4),
                  2<<2), 2+2));
            peChanData->sfbConstPart[sfbGrp+sfb] = extract_l(L_shr( L_add( fixmul_32x16b(
                    L_shl(L_add(C2_I, L_mult(C3_I, peChanData->sfbLdEnergy[sfbGrp+sfb])), 4),
                    nLines4),
                  2<<2), 2+2));
            nLines4 = extract_l(L_shr(L_add(L_mult(nLines4,C3_I),1024<<2),11));
          }
          peChanData->sfbNActiveLines[sfbGrp+sfb] = shr(nLines4,2);
        }
        else {
          peChanData->sfbPe[sfbGrp+sfb] = 0;                            move16();
          peChanData->sfbConstPart[sfbGrp+sfb] = 0;                     move16();
          peChanData->sfbNActiveLines[sfbGrp+sfb] = 0;                  move16();
        }
        peChanData->pe = add(peChanData->pe, peChanData->sfbPe[sfbGrp+sfb]);
        peChanData->constPart = add(peChanData->constPart, peChanData->sfbConstPart[sfbGrp+sfb]);
        peChanData->nActiveLines = add(peChanData->nActiveLines, peChanData->sfbNActiveLines[sfbGrp+sfb]);
      }
    }

    peData->pe = add(peData->pe, peChanData->pe);
    peData->constPart = add(peData->constPart, peChanData->constPart);
    peData->nActiveLines = add(peData->nActiveLines, peChanData->nActiveLines);
  } 
}
