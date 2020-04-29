/*  
   DPCM envelope coding
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sbr.h"
#include "code_env.h"
#include "sbr_rom.h"

#include "count.h"


/*****************************************************************************

 functionname: InitSbrHuffmanTables
 description:  initializes Huffman Tables dependent on chosen amp_res
 returns:      error handle
 input:        
 output:       
 
*****************************************************************************/
Word32
InitSbrHuffmanTables (HANDLE_SBR_ENV_DATA       sbrEnvData,
                      HANDLE_SBR_CODE_ENVELOPE  henv,
                      HANDLE_SBR_CODE_ENVELOPE  hnoise,
                      AMP_RES                   amp_res)
{
  test(); test(); test();
  if ( (!henv)  ||  (!hnoise)  || (!sbrEnvData) )
    return (1); /* not init. */

  sbrEnvData->init_sbr_amp_res = amp_res;                                                    move16();

  test();
  switch (amp_res) {
  case  SBR_AMP_RES_3_0:
    /*envelope data*/

    /*Level/Pan - coding */
    sbrEnvData->hufftableLevelTimeC   = v_Huff_envelopeLevelC11T;                            move16();
    sbrEnvData->hufftableLevelTimeL   = v_Huff_envelopeLevelL11T;                            move16();
    sbrEnvData->hufftableBalanceTimeC = bookSbrEnvBalanceC11T;                               move16();
    sbrEnvData->hufftableBalanceTimeL = bookSbrEnvBalanceL11T;                               move16();

    sbrEnvData->hufftableLevelFreqC   = v_Huff_envelopeLevelC11F;                            move16();
    sbrEnvData->hufftableLevelFreqL   = v_Huff_envelopeLevelL11F;                            move16();
    sbrEnvData->hufftableBalanceFreqC = bookSbrEnvBalanceC11F;                               move16();
    sbrEnvData->hufftableBalanceFreqL = bookSbrEnvBalanceL11F;                               move16();

    /*Right/Left - coding */
    sbrEnvData->hufftableTimeC        = v_Huff_envelopeLevelC11T;                            move16();
    sbrEnvData->hufftableTimeL        = v_Huff_envelopeLevelL11T;                            move16();
    sbrEnvData->hufftableFreqC        = v_Huff_envelopeLevelC11F;                            move16();
    sbrEnvData->hufftableFreqL        = v_Huff_envelopeLevelL11F;                            move16();

    sbrEnvData->codeBookScfLavBalance  = CODE_BOOK_SCF_LAV_BALANCE11;                        move16();
    sbrEnvData->codeBookScfLav         = CODE_BOOK_SCF_LAV11;                                move16();
    
    sbrEnvData->si_sbr_start_env_bits           = SI_SBR_START_ENV_BITS_AMP_RES_3_0;         move16();
    sbrEnvData->si_sbr_start_env_bits_balance   = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0; move16();
    break;

  case SBR_AMP_RES_1_5:
    /*envelope data*/

    /*Level/Pan - coding */
    sbrEnvData->hufftableLevelTimeC   = v_Huff_envelopeLevelC10T;                            move16();
    sbrEnvData->hufftableLevelTimeL   = v_Huff_envelopeLevelL10T;                            move16();
    sbrEnvData->hufftableBalanceTimeC = bookSbrEnvBalanceC10T;                               move16();
    sbrEnvData->hufftableBalanceTimeL = bookSbrEnvBalanceL10T;                               move16();

    sbrEnvData->hufftableLevelFreqC   = v_Huff_envelopeLevelC10F;                            move16();
    sbrEnvData->hufftableLevelFreqL   = v_Huff_envelopeLevelL10F;                            move16();
    sbrEnvData->hufftableBalanceFreqC = bookSbrEnvBalanceC10F;                               move16();
    sbrEnvData->hufftableBalanceFreqL = bookSbrEnvBalanceL10F;                               move16();

    /*Right/Left - coding */
    sbrEnvData->hufftableTimeC        = v_Huff_envelopeLevelC10T;                            move16();
    sbrEnvData->hufftableTimeL        = v_Huff_envelopeLevelL10T;                            move16();
    sbrEnvData->hufftableFreqC        = v_Huff_envelopeLevelC10F;                            move16();
    sbrEnvData->hufftableFreqL        = v_Huff_envelopeLevelL10F;                            move16();

    sbrEnvData->codeBookScfLavBalance = CODE_BOOK_SCF_LAV_BALANCE10;                         move16();
    sbrEnvData->codeBookScfLav = CODE_BOOK_SCF_LAV10;                                        move16();
    
    sbrEnvData->si_sbr_start_env_bits           = SI_SBR_START_ENV_BITS_AMP_RES_1_5;         move16();
    sbrEnvData->si_sbr_start_env_bits_balance   = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5; move16();
    break;

  default:
    return (1); /* undefined amp_res mode */
    break;
  }

  /* these are common to both amp_res values */
  /*Noise data*/
  
  /*Level/Pan - coding */
  sbrEnvData->hufftableNoiseLevelTimeC   = v_Huff_NoiseLevelC11T;                            move16();
  sbrEnvData->hufftableNoiseLevelTimeL   = v_Huff_NoiseLevelL11T;                            move16();
  sbrEnvData->hufftableNoiseBalanceTimeC = bookSbrNoiseBalanceC11T;                          move16();
  sbrEnvData->hufftableNoiseBalanceTimeL = bookSbrNoiseBalanceL11T;                          move16();
  
  sbrEnvData->hufftableNoiseLevelFreqC   = v_Huff_envelopeLevelC11F;                         move16();
  sbrEnvData->hufftableNoiseLevelFreqL   = v_Huff_envelopeLevelL11F;                         move16();
  sbrEnvData->hufftableNoiseBalanceFreqC = bookSbrEnvBalanceC11F;                            move16();
  sbrEnvData->hufftableNoiseBalanceFreqL = bookSbrEnvBalanceL11F;                            move16();
  
  
  /*Right/Left - coding */
  sbrEnvData->hufftableNoiseTimeC        = v_Huff_NoiseLevelC11T;                            move16();
  sbrEnvData->hufftableNoiseTimeL        = v_Huff_NoiseLevelL11T;                            move16();
  sbrEnvData->hufftableNoiseFreqC        = v_Huff_envelopeLevelC11F;                         move16();
  sbrEnvData->hufftableNoiseFreqL        = v_Huff_envelopeLevelL11F;                         move16();
  
  sbrEnvData->si_sbr_start_noise_bits         = SI_SBR_START_NOISE_BITS_AMP_RES_3_0;         move16();
  sbrEnvData->si_sbr_start_noise_bits_balance = SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0; move16();

  
  /* init envelope tables and codebooks */
  henv->codeBookScfLavBalanceTime = sbrEnvData->codeBookScfLavBalance;                       move16();
  henv->codeBookScfLavBalanceFreq = sbrEnvData->codeBookScfLavBalance;                       move16();
  henv->codeBookScfLavLevelTime = sbrEnvData->codeBookScfLav;                                move16();
  henv->codeBookScfLavLevelFreq = sbrEnvData->codeBookScfLav;                                move16();
  henv->codeBookScfLavTime = sbrEnvData->codeBookScfLav;                                     move16();
  henv->codeBookScfLavFreq = sbrEnvData->codeBookScfLav;                                     move16();

  henv->hufftableLevelTimeL = sbrEnvData->hufftableLevelTimeL;                               move16();
  henv->hufftableBalanceTimeL = sbrEnvData->hufftableBalanceTimeL;                           move16();
  henv->hufftableTimeL = sbrEnvData->hufftableTimeL;                                         move16();
  henv->hufftableLevelFreqL = sbrEnvData->hufftableLevelFreqL;                               move16();
  henv->hufftableBalanceFreqL = sbrEnvData->hufftableBalanceFreqL;                           move16();
  henv->hufftableFreqL = sbrEnvData->hufftableFreqL;                                         move16();

  henv->codeBookScfLavFreq = sbrEnvData->codeBookScfLav;                                     move16();
  henv->codeBookScfLavTime = sbrEnvData->codeBookScfLav;                                     move16();

  henv->start_bits = sbrEnvData->si_sbr_start_env_bits;                                      move16();
  henv->start_bits_balance = sbrEnvData->si_sbr_start_env_bits_balance;                      move16();

  
  /* init noise tables and codebooks */

  hnoise->codeBookScfLavBalanceTime = CODE_BOOK_SCF_LAV_BALANCE11;                           move16();
  hnoise->codeBookScfLavBalanceFreq = CODE_BOOK_SCF_LAV_BALANCE11;                           move16();
  hnoise->codeBookScfLavLevelTime = CODE_BOOK_SCF_LAV11;                                     move16();
  hnoise->codeBookScfLavLevelFreq = CODE_BOOK_SCF_LAV11;                                     move16();
  hnoise->codeBookScfLavTime = CODE_BOOK_SCF_LAV11;                                          move16();
  hnoise->codeBookScfLavFreq = CODE_BOOK_SCF_LAV11;                                          move16();

  hnoise->hufftableLevelTimeL = sbrEnvData->hufftableNoiseLevelTimeL;                        move16();
  hnoise->hufftableBalanceTimeL = sbrEnvData->hufftableNoiseBalanceTimeL;                    move16();
  hnoise->hufftableTimeL = sbrEnvData->hufftableNoiseTimeL;                                  move16();
  hnoise->hufftableLevelFreqL = sbrEnvData->hufftableNoiseLevelFreqL;                        move16();
  hnoise->hufftableBalanceFreqL = sbrEnvData->hufftableNoiseBalanceFreqL;                    move16();
  hnoise->hufftableFreqL = sbrEnvData->hufftableNoiseFreqL;                                  move16();

  
  hnoise->start_bits = sbrEnvData->si_sbr_start_noise_bits;                                  move16();
  hnoise->start_bits_balance = sbrEnvData->si_sbr_start_noise_bits_balance;                  move16();
  
  /* No delta coding in time from the previous frame due to 1.5dB FIx-FIX rule */
  henv->upDate = 0;                                                                          move16();
  hnoise->upDate = 0;                                                                        move16();
  return  (0);
}

