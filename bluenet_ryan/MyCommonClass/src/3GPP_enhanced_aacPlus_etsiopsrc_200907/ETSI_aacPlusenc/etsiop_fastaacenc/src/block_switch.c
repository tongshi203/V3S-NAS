/*
   Block switching
 */
#include <stdio.h>
#include "psy_const.h"
#include "block_switch.h"
#include "ffr.h"
#include "count.h"


#define ENERGY_SHIFT 8

/**************** internal function prototypes ***********/
static Word16
IIRFilter(const Word16 in, const Word32 coeff[], Word32 states[]);

static Word32
SrchMaxWithIndex(const Word32 *in, Word16 *index, Word16 n);


Word32
CalcWindowEnergy(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                 Word16 *timeSignal,
                 Word16 chIncrement,
                 Word16 windowLen);



/****************** Constants *****************************/


/*
  IIR high pass coeffs
*/
Word32 hiPassCoeff[BLOCK_SWITCHING_IIR_LEN] = {
  0xbec8b439, 0x609d4952
};

static const Word32 accWindowNrgFac = 0x26666666;                   /* factor for accumulating filtered window energies */
static const Word32 oneMinusAccWindowNrgFac = 0x5999999a;
static const Word32 invAttackRatioHighBr = 0x0ccccccd;              /* inverted lower ratio limit for attacks */
static const Word32 invAttackRatioLowBr =  0x072b020c;              
static const Word32 minAttackNrg = 0x00001e84;                      /* minimum energy for attacks */


/****************** Routines ****************************/
Word16 InitBlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                          const Word32 bitRate, const Word16 nChannels)
{
  /* select attackRatio */
  test(); test(); test(); test(); move32();
  if ((sub(nChannels,1)==0 && L_sub(bitRate, 24000) > 0) || 
      (sub(nChannels,1)>0 && L_sub(bitRate, ffr_Integer_Mult16x16(nChannels, 16000)) > 0)) {
    blockSwitchingControl->invAttackRatio = invAttackRatioHighBr;
  }
  else  {
    blockSwitchingControl->invAttackRatio = invAttackRatioLowBr;
  }

  return(TRUE);
}

static Word16 suggestedGroupingTable[TRANS_FAC][MAX_NO_OF_GROUPS] = {
  /* Attack in Window 0 */ {1,  3,  3,  1},
  /* Attack in Window 1 */ {1,  1,  3,  3},
  /* Attack in Window 2 */ {2,  1,  3,  2},
  /* Attack in Window 3 */ {3,  1,  3,  1},
  /* Attack in Window 4 */ {3,  1,  1,  3},
  /* Attack in Window 5 */ {3,  2,  1,  2},
  /* Attack in Window 6 */ {3,  3,  1,  1},
  /* Attack in Window 7 */ {3,  3,  1,  1}
};

