/*
   Resampler using the FIR filter
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "downsample_FIR.h"
#include "basic_op.h"
#include "count.h"
#include "intrinsics.h"

/*!
  
\brief FIR filter with 49 coeffs with 0.1 db ripple in the passband and 80 db attenuation in the stop band. 
the filter coefficients are scaled down by a factor of 2 to avoid overflow in the intermediate result.

*/

static const Word16 FIRenc_band24 [49] ={
  (Word16)0xfffa, (Word16)0xfffb, (Word16)0x11, (Word16)0x33, (Word16)0x2b, (Word16)0xffe7, (Word16)0xffb5, (Word16)0xfff5,
  (Word16)0x6c, (Word16)0x5c, (Word16)0xffa0, (Word16)0xff39, (Word16)0x9, (Word16)0x123, (Word16)0xad, (Word16)0xfece,
  (Word16)0xfe45, (Word16)0xa2, (Word16)0x2f9, (Word16)0xf7, (Word16)0xfbd8, (Word16)0xfb59, (Word16)0x502, (Word16)0x139c,
  (Word16)0x1aaf, (Word16)0x139c, (Word16)0x502, (Word16)0xfb59, (Word16)0xfbd8, (Word16)0xf7, (Word16)0x2f9, (Word16)0xa2,
  (Word16)0xfe45, (Word16)0xfece, (Word16)0xad, (Word16)0x123, (Word16)0x9, (Word16)0xff39, (Word16)0xffa0, (Word16)0x5c,
  (Word16)0x6c, (Word16)0xfff5, (Word16)0xffb5, (Word16)0xffe7, (Word16)0x2b, (Word16)0x33, (Word16)0x11, (Word16)0xfffb,
  (Word16)0xfffa
};

/*!
  
\brief FIR filter with 127 coeffs with 0.1 db ripple in the passband and 74 db attenuation in the stop band. 
the filter coefficients are scaled down by a factor of 2 to avoid overflow in the intermediate result.

*/

static const Word16 FIRenc_band63 [127] ={
  4, 3, -1, -11, -20, -23, -15, 0, 
  14, 15, 2, -16, -23, -10, 14, 29, 
  22, -7, -34, -35, -4, 35, 50, 21, 
  -31, -63, -43, 18, 72, 69, 4, -75, 
  -97, -35, 67, 124, 76, -46, -145, -127, 
  9, 156, 184, 49, -151, -244, -130, 122, 
  305, 241, -59, -363, -392, -59, 412, 616, 
  282, -451, -1028, -810, 475, 2455, 4254, 4978, 
  4254, 2455, 475, -810, -1028, -451, 282, 616, 
  412, -59, -392, -363, -59, 241, 305, 122, 
  -130, -244, -151, 49, 184, 156, 9, -127, 
  -145, -46, 76, 124, 67, -35, -97, -75, 
  4, 69, 72, 18, -43, -63, -31, 21, 
  50, 35, -4, -35, -34, -7, 22, 29, 
  14, -10, -23, -16, 2, 15, 14, 0, 
  -15, -23, -20, -11, -1, 3, 4
};


/*!

\brief Initialisation of the FIR filter. It sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.

*/

Word32 InitResampler_firDown2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                              Word32 fIn,                   /*!< Input Sampling Rate */
                              Word32 fOut                   /*!< Output Sampling Rate */
                              )
     
{
  /*
    find applicable parameter set
  */
  
  ReSampler_fir->firFilter.coeffFIR =  FIRenc_band24;                                            move16();
  ReSampler_fir->firFilter.noOffCoeffs = 49;                                                     move32();

  ReSampler_fir->delay = 25;                                                                     move16();
  ReSampler_fir->fIn =   fIn;                                                                    move32();
  ReSampler_fir->fOut =  fOut;                                                                   move32();
   
  return ReSampler_fir->delay;
}



/*!

\brief Initialisation of the FIR filter. It sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.

*/

Word32 InitResampler_firDown3(RESAMPLER_FIR_3_2 *ReSampler_fir, /*!< pointer to downsampler instance */
                              Word32 fIn,                   /*!< Input Sampling Rate */
                              Word32 fOut                   /*!< Output Sampling Rate */
                              )

