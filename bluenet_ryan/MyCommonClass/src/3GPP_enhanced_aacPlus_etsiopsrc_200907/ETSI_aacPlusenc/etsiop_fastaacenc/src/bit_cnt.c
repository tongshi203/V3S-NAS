/*
   Huffman Bitcounter & coder
 */
#include <stdlib.h>
#include "bit_cnt.h"
#include "aac_rom.h"
#include "count.h"



#define HI_LTAB(a) (a>>8)
#define LO_LTAB(a) (a & 0xff)

#define EXPAND(a)  (L_shl(((Word32)(a&0xff00)),8)|(Word32)(a&0xff)) 


/*****************************************************************************


    functionname: count1_2_3_4_5_6_7_8_9_10_11
    description:  counts tables 1-11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 1-11

*****************************************************************************/

static void count1_2_3_4_5_6_7_8_9_10_11(const Word16 *values,
                                         const Word16  width,
                                         Word16       *bitCount)
{
  Word16 i;
  Word32 bc1_2,bc3_4,bc5_6,bc7_8,bc9_10;
  Word16 bc11,sc;
  Word16 t0,t1,t2,t3;
  bc1_2=0;                              move32();
  bc3_4=0;                              move32();
  bc5_6=0;                              move32();
  bc7_8=0;                              move32();
  bc9_10=0;                             move32();
  bc11=0;                               move16();
  sc=0;                                 move16();

  for(i=0;i<width;i+=4){
    
    t0= values[i+0];                    move16();
    t1= values[i+1];                    move16();
    t2= values[i+2];                    move16();
    t3= values[i+3];                    move16();
  
    /* 1,2 */

    bc1_2 = L_add(bc1_2, EXPAND(huff_ltab1_2[t0+1][t1+1][t2+1][t3+1]));         logic16(); logic16(); logic32();

    /* 5,6 */
    bc5_6 = L_add(bc5_6, EXPAND(huff_ltab5_6[t0+4][t1+4]));                     logic16(); logic16(); logic32();
    bc5_6 = L_add(bc5_6, EXPAND(huff_ltab5_6[t2+4][t3+4]));                     logic16(); logic16(); logic32();

    t0=abs_s(t0);
    t1=abs_s(t1);
    t2=abs_s(t2);
    t3=abs_s(t3);

    
    bc3_4 = L_add(bc3_4, EXPAND(huff_ltab3_4[t0][t1][t2][t3]));                 logic16(); logic16(); logic32();
    
    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t0][t1]));                         logic16(); logic16(); logic32();
    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t2][t3]));                         logic16(); logic16(); logic32();
    
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t0][t1]));                      logic16(); logic16(); logic32();
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t2][t3]));                      logic16(); logic16(); logic32();
    
    bc11 = add(bc11, huff_ltab11[t0][t1]);
    bc11 = add(bc11, huff_ltab11[t2][t3]);
   
    test(); test(); test(); test();
    sc = add(sc, add(add((t0>0),(t1>0)), add((t2>0),(t3>0))));
  }
  
  bitCount[1]=extract_h(bc1_2);
  bitCount[2]=extract_l(bc1_2);
  bitCount[3]=add(extract_h(bc3_4),sc);
  bitCount[4]=add(extract_l(bc3_4),sc);
  bitCount[5]=extract_h(bc5_6);
  bitCount[6]=extract_l(bc5_6);
  bitCount[7]=add(extract_h(bc7_8),sc);
  bitCount[8]=add(extract_l(bc7_8),sc);
  bitCount[9]=add(extract_h(bc9_10),sc);
  bitCount[10]=add(extract_l(bc9_10),sc);
  bitCount[11]=add(bc11,sc);
}


/*****************************************************************************

    functionname: count3_4_5_6_7_8_9_10_11
    description:  counts tables 3-11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 3-11

*****************************************************************************/