Word16 BlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                      Word16 *timeSignal,
                      Word16 chIncrement)
{
  Word16 i, w;
  Word32 enM1, enMax;

  /* Reset grouping info */
  for (i=0; i<TRANS_FAC; i++) {
    blockSwitchingControl->groupLen[i] = 0;                                     move16();
  }


  /* Search for position and amplitude of attack in last frame (1 windows delay) */
  blockSwitchingControl->maxWindowNrg = SrchMaxWithIndex( &blockSwitchingControl->windowNrg[0][BLOCK_SWITCH_WINDOWS-1],
                                                          &blockSwitchingControl->attackIndex,
                                                          BLOCK_SWITCH_WINDOWS);

  blockSwitchingControl->attackIndex = blockSwitchingControl->lastAttackIndex;  move16();

  /* Set grouping info */
  blockSwitchingControl->noOfGroups = MAX_NO_OF_GROUPS;                         move16();

  for (i=0; i<MAX_NO_OF_GROUPS; i++) {
    blockSwitchingControl->groupLen[i] = suggestedGroupingTable[blockSwitchingControl->attackIndex][i]; move16();
  }



  /* Save current window energy as last window energy */
  for (w=0; w<BLOCK_SWITCH_WINDOWS; w++) {
    blockSwitchingControl->windowNrg[0][w] = blockSwitchingControl->windowNrg[1][w];            move32();
    blockSwitchingControl->windowNrgF[0][w] = blockSwitchingControl->windowNrgF[1][w];          move32();
  }


  /* Calculate unfiltered and filtered energies in subwindows and combine to segments */
  CalcWindowEnergy(blockSwitchingControl, timeSignal, chIncrement, BLOCK_SWITCH_WINDOW_LEN);

  /* reset attack */
  blockSwitchingControl->attack = FALSE;                                        move16();

  enMax = 0;                                                                    move32();
  enM1 = blockSwitchingControl->windowNrgF[0][BLOCK_SWITCH_WINDOWS-1];          move32();

  for (w=0; w<BLOCK_SWITCH_WINDOWS; w++) {
    Word32 enM1_Tmp, accWindowNrg_Tmp, windowNrgF_Tmp;
    Word16 enM1_Shf, accWindowNrg_Shf, windowNrgF_Shf;

    accWindowNrg_Shf = ffr_norm32(blockSwitchingControl->accWindowNrg);
    enM1_Shf = ffr_norm32(enM1);
    windowNrgF_Shf = ffr_norm32(blockSwitchingControl->windowNrgF[1][w]);

    accWindowNrg_Tmp = L_shl(blockSwitchingControl->accWindowNrg, accWindowNrg_Shf);
    enM1_Tmp = L_shl(enM1, enM1_Shf);
    windowNrgF_Tmp = L_shl(blockSwitchingControl->windowNrgF[1][w], windowNrgF_Shf);
      
    blockSwitchingControl->accWindowNrg = L_add(L_shr(fixmul(oneMinusAccWindowNrgFac, accWindowNrg_Tmp), accWindowNrg_Shf),
                                                L_shr(fixmul(accWindowNrgFac, enM1_Tmp), enM1_Shf));

    test();
    if (L_sub(L_shr(fixmul(windowNrgF_Tmp, blockSwitchingControl->invAttackRatio), windowNrgF_Shf),
              blockSwitchingControl->accWindowNrg) > 0) {
      blockSwitchingControl->attack = TRUE;                                     move16();
      blockSwitchingControl->lastAttackIndex = w;                               move16();
    }
    enM1 = blockSwitchingControl->windowNrgF[1][w];                             move32();
    enMax = L_max(enMax, enM1);
  }
  test();
  if (L_sub(enMax, minAttackNrg) < 0) {
    blockSwitchingControl->attack = FALSE;                                      move16();
  }

  /* Check if attack spreads over frame border */
  test(); test();
  if ((!blockSwitchingControl->attack) && (blockSwitchingControl->lastattack)) {
    test();
    if (sub(blockSwitchingControl->attackIndex, TRANS_FAC-1) == 0) {
      blockSwitchingControl->attack = TRUE;                                     move16();
    }

    blockSwitchingControl->lastattack = FALSE;                                  move16();
  }
  else {
    blockSwitchingControl->lastattack = blockSwitchingControl->attack;          move16();
  }

  blockSwitchingControl->windowSequence =  blockSwitchingControl->nextwindowSequence;   move16();

  test(); move16();
  if (blockSwitchingControl->attack) {
    blockSwitchingControl->nextwindowSequence = SHORT_WINDOW;
  }
  else {
    blockSwitchingControl->nextwindowSequence = LONG_WINDOW;
  }

  test();
  if (sub(blockSwitchingControl->nextwindowSequence, SHORT_WINDOW) == 0) {
    test();
    if (sub(blockSwitchingControl->windowSequence, LONG_WINDOW) == 0) {
      blockSwitchingControl->windowSequence = START_WINDOW;                     move16();
    }
    test();
    if (sub(blockSwitchingControl->windowSequence, STOP_WINDOW) == 0) {
      blockSwitchingControl->windowSequence = SHORT_WINDOW;                     move16();
      blockSwitchingControl->noOfGroups = 3;                                    move16();
      blockSwitchingControl->groupLen[0] = 3;                                   move16();
      blockSwitchingControl->groupLen[1] = 3;                                   move16();
      blockSwitchingControl->groupLen[2] = 2;                                   move16();
    }
  }

  test();
  if (sub(blockSwitchingControl->nextwindowSequence, LONG_WINDOW) == 0) {
    test();
    if (sub(blockSwitchingControl->windowSequence, SHORT_WINDOW) == 0) {
      blockSwitchingControl->nextwindowSequence = STOP_WINDOW;                  move16();
    }
  }

  return(TRUE);
}



static Word32 SrchMaxWithIndex(const Word32 in[], Word16 *index, Word16 n)
{
  Word32 max;
  Word16 i, idx;

  /* Search maximum value in array and return index and value */
  max = 0;                                                      move32();
  idx = 0;                                                      move16();

  for (i = 0; i < n; i++) {
    test();
    if (L_sub(in[i+1], max) > 0) {
      max = in[i+1];                                            move32();
      idx = i;                                                  move16();
    }
  }
  *index = idx;                                                 move16();

  return(max);
}




