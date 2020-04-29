/*
   Declaration of constant tables
 */
#ifndef __SBR_ROM_H
#define __SBR_ROM_H

#include "ffr.h"
#include "ps_enc.h"


/*
  QMF
*/

extern const Word16 sbr_qmf_64_640_enc[330];
extern const Word16 sbr_cos_twiddle_L32_enc[];
extern const Word16 sbr_sin_twiddle_L32_enc[];
extern const Word16 sbr_alt_sin_twiddle_L32_enc[];

extern const Word16 sbr_cos_twiddle_L64_enc[];
extern const Word16 sbr_sin_twiddle_L64_enc[];
extern const Word16 sbr_alt_sin_twiddle_L64_enc[];

extern const Word32 sbr_waveCodSfb[];
extern const Word32 sbr_waveTwiddleSin[];
extern const Word32 sbr_waveTwiddleCos[];


/*
  huffman tables
*/
extern const Word32           v_Huff_envelopeLevelC10T[121];
extern const UWord8 v_Huff_envelopeLevelL10T[121];
extern const Word32           v_Huff_envelopeLevelC10F[121];
extern const UWord8 v_Huff_envelopeLevelL10F[121];
extern const Word32           bookSbrEnvBalanceC10T[49];
extern const UWord8 bookSbrEnvBalanceL10T[49];
extern const Word32           bookSbrEnvBalanceC10F[49];
extern const UWord8 bookSbrEnvBalanceL10F[49];
extern const Word32           v_Huff_envelopeLevelC11T[63];
extern const UWord8 v_Huff_envelopeLevelL11T[63];
extern const Word32           v_Huff_envelopeLevelC11F[63];
extern const UWord8 v_Huff_envelopeLevelL11F[63];
extern const Word32           bookSbrEnvBalanceC11T[25];
extern const UWord8 bookSbrEnvBalanceL11T[25];
extern const Word32           bookSbrEnvBalanceC11F[25];
extern const UWord8 bookSbrEnvBalanceL11F[25];
extern const Word32           v_Huff_NoiseLevelC11T[63];
extern const UWord8 v_Huff_NoiseLevelL11T[63];
extern const Word32           bookSbrNoiseBalanceC11T[25];
extern const UWord8 bookSbrNoiseBalanceL11T[25];

#ifndef MONO_ONLY
extern const UWord8 hiResBandBorders[8+12+1];
extern const UWord8 groupBordersMix[28+1];
extern const UWord32  bins2groupMap[28+1];

extern const Word32 iidQuantLeft[7];
extern const Word32 iidQuantRight[7];
extern const Word32 iccQuant[8];

extern const Word32 aBookPsIidTimeCode[29];
extern const Word8 aBookPsIidTimeLength[29];
extern const Word32 aBookPsIidFreqCode[29];
extern const Word8 aBookPsIidFreqLength[29];

extern const Word16 aBookPsIccTimeCode[15];
extern const Word8 aBookPsIccTimeLength[15];
extern const Word16 aBookPsIccFreqCode[15];
extern const Word8 aBookPsIccFreqLength[15];
extern const Word32 p4_13[13];
extern const Word32 p8_13_enc[13];
extern const Word16 aHybridResolution[3];
#endif

#endif