static void count3_4_5_6_7_8_9_10_11(const Word16 *values,
                                     const Word16  width,
                                     Word16       *bitCount)
{

  Word16 i;
  Word32 bc3_4,bc5_6,bc7_8,bc9_10;
  Word16 bc11,sc;
  Word16 t0,t1,t2,t3;
  
  bc3_4=0;                              move32();
  bc5_6=0;                              move32();
  bc7_8=0;                              move32();
  bc9_10=0;                             move32();
  bc11=0;                               move16();
  sc=0;                                 move16();

  for(i=0;i<width;i+=4){

    t0= values[i+0];                    move16();
    t1= values[i+1];                    move16();
    t2= values[i+2];                    move16();
    t3= values[i+3];                    move16();
    
    /*
      5,6
    */
    bc5_6 = L_add(bc5_6, EXPAND(huff_ltab5_6[t0+4][t1+4]));                     logic16(); logic16(); logic32();
    bc5_6 = L_add(bc5_6, EXPAND(huff_ltab5_6[t2+4][t3+4]));                     logic16(); logic16(); logic32();

    t0=abs_s(t0);
    t1=abs_s(t1);
    t2=abs_s(t2);
    t3=abs_s(t3);


    bc3_4 = L_add(bc3_4, EXPAND(huff_ltab3_4[t0][t1][t2][t3]));                 logic16(); logic16(); logic32();
                                                                                                                
    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t0][t1]));                         logic16(); logic16(); logic32();
    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t2][t3]));                         logic16(); logic16(); logic32();
    
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t0][t1]));                      logic16(); logic16(); logic32();
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t2][t3]));                      logic16(); logic16(); logic32();
                                                                                                                
    bc11 = add(bc11, huff_ltab11[t0][t1]);
    bc11 = add(bc11, huff_ltab11[t2][t3]);

    test(); test(); test(); test();
    sc = add(sc, add(add((t0>0),(t1>0)),add((t2>0),(t3>0))));
      
   
  }
  bitCount[1]=INVALID_BITCOUNT;                         move16();
  bitCount[2]=INVALID_BITCOUNT;                         move16();
  bitCount[3]=add(extract_h(bc3_4),sc);
  bitCount[4]=add(extract_l(bc3_4),sc);
  bitCount[5]=extract_h(bc5_6);
  bitCount[6]=extract_l(bc5_6);
  bitCount[7]=add(extract_h(bc7_8),sc);
  bitCount[8]=add(extract_l(bc7_8),sc);
  bitCount[9]=add(extract_h(bc9_10),sc);
  bitCount[10]=add(extract_l(bc9_10),sc);
  bitCount[11]=add(bc11,sc);
  
}



/*****************************************************************************

    functionname: count5_6_7_8_9_10_11
    description:  counts tables 5-11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 5-11

*****************************************************************************/


static void count5_6_7_8_9_10_11(const Word16 *values,
                                 const Word16  width,
                                 Word16       *bitCount)
{

  Word16 i;
  Word32 bc5_6,bc7_8,bc9_10;
  Word16 bc11,sc;
  Word16 t0,t1;
  bc5_6=0;                              move32();
  bc7_8=0;                              move32();
  bc9_10=0;                             move32();
  bc11=0;                               move16();
  sc=0;                                 move16();

  for(i=0;i<width;i+=2){

    t0 = values[i+0];                   move16();
    t1 = values[i+1];                   move16();

    bc5_6 = L_add(bc5_6, EXPAND(huff_ltab5_6[t0+4][t1+4]));             logic16(); logic16(); logic32();

    t0=abs_s(t0);
    t1=abs_s(t1);
     
    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t0][t1]));                 logic16(); logic16(); logic32();
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t0][t1]));              logic16(); logic16(); logic32();
    bc11 = add(bc11, huff_ltab11[t0][t1]);
    
    test(); test();
    sc = add(sc, add((t0>0),(t1>0)));
  }
  bitCount[1]=INVALID_BITCOUNT;                         move16();
  bitCount[2]=INVALID_BITCOUNT;                         move16();
  bitCount[3]=INVALID_BITCOUNT;                         move16();
  bitCount[4]=INVALID_BITCOUNT;                         move16();
  bitCount[5]=extract_h(bc5_6);
  bitCount[6]=extract_l(bc5_6);
  bitCount[7]=add(extract_h(bc7_8),sc);
  bitCount[8]=add(extract_l(bc7_8),sc);
  bitCount[9]=add(extract_h(bc9_10),sc);
  bitCount[10]=add(extract_l(bc9_10),sc);
  bitCount[11]=add(bc11,sc);
  
}


