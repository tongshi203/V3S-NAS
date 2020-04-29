/*
   Transient detector prototypes
 */
#ifndef __TRAN_DET_H
#define __TRAN_DET_H

#include "sbr_ram.h"
#include "ffr.h"
#include "ffr.h"
#include "intrinsics.h"

typedef struct
{
  Word32 *transients;
  Word32 *thresholds;
  
  Word32 tran_thr;             /* Master threshold for transient signals */
  Word32 abs_thr;              /* Minimum threshold for detecting changes */ 
  Word32 split_thr;            /* Threshold for splitting FIXFIX-frames into 2 env , times 1000*/
  Word16 tran_fc;              /* Number of lowband subbands to discard  */
  Word16 buffer_length;        /* 5*nCol/2 (2.5 granule) */
  Word16 no_cols;
  Word16 no_rows;
  Word16 mode;
 
  Word32 prevLowBandEnergy;
}
SBR_TRANSIENT_DETECTOR;


typedef SBR_TRANSIENT_DETECTOR *HANDLE_SBR_TRANSIENT_DETECTOR;


void transientDetect (Word32 **Energies,
                      Word16 *yBufferScale,
                      HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector,
                      Word16 *tran_vector,
                      Word16 timeStep,
                      Word16 frameMiddleBorder);

Word32 
CreateSbrTransientDetector (Word16 chan,
                            HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector,
                            Word32   sampleFreq,
                            Word32   totalBitrate,
                            Word32   codecBitrate,
                            Word32   tran_thr,
                            Word16   mode,
                            Word16   tran_fc,
                            Word16   no_cols,
                            Word16   no_rows
                            );

void deleteSbrTransientDetector (HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector);

void
frameSplitter(Word32 **Energies,
              Word16 *yBufferScale,
              HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector,
              UWord16 * freqBandTable,
              Word16 nSfb,
              Word16 timeStep,
              Word16 no_cols,
              Word16 *tran_vector,
              Word32 **EnergiesM);

#endif