/*******************************************************************************
 Functionname:  indexLow2High
 *******************************************************************************

 Description:   Nice small patch-functions in order to cope with non-factor-2
                ratios between high-res and low-res

 Arguments:     Word32 offset, Word32 index, FREQ_RES res

 Return:        Word32

 Written:       Kristofer Kjoerling, CTS AB, April 2001
 Modified:      Andreas Schneider, CTG 05/02/2002 made res be of type FREQ_RES
 Revised:
         
*******************************************************************************/
static Word16 indexLow2High(Word16 offset, Word16 index, FREQ_RES res)
{

  test(); sub(1 ,1);
  if(res == FREQ_RES_LOW)
    {
      test();
      if (offset >= 0)
        {
          test(); sub(1, 1);
          if (index < offset)
            return(index);
          else
            return sub(shl(index, 1), offset);
        }
      else
        {
          offset = negate(offset);
          test(); sub(1, 1);
          if (index < offset)
            return add(shl(index, 1), index);
          else
            return add(shl(index, 1), offset);
        }
    }
  else
    return(index);
}



/*******************************************************************************
 Functionname:  mapLowResEnergyVal
 *******************************************************************************

 Description:   

 Arguments:     Word32 currVal,Word32* prevData, Word32 offset, Word32 index, FREQ_RES res

 Return:        none

 Written:       Kristofer Kjoerling, CTS AB, April 2001
 Modified:      Andreas Schneider, CTG 05/02/2002 made res be of type FREQ_RES
 Revised:
         
*******************************************************************************/
static void mapLowResEnergyVal(Word16 currVal,Word16* prevData, Word16 offset, Word16 index, FREQ_RES res)
{
  test(); sub(1, 1);
  if(res == FREQ_RES_LOW)
    {
      test();
      if (offset >= 0)
        {
          test(); sub(1, 1);
          if(index < offset) {
            prevData[index] = currVal;                                                  move16();
          }
          else
            {
              prevData[2*index - offset] = currVal;                                     move16();
              prevData[2*index+1 - offset] = currVal;                                   move16();
            }
        }
      else
        {
          offset = negate(offset);
          test(); sub(1, 1);
          if (index < offset)
            {
              prevData[3*index] = currVal;                                              move16();
              prevData[3*index+1] = currVal;                                            move16();
              prevData[3*index+2] = currVal;                                            move16();
            }
          else
            {
              prevData[2*index + offset] = currVal;                                     move16();
              prevData[2*index + 1 + offset] = currVal;                                 move16();
            }
        }    
    }
  else
    prevData[index] = currVal;                                                          move16();
}



