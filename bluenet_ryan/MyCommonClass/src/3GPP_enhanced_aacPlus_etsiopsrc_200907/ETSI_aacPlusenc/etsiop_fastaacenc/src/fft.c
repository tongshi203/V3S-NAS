/*
   Complex FFT Analysis/Synthesis
 */

#include <assert.h>
#include "intrinsics.h"
#include "fft.h"
#include "count.h"

/*
  table sin k*pi/1024
*/

#define LD_TRIG_DATA 9
#define TRIG_DATA_SIZE (1<<LD_TRIG_DATA)


void inv_dit_fft_4pt(Word32 *x,               /*!< Pointer to input vector */ 
                     Word16 scale              /*!< 0: scale off; 1: scale on */
                     )
{
  Word16 i, scale2;
  Word32 a0, a1, a2, a3, a00, a10, a20, a30;
  
  test();
  if (scale != 0) {
    scale2 = 2;                                                            move16();
  }
  else {
    scale2 = 0;                                                            move16();
  }
  
  i = 0;                                                                   move16();
  a00 = L_add(x[i], x[i + 4]);  /* Re A + Re B */
  a10 = L_add(x[i + 2], x[i + 6]);  /* Re C + Re D */
  
  a20 = L_add(x[i + 1], x[i + 5]);  /* Im A + Im B */
  a30 = L_add(x[i + 3], x[i + 7]);  /* Im C + Im D */
  
  a0 = L_sub(x[i], x[i + 4]); /* Re A - Re B */
  a2 = L_sub(x[i + 2], x[i + 6]);   /* Re C - Re D */
  a3 = L_sub(x[i + 1], x[i + 5]);   /* Im A - Im B */
  a1 = L_sub(x[i + 3], x[i + 7]);   /* Im C - Im D */
      
      
  x[i]     = L_shr(L_add(a00, a10),scale2);  /* Re A '= ReA + ReB +ReC + ReD */    move32();
  x[i + 4] = L_shr(L_sub(a00, a10),scale2);  /* Re C' = -(ReC+ReD) + (ReA+ReB) */  move32();
  x[i + 1] = L_shr(L_add(a20, a30),scale2);  /* Im A' = sum of imag values */      move32();
  x[i + 5] = L_shr(L_sub(a20, a30),scale2);  /* Im C' = -Im C -Im D +Im A +Im B */ move32();
      
  x[i + 2] = L_shr(L_sub(a0, a1),scale2);  /* Re B' = Im C - Im D  + Re A - Re B */move32();
  x[i + 6] = L_shr(L_add(a0, a1),scale2);  /* Re D' = -Im C + Im D + Re A - Re B */move32();
  x[i + 3] = L_shr(L_add(a3, a2),scale2);  /* Im B'= -Re C + Re D + Im A - Im B */ move32();
  x[i + 7] = L_shr(L_sub(a3, a2),scale2);  /* Im D'= Re C - Re D + Im A - Im B */  move32();
   
}