/*****************************************************************************

    functionname: count7_8_9_10_11
    description:  counts tables 7-11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 7-11

*****************************************************************************/

static void count7_8_9_10_11(const Word16 *values,
                             const Word16  width,
                             Word16       *bitCount)
{

  Word16 i;
  Word32 bc7_8,bc9_10;
  Word16 bc11,sc;
  Word16 t0,t1;
  
  bc7_8=0;                      move32();
  bc9_10=0;                     move32();
  bc11=0;                       move16();
  sc=0;                         move16();

  for(i=0;i<width;i+=2){

    t0=abs_s(values[i+0]);
    t1=abs_s(values[i+1]);

    bc7_8 = L_add(bc7_8, EXPAND(huff_ltab7_8[t0][t1]));                 logic16(); logic16(); logic32();
    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t0][t1]));              logic16(); logic16(); logic32();
    bc11 = add(bc11, huff_ltab11[t0][t1]);
   
    test(); test();
    sc = add(sc, add((t0>0),(t1>0)));
  }
  bitCount[1]=INVALID_BITCOUNT;                 move16();
  bitCount[2]=INVALID_BITCOUNT;                 move16();
  bitCount[3]=INVALID_BITCOUNT;                 move16();
  bitCount[4]=INVALID_BITCOUNT;                 move16();
  bitCount[5]=INVALID_BITCOUNT;                 move16();
  bitCount[6]=INVALID_BITCOUNT;                 move16();
  bitCount[7]=add(extract_h(bc7_8),sc);
  bitCount[8]=add(extract_l(bc7_8),sc);
  bitCount[9]=add(extract_h(bc9_10),sc);
  bitCount[10]=add(extract_l(bc9_10),sc);
  bitCount[11]=add(bc11,sc);
  
}

/*****************************************************************************

    functionname: count9_10_11
    description:  counts tables 9-11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 9-11

*****************************************************************************/



static void count9_10_11(const Word16 *values,
                         const Word16  width,
                         Word16       *bitCount)
{

  Word16 i;
  Word32 bc9_10;
  Word16 bc11,sc;
  Word16 t0,t1;

  bc9_10=0;                             move32();
  bc11=0;                               move16();
  sc=0;                                 move16();

  for(i=0;i<width;i+=2){

    t0=abs_s(values[i+0]);
    t1=abs_s(values[i+1]);
    

    bc9_10 = L_add(bc9_10, EXPAND(huff_ltab9_10[t0][t1]));      logic16(); logic16(); logic32();
    bc11 = add(bc11, huff_ltab11[t0][t1]);

    test(); test();
    sc = add(sc, add((t0>0),(t1>0)));
  }
  bitCount[1]=INVALID_BITCOUNT;         move16();
  bitCount[2]=INVALID_BITCOUNT;         move16();
  bitCount[3]=INVALID_BITCOUNT;         move16();
  bitCount[4]=INVALID_BITCOUNT;         move16();
  bitCount[5]=INVALID_BITCOUNT;         move16();
  bitCount[6]=INVALID_BITCOUNT;         move16();
  bitCount[7]=INVALID_BITCOUNT;         move16();
  bitCount[8]=INVALID_BITCOUNT;         move16();
  bitCount[9]=add(extract_h(bc9_10),sc);
  bitCount[10]=add(extract_l(bc9_10),sc);
  bitCount[11]=add(bc11,sc);
  
}
 
