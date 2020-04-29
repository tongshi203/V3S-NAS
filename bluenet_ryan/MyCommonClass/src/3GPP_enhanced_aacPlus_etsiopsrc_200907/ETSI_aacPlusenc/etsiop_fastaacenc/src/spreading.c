/*
   Spreading of energy
 */
#include <stdlib.h>
#include <stdio.h>
#include "spreading.h"


void SpreadingMax(const Word16 pbCnt,
                  const Word16 *maskLowFactor,
                  const Word16 *maskHighFactor,
                  Word32       *pbSpreadedEnergy)
{
  Word16 i;

  /* slope to higher frequencies */
  for (i=1; i<pbCnt; i++) {
    pbSpreadedEnergy[i] = L_max(pbSpreadedEnergy[i],
                                fixmul_32x16b(pbSpreadedEnergy[sub(i,1)], maskHighFactor[i]));
  }
  /* slope to lower frequencies */
  for (i=sub(pbCnt,2); i>=0; i--) {
    pbSpreadedEnergy[i] = L_max(pbSpreadedEnergy[i],
                                fixmul_32x16b(pbSpreadedEnergy[add(i,1)], maskLowFactor[i]));
  }
}