/*******************************************************************************
 Functionname:  computeBits
 *******************************************************************************

 Description:   

 Arguments:     Word32 delta,
                Word32 codeBookScfLavLevel,
                Word32 codeBookScfLavBalance,
                const UWord8 * hufftableLevel,
                const UWord8 * hufftableBalance, Word32 coupling, Word32 channel)

 Return:        Word32

 Written:       Kristofer Kjoerling, CTS AB
 Revised:
         
*******************************************************************************/
static Word16
computeBits (Word16 delta,
             Word16 codeBookScfLavLevel,
             Word16 codeBookScfLavBalance,
             const UWord8 * hufftableLevel,
             const UWord8 * hufftableBalance, Word32 coupling, Word32 channel)
{
  Word16 index;
  Word16 delta_bits = 0;

  test();
  if (coupling) {
    test(); sub(1, 1);
    if (channel == 1)           
      {
        test();
        index =
          (delta < 0) ? S_max (delta, negate(codeBookScfLavBalance)) : S_min (delta,
                                                                   codeBookScfLavBalance);
        test(); sub(1, 1);
        if (index != delta) {
          assert(0);
          return (10000);
        }
        
        logic16(); logic16(); /* Word8 read access */ 
        delta_bits = hufftableBalance[index + codeBookScfLavBalance];                      move16();
      }
    else {
      test();
      index =
        (delta < 0) ? S_max (delta, negate(codeBookScfLavLevel)) : S_min (delta,
                                                               codeBookScfLavLevel);
      test(); sub(1, 1);
      if (index != delta) {
        assert(0);
        return (10000);
      }

      logic16(); logic16(); /* Word8 read access */ 
      delta_bits = hufftableLevel[index + codeBookScfLavLevel];                            move16();
    }
  }
  else {
    test();
    index =
      (delta < 0) ? S_max (delta, negate(codeBookScfLavLevel)) : S_min (delta,
                                                             codeBookScfLavLevel);
    test(); sub(1, 1);
    if (index != delta) {
      assert(0);
      return (10000);
    }

    logic16(); logic16(); /* Word8 read access */ 
    delta_bits = hufftableLevel[index + codeBookScfLavLevel];                              move16();
  }

  return (delta_bits);
}




