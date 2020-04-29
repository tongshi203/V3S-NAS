/*
   Pre echo control
 */
#include "pre_echo_control.h"
#include "count.h"


void InitPreEchoControl(Word32 *pbThresholdNm1,
                        Word16  numPb,
                        Word32 *pbThresholdQuiet)
{
  Word16 pb;

  for(pb=0; pb<numPb; pb++) {
    pbThresholdNm1[pb] = pbThresholdQuiet[pb];                                  move32();
  }
}


void PreEchoControl(Word32 *pbThresholdNm1,
                    Word16  numPb,
                    Word32  maxAllowedIncreaseFactor,
                    Word16  minRemainingThresholdFactor,
                    Word32 *pbThreshold,
                    Word16  mdctScale,
                    Word16  mdctScalenm1)
{
  Word32 i;
  Word32 tmpThreshold1, tmpThreshold2;
  Word16 scaling;

  /* maxAllowedIncreaseFactor is hard coded to 2 */
  (void)maxAllowedIncreaseFactor;

  scaling = shl(sub(mdctScale, mdctScalenm1), 1);
  test();
  if ( scaling > 0 ) {
    for(i = 0; i < numPb; i++) {
      tmpThreshold1 = L_shr(pbThresholdNm1[i],scaling-1);
      tmpThreshold2 = fixmul_32x16b(pbThreshold[i], minRemainingThresholdFactor);

      /* copy thresholds to internal memory */
      pbThresholdNm1[i] = pbThreshold[i];                                       move32();

      test();
      if(L_sub(pbThreshold[i], tmpThreshold1) > 0) {
        pbThreshold[i] = tmpThreshold1;                                         move32();
      }
      test();
      if(L_sub(tmpThreshold2, pbThreshold[i]) > 0) {
        pbThreshold[i] = tmpThreshold2;                                         move32();
      }

    }
  }
  else {
    scaling = negate(scaling);
    for(i = 0; i < numPb; i++) {

      tmpThreshold1 = L_shl(pbThresholdNm1[i], 1);
      tmpThreshold2 = fixmul_32x16b(pbThreshold[i], minRemainingThresholdFactor);

      /* copy thresholds to internal memory */
      pbThresholdNm1[i] = pbThreshold[i];                                       move32();

      test();
      if(L_sub(L_shr(pbThreshold[i],scaling), tmpThreshold1) > 0) {
        pbThreshold[i] = L_shl(tmpThreshold1, scaling);
      }
      test();
      if(L_sub(tmpThreshold2, pbThreshold[i]) > 0) {
        pbThreshold[i] = tmpThreshold2;                                         move32();
      }

    }
  }

}