{
  /*
    find applicable parameter set
  */
  
  ReSampler_fir->firFilter.coeffFIR =  FIRenc_band63;                                            move16();
  ReSampler_fir->firFilter.noOffCoeffs = 127;                                                    move16();

  ReSampler_fir->delay = 63;                                                                     move16();
  ReSampler_fir->fIn =   fIn;                                                                    move32();
  ReSampler_fir->fOut =  fOut;                                                                   move32();
   
  return ReSampler_fir->delay;
}



/*!

\brief Initialisation of the FIR filter. It sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.

*/

Word32 InitResampler_firUp2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                            Word32 fIn,                   /*!< Input Sampling Rate */
                            Word32 fOut                   /*!< Output Sampling Rate */
                            )

{
  /*
    find applicable parameter set
  */
  
  ReSampler_fir->firFilter.coeffFIR =  FIRenc_band24;                                            move16();
  ReSampler_fir->firFilter.noOffCoeffs = 49;                                                     move16();

  ReSampler_fir->delay = 25;                                                                     move16();
  ReSampler_fir->fIn =   fIn;                                                                    move32();
  ReSampler_fir->fOut =  fOut;                                                                   move32();
   
  return ReSampler_fir->delay;
}



/*!

\brief Initialisation of the FIR filter. It sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.

*/

Word32 InitResampler_firDown32(RESAMPLER_FIR_2_1 *ReSampler_firUp2,   /*!< pointer to downsampler instance */
                               RESAMPLER_FIR_3_2 *ReSampler_firDown3, /*!< pointer to downsampler instance */
                               Word32 fIn,                        /*!< Input Sampling Rate */
                               Word32 fOut                        /*!< Output Sampling Rate */
                               )

{
  assert((2 * fIn) == (3 * fOut));

  InitResampler_firUp2(ReSampler_firUp2,fIn,2*fIn);

  InitResampler_firDown3(ReSampler_firDown3,2*fIn,fOut);

  return(ReSampler_firUp2->delay + ReSampler_firDown3->delay);
}



/*!

\brief AdvanceFIR filter. It initialises the delay line with zero and sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.FIR filter which operates on the output sampling rate.

*/

static Word16
AdvanceFIRFilterDown2(FIR_FILTER_2_1 *firFilter, /*!< pointer to fir filter instance */
                      Word16 *input,         /*!< input of filter                */
                      Word16 inStride        /*!< increment of input samples */
                      )
     
{
  Word32 y;
  Word32 i;

  firFilter->delayLine[firFilter->noOffCoeffs-1] = firFilter->delayLine[firFilter->noOffCoeffs-3]; move16();
  firFilter->delayLine[firFilter->noOffCoeffs-2] = firFilter->delayLine[firFilter->noOffCoeffs-4]; move16();

  y = L_mult(firFilter->coeffFIR[firFilter->noOffCoeffs-1], firFilter->delayLine[firFilter->noOffCoeffs-1]); 
  y = L_macNs(y, firFilter->coeffFIR[firFilter->noOffCoeffs-2], firFilter->delayLine[firFilter->noOffCoeffs-2]);
  
  for ( i = (firFilter->noOffCoeffs-3); i >= 2; i--){ 
    firFilter->delayLine[i] = firFilter->delayLine[i-2]; move16();
    y = L_macNs(y, firFilter->coeffFIR[i], firFilter->delayLine[i]); /* Here we MUST use a non-saturating addition because of possible overflows in the intermediate result.Here the filtercoefficients are already scaled by a factor of 2 and we get one extra bit for the intermediate result. */
  }
  
  firFilter->delayLine[1] = *input;       /* update the delay line with new sample */              move16();
  firFilter->delayLine[0] = *(input+inStride);   /* update the delay line with the next sample */  move16();
  y = L_macNs(y, firFilter->coeffFIR[1], firFilter->delayLine[1]);
  y = L_macNs(y, firFilter->coeffFIR[0], firFilter->delayLine[0]);
  
  return etsiopround(L_shl(y, 1)); /* Finally we shift the filter output to compensate for the scaling down of the filter coefficients. */
}



/*!

\brief AdvanceFIR filter. It initialises the delay line with zero and sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.FIR filter which operates on the output sampling rate.

*/

static Word16
AdvanceFIRFilterDown3(FIR_FILTER_3_2 *firFilter, /*!< pointer to fir filter instance */
                      Word16 *input,         /*!< input of filter                */
                      Word16 inStride        /*!< increment of input samples */
                      )
     