/*******************************************************************************
 Functionname:  codeEnvelope
 *******************************************************************************

 Description:   

 Arguments:     Word32 *sfb_nrg,
                const FREQ_RES *freq_res,
                SBR_CODE_ENVELOPE * h_sbrCodeEnvelope,
                Word32 *directionVec, Word32 scalable, Word32 nEnvelopes, Word32 channel,
                Word32 headerActive)

 Return:        none
                h_sbrCodeEnvelope->sfb_nrg_prev is modified !
                sfb_nrg is modified
                h_sbrCodeEnvelope->update is modfied !
                *directionVec is modified


 Written:       Kristofer Kjoerling, CTS AB
 Modified:      Andreas Schneider, CTG 05/02/2002 made freq_res be of type FREQ_RES *
 Revised:
         
*******************************************************************************/
void
codeEnvelope (Word16            *sfb_nrg,
              const FREQ_RES    *freq_res,
              SBR_CODE_ENVELOPE *h_sbrCodeEnvelope,
              Word16 *directionVec, 
              Word16 coupling, 
              Word16 nEnvelopes, 
              Word16 channel,
              Word16 headerActive)
{
  Word16 i, no_of_bands, band, last_nrg, curr_nrg;
  Word16 *ptr_nrg;

  Word16 codeBookScfLavLevelTime;
  Word16 codeBookScfLavLevelFreq;
  Word16 codeBookScfLavBalanceTime;
  Word16 codeBookScfLavBalanceFreq;
  const UWord8 *hufftableLevelTimeL;
  const UWord8 *hufftableBalanceTimeL;
  const UWord8 *hufftableLevelFreqL;
  const UWord8 *hufftableBalanceFreqL;
  
  Word16 offset = h_sbrCodeEnvelope->offset;
  Word16 envDataTableCompFactor;

  Word16 delta_F_bits = 0, delta_T_bits = 0;

  Word16 delta_F[MAX_FREQ_COEFFS]; 
  Word16 delta_T[MAX_FREQ_COEFFS]; 

  test();
  if (coupling) {
    codeBookScfLavLevelTime = h_sbrCodeEnvelope->codeBookScfLavLevelTime;                 move16();
    codeBookScfLavLevelFreq = h_sbrCodeEnvelope->codeBookScfLavLevelFreq;                 move16();
    codeBookScfLavBalanceTime = h_sbrCodeEnvelope->codeBookScfLavBalanceTime;             move16();
    codeBookScfLavBalanceFreq = h_sbrCodeEnvelope->codeBookScfLavBalanceFreq;             move16();
    hufftableLevelTimeL = h_sbrCodeEnvelope->hufftableLevelTimeL;                         move16();
    hufftableBalanceTimeL = h_sbrCodeEnvelope->hufftableBalanceTimeL;                     move16();
    hufftableLevelFreqL = h_sbrCodeEnvelope->hufftableLevelFreqL;                         move16();
    hufftableBalanceFreqL = h_sbrCodeEnvelope->hufftableBalanceFreqL;                     move16();
  }
  else {
    codeBookScfLavLevelTime = h_sbrCodeEnvelope->codeBookScfLavTime;                      move16();
    codeBookScfLavLevelFreq = h_sbrCodeEnvelope->codeBookScfLavFreq;                      move16();
    codeBookScfLavBalanceTime = h_sbrCodeEnvelope->codeBookScfLavTime;                    move16();
    codeBookScfLavBalanceFreq = h_sbrCodeEnvelope->codeBookScfLavFreq;                    move16();
    hufftableLevelTimeL = h_sbrCodeEnvelope->hufftableTimeL;                              move16();
    hufftableBalanceTimeL = h_sbrCodeEnvelope->hufftableTimeL;                            move16();
    hufftableLevelFreqL = h_sbrCodeEnvelope->hufftableFreqL;                              move16();
    hufftableBalanceFreqL = h_sbrCodeEnvelope->hufftableFreqL;                            move16();
  }

  test(); test(); sub(1, 1); sub(1, 1);
  if(coupling == 1 && channel == 1)
    envDataTableCompFactor = 1;       /*should be one when the new huffman-tables are ready*/
  else
    envDataTableCompFactor = 0;


  test();
  if (h_sbrCodeEnvelope->deltaTAcrossFrames == 0)
    h_sbrCodeEnvelope->upDate = 0;

  /* no delta coding in time in case of a header */
  test();
  if (headerActive)
    h_sbrCodeEnvelope->upDate = 0;


  for (i = 0; i < nEnvelopes; i++) 
    {
      test(); sub(1, 1);
      if (freq_res[i] == FREQ_RES_HIGH)
        no_of_bands = h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH];
      else
        no_of_bands = h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW];

    
      ptr_nrg = sfb_nrg;
      curr_nrg = *ptr_nrg;

      delta_F[0] = shr(curr_nrg, envDataTableCompFactor);

      test(); test(); sub(1, 1);
      if (coupling && channel == 1)
        delta_F_bits = h_sbrCodeEnvelope->start_bits_balance;
      else
        delta_F_bits = h_sbrCodeEnvelope->start_bits;

      test();
      if(h_sbrCodeEnvelope->upDate != 0)
        {
          delta_T[0] = shr(sub(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev[0]), envDataTableCompFactor);

          delta_T_bits = computeBits (delta_T[0],
                                      codeBookScfLavLevelTime,
                                      codeBookScfLavBalanceTime,
                                      hufftableLevelTimeL,
                                      hufftableBalanceTimeL, coupling, channel);
        }
    

      mapLowResEnergyVal(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev, offset, 0, freq_res[i]);


      /* ensure that nrg difference is not higher than codeBookScfLavXXXFreq */

      test(); test(); sub(1,1);
      if ( coupling && channel == 1 ) {
        for (band = no_of_bands - 1; band > 0; band--) {
          test(); sub(1,1);
          if ( sub(ptr_nrg[band], ptr_nrg[band-1]) > codeBookScfLavBalanceFreq ) {
            ptr_nrg[band-1] = sub(ptr_nrg[band], codeBookScfLavBalanceFreq);       move16();
          }
        }
        for (band = 1; band < no_of_bands; band++) {
          test(); sub(1,1);
          if ( sub(ptr_nrg[band-1], ptr_nrg[band]) > codeBookScfLavBalanceFreq ) {
            ptr_nrg[band] = sub(ptr_nrg[band-1], codeBookScfLavBalanceFreq);       move16();
          }
        }
      }
      else {
        for (band = sub(no_of_bands, 1); band > 0; band--) {
          test(); sub(1,1);
          if ( sub(ptr_nrg[band], ptr_nrg[band-1]) > codeBookScfLavLevelFreq ) {
            ptr_nrg[band-1] = sub(ptr_nrg[band], codeBookScfLavLevelFreq);         move16();
          }
        }
        for (band = 1; band < no_of_bands; band++) {
          test(); sub(1,1);          
          if ( sub(ptr_nrg[band-1], ptr_nrg[band]) > codeBookScfLavLevelFreq ) {
            ptr_nrg[band] = sub(ptr_nrg[band-1], codeBookScfLavLevelFreq);         move16();
          }
        }
      }


      /* Coding loop*/
      for (band = 1; band < no_of_bands; band++) 
        {
          last_nrg = (*ptr_nrg);
          ptr_nrg++;
          curr_nrg = (*ptr_nrg);
    
          delta_F[band] = shr(sub(curr_nrg, last_nrg), envDataTableCompFactor);

          delta_F_bits = add(delta_F_bits, computeBits (delta_F[band],
                                                        codeBookScfLavLevelFreq,
                                                        codeBookScfLavBalanceFreq,
                                                        hufftableLevelFreqL,
                                                        hufftableBalanceFreqL, coupling, channel));
          
          test();
          if(h_sbrCodeEnvelope->upDate != 0)
            {
              delta_T[band] = sub(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev[indexLow2High(offset, band, freq_res[i])]);      move16();
              delta_T[band] = shr(delta_T[band], envDataTableCompFactor);                                                    move16();
            }
       
          mapLowResEnergyVal(curr_nrg, h_sbrCodeEnvelope->sfb_nrg_prev, offset, band, freq_res[i]);
      
          test();
          if(h_sbrCodeEnvelope->upDate != 0)
            {

              delta_T_bits = add(delta_T_bits, computeBits (delta_T[band],
                                                            codeBookScfLavLevelTime,
                                                            codeBookScfLavBalanceTime,
                                                            hufftableLevelTimeL,
                                                            hufftableBalanceTimeL, coupling, channel));
            }
        }
      
      /* Replace sfb_nrg with deltacoded samples and set flag */
      test(); test();
      if (delta_T_bits < delta_F_bits && h_sbrCodeEnvelope->upDate != 0)
        {
          directionVec[i] = TIME;                                                                    move16();
          memcpy (sfb_nrg, delta_T, no_of_bands * sizeof (*sfb_nrg));                                memop16(no_of_bands);
        }
      else {
        h_sbrCodeEnvelope->upDate = 0;                                                               move16();
        directionVec[i] = FREQ;                                                                      move16();
        memcpy (sfb_nrg, delta_F, no_of_bands * sizeof (*sfb_nrg));                                  memop16(no_of_bands);
      }
    
      sfb_nrg = (Word16*)L_add((Word32)sfb_nrg, no_of_bands * sizeof(*sfb_nrg));
      h_sbrCodeEnvelope->upDate = 1;                                                                 move16();
    }

}