Word32 CalcWindowEnergy(BLOCK_SWITCHING_CONTROL *blockSwitchingControl,
                        Word16 *timeSignal,
                        Word16 chIncrement,
                        Word16 windowLen)
{
  Word16 w, i, wOffset, tidx, ch;
  Word32 accuUE, accuFE;
  Word16 tempUnfiltered;
  Word16 tempFiltered;

  wOffset = 0;                                                  move16();
  for (w=0; w < BLOCK_SWITCH_WINDOWS; w++) {

    accuUE = 0;                                                 move32();
    accuFE = 0;                                                 move32();

    tidx = wOffset;                                             move16();
    for(i=0; i<windowLen; i++) {
      tempUnfiltered = timeSignal[tidx];
      tidx = add(tidx, chIncrement);
      tempFiltered = IIRFilter(tempUnfiltered, hiPassCoeff, blockSwitchingControl->iirStates);
      accuUE = L_add(accuUE, L_shr(L_mult(tempUnfiltered, tempUnfiltered), ENERGY_SHIFT));
      accuFE = L_add(accuFE, L_shr(L_mult(tempFiltered, tempFiltered), ENERGY_SHIFT));
    }

    blockSwitchingControl->windowNrg[1][w] = accuUE;            move32();
    blockSwitchingControl->windowNrgF[1][w] = accuFE;           move32();

    for (ch=0; ch<chIncrement; ch++)
      wOffset = add(wOffset, windowLen);
  }

  return(TRUE);
}


static Word16 IIRFilter(const Word16 in, const Word32 coeff[], Word32 states[])
{
  Word32 accu1, accu2, accu3;
  Word32 out;

  accu1 = fixmul_32x16b(coeff[1], in);
  accu3 = L_sub( accu1, states[0] );
  accu2 = fixmul( coeff[0], states[1] );
  out = L_sub( accu3, accu2 );

  states[0] = accu1;            move32();
  states[1] = out;              move32();

  return etsiopround(out);
}


static Word16 synchronizedBlockTypeTable[4][4] = {
  /*                 LONG_WINDOW   START_WINDOW  SHORT_WINDOW  STOP_WINDOW */
  /* LONG_WINDOW  */{LONG_WINDOW,  START_WINDOW, SHORT_WINDOW, STOP_WINDOW},
  /* START_WINDOW */{START_WINDOW, START_WINDOW, SHORT_WINDOW, SHORT_WINDOW},
  /* SHORT_WINDOW */{SHORT_WINDOW, SHORT_WINDOW, SHORT_WINDOW, SHORT_WINDOW},
  /* STOP_WINDOW  */{STOP_WINDOW,  SHORT_WINDOW, SHORT_WINDOW, STOP_WINDOW}
};

Word16 SyncBlockSwitching(BLOCK_SWITCHING_CONTROL *blockSwitchingControlLeft,
                          BLOCK_SWITCHING_CONTROL *blockSwitchingControlRight,
                          const Word16 nChannels)
{
  Word16 i;
  Word16 patchType = LONG_WINDOW;               move16();

  test();
  if (sub(nChannels, 1) == 0) { /* Mono */
    if (sub(blockSwitchingControlLeft->windowSequence, SHORT_WINDOW) != 0) {
      blockSwitchingControlLeft->noOfGroups = 1;                        move16();
      blockSwitchingControlLeft->groupLen[0] = 1;                       move16();

      for (i=1; i<TRANS_FAC; i++) {
        blockSwitchingControlLeft->groupLen[i] = 0;                     move16();
      }
    }
  }
  else { /* Stereo common Window */
    patchType = synchronizedBlockTypeTable[patchType][blockSwitchingControlLeft->windowSequence];       move16();
    patchType = synchronizedBlockTypeTable[patchType][blockSwitchingControlRight->windowSequence];      move16();

    /* Set synchronized Blocktype */
    blockSwitchingControlLeft->windowSequence = patchType;              move16();
    blockSwitchingControlRight->windowSequence = patchType;             move16();

    /* Synchronize grouping info */
    test();
    if(sub(patchType, SHORT_WINDOW) != 0) { /* Long Blocks */
      /* Set grouping info */
      blockSwitchingControlLeft->noOfGroups = 1;                        move16();
      blockSwitchingControlRight->noOfGroups = 1;                       move16();
      blockSwitchingControlLeft->groupLen[0] = 1;                       move16();
      blockSwitchingControlRight->groupLen[0] = 1;                      move16();

      for (i=1; i<TRANS_FAC; i++) {
        blockSwitchingControlLeft->groupLen[i] = 0;                     move16();
        blockSwitchingControlRight->groupLen[i] = 0;                    move16();
      }
    }
    else {
      test();
      if (L_sub(blockSwitchingControlLeft->maxWindowNrg, blockSwitchingControlRight->maxWindowNrg) > 0) {
        /* Left Channel wins */
        blockSwitchingControlRight->noOfGroups = blockSwitchingControlLeft->noOfGroups;         move16();
        for (i=0; i<TRANS_FAC; i++) {
          blockSwitchingControlRight->groupLen[i] = blockSwitchingControlLeft->groupLen[i];     move16();
        }
      }
      else {
        /* Right Channel wins */
        blockSwitchingControlLeft->noOfGroups = blockSwitchingControlRight->noOfGroups;         move16();
        for (i=0; i<TRANS_FAC; i++) {
          blockSwitchingControlLeft->groupLen[i] = blockSwitchingControlRight->groupLen[i];     move16();
        }
      }
    }
  } /*endif Mono or Stereo */

  return(TRUE);
}
