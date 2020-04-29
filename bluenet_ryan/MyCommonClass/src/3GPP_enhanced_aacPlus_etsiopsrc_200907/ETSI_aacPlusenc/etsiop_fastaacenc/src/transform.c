/*
   MDCT Transform
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "psy_const.h"
#include "transform.h"
#include "fft.h"
#include "aac_ram.h"
#include "aac_rom.h"
#include "count.h"


#define LS_TRANS ((FRAME_LEN_LONG-FRAME_LEN_SHORT)/2) /* 448 */


/*!
 *
 *  \brief Calculates max possible scale factor for input vector of Word32s
 *         so that SUM(vec*vec < 1.0)
 *  \return Maximum scale factor
 *
 */
static Word16 getScalefactorSpec(Word32 *vector,
                                 Word32 len)
{
  Word16 Sfc;
  Word32 i, accu = 0;

  move32();

  for ( i = 0; i < len; i++ ) {
    Word32 tmp = vector[i];
    tmp = fixmul(tmp, tmp);
    accu = L_add(accu, tmp);
  }
  
  Sfc = (accu == 0) ? (INT_BITS-1) : shr(ffr_norm32(accu),1);
  return S_min(INT_BITS-1,Sfc);
}


/*!
 *
 *  \brief  Multiply input vector by \f$ 2^{scalefactor} \f$
 *
 *  \return 
 *
 */
static void scaleValues(Word32 *vector,    /*!< Vector */  
                        Word32 len,        /*!< Length */
                        Word16 scalefactor /*!< Scalefactor */ 
                        )
{
  Word32 i;

  for ( i = 0; i < len; i++ ) {
    vector[i] = L_shl(vector[i], scalefactor); move32();
  }
}





static void preModulationDCT(Word32 *x, Word32 m, const Word16 *sineWindow)
{
  Word32 i,k;
  Word32 re1, re2, im1, im2;
  Word16 wre, wim;

  for(i=0,k=0;i<m/4;i++){
    re1 = x[2*i];
    im2 = x[2*i+1];
    re2 = x[m-2-2*i];
    im1 = x[m-1-2*i];

    wim = sineWindow[i*2];
    wre = sineWindow[m-1-2*i];

    x[2*i]   = L_shr(L_add( fixmul_32x16b(im1, wim), fixmul_32x16b(re1, wre) ), 1);    move32();
    x[2*i+1] = L_shr(L_sub( fixmul_32x16b(im1, wre), fixmul_32x16b(re1, wim) ), 1);    move32();

    wim = sineWindow[m-2-2*i];
    wre = sineWindow[2*i+1];

    x[m-2-2*i] = L_shr(L_add( fixmul_32x16b(im2, wim), fixmul_32x16b(re2, wre) ), 1);  move32();
    x[m-1-2*i] = L_shr(L_sub( fixmul_32x16b(im2, wre), fixmul_32x16b(re2, wim) ), 1);  move32();
  } 
}


static void postModulationDCT(Word32 *x,Word32 m, const Word16 *trigData, Word32 step,Word32 trigDataSize)
{
  Word32 i;
  Word32 re1, re2, im1, im2;
  Word16 wre, wim;
  const Word16 *sinPtr = trigData;
  const Word16 *cosPtr = trigData+trigDataSize;

  wim = *sinPtr;
  wre = *cosPtr;

  for(i=0;i<m/4;i++){
    re1=x[2*i];
    im1=x[2*i+1];
    re2=x[m-2-2*i];
    im2=x[m-1-2*i];

    x[2*i]     = L_add( fixmul_32x16b(re1, wre), fixmul_32x16b(im1, wim) );  move32();
    x[m-1-2*i] = L_sub( fixmul_32x16b(re1, wim), fixmul_32x16b(im1, wre) );  move32();

    sinPtr+=step;
    cosPtr-=step;
    wim=*sinPtr;
    wre=*cosPtr;

    x[m-2-2*i] = L_add( fixmul_32x16b(re2, wim), fixmul_32x16b(im2, wre) );  move32();
    x[2*i+1]   = L_sub( fixmul_32x16b(re2, wre), fixmul_32x16b(im2, wim) );  move32();
  }

}


