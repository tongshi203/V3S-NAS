/*
   frequency scale prototypes
 */
#ifndef __FREQ_SCA2_H
#define __FREQ_SCA2_H
#include "sbr_misc.h"
#include "sbr_def.h"

#define MAX_OCTAVE        29
#define MAX_SECOND_REGION 50


Word32       
UpdateFreqScale(UWord16 *v_k_master, Word16 *h_num_bands,
                const Word16 k0, const Word16 k2,
                const Word16 freq_scale,
                const Word16 alter_scale);

Word16
UpdateHiRes(UWord16 *h_hires, 
            Word16 *num_hires,
            UWord16 *v_k_master, 
            Word16 num_master , 
            Word16 *xover_band, 
            SR_MODE drOrSr,
            Word16 noQMFChannels);

void  UpdateLoRes(UWord16 * v_lores,
                  Word16 *num_lores,
                  UWord16 * v_hires,
                  Word16 num_hires);

Word16       
FindStartAndStopBand(const Word32 samplingFreq,
                     const Word16 noChannels,
                     const Word16 startFreq,
                     const Word16 stop_freq,
                     const SR_MODE sampleRateMode,
                     Word16 *k0,
                     Word16 *k2);

Word16 getSbrStartFreqRAW (Word16 startFreq, Word16 QMFbands, Word32 fs );
Word16 getSbrStopFreqRAW  (Word16 stopFreq, Word16 QMFbands, Word32 fs);
#endif