void inv_dit_fft_8pt_enc(Word32 *x,               /*!< Pointer to input vector */ 
                         Word16 scale)            /*!< 0: scale off; 1: scale on */
{  
  Word16 scale1, scale2;
  Word32 a0, a1, a2, a3, a00, a10, a20, a30;
  Word32 vr, vi, ur, ui;
  
  Word32 temp;
  Word32 t1;
  Word32 t2;
  Word32 i;
  
  Word32 s,c;

  test();
  if (scale != 0) {
    scale1 = 1;                                              move16();
    scale2 = 2;                                              move16();
  }
  else {
    scale1 = 0;                                              move16();
    scale2 = 0;                                              move16();
  }  
  
  i=0;
  
  temp = x[2];
  x[2] = x[8];            move32();
  x[8] = temp ;           move32();
  
  temp = x[3];
  x[3] = x[9];            move32();
  x[9] = temp ;           move32();
  
  temp = x[6];
  x[6] = x[12];           move32();
  x[12] = temp ;          move32();
  
  temp = x[7];
  x[7] = x[13];           move32();
  x[13] = temp ;          move32();   
      
      
  a00 = L_add(x[i], x[i + 2]);  /* Re A + Re B */
  a0  = L_sub(x[i], x[i + 2]); /* Re A - Re B */
  
  
  a20 = L_add(x[i + 1], x[i + 3]);  /* Im A + Im B */
  a3  = L_sub(x[i + 1], x[i + 3]);   /* Im A - Im B */
  
  a10 = L_add(x[i + 4], x[i + 6]);  /* Re C + Re D */
  a2  = L_sub(x[i + 4], x[i + 6]);   /* Re C - Re D */
  
  a30 = L_add(x[i + 5], x[i + 7]);  /* Im C + Im D */      
  a1  = L_sub(x[i + 5], x[i + 7]);   /* Im C - Im D */
  
  
  x[i]     = L_shr(L_add(a00, a10),scale2);  /* Re A '= ReA + ReB +ReC + ReD */    move32();
  x[i + 4] = L_shr(L_sub(a00, a10),scale2);  /* Re C' = -(ReC+ReD) + (ReA+ReB) */  move32();
  x[i + 1] = L_shr(L_add(a20, a30),scale2);  /* Im A' = sum of imag values */      move32();
  x[i + 5] = L_shr(L_sub(a20, a30),scale2);  /* Im C' = -Im C -Im D +Im A +Im B */ move32();
  
  
  
  x[i + 2] = L_shr(L_sub(a0, a1),scale2);  /* Re B' = Im C - Im D  + Re A - Re B */ move32();
  x[i + 6] = L_shr(L_add(a0, a1),scale2);  /* Re D' = -Im C + Im D + Re A - Re B */ move32();
  x[i + 3] = L_shr(L_add(a3, a2),scale2);  /* Im B'= -Re C + Re D + Im A - Im B */  move32();
  x[i + 7] = L_shr(L_sub(a3, a2),scale2);  /* Im D'= Re C - Re D + Im A - Im B */   move32();

  i += 8; 
  
  a00 = L_add(x[i], x[i + 2]);       /* Re A + Re B */
  a0  = L_sub(x[i], x[i + 2]);       /* Re A - Re B */
  
  a20 = L_add(x[i + 1], x[i + 3]);   /* Im A + Im B */
  a3  = L_sub(x[i + 1], x[i + 3]);   /* Im A - Im B */
  
  a10 = L_add(x[i + 4], x[i + 6]);   /* Re C + Re D */
  a2  = L_sub(x[i + 4], x[i + 6]);   /* Re C - Re D */
  
  a30 = L_add(x[i + 5], x[i + 7]);   /* Im C + Im D */
  a1  = L_sub(x[i + 5], x[i + 7]);   /* Im C - Im D */
  
  
  x[i]     = L_shr(L_add(a00, a10),scale2);  /* Re A '= ReA + ReB +ReC + ReD */    move32();
  x[i+ 4]  = L_shr(L_sub(a00, a10),scale2);  /* Re C' = -(ReC+ReD) + (ReA+ReB) */  move32();
  x[i + 1] = L_shr(L_add(a20, a30),scale2);  /* Im A' = sum of imag values */      move32();
  x[i + 5] = L_shr(L_sub(a20, a30),scale2);  /* Im C' = -Im C -Im D +Im A +Im B */ move32();
  
  x[i + 2] = L_shr(L_sub(a0, a1),scale2);  /* Re B' = Im C - Im D  + Re A - Re B */move32();
  x[i + 6] = L_shr(L_add(a0, a1),scale2);  /* Re D' = -Im C + Im D + Re A - Re B */move32();
  x[i + 3] = L_shr(L_add(a3, a2),scale2);  /* Im B'= -Re C + Re D + Im A - Im B */ move32();
  x[i + 7] = L_shr(L_sub(a3, a2),scale2);  /* Im D'= Re C - Re D + Im A - Im B */  move32();
  
  
  t1=0;
  t2=4;
  
  vr=x[t2*2];
  vi=x[t2*2+1];
  ur=x[t1*2];
  ui=x[t1*2+1];
  
  x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                          move32();
  x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                          move32();
  
  x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                          move32();
  x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                          move32();
  
  t1=2;
  t2=6;
  
  vr=-x[t2*2+1];
  vi=x[t2*2];
  ur=x[t1*2];
  ui=x[t1*2+1];
  
  x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                          move32();
  x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                          move32();
  
  x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                          move32();
  x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                          move32();
  
  
  s = 0x5A820000;                                                                  move32();
  c = 0x5A820000;                                                                  move32();
  
  t1=1;
  t2=t1+4;
  
  vr = L_sub(fixmul(x[t2*2], c), fixmul(x[t2*2+1], s));
  vi = L_add(fixmul(x[t2*2], s), fixmul(x[t2*2+1], c));
  ur=x[t1*2];
  ui=x[t1*2+1];
  
  x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                          move32();
  x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                          move32();
  
  x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                          move32();
  x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                          move32();
  
  t1=1+2;
  t2=t1+4;
  
  
  vr= L_sub(fixmul(L_negate(x[t2*2]), s), fixmul(x[t2*2+1], c));
  vi =L_sub(fixmul(x[t2*2], c), fixmul(x[t2*2+1], s));
  ur=x[t1*2];
  ui=x[t1*2+1];
  
  x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                          move32();
  x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                          move32();
  
  x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                          move32();
  x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                          move32();  

} 


