/*
    Complex FFT Analysis/Synthesis
 */
#ifndef __FFT_H
#define __FFT_H

/*!< Size of the fft twiddle table, <br>
     The table can be used in other algorithmic sections
     sin k*pi/(LSI_FFT_TWIDDLE_TABLE_SIZE*2) k=0..LSI_FFT_TWIDDLE_TABLE_SIZE
*/
#define LSI_LD_FFT_TWIDDLE_TABLE_SIZE 9
#define LSI_FFT_TWIDDLE_TABLE_SIZE (1<<LSI_LD_FFT_TWIDDLE_TABLE_SIZE)

/* twiddle tables */
extern const Word16 fftTwiddleTable[LSI_FFT_TWIDDLE_TABLE_SIZE+1];

void cfft32(Word32 *x, Word16 size, Word16 isign, Word8 scale);
void cfft16(Word16 *x, Word16 size, Word16 isign);
void inv_dit_fft_4pt(Word32 *In ,Word16 );
void inv_dit_fft_8pt_enc(Word32 *In ,Word16 );

#endif
