/*
   QMF analysis structs and prototypes
 */
#ifndef __QMF_ENC_H
#define __QMF_ENC_H

#include "sbr_ram.h"
#include "aac_ram.h"
#include "ffr.h"

typedef struct
{
  Word16 no_channels;
  Word16 p_filter_length;

  const Word16 *p_filter;
   /*! 
    twiddle factors in cosMod() and sinMod() 
  */
  const Word16 *cos_twiddle;
  const Word16 *sin_twiddle;
  const Word16 *alt_sin_twiddle;
 
  
  /*!  
    static modulo buffer for qmf filter states  
  */
  Word16 *qmf_states_buffer;
  /*!
    qmf analysis output scale factor
  */
  Word16 qmfScale;

  /*!
    qmf states scale factor
  */
  Word16 qmfStatesScale;

  /*!
    parameters depending on p_filter 
  */
  Word16 no_poly;
  Word16 no_col;

  Word32 *FilterStates;          /*!< Pointer to buffer of filter states */
}
SBR_QMF_FILTER_BANK;

typedef SBR_QMF_FILTER_BANK *HANDLE_SBR_QMF_FILTER_BANK;


void sbrAnalysisFiltering (const Word16 *timeIn,
                           Word16 timeInStride,  /*!< stride of time domain input data */
                           Word16 **rAnalysis,
                           Word16 **iAnalysis,
                           HANDLE_SBR_QMF_FILTER_BANK qmfBank);


#ifndef MONO_ONLY
void SynthesisQmfFilteringEnc(Word32 **qmfReal,
                              Word32 **qmfImag,
                              Word16 scale,
                              Word16 *timeOut,
                              HANDLE_SBR_QMF_FILTER_BANK synQmf
                              );


Word16 CreateSynthesisQmfBank (HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf,
                               Word16 noCols);
#endif

Word16 createQmfBank (Word16 chan,
                      HANDLE_SBR_QMF_FILTER_BANK h_sbrQmf);

void deleteQmfBank (HANDLE_SBR_QMF_FILTER_BANK  h_sbrQmf);



void
getEnergyFromCplxQmfData (Word32 **energyValues,
                          Word16 **realValues,
                          Word16 **imagValues,
                          Word16 numberBands,
                          Word16 numberCols);

#endif