/*!
    \brief bitreversal of input data

*/
static void scramble16(Word16 *x, /*!< Pointer to input vector */ 
                       Word16 n   /*!< Length of input vector */
                       )
{
  Word16 m,k,j;

  for (m=1,j=0; m<n-1; m++)
  {
    {for(k=n>>1; (!((j^=k)&k)); k>>=1);}
 
    test(); sub(1, 1);
    if (j>m)
    {
      Word16 tmp;
      tmp=x[2*m];
      x[2*m]=x[2*j];      move16();
      x[2*j]=tmp;         move16();
      
      tmp=x[2*m+1];
      x[2*m+1]=x[2*j+1];  move16();
      x[2*j+1]=tmp;       move16();
    }
  }
}


/*!

  \brief dit_fft (analysis) 

  dit-tukey-algorithm, scrambles data at entry i.e. loop is made with
  scrambled data.

*/
static void dit_fft16(Word16 *x,               /*!< Pointer to input vector */ 
                      Word16 ldn,              /*!< Number of stages */
                      const Word16 *trigdata,  /*!< Pointer to coefficient table */
                      Word16 trigDataSize      /*!< Size of coefficient table */
                      )
{
  const Word16 n = shl(1, ldn);
  Word16 trigstep,i,ldm;
  
  scramble16(x,n);
  
  /*
    1+2 stage radix 4
  */
  
  for(i=0;i<n*2;i+=8){
    Word16 a0, a1, a2, a3, a00, a10, a20, a30;
    Word16 x0, x1, x2, x3, x4, x5, x6, x7;
    
    x0 = x[i];
    x1 = x[i + 1];
    x2 = x[i + 2];
    x3 = x[i + 3];
    x4 = x[i + 4];
    x5 = x[i + 5];
    x6 = x[i + 6];
    x7 = x[i + 7];
    
    a00 = add(x0, x2);  /* Re A + Re B */
    a10 = add(x4, x6);  /* Re C + Re D */
    a20 = add(x1, x3);  /* Im A + Im B */
    a30 = add(x5, x7);  /* Im C + Im D */
    a0 =  sub(x0, x2);  /* Re A - Re B */
    a2 =  sub(x4, x6);  /* Re C - Re D */
    a3 =  sub(x1, x3);  /* Im A - Im B */
    a1 =  sub(x5, x7);  /* Im C - Im D */
    
    x[i]     = shr(add(shr(a00, 1), shr(a10, 1)), 1);  /* Re A '= ReA + ReB +ReC + ReD */          move16();
    x[i + 4] = shr(sub(shr(a00, 1), shr(a10, 1)), 1);  /* Re C' = -(ReC+ReD) + (ReA+ReB) */        move16();
    x[i + 1] = shr(add(shr(a20, 1), shr(a30, 1)), 1);  /* Im A' = sum of imag values */            move16();
    x[i + 5] = shr(sub(shr(a20, 1), shr(a30, 1)), 1);  /* Im C' = -Im C -Im D +Im A +Im B */       move16();
    x[i + 2] = shr(add(shr(a1,  1), shr(a0,  1)), 1);  /* Re B' = Im C - Im D  + Re A - Re B */    move16();
    x[i + 6] = shr(sub(shr(a0,  1), shr(a1,  1)), 1);  /* Re D' = -Im C + Im D + Re A - Re B */    move16();
    x[i + 3] = shr(sub(shr(a3,  1), shr(a2,  1)), 1);  /* Im B'= -Re C + Re D + Im A - Im B */     move16();
    x[i + 7] = shr(add(shr(a2,  1), shr(a3,  1)), 1);  /* Im D'= Re C - Re D + Im A - Im B */      move16();  
  }
  
  
  for(ldm=3; ldm<=ldn; ++ldm) {
    const Word16 m = shl(1, ldm);            
    const Word16 mh = shr(m, 1);             
    Word16 j,r;
    
    trigstep=((trigDataSize*4)>>ldm);
    
    for(j=0; j<mh/2; ++j) {
      Word16 s,c;
      
      s = trigdata[j*trigstep]; 
      c = trigdata[trigDataSize-j*trigstep]; 
      
      for(r=0; r<n; r+=m) {
        Word32 t1=r+j;
        Word32 t2=t1+mh;
        Word16 vr,vi,ur,ui;
        Word16 tmpr, tmpi;
        
        tmpr = x[t2*2];
        tmpi = x[t2*2+1];
        
        vr = mac_r( L_mult(tmpr, c), tmpi, s );
        vi = msu_r( L_mult(tmpi, c), tmpr, s );
        
        ur = x[t1*2];
        ui = x[t1*2+1];
        
        x[t1*2]  = shr(add(ur, vr), 1);             move16();
        x[t1*2+1]= shr(add(ui, vi), 1);             move16();
        
        x[t2*2]  = shr(sub(ur, vr), 1);             move16();
        x[t2*2+1]= shr(sub(ui, vi), 1);             move16();
        
        t1=r+j+mh/2;
        t2=t1+mh;
        
        tmpr = x[t2*2];
        tmpi = x[t2*2+1];
        
        vr = msu_r( L_mult(tmpi, c), tmpr, s );
        vi = negate( mac_r( L_mult(tmpr,  c), tmpi, s ) );
        
        ur = x[t1*2];
        ui = x[t1*2+1];
        
        x[t1*2]  = shr(add(ur, vr), 1);             move16();
        x[t1*2+1]= shr(add(ui, vi), 1);             move16();
        
        x[t2*2]  = shr(sub(ur, vr), 1);             move16();
        x[t2*2+1]= shr(sub(ui, vi), 1);             move16();      
      }
    }
  }
}


