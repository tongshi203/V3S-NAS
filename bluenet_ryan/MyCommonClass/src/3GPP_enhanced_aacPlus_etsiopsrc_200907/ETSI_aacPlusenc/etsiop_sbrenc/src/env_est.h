/*
   Envelope estimation
 */
#ifndef __ENV_EST_H
#define __ENV_EST_H

#include "sbr_def.h"
#include "sbr_ram.h"
#include "fram_gen.h"
#include "ffr.h"
#include "qmf_enc.h"

typedef struct
{
  Word16 pre_transient_info[2];
  Word16  *rBuffer[QMF_TIME_SLOTS];
  Word16  *iBuffer[QMF_TIME_SLOTS];
  Word32  *YBuffer[QMF_TIME_SLOTS*2];
  Word32  *tmpEnergiesM[NUMBER_TIME_SLOTS_2048]; /* used in frameSplitter(), use highest number */
  Word16  yBufferScale[2];
 
  Word16 envelopeCompensation[MAX_FREQ_COEFFS];
  
  Word16 YBufferReadOffset;
  Word16 YBufferWriteOffset;
  Word16 rBufferReadOffset;
  Word16 rBufferWriteOffset;

  
  Word16 no_cols;
  Word16 no_rows;
  Word16 start_index;

  Word16 time_slots;
  Word16 time_step;
}
SBR_EXTRACT_ENVELOPE; /* size Word16: 184 */
typedef SBR_EXTRACT_ENVELOPE *HANDLE_SBR_EXTRACT_ENVELOPE;


/************  Function Declarations ***************/

Word32
CreateExtractSbrEnvelope (Word16 chan,
                          HANDLE_SBR_EXTRACT_ENVELOPE hSbr,
                          Word16 no_cols,
                          Word16 no_rows,
                          Word16 start_index,
                          Word16 time_slots, 
                          Word16 time_step                          );

void deleteExtractSbrEnvelope (HANDLE_SBR_EXTRACT_ENVELOPE hSbrCut); 
                              


struct SBR_CONFIG_DATA;
struct SBR_HEADER_DATA;
struct SBR_BITSTREAM_DATA;
struct ENV_CHANNEL;
struct COMMON_DATA;
struct PS_ENC;


void
extractSbrEnvelope(const Word16 *timeInPtr,
                   Word16 timeInStride,
                   struct SBR_CONFIG_DATA *h_con,
                   struct SBR_HEADER_DATA  *sbrHeaderData,
                   struct SBR_BITSTREAM_DATA *sbrBitstreamData,
                   struct ENV_CHANNEL      *h_envChan[MAX_CHANNELS],
                   struct PS_ENC           *h_ps_e,
                   HANDLE_SBR_QMF_FILTER_BANK hSynthesisQmfBank,
                   Word16  *pCoreBuffer,
                   struct COMMON_DATA      *cmonData);

#endif