/*****************************************************************************

    functionname: count11
    description:  counts table 11 
    returns:      
    input:        quantized spectrum
    output:       bitCount for table 11

*****************************************************************************/
 
static void count11(const Word16 *values,
                    const Word16  width,
                    Word16        *bitCount)
{
  Word16 i;
  Word16 bc11,sc;
  Word16 t0,t1;

  bc11=0;                       move16();
  sc=0;                         move16();
  for(i=0;i<width;i+=2){
    t0=abs_s(values[i+0]);
    t1=abs_s(values[i+1]);
    bc11 = add(bc11, huff_ltab11[t0][t1]);

    test(); test();
    sc = add(sc, add((t0>0),(t1>0)));
  }

  bitCount[1]=INVALID_BITCOUNT;                 move16();
  bitCount[2]=INVALID_BITCOUNT;                 move16();
  bitCount[3]=INVALID_BITCOUNT;                 move16();
  bitCount[4]=INVALID_BITCOUNT;                 move16();
  bitCount[5]=INVALID_BITCOUNT;                 move16();
  bitCount[6]=INVALID_BITCOUNT;                 move16();
  bitCount[7]=INVALID_BITCOUNT;                 move16();
  bitCount[8]=INVALID_BITCOUNT;                 move16();
  bitCount[9]=INVALID_BITCOUNT;                 move16();
  bitCount[10]=INVALID_BITCOUNT;                move16();
  bitCount[11]=add(bc11,sc);
}

/*****************************************************************************

    functionname: countEsc
    description:  counts table 11 (with Esc) 
    returns:      
    input:        quantized spectrum
    output:       bitCount for tables 11 (with Esc)

*****************************************************************************/

static void countEsc(const Word16 *values,
                     const Word16  width,
                     Word16       *bitCount)
{

  Word16 i;
  Word16 bc11,ec,sc;
  Word16 t0,t1,t00,t01;

  bc11=0;                               move16();
  sc=0;                                 move16();
  ec=0;                                 move16();
  for(i=0;i<width;i+=2){
    t0=abs_s(values[i+0]);
    t1=abs_s(values[i+1]);
    
    test(); test();
    sc = add(sc, add((t0>0),(t1>0)));

    t00 = S_min(t0,16);
    t01 = S_min(t1,16);
    bc11 = add(bc11, huff_ltab11[t00][t01]);
    
    test();
    if(sub(t0,16) >= 0){
      ec = add(ec,5);
      while(sub(t0=shr(t0,1), 16) >= 0) {
        ec = add(ec,2);
      }
    }
    
    test();
    if(sub(t1,16) >= 0){
      ec = add(ec,5);
      while(sub(t1=shr(t1,1), 16) >= 0) {
        ec = add(ec,2);
      }
    }
  }
  bitCount[1]=INVALID_BITCOUNT;         move16();
  bitCount[2]=INVALID_BITCOUNT;         move16();
  bitCount[3]=INVALID_BITCOUNT;         move16();
  bitCount[4]=INVALID_BITCOUNT;         move16();
  bitCount[5]=INVALID_BITCOUNT;         move16();
  bitCount[6]=INVALID_BITCOUNT;         move16();
  bitCount[7]=INVALID_BITCOUNT;         move16();
  bitCount[8]=INVALID_BITCOUNT;         move16();
  bitCount[9]=INVALID_BITCOUNT;         move16();
  bitCount[10]=INVALID_BITCOUNT;        move16();
  bitCount[11]=add(add(bc11,sc),ec);
}


typedef void (*COUNT_FUNCTION)(const Word16 *values,
                               const Word16  width,
                               Word16       *bitCount);