/*!

  \brief complex fft of input data (in place)

*/
void cfft16(Word16 *x,    /*!< Pointer to input vector */
            Word16 size,  /*!< Length of fft */
            Word16 isign  /*!< Controls analysis/synthesis */
            )
{
  Word16 ld_size;

  for(ld_size=1;(1<<ld_size) < size;ld_size++);

  test(); sub(1, 1);
  if(isign == -1) {
    dit_fft16(x,ld_size,fftTwiddleTable,LSI_FFT_TWIDDLE_TABLE_SIZE);
  }
  else {
    /* not converted yet, not used anyway ! */
    assert( 0 );
  }
}


/*!
    \brief bitreversal of input data

*/
static void scramble32(Word32 *x, /*!< Pointer to input vector */ 
                       Word16 n   /*!< Length of input vector */
                       )
{
  Word16 m,k,j;

  for (m=1,j=0; m<n-1; m++)
  {
    {for(k=n>>1; (!((j^=k)&k)); k>>=1);}
    
    test(); sub(1, 1);
    if (j>m)
    {
      Word32 tmp;
      tmp=x[2*m];
      x[2*m]=x[2*j];         move32();
      x[2*j]=tmp;            move32();
      
      tmp=x[2*m+1];
      x[2*m+1]=x[2*j+1];     move32();
      x[2*j+1]=tmp;          move32();
    }
  }
}




