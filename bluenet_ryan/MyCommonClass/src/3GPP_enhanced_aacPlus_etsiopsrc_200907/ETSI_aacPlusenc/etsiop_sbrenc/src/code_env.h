/*
   DPCM Envelope coding
 */

#ifndef __CODE_ENV_H
#define __CODE_ENV_H

#include "sbr_main.h"
#include "sbr_def.h"
#include "fram_gen.h"

typedef struct
{
  Word16 offset;
  Word16 upDate;
  Word16 nSfb[2];
  Word16 sfb_nrg_prev[MAX_FREQ_COEFFS];
  Word16 deltaTAcrossFrames;

  Word16 codeBookScfLavTime;
  Word16 codeBookScfLavFreq;

  Word16 codeBookScfLavLevelTime;
  Word16 codeBookScfLavLevelFreq;
  Word16 codeBookScfLavBalanceTime;
  Word16 codeBookScfLavBalanceFreq;

  Word16 start_bits;
  Word16 start_bits_balance;
  

  const UWord8 *hufftableTimeL;
  const UWord8 *hufftableFreqL;

  const UWord8 *hufftableLevelTimeL;
  const UWord8 *hufftableBalanceTimeL;
  const UWord8 *hufftableLevelFreqL;
  const UWord8 *hufftableBalanceFreqL;
}
SBR_CODE_ENVELOPE;
typedef SBR_CODE_ENVELOPE *HANDLE_SBR_CODE_ENVELOPE;



void
codeEnvelope (Word16 *sfb_nrg,
              const FREQ_RES *freq_res,
              SBR_CODE_ENVELOPE * h_sbrCodeEnvelope,
              Word16 *directionVec, Word16 coupling, Word16 nEnvelopes, Word16 channel,
              Word16 headerActive);

Word32
CreateSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope,
                       Word16 *nSfb,
                       Word16 deltaTAcrossFrames);

void deleteSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope);

struct SBR_ENV_DATA;

Word32
InitSbrHuffmanTables (struct SBR_ENV_DATA*      sbrEnvData,
                      HANDLE_SBR_CODE_ENVELOPE  henv,
                      HANDLE_SBR_CODE_ENVELOPE  hnoise,
                      AMP_RES                   amp_res);
                      
#endif