static COUNT_FUNCTION countFuncTable[CODE_BOOK_ESC_LAV+1] =
  {

    count1_2_3_4_5_6_7_8_9_10_11,  /* 0  */
    count1_2_3_4_5_6_7_8_9_10_11,  /* 1  */
    count3_4_5_6_7_8_9_10_11,      /* 2  */
    count5_6_7_8_9_10_11,          /* 3  */
    count5_6_7_8_9_10_11,          /* 4  */
    count7_8_9_10_11,              /* 5  */
    count7_8_9_10_11,              /* 6  */
    count7_8_9_10_11,              /* 7  */
    count9_10_11,                  /* 8  */
    count9_10_11,                  /* 9  */
    count9_10_11,                  /* 10 */
    count9_10_11,                  /* 11 */
    count9_10_11,                  /* 12 */
    count11,                       /* 13 */
    count11,                       /* 14 */
    count11,                       /* 15 */
    countEsc                       /* 16 */
  };



Word16 bitCount(const Word16 *values,
                const Word16  width,
                Word16        maxVal,
                Word16       *bitCount)
{
  /*
    check if we can use codebook 0
  */
  test(); move16();
  if(maxVal == 0)
    bitCount[0] = 0;
  else
    bitCount[0] = INVALID_BITCOUNT;

  maxVal = S_min(maxVal, CODE_BOOK_ESC_LAV);
  countFuncTable[maxVal](values,width,bitCount);

  return(0);
}