{
  Word32 y;
  Word32 i;

  firFilter->delayLine[firFilter->noOffCoeffs-1] = firFilter->delayLine[firFilter->noOffCoeffs-4]; move16();
  firFilter->delayLine[firFilter->noOffCoeffs-2] = firFilter->delayLine[firFilter->noOffCoeffs-5]; move16();
  firFilter->delayLine[firFilter->noOffCoeffs-3] = firFilter->delayLine[firFilter->noOffCoeffs-6]; move16();

  y = L_mult(firFilter->coeffFIR[firFilter->noOffCoeffs-1], firFilter->delayLine[firFilter->noOffCoeffs-1]); 
  y = L_macNs(y, firFilter->coeffFIR[firFilter->noOffCoeffs-2], firFilter->delayLine[firFilter->noOffCoeffs-2]);
  y = L_macNs(y, firFilter->coeffFIR[firFilter->noOffCoeffs-3], firFilter->delayLine[firFilter->noOffCoeffs-3]);
  
  for ( i = (firFilter->noOffCoeffs-4); i >= 3; i--){ 
    firFilter->delayLine[i] = firFilter->delayLine[i-3];                                           move16();
    y = L_macNs(y, firFilter->coeffFIR[i], firFilter->delayLine[i]); /* Here we MUST use a non-saturating addition because of possible overflows in the intermediate result.Here the filtercoefficients are already scaled by a factor of 2 and we get one extra bit for the intermediate result. */
  }
  
  firFilter->delayLine[2] = *input;              /* update the delay line with new sample */       move16();
  firFilter->delayLine[1] = *(input+inStride);   /* update the delay line with the next sample */  move16();
  firFilter->delayLine[0] = *(input+2*inStride); /* update the delay line with the next sample */  move16();
  y = L_macNs(y, firFilter->coeffFIR[2], firFilter->delayLine[2]);
  y = L_macNs(y, firFilter->coeffFIR[1], firFilter->delayLine[1]);
  y = L_macNs(y, firFilter->coeffFIR[0], firFilter->delayLine[0]);
  
  return etsiopround(L_shl(y, 1)); /* Finally we shift the filter output to compensate for the scaling down of the filter coefficients. */
}



/*!

\brief AdvanceFIR filter. It initialises the delay line with zero and sets the FIR filter's no of coeffs etc.
Can be used to select between different filters.FIR filter which operates on the output sampling rate.

*/

static void
AdvanceFIRFilterUp2(FIR_FILTER_2_1 *firFilter, /*!< pointer to fir filter instance */
                    Word16 *input,         /*!< input of filter                */
                    Word16 *output)        /*!< output of filter               */
     
{
  Word32 y;
  Word32 i;

  firFilter->delayLine[firFilter->noOffCoeffs-1] = firFilter->delayLine[firFilter->noOffCoeffs-2]; move16();

  y = L_mult(firFilter->coeffFIR[firFilter->noOffCoeffs-1], firFilter->delayLine[firFilter->noOffCoeffs-1]); 
  
  for ( i = (firFilter->noOffCoeffs-2); i >= 1; i--){ 
    firFilter->delayLine[i] = firFilter->delayLine[i-1]; move16();
    y = L_macNs(y, firFilter->coeffFIR[i], firFilter->delayLine[i]);
  }
  
  firFilter->delayLine[0] = *input;       /* update the delay line with new sample */              move16();
  y = L_macNs(y, firFilter->coeffFIR[0], firFilter->delayLine[0]);
  
  output[0] = etsiopround(L_shl(y, 2));                                                            move16();

  /* insert 0 */
  firFilter->delayLine[firFilter->noOffCoeffs-1] = firFilter->delayLine[firFilter->noOffCoeffs-2]; move16();

  y = L_mult(firFilter->coeffFIR[firFilter->noOffCoeffs-1], firFilter->delayLine[firFilter->noOffCoeffs-1]); 
  
  for ( i = (firFilter->noOffCoeffs-2); i >= 1; i--){ 
    firFilter->delayLine[i] = firFilter->delayLine[i-1]; move16();
    y = L_macNs(y, firFilter->coeffFIR[i], firFilter->delayLine[i]);
  }
  
  firFilter->delayLine[0] = 0;       /* update the delay line with new sample */                   move16();
  y = L_macNs(y, firFilter->coeffFIR[0], firFilter->delayLine[0]);
  
  output[1] = etsiopround(L_shl(y, 2));                                                            move16();
}