/*!

  \brief dit_fft (analysis) 

  dit-tukey-algorithm, scrambles data at entry i.e. loop is made with
  scrambled data.

*/
static void dit_fft32(Word32 *x,              /*!< Pointer to input vector */
                      Word16 ldn,             /*!< Number of stages */
                      const Word16 *trigdata, /*!< Pointer to coefficient table */
                      Word16 trigDataSize,    /*!< Size of coefficient table */
                      Word8 scale             /*!< 0: scale off; 1: scale on */
                      )
{
  const Word16 n = shl(1, ldn);
  Word16 trigstep,i,ldm;
  Word16 scale1,scale2;
  
  test();
  if (scale != 0) {
    scale1 = 1;                                                           move16();
    scale2 = 2;                                                           move16();
  }
  else {
    scale1 = 0;                                                           move16();
    scale2 = 0;                                                           move16();
  }
  
  scramble32(x,n);
  
  /*
    1+2 stage radix 4
  */
  
  for(i=0;i<n*2;i+=8){
    Word32 a0, a1, a2, a3, a00, a10, a20, a30;
    
    a00 = L_shr(L_add(x[i],     x[i + 2]),scale2);  /* Re A + Re B */
    a10 = L_shr(L_add(x[i + 4], x[i + 6]),scale2);  /* Re C + Re D */
    a20 = L_shr(L_add(x[i + 1], x[i + 3]),scale2);  /* Im A + Im B */
    a30 = L_shr(L_add(x[i + 5], x[i + 7]),scale2);  /* Im C + Im D */
    a0 =  L_shr(L_sub(x[i],     x[i + 2]),scale2);  /* Re A - Re B */
    a2 =  L_shr(L_sub(x[i + 4], x[i + 6]),scale2);  /* Re C - Re D */
    a3 =  L_shr(L_sub(x[i + 1], x[i + 3]),scale2);  /* Im A - Im B */
    a1 =  L_shr(L_sub(x[i + 5], x[i + 7]),scale2);  /* Im C - Im D */
    
    x[i]     = L_add(a00,a10);  /* Re A '= ReA + ReB +ReC + ReD */     move32();
    x[i + 4] = L_sub(a00,a10);  /* Re C' = -(ReC+ReD) + (ReA+ReB) */   move32();
    x[i + 1] = L_add(a20,a30);  /* Im A' = sum of imag values */       move32();
    x[i + 5] = L_sub(a20,a30);  /* Im C' = -Im C -Im D +Im A +Im B */  move32();
    x[i + 2] = L_add(a1,a0);  /* Re B' = Im C - Im D  + Re A - Re B */ move32();
    x[i + 6] = L_sub(a0,a1);  /* Re D' = -Im C + Im D + Re A - Re B */ move32();
    x[i + 3] = L_sub(a3,a2);  /* Im B'= -Re C + Re D + Im A - Im B */  move32();
    x[i + 7] = L_add(a2,a3);  /* Im D'= Re C - Re D + Im A - Im B */   move32();
  }
  
  
  for(ldm=3; ldm<=ldn; ++ldm) {
    const Word16 m = shl(1, ldm);            
    const Word16 mh = shr(m, 1);             
    Word16 j,r;
    
    trigstep=((trigDataSize*4)>>ldm);
    
    
    for(j=0; j<mh/2; ++j) {
      Word16 s,c;
      
      s = trigdata[j*trigstep];
      c = trigdata[trigDataSize-j*trigstep];
      
      for(r=0; r<n; r+=m) {
        Word32 t1=r+j;
        Word32 t2=t1+mh;
        Word32 vr,vi,ur,ui;
        Word32 x20, x21;
        
        x20 = x[t2*2];
        x21 = x[t2*2+1];
        
        vr= L_add( fixmul_32x16b(x20, c), fixmul_32x16b(x21, s) );
        vi= L_sub( fixmul_32x16b(x21, c), fixmul_32x16b(x20, s) );
        
        ur=x[t1*2];
        ui=x[t1*2+1];
        
        x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                 move32();
        x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                 move32();
        
        x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                 move32();
        x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                 move32();
        
        t1=r+j+mh/2;
        t2=t1+mh;
        
        x20 = x[t2*2];
        x21 = x[t2*2+1];
        
        vr= L_sub( fixmul_32x16b(x21, c), fixmul_32x16b(x20, s) );
        vi= L_sub( L_negate(fixmul_32x16b(x20, c)), fixmul_32x16b(x21, s) );
        
        ur=x[t1*2];
        ui=x[t1*2+1];
        
        x[t1*2]  = L_shr(L_add(ur, vr),scale1);                                 move32();
        x[t1*2+1]= L_shr(L_add(ui, vi),scale1);                                 move32();
        
        x[t2*2]  = L_shr(L_sub(ur, vr),scale1);                                 move32();
        x[t2*2+1]= L_shr(L_sub(ui, vi),scale1);                                 move32();
      }
    }
  }
} 


/*!

  \brief complex fft of input data (in place)

*/
void cfft32(Word32 *x,    /*!< Pointer to input vector */
            Word16 size,  /*!< Length of fft */
            Word16 isign, /*!< Controls analysis/synthesis */
            Word8 scale   /*!< 0: scale off; 1: scale on */
            )
{
  Word16 ld_size;

  assert (LSI_FFT_TWIDDLE_TABLE_SIZE >= size);

  for(ld_size=1;(1<<ld_size) < size;ld_size++);

  test(); sub(1, 1);
  if(isign == -1) {
    dit_fft32(x,ld_size,fftTwiddleTable,LSI_FFT_TWIDDLE_TABLE_SIZE,scale);
  }
  else {
    /* not converted yet, not used anyway ! */
    assert( 0 );
  }
}