void mdct(Word32 *dctdata,
          const Word16 *trigData,
          const Word16 *sineWindow,
          Word32 n,
          Word32 ld_n)
{
  preModulationDCT(dctdata,n,sineWindow);
  
  cfft32(dctdata,n/2,-1,1);

  assert (LSI_LD_FFT_TWIDDLE_TABLE_SIZE >= ld_n-1);

  postModulationDCT(dctdata,n,trigData,1<<(LSI_LD_FFT_TWIDDLE_TABLE_SIZE-(ld_n-1)),LSI_FFT_TWIDDLE_TABLE_SIZE);
}




/*!
  \brief 
  the mdct delay buffer has a size of 1600,
  so the calculation of LONG,STOP must be  spilt in two 
  passes with 1024 samples and a mid shift,
  the SHORT transforms can be completed in the delay buffer,
  and afterwards a shift
*/

static void shiftMdctDelayBuffer(Word16 *mdctDelayBuffer, /*! start of mdct delay buffer */
                                 Word16 *timeSignal,      /*! pointer to new time signal samples, interleaved */
                                 Word16 chIncrement       /*! number of channels */
                                 )
{
  Word32 i;
  memmove(mdctDelayBuffer,mdctDelayBuffer+FRAME_LEN_LONG,(BLOCK_SWITCHING_OFFSET-FRAME_LEN_LONG)*sizeof(Word16));
  for(i=0;i<FRAME_LEN_LONG ;i++)
    mdctDelayBuffer[BLOCK_SWITCHING_OFFSET-FRAME_LEN_LONG+i] = timeSignal[i*chIncrement]; move16();

}