/*!
  \brief   Downsample numInSamples of type Word16
  Returns number of output samples in numOutSamples
           
  \return  success of operation
*/

Word32 Resample_firDown2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                         Word16 *inSamples,            /*!< pointer to input samples */
                         Word16 numInSamples,          /*!< number  of input samples  */
                         Word16 inStride,              /*!< increment of input samples */
                         Word16 *outSamples,           /*!< pointer to output samples */ 
                         Word16 *numOutSamples,        /*!< pointer to number of output samples */
                         Word16 outStride              /*!< increment of output samples */
                         )
{
  Word32 i;
  *numOutSamples=0;
  
  for(i=0;i<numInSamples;i+=2){
    Word16 firOut;
    
    firOut=AdvanceFIRFilterDown2(&(ReSampler_fir->firFilter), inSamples+i*inStride, inStride);
    
    /*
      Output all the samples out from the FIR filter
    */
    outSamples[(*numOutSamples)*outStride] = firOut; move16();
    (*numOutSamples)++;
  }
  
  return 0;
}


/*!
  \brief   Upsample numInSamples of type Word16
  Returns number of output samples in numOutSamples
           
  \return  success of operation
*/

Word32 Resample_firUp2(RESAMPLER_FIR_2_1 *ReSampler_fir, /*!< pointer to downsampler instance */
                       Word16 *inSamples,            /*!< pointer to input samples */
                       Word16 numInSamples,          /*!< number  of input samples  */
                       Word16 inStride,              /*!< increment of input samples */
                       Word16 *outSamples,           /*!< pointer to output samples */ 
                       Word16 *numOutSamples,        /*!< pointer to number of output samples */
                       Word16 outStride              /*!< increment of output samples */
                       )
{
  Word32 i;
  *numOutSamples=0;
  
  for(i=0;i<numInSamples;i++){
    Word16 firOut[2];
    
    AdvanceFIRFilterUp2(&(ReSampler_fir->firFilter), inSamples+i*inStride, firOut);
    
    /*
      Output all the samples out from the FIR filter
    */
    outSamples[(*numOutSamples+0)*outStride] = firOut[0]; move16();
    outSamples[(*numOutSamples+1)*outStride] = firOut[1]; move16();
    (*numOutSamples)+=2;
  }
  
  return 0;
}

/*!
  \brief   Downsample numInSamples of type Word16
  Returns number of output samples in numOutSamples
           
  \return  success of operation
*/

Word32 Resample_firDown32(RESAMPLER_FIR_2_1 *ReSampler_firUp2,   /*!< pointer to downsampler instance */
                          RESAMPLER_FIR_3_2 *ReSampler_firDown3, /*!< pointer to downsampler instance */
                          Word16 *inSamples,                 /*!< pointer to input samples */
                          Word16 numInSamples,               /*!< number  of input samples  */
                          Word16 inStride,                   /*!< increment of input samples */
                          Word16 *outSamples,                /*!< pointer to output samples */ 
                          Word16 *numOutSamples,             /*!< pointer to number of output samples */
                          Word16 outStride                   /*!< increment of output samples */
                          )
{
  Word32 i;
  *numOutSamples=0;
  
  for(i=0;i<numInSamples;i+=3){
    Word16 firOut[6];
    Word16 output;

    /* upsampling */
    AdvanceFIRFilterUp2(&(ReSampler_firUp2->firFilter), inSamples+(i+0)*inStride, &firOut[0]);    
    AdvanceFIRFilterUp2(&(ReSampler_firUp2->firFilter), inSamples+(i+1)*inStride, &firOut[2]);    
    AdvanceFIRFilterUp2(&(ReSampler_firUp2->firFilter), inSamples+(i+2)*inStride, &firOut[4]);    

    /* downsampling */
    output=AdvanceFIRFilterDown3(&(ReSampler_firDown3->firFilter), &firOut[0], 1);
    outSamples[(*numOutSamples)*outStride] = output; move16();
    (*numOutSamples)++;
    output=AdvanceFIRFilterDown3(&(ReSampler_firDown3->firFilter), &firOut[3], 1);
    outSamples[(*numOutSamples)*outStride] = output; move16();
    (*numOutSamples)++;
  }
      
  return 0;
}