/*******************************************************************************
 Functionname:  CreateSbrCodeEnvelope
 *******************************************************************************

 Description:   

 Arguments:     

 Return:        

 Written:       Kristofer Kjoerling, CTS AB
 Modified:      Andreas Schneider, CTG 05/02/2002
                  minor changes regarding the use of HIGH/LOW-RES
 Revised:
         
*******************************************************************************/
Word32
CreateSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE  h_sbrCodeEnvelope,
                       Word16 *nSfb,
                       Word16 deltaTAcrossFrames)
{

  memset(h_sbrCodeEnvelope,0,sizeof(SBR_CODE_ENVELOPE));                          memop16((sizeof(SBR_CODE_ENVELOPE)+1)/sizeof(Word16));

  h_sbrCodeEnvelope->deltaTAcrossFrames = deltaTAcrossFrames;                                                                 move16();
  h_sbrCodeEnvelope->upDate = 0;                                                                                              move16();
  h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW] = nSfb[FREQ_RES_LOW];                                                                 move16();
  h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH] = nSfb[FREQ_RES_HIGH];                                                               move16();
  h_sbrCodeEnvelope->offset = sub(shl(h_sbrCodeEnvelope->nSfb[FREQ_RES_LOW], 1), h_sbrCodeEnvelope->nSfb[FREQ_RES_HIGH]);     move16();

  return (0);
}





/*******************************************************************************
 Functionname:  deleteSbrCodeEnvelope
 *******************************************************************************

 Description:   

 Arguments:     

 Return:        

 Written:       Kristofer Kjoerling, CTS AB
 Revised:
         
*******************************************************************************/
void
deleteSbrCodeEnvelope (HANDLE_SBR_CODE_ENVELOPE h_sbrCodeEnvelope)
{
  
  /*
    nothing to do
  */
  (void)h_sbrCodeEnvelope;
}