void Transform_Real(Word16 *mdctDelayBuffer,
                    Word16 *timeSignal,
                    Word16 chIncrement,
                    Word32 *realOut,
                    Word16 *mdctScale,
                    Word16 blockType
                    )
{
  Word16 i,w;
  Word16 timeSignalSample;
  Word32 ws1,ws2;
  Word32 *dctIn;

  Word16 delayBufferSf,timeSignalSf,minSf;
  Word16 headRoom=0;

  switch(blockType){


  case LONG_WINDOW:
    dctIn = realOut;

    /*
      we access BLOCK_SWITCHING_OFFSET (1600 ) delay buffer samples + 448 new timeSignal samples
    */
    delayBufferSf = ffr_getScalefactorOfShortVectorStride(mdctDelayBuffer,BLOCK_SWITCHING_OFFSET,1);
    timeSignalSf  = ffr_getScalefactorOfShortVectorStride(timeSignal,2*FRAME_LEN_LONG-BLOCK_SWITCHING_OFFSET,chIncrement);
    minSf = S_min(delayBufferSf,timeSignalSf);
    minSf = S_min(minSf,2+9);
    for(i=0;i<FRAME_LEN_LONG/2;i++){
      timeSignalSample = shl(mdctDelayBuffer[i], minSf); 
      ws1 = L_mult( timeSignalSample, LongWindowKBD[i] );
      timeSignalSample = shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1)], minSf);
      ws2 = L_mult( timeSignalSample, LongWindowKBD[FRAME_LEN_LONG-i-1] );
      dctIn[FRAME_LEN_LONG/2+i] = L_sub(L_shr(ws1,1), L_shr(ws2,1));   move32();
    }

    shiftMdctDelayBuffer(mdctDelayBuffer,timeSignal,chIncrement);

    for(i=0;i<FRAME_LEN_LONG/2;i++){    
      timeSignalSample = shl(mdctDelayBuffer[i], minSf);
      ws1 = L_mult( timeSignalSample, LongWindowKBD[FRAME_LEN_LONG-i-1] );
      timeSignalSample = shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1)], minSf);
      ws2 = L_mult( timeSignalSample, LongWindowKBD[i] );
      dctIn[FRAME_LEN_LONG/2-i-1] = L_negate(L_add(L_shr(ws1,1), L_shr(ws2,1)));   move32();
    }

    mdct(dctIn,fftTwiddleTable,LongWindowSine,(Word32)FRAME_LEN_LONG,10);
    headRoom = sub(getScalefactorSpec(dctIn,FRAME_LEN_LONG),1);
    minSf = sub(2+9, minSf);
    headRoom = S_min(headRoom,minSf);
    scaleValues(dctIn,FRAME_LEN_LONG,headRoom);
    *mdctScale=sub(minSf, headRoom); /* 2 shifts in pre/postmodulation + 9 shifts in cfft */

    break;

  case START_WINDOW:
    dctIn = realOut;

    /*
      we access BLOCK_SWITCHING_OFFSET (1600 ) delay buffer samples + no timeSignal samples
    */
    minSf = ffr_getScalefactorOfShortVectorStride(mdctDelayBuffer,BLOCK_SWITCHING_OFFSET,1);
    minSf = S_min(minSf,2+9);
      

    for(i=0;i<FRAME_LEN_LONG/2;i++){
      timeSignalSample = shl(mdctDelayBuffer[i], minSf);
      ws1 = L_mult( timeSignalSample, LongWindowKBD[i] );
      timeSignalSample = shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1)], minSf);
      ws2 = L_mult( timeSignalSample, LongWindowKBD[FRAME_LEN_LONG-i-1] );
      dctIn[FRAME_LEN_LONG/2+i] = L_sub(L_shr(ws1, 1), L_shr(ws2, 1));   move32();
    }
      
    shiftMdctDelayBuffer(mdctDelayBuffer,timeSignal,chIncrement);

    for(i=0;i<LS_TRANS;i++){
      timeSignalSample = shl(mdctDelayBuffer[i], minSf);
      dctIn[FRAME_LEN_LONG/2-i-1] = L_shr(L_deposit_h(negate(timeSignalSample)), 1); move32();
    }
    
    for(i=0;i<FRAME_LEN_SHORT/2;i++){
      timeSignalSample= shl(mdctDelayBuffer[i+LS_TRANS], minSf);
      ws1 = L_mult( timeSignalSample, ShortWindowSine[FRAME_LEN_SHORT-i-1] );
      timeSignalSample= shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1-LS_TRANS)], minSf);
      ws2 = L_mult( timeSignalSample, ShortWindowSine[i] );
      dctIn[FRAME_LEN_LONG/2-i-1-LS_TRANS] = L_negate(L_add(L_shr(ws1, 1), L_shr(ws2, 1)));   move32();
    }

    mdct(dctIn,fftTwiddleTable,LongWindowSine,FRAME_LEN_LONG,10);
    headRoom=getScalefactorSpec(dctIn,FRAME_LEN_LONG)-1;
    minSf = sub(2+9,minSf);
    headRoom = S_min(headRoom,minSf);
    scaleValues(dctIn,FRAME_LEN_LONG,headRoom);
    *mdctScale=sub(minSf, headRoom); /* 2 shifts in pre/postmodulation + 9 shifts in cfft */

    break;

  case STOP_WINDOW:
    dctIn = realOut;

    /*
      we access BLOCK_SWITCHING_OFFSET-LS_TRANS (1600-448 ) delay buffer samples + 448 new timeSignal samples
    */
    delayBufferSf = ffr_getScalefactorOfShortVectorStride(mdctDelayBuffer+LS_TRANS,BLOCK_SWITCHING_OFFSET-LS_TRANS,1);
    timeSignalSf  = ffr_getScalefactorOfShortVectorStride(timeSignal,2*FRAME_LEN_LONG-BLOCK_SWITCHING_OFFSET,chIncrement);
    minSf = S_min(delayBufferSf,timeSignalSf);    
    minSf = S_min(minSf,2+9);

    for(i=0;i<LS_TRANS;i++){
      timeSignalSample= shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1)], minSf);
      dctIn[FRAME_LEN_LONG/2+i] = L_shr(L_deposit_h(negate(timeSignalSample)), 1);   move32();
    }
    
    for(i=0;i<FRAME_LEN_SHORT/2;i++){
      timeSignalSample = shl(mdctDelayBuffer[(i+LS_TRANS)], minSf);
      ws1 = L_mult( timeSignalSample, ShortWindowSine[i] );
      timeSignalSample= shl(mdctDelayBuffer[(FRAME_LEN_LONG-LS_TRANS-i-1)], minSf);
      ws2 = L_mult( timeSignalSample, ShortWindowSine[FRAME_LEN_SHORT-i-1] );
      dctIn[FRAME_LEN_LONG/2+i+LS_TRANS] = L_sub(L_shr(ws1, 1), L_shr(ws2, 1));   move32();
    }

    shiftMdctDelayBuffer(mdctDelayBuffer,timeSignal,chIncrement);
 
    for(i=0;i<FRAME_LEN_LONG/2;i++){
      timeSignalSample= shl(mdctDelayBuffer[i], minSf);
      ws1 = L_mult( timeSignalSample, LongWindowKBD[FRAME_LEN_LONG-i-1] );
      timeSignalSample= shl(mdctDelayBuffer[(FRAME_LEN_LONG-i-1)], minSf);
      ws2 = L_mult( timeSignalSample, LongWindowKBD[i] );
      dctIn[FRAME_LEN_LONG/2-i-1] = L_negate(L_add(L_shr(ws1, 1), L_shr(ws2, 1)));   move32();
    }

    mdct(dctIn,fftTwiddleTable,LongWindowSine,FRAME_LEN_LONG,10);
    headRoom=getScalefactorSpec(dctIn,FRAME_LEN_LONG)-1;
    minSf = sub(2+9,minSf);
    headRoom = S_min(headRoom,minSf);
    scaleValues(dctIn,FRAME_LEN_LONG,headRoom);
    *mdctScale= sub(minSf, headRoom); /* 2 shifts in pre/postmodulation + 9 shifts in cfft */

    break;

  case SHORT_WINDOW:
    /*
      we access BLOCK_SWITCHING_OFFSET (1600 ) delay buffer samples + no new timeSignal samples
    */
    
    minSf = ffr_getScalefactorOfShortVectorStride(mdctDelayBuffer+TRANSFORM_OFFSET_SHORT,9*FRAME_LEN_SHORT,1);
    minSf = S_min(minSf,2+6);
      
      
    for(w=0;w<TRANS_FAC;w++){
      dctIn = realOut+w*FRAME_LEN_SHORT;

      for(i=0;i<FRAME_LEN_SHORT/2;i++){
        timeSignalSample= shl(mdctDelayBuffer[TRANSFORM_OFFSET_SHORT+w*FRAME_LEN_SHORT+i], minSf);
        ws1 = L_mult( timeSignalSample, ShortWindowSine[i] );
        timeSignalSample= shl(mdctDelayBuffer[TRANSFORM_OFFSET_SHORT+w*FRAME_LEN_SHORT+FRAME_LEN_SHORT-i-1], minSf);
        ws2 = L_mult( timeSignalSample, ShortWindowSine[FRAME_LEN_SHORT-i-1] );
        dctIn[FRAME_LEN_SHORT/2+i] = L_sub(L_shr(ws1, 1), L_shr(ws2, 1));   move32();
        
        timeSignalSample= shl(mdctDelayBuffer[TRANSFORM_OFFSET_SHORT+w*FRAME_LEN_SHORT+FRAME_LEN_SHORT+i], minSf);
        ws1 = L_mult( timeSignalSample, ShortWindowSine[FRAME_LEN_SHORT-i-1] );
        timeSignalSample= shl(mdctDelayBuffer[TRANSFORM_OFFSET_SHORT+w*FRAME_LEN_SHORT+FRAME_LEN_SHORT*2-i-1], minSf);
        ws2 = L_mult( timeSignalSample, ShortWindowSine[i] );
        dctIn[FRAME_LEN_SHORT/2-i-1] = L_negate(L_add(L_shr(ws1, 1), L_shr(ws2, 1)));   move32();
      }

      mdct(dctIn,fftTwiddleTable,ShortWindowSine,FRAME_LEN_SHORT,7);
    }
    headRoom=sub(getScalefactorSpec(realOut,FRAME_LEN_SHORT*TRANS_FAC),1);
    minSf = sub(2+6,minSf);
    headRoom = S_min(headRoom,minSf);
    scaleValues(realOut,FRAME_LEN_SHORT*TRANS_FAC,headRoom);
    *mdctScale = sub(minSf, headRoom);/* 2 shifts in pre/postmodulation + 6 shifts in cfft */

    shiftMdctDelayBuffer(mdctDelayBuffer,timeSignal,chIncrement);
    break;

  }

}