Word16 codeValues(Word16 *values, Word16 width, Word16 codeBook, HANDLE_BIT_BUF hBitstream)
{

  Word16 i, t0, t1, t2, t3, t00, t01;
  Word16 codeWord, codeLength;
  Word16 sign, signLength;

  test();
  switch (codeBook) {
    case CODE_BOOK_ZERO_NO:
      break;

    case CODE_BOOK_1_NO:
      for(i=0; i<width; i+=4) {
        t0         = values[i+0];                                       move16();
        t1         = values[i+1];                                       move16();
        t2         = values[i+2];                                       move16();
        t3         = values[i+3];                                       move16();
        codeWord   = huff_ctab1[t0+1][t1+1][t2+1][t3+1];                move16();
        codeLength = HI_LTAB(huff_ltab1_2[t0+1][t1+1][t2+1][t3+1]);     move16();
        WriteBits(hBitstream, codeWord, codeLength);        
      }
      break;

    case CODE_BOOK_2_NO:
      for(i=0; i<width; i+=4) {
        t0         = values[i+0];                                       move16();
        t1         = values[i+1];                                       move16();
        t2         = values[i+2];                                       move16();
        t3         = values[i+3];                                       move16();
        codeWord   = huff_ctab2[t0+1][t1+1][t2+1][t3+1];                move16();
        codeLength = LO_LTAB(huff_ltab1_2[t0+1][t1+1][t2+1][t3+1]);     move16();
        WriteBits(hBitstream,codeWord,codeLength);
      }
      break;

    case CODE_BOOK_3_NO:
      for(i=0; i<width; i+=4) {
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();
        if(t0 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t0 < 0){
            sign|=1;                                                    logic16();
            t0=abs_s(t0);
          }
        }
        t1 = values[i+1];                                               move16();
        test();
        if(t1 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t1 < 0){
            sign|=1;                                                    logic16();
            t1=abs_s(t1);
          }
        }
        t2 = values[i+2];                                               move16();
        test();
        if(t2 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t2 < 0){
            sign|=1;                                                    logic16();
            t2=abs_s(t2);
          }
        }
        t3 = values[i+3];                                               move16();
        if(t3 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t3 < 0){
            sign|=1;                                                    logic16();
            t3=abs_s(t3);
          }
        }

        codeWord   = huff_ctab3[t0][t1][t2][t3];                        move16();
        codeLength = HI_LTAB(huff_ltab3_4[t0][t1][t2][t3]);             move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
      }
      break;

    case CODE_BOOK_4_NO:
      for(i=0; i<width; i+=4) {
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();
        if(t0 != 0){                                                             
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          if(t0 < 0){                                                            
            sign|=1;                                                    logic16();
            t0=abs_s(t0);                                                          
          }
        }                                                                        
        t1 = values[i+1];                                               move16();
        test();
        if(t1 != 0){                                                             
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t1 < 0){                                                            
            sign|=1;                                                    logic16();
            t1=abs_s(t1);                                                          
          }                                                                      
        }                                                                        
        t2 = values[i+2];                                               move16();
        test();
        if(t2 != 0){                                                    
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t2 < 0){                                                   
            sign|=1;                                                    logic16();
            t2=abs_s(t2);                                                 
          }                                                             
        }                                                               
        t3 = values[i+3];                                               move16();
        test();
        if(t3 != 0){                                                    
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t3 < 0){                                                   
            sign|=1;                                                    logic16();
            t3=abs_s(t3);                                                 
          }                                                             
        }                                                               
        codeWord   = huff_ctab4[t0][t1][t2][t3];                        move16();
        codeLength = LO_LTAB(huff_ltab3_4[t0][t1][t2][t3]);             move16();
        WriteBits(hBitstream,codeWord,codeLength);                      
        WriteBits(hBitstream,sign,signLength);                          
      }                                                                 
      break;                                                            
                                                                        
    case CODE_BOOK_5_NO:                                                
      for(i=0; i<width; i+=2) {                                         
        t0         = values[i+0];                                       move16(); 
        t1         = values[i+1];                                       move16();
        codeWord   = huff_ctab5[t0+4][t1+4];                            move16();
        codeLength = HI_LTAB(huff_ltab5_6[t0+4][t1+4]);                 move16();
        WriteBits(hBitstream,codeWord,codeLength);
      }
      break;

    case CODE_BOOK_6_NO:
      for(i=0; i<width; i+=2) {
        t0         = values[i+0];                                       move16();
        t1         = values[i+1];                                       move16();
        codeWord   = huff_ctab6[t0+4][t1+4];                            move16();
        codeLength = LO_LTAB(huff_ltab5_6[t0+4][t1+4]);                 move16();
        WriteBits(hBitstream,codeWord,codeLength);
      }
      break;

    case CODE_BOOK_7_NO:
      for(i=0; i<width; i+=2){
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();
        if(t0 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t0 < 0){
            sign|=1;                                                    logic16();
            t0=abs_s(t0);
          }
        }

        t1 = values[i+1];                                               move16();
        test();
        if(t1 != 0){
          signLength = add(signLength, 1);
          sign = shl(sign, 1); 
          test();
          if(t1 < 0){
            sign|=1;                                                    logic16();
            t1=abs_s(t1);
          }
        }
        codeWord   = huff_ctab7[t0][t1];                                move16();
        codeLength = HI_LTAB(huff_ltab7_8[t0][t1]);                     move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
      }
      break;

    case CODE_BOOK_8_NO:
      for(i=0; i<width; i+=2) {
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();                                                                  
        if(t0 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t0 < 0){                                                            
            sign|=1;                                                    logic16();
            t0=abs_s(t0);                                                        
          }                                                                      
        }                                                                        
                                                                                 
        t1 = values[i+1];                                               move16();
        test();                                                                  
        if(t1 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t1 < 0){                                                            
            sign|=1;                                                    logic16();
            t1=abs_s(t1);                                                        
          }                                                                      
        }                                                                        
        codeWord   = huff_ctab8[t0][t1];                                move16();
        codeLength = LO_LTAB(huff_ltab7_8[t0][t1]);                     move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
      }
      break;

    case CODE_BOOK_9_NO:
      for(i=0; i<width; i+=2) {
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();                                                                  
        if(t0 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t0 < 0){                                                            
            sign|=1;                                                    logic16();
            t0=abs_s(t0);                                                        
          }                                                                      
        }                                                                        
                                                                                 
        t1 = values[i+1];                                               move16();
        test();                                                                  
        if(t1 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t1 < 0){                                                            
            sign|=1;                                                    logic16();
            t1=abs_s(t1);                                                        
          }                                                                      
        }                                                                        
        codeWord   = huff_ctab9[t0][t1];                                move16();
        codeLength = HI_LTAB(huff_ltab9_10[t0][t1]);                    move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
      }
      break;

    case CODE_BOOK_10_NO:
      for(i=0; i<width; i+=2) {
        sign=0;                                                         move16();
        signLength=0;                                                   move16();
        t0 = values[i+0];                                               move16();
        test();                                                                  
        if(t0 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t0 < 0){                                                            
            sign|=1;                                                    logic16();
            t0=abs_s(t0);                                                        
          }                                                                      
        }                                                                        
                                                                                 
        t1 = values[i+1];                                               move16();
        test();                                                                  
        if(t1 != 0){                                                             
          signLength = add(signLength, 1);                                       
          sign = shl(sign, 1);                                                   
          test();                                                                
          if(t1 < 0){                                                            
            sign|=1;                                                    logic16();
            t1=abs_s(t1);                                                        
          }                                                                      
        }                                                                        
        codeWord   = huff_ctab10[t0][t1];                               move16();
        codeLength = LO_LTAB(huff_ltab9_10[t0][t1]);                    move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
      }
      break;

    case CODE_BOOK_ESC_NO:
      for(i=0; i<width; i+=2) {
        sign=0;                                                 move16();
        signLength=0;                                           move16();
        t0 = values[i+0];                                       move16();
        test();                                                          
        if(t0 != 0){                                                     
          signLength = add(signLength, 1);                               
          sign = shl(sign, 1);                                           
          test();                                                        
          if(t0 < 0){                                                    
            sign|=1;                                            logic16();
            t0=abs_s(t0);                                                
          }                                                              
        }                                                                
                                                                         
        t1 = values[i+1];                                       move16();
        test();                                                          
        if(t1 != 0){                                                     
          signLength = add(signLength, 1);                               
          sign = shl(sign, 1);                                           
          test();                                                        
          if(t1 < 0){                                                    
            sign|=1;                                            logic16();
            t1=abs_s(t1);                                                
          }                                                              
        }                                                                
        t00 = S_min(t0,16);
        t01 = S_min(t1,16);

        codeWord   = huff_ctab11[t00][t01];                     move16();
        codeLength = huff_ltab11[t00][t01];                     move16();
        WriteBits(hBitstream,codeWord,codeLength);
        WriteBits(hBitstream,sign,signLength);
        test();
        if(sub(t0, 16) >= 0){
          Word16 n, p;
          n=0;                                                  move16();
          p=t0;                                                 move16();
          while(sub(p=shr(p,1), 16) >= 0){
            test();
            WriteBits(hBitstream,1,1);
            n = add(n, 1);
          }
          WriteBits(hBitstream,0,1);
          n = add(n, 4);
          WriteBits(hBitstream,sub(t0, shl(1,n)),n);
        }
        test();
        if(sub(t1, 16) >= 0){
          Word16 n, p;
          n=0;                                                  move16();
          p=t1;                                                 move16();
          while(sub(p=shr(p,1), 16) >= 0){
            test();
            WriteBits(hBitstream,1,1);
            n = add(n, 1);
          }
          WriteBits(hBitstream,0,1);
          n = add(n, 4);
          WriteBits(hBitstream,sub(t1, shl(1,n)),n);
        }
      }
      break;

    default:
      break;
  }
  return(0);
}

Word16 bitCountScalefactorDelta(Word16 delta)
{
  return(huff_ltabscf[delta+CODE_BOOK_SCF_LAV]);
}

Word16 codeScalefactorDelta(Word16 delta, HANDLE_BIT_BUF hBitstream)
{
  Word32 codeWord; 
  Word16 codeLength;
  
  test();
  if(sub(abs_s(delta),CODE_BOOK_SCF_LAV) > 0)
    return(1);
  
  codeWord   = huff_ctabscf[add(delta,CODE_BOOK_SCF_LAV)];           move32();
  codeLength = huff_ltabscf[add(delta,CODE_BOOK_SCF_LAV)];           move16();
  WriteBits(hBitstream,codeWord,codeLength);
  return(0);
}
