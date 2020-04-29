/*
   Resampler using the FIR filter
 */

#ifndef __DOWNSAMPLE_FIR_H
#define __DOWNSAMPLE_FIR_H

#include "basic_op.h"

#define BUFFER_SIZE_2_1  64 
#define BUFFER_SIZE_3_2 128 


typedef struct
{
  const Word16 *coeffFIR;             /*! pointer to filter coeffs */
  Word16 noOffCoeffs;                 /*! number of filter coeffs */
  Word16 delayLine[BUFFER_SIZE_2_1];  /*! ringbuffer 1 input delay line */
} FIR_FILTER_2_1;

typedef struct
{
  const Word16 *coeffFIR;             /*! pointer to filter coeffs */
  Word16 noOffCoeffs;                 /*! number of filter coeffs */
  Word16 delayLine[BUFFER_SIZE_3_2];  /*! ringbuffer 1 input delay line */
} FIR_FILTER_3_2;


typedef struct
{
  FIR_FILTER_2_1 firFilter;       /*! fir filter instance */
  Word32 fIn;                     /*! input fs            */
  Word32 fOut ;                   /*! output fs           */
  Word32 delay;                   /*! delay input vs. output in samples */
} RESAMPLER_FIR_2_1;

typedef struct
{
  FIR_FILTER_3_2 firFilter;       /*! fir filter instance */
  Word32 fIn;                     /*! input fs            */
  Word32 fOut ;                   /*! output fs           */
  Word32 delay;                   /*! delay input vs. output in samples */
} RESAMPLER_FIR_3_2;


Word32 InitResampler_firDown2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                              Word32 fIn,                       /*!< Input Sampling frequency */
                              Word32 fOut);                     /*!< Output Sampling frequency */

Word32 InitResampler_firDown3(RESAMPLER_FIR_3_2 *ReSampler_fir, /*!< pointer to downsampler instance */
                              Word32 fIn,                       /*!< Input Sampling frequency */
                              Word32 fOut);                     /*!< Output Sampling frequency */

Word32 InitResampler_firUp2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                            Word32 fIn,                       /*!< Input Sampling frequency */
                            Word32 fOut);                     /*!< Output Sampling frequency */

Word32 InitResampler_firDown32(RESAMPLER_FIR_2_1 *ReSampler_firUp2,   /*!< pointer to downsampler instance */
                               RESAMPLER_FIR_3_2 *ReSampler_firDown3, /*!< pointer to downsampler instance */
                               Word32 fIn,                        /*!< Input Sampling frequency */
                               Word32 fOut);                      /*!< Output Sampling frequency */

Word32 Resample_firDown2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                         Word16 *inSamples,            /*!< pointer to input samples */
                         Word16 numInSamples,          /*!< number  of input samples  */
                         Word16 inStride,              /*!< increment of input samples */
                         Word16 *outSamples,           /*!< pointer to output samples */ 
                         Word16 *numOutSamples,        /*!< pointer to number of output samples */
                         Word16 outStride              /*!< increment of output samples */
                         );            

Word32 Resample_firUp2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                       Word16 *inSamples,            /*!< pointer to input samples */
                       Word16 numInSamples,          /*!< number  of input samples  */
                       Word16 inStride,              /*!< increment of input samples */
                       Word16 *outSamples,           /*!< pointer to output samples */ 
                       Word16 *numOutSamples,        /*!< pointer tp number of output samples */
                       Word16 outStride              /*!< increment of output samples */
                       );

Word32 Resample_firDown32(RESAMPLER_FIR_2_1 *ReSampler_firUp2,   /*!< pointer to downsampler instance */
                          RESAMPLER_FIR_3_2 *ReSampler_firDown3, /*!< pointer to downsampler instance */
                          Word16 *inSamples,                 /*!< pointer to input samples */
                          Word16 numInSamples,               /*!< number  of input samples  */
                          Word16 inStride,                   /*!< increment of input samples */
                          Word16 *outSamples,                /*!< pointer to output samples */ 
                          Word16 *numOutSamples,             /*!< pointer tp number of output samples */
                          Word16 outStride                   /*!< increment of output samples */
                          );

#endif
