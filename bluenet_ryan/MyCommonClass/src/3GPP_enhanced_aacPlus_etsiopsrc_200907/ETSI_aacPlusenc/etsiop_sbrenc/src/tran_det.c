/*
   Transient detector
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tran_det.h"
#include "sbr_def.h"
#include "fram_gen.h"
#include "sbr_ram.h"
#include "count.h"


/*******************************************************************************
 Functionname:  spectralChange
 *******************************************************************************
 \author  Klaus Peichl
 \brief   Calculates a measure for the spectral change within the frame
 
 The function says how good it would be to split the frame at the given border
 position into 2 envelopes.

 \return  calculated value
*******************************************************************************/
static Word32 spectralChange(Word32 **Energies,
                             Word16 *yBufferScale,
                             Word32 EnergyTotal,
                             Word16 nSfb,
                             Word16 start,
                             Word16 border,
                             Word16 stop)
{
  Word16 i,j;
  Word16 len1 = sub(border, start);
  Word16 len2 = sub(stop, border);
  Word16 mls;
  
  Word16 delta;
  Word16 deltafac;
  Word32 delta_sum= 0;
  Word32 delta_g;

  Word32 nrg1,nrg2;
  Word32 pos_weight;
  Word16 pos_weight16;

  Word16 len1fac = len1;
  Word16 len2fac = len2;

  Word32 tmp1, tmp2;

  mls = ffr_norm32(L_deposit_h(S_max(len1fac, len2fac)));
  
  len1fac = shl(len1fac, mls);
  len2fac = shl(len2fac, mls);


  /* prefer borders near the middle of the frame */
  pos_weight16 = div_s(len1, add(len1, len2));
  pos_weight16 = shl(sub(0x4000, pos_weight16), 1);
  pos_weight16 = sub(0x7fff, mult(pos_weight16, pos_weight16));
  pos_weight = fixmul_32x16b(0x000ad497, pos_weight16);  /* 1024.0: delta_scale, 0.69 ln(2) , 1000: thrscale) */


  /* Sum up energies of all QMF-timeslots for both halfs */
  for ( j=0; j<nSfb; j++ ) {
    Word32 ampWeight;
    /* init with some energy to prevent division by zero
       and to prevent splitting for very low levels */
    nrg1 = len1;                                                                       move32();
    nrg2 = len2;                                                                       move32();

    /* Sum up energies in first half */
    for (i=start; i<border; i++) {
      nrg1 = L_add(nrg1, Energies[i][j]);
    }
    /* Sum up energies in second half */
    for (i=border; i<stop; i++) {
      nrg2 = L_add(nrg2, Energies[i][j]);
    }
    /*
      delta = (float) fabs( log( nrg2[j] / nrg1[j] * lenRatio) );
      we estimate delta by norm functions 
    */
    tmp1 = fixmul_32x16b(nrg2, len1fac);
    tmp2 = fixmul_32x16b(nrg1, len2fac);
    delta = abs(sub(ffr_norm32(tmp1), ffr_norm32(tmp2)));
    deltafac = shl(delta, SHORT_BITS-1-10);                          /* a scalefactor of 2^-10 should be sufficient */

    /* Energy change in current band */
    /* Weighting with amplitude ratio of this band */
     
    nrg1 = L_shr(L_add(nrg1, nrg2), yBufferScale[0]);
    nrg2 = L_add(EnergyTotal, 1);
    ampWeight = ffr_div32_32(nrg1, nrg2);

    delta_sum = L_add(delta_sum, fixmul_32x16b(ffr_sqrt(ampWeight,INT_BITS), deltafac));
  
  }
  delta_g =  fixmul(delta_sum, pos_weight);


  return delta_g;
}


/*******************************************************************************
 Functionname:  addLowbandEnergies
 *******************************************************************************
 \author  Klaus Peichl
 \brief   Calculates total lowband energy
 \return  total energy in the lowband, scaled by 1.0/(slots)
*******************************************************************************/
static Word32 addLowbandEnergies(Word32 **Energies,
                                 Word16 *yBufferScale,
                                 UWord16 * freqBandTable,
                                 Word16 slots)
{
  Word16 i,k;
  Word16 scf0,scf1;
  Word16 minScale;
  Word16 iScale;

  Word32 nrgTotal=0;                                                                       move32();

  iScale = div_s(1, slots);

  /*
    determinate min scale factor
  */

  minScale = S_min(yBufferScale[0],yBufferScale[1]);
  
  scf0 = sub(yBufferScale[0], minScale);
  scf1 = sub(yBufferScale[1], minScale);




  /* Sum up lowband energy */
  for (k = 0; k < freqBandTable[0]; k++) {
    for (i=0; i< shr(slots, 1);i++){
      nrgTotal = L_add(nrgTotal, L_shr(fixmul_32x16b(Energies[(i+slots/2)/2][k], iScale), scf0));
    }
    for (; i<slots; i++) {
      nrgTotal = L_add(nrgTotal, L_shr(fixmul_32x16b(Energies[(i+slots/2)/2][k], iScale), scf1));
    }
  }
  
  nrgTotal = L_shr(nrgTotal, minScale);

  return(nrgTotal);
}


/*******************************************************************************
 Functionname:  addHighbandEnergies
 *******************************************************************************
 \author  Klaus Peichl
 \brief   Add highband energies
 
 Highband energies are mapped to an array with smaller dimension:
 Its time resolution is only 1 SBR-timeslot and its frequency resolution
 is 1 SBR-band. Therefore the data to be fed into the spectralChange
 function is reduced.

 \return  total energy in the highband, sclaed by 1.0/(sbrSlots * timeStep)
*******************************************************************************/
static Word32 addHighbandEnergies(Word32 **Energies, /*!< input */
                                  Word16 *yBufferScale,
                                  Word32 **EnergiesM, /*!< Combined output */
                                  UWord16 * freqBandTable,
                                  Word16 nSfb,
                                  Word16 sbrSlots,
                                  Word16 timeStep)
{
  Word16 i,j,k,slotIn,slotOut;
  Word16 li,ui;
  Word16 iScale;
  Word32 nrgTotal=0;                                                                               move32();

  iScale = extract_h(ffr_div32_32(1, L_shr(L_mult(sbrSlots, timeStep), 1)));

  /* Combine QMF-timeslots to SBR-timeslots,
     combine QMF-bands to SBR-bands,
     combine Left and Right channel */
  for (slotOut=0; slotOut<sbrSlots; slotOut++) {
    slotIn = shl(slotOut, 1);

    for ( j=0; j<nSfb; j++ ) {
      Word32 accu=0;                                                                               move32();      
      
      li = freqBandTable[j];
      ui = freqBandTable[j + 1];

      for (k = li; k < ui; k++) {

        for (i=0; i<timeStep; i++) {
          accu = L_add(accu, fixmul_32x16b(Energies[(slotIn+i)/2][k], iScale));
        }
      }
      EnergiesM[slotOut][j] = accu;     /* need to scaled down by yBufferScale[0] later */         move32();
      nrgTotal = L_add(nrgTotal, accu);
    }
  }
  nrgTotal = L_shr(nrgTotal, yBufferScale[0]);
  
  return(nrgTotal);
}


/*******************************************************************************
 Functionname:  frameSplitter
 *******************************************************************************
 \author  Klaus Peichl
 \brief   Decides if a FIXFIX-frame shall be splitted into 2 envelopes
 
 If no transient has been detected before, the frame can still be splitted
 into 2 envelopes.
*******************************************************************************/
void
frameSplitter(Word32 **Energies,
              Word16 *yBufferScale,
              HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector,
              UWord16 * freqBandTable,
              Word16 nSfb,
              Word16 timeStep,
              Word16 no_cols,
              Word16 *tran_vector,
              Word32 **EnergiesM)
{
  test();
  if (tran_vector[1]==0) { /* no transient was detected */
    Word32 delta;
    Word32 EnergyTotal, newLowbandEnergy;
    Word16 border;
    Word16 sbrSlots = ffr_Short_Div(no_cols, timeStep);

    assert( sbrSlots * timeStep == no_cols );

    /*
      Get Lowband-energy over a range of 2 frames (Look half a frame back and ahead).
    */
    newLowbandEnergy = addLowbandEnergies(Energies,
                                          yBufferScale,
                                          freqBandTable,
                                          no_cols);

    /* prevLowBandEnergy: Corresponds to 1 frame, starting with half a frame look-behind
       newLowbandEnergy:  Corresponds to 1 frame, starting in the middle of the current frame */
    EnergyTotal = L_shr(L_add(newLowbandEnergy, h_sbrTransientDetector->prevLowBandEnergy), 1);

    EnergyTotal = L_add(EnergyTotal, addHighbandEnergies(Energies,
                                                         yBufferScale,
                                                         EnergiesM,
                                                         freqBandTable,
                                                         nSfb,
                                                         sbrSlots,
                                                         timeStep));

  
    /* The below border should specify the same position as the middle border
       of a FIXFIX-frame with 2 envelopes. */
    
    border = shr(add(sbrSlots, 1), 1);

    delta = spectralChange( EnergiesM,
                            yBufferScale,
                            EnergyTotal,
                            nSfb,
                            0,
                            border,
                            sbrSlots);

    test(); L_sub(1, 1);
    if (delta > h_sbrTransientDetector->split_thr) {
      tran_vector[0] = 1; /* Set flag for splitting */                              move16();
    }
    else {
      tran_vector[0] = 0;                                                           move16();
    }

    /* Update prevLowBandEnergy */
    /*! \todo  Could also be done later, if the variable is of use somewhere else */
    h_sbrTransientDetector->prevLowBandEnergy = newLowbandEnergy;                   move32();
  }
}



static void
calculateThresholds (Word32 **Energies,
                     Word32 absThreshold,
                     Word16 *yBufferScale,
                     Word16 noRows,
                     Word32 *thresholds)
{
  Word32 i, j;
  Word16 scf0,scf1,scftmp;
  Word16 minScale;
  Word32 accu1, accu2;    
  Word32 std_val, accu3;
  Word16 mean_val, temp;
  Word16 i_noCols;
  Word16 i_noCols1;

  i_noCols = 0x2aa;                                                                                move16();
  i_noCols1 = 0x2b9;                                                                               move16();
   
  /*
    determinate min scale factor
  */

  minScale = S_min(yBufferScale[0],yBufferScale[1]);
  
  scf0 = sub(yBufferScale[0], minScale);
  scf1 = sub(yBufferScale[1], minScale);

  /* calculate standard deviation in every subband */
  for (i = 0; i < noRows; i++)
    {
      Word32 maxEnergy = 0;
      Word16 energyShift;

      move32();
    
      for (j = 16; j < 64; j++) {
        maxEnergy = L_max(maxEnergy,Energies[j>>1][i]);
      }
      energyShift = ffr_norm32(maxEnergy);

      accu1 = 0;                                                                                   move32();
      accu2 = 0;                                                                                   move32();

      for (j = 16; j < 32; j++) {
        accu1 = L_mac(accu1, etsiopround(L_shl(Energies[j>>1][i], energyShift)), i_noCols);
      }
    
      for (j = 32; j < 64; j++) {
        accu2 = L_mac(accu2, etsiopround(L_shl(Energies[j>>1][i], energyShift)), i_noCols);
      }
    
      mean_val = add(shr(etsiopround(accu1), scf0), shr(etsiopround(accu2), scf1));   /* mean_val now scaled by energyShift */
    
      accu3 = 0;                                                                                   move32();

      scftmp = sub(energyShift, scf0);
      for (j = 16; j < 32; j++) {
        temp = sub(mean_val, etsiopround(L_shl(Energies[j>>1][i], scftmp)));
        accu3 = L_mac(accu3, mult(temp, temp), i_noCols1);
      }
    
      scftmp = sub(energyShift, scf1);
      for (j = 32; j < 64; j++) {
        temp = sub(mean_val, etsiopround(L_shl(Energies[j>>1][i], scftmp)));
        accu3 = L_mac(accu3, mult(temp, temp), i_noCols1);
      }
    
      std_val = ffr_sqrt(accu3,INT_BITS);
    
      std_val = L_shr(std_val, S_min(add(energyShift, minScale),INT_BITS-1));
      assert(energyShift+minScale >= 0);
    
      /*
        Take new threshold as average of calculated standard deviation ratio
        and old threshold if greater than absolute threshold 
      */
   
      accu3 = L_add(fixmul_32x16b(thresholds[i], 0x547b), fixmul_32x16b(std_val, 0x2b85));

      thresholds[i] = L_max(absThreshold, accu3);                                                    move32();    
    }  
}


static void
extractTransientCandidates (Word32 **Energies,
                            Word16 *yBufferScale,
                            Word16 noCols,
                            Word16 start_band,
                            Word16 stop_band,
                            Word32 *thresholds,
                            Word32 *transients,
                            Word16 bufferLength)
                            
{
  Word16 i, j;
  Word16 minScale;
  Word32 eminus03,eminus21,eplus01,eplus23,eplusnew;
  
  Word32 delta_1, delta_2, delta_3;
  Word16 bufferMove = shr(bufferLength, 1);
  Word16  ridx = (32/4)-2;
  Word16  ilcnt=32+32/2 - 4; 
  Word16  temp = 32/2 + 4;
  Word16  shift1,shift2;
  Word16 ithrtmp;

  Word32 *ptransients = transients+bufferMove;

  move16(); move16(); move16(); 

  memmove(transients, transients + noCols, bufferMove * sizeof (Word32));                   memop32(bufferMove);
  memset (transients + bufferMove, 0, (bufferLength-bufferMove) * sizeof (Word32));         memop32(bufferLength-bufferMove);

  /*
    determinate min scale factor of both partitions
  */
  minScale = sub(S_min(yBufferScale[0],yBufferScale[1]), 2);
  shift1 = sub(yBufferScale[0], minScale);
  shift2 = sub(yBufferScale[1], minScale);

  /*  Compute differential values with two different weightings in every subband */
  for (i = start_band; i < stop_band; i++) {
    Word32 diffValue;
    Word32 threshold = thresholds[i];
    Word16 divShift = ffr_norm32(threshold);
    Word16 iThresholdShift = S_min(add(sub(INT_BITS-2, divShift), minScale), INT_BITS-1); /* INT_BITS-divShift-2 : compensation for divShift
                                                                                                 minScale :compensation for delta scale */  
    Word32 iThreshold = L_shr(0x40000000, divShift);


    test(); L_sub(1, 1);
    if(iThreshold == threshold) {
      iThreshold = 0x7fffffff;                                               move32();
    }
    else {
      Word16 denom_h, denom_l;
      L_Extract(L_shl(threshold, divShift), &denom_h, &denom_l);
      iThreshold = Div_32(0x40000000, denom_h, denom_l);
    }

    ithrtmp = etsiopround(iThreshold);

    eminus03 = L_shr(Energies[ridx][i],   shift1);
    eminus21 = L_shr(Energies[ridx+1][i], shift1);
    eplus01  = L_shr(Energies[ridx+2][i], shift1);
    eplus23  = L_shr(Energies[ridx+3][i], shift1);
  
    {	
      Word32 temp2;

      for (j = 0; j < 12; j+=2) {
        delta_1   = L_sub( eplus01, eminus21 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_1), ithrtmp), iThresholdShift),1);
        
        temp2 = ptransients[j];                                                                     move32();

        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_2   = L_add( L_sub( eplus23, eminus21 ), delta_1 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_2), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_3 = L_add( L_sub( eplus23, eminus03 ), delta_2 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_3), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        ptransients[j] = temp2;                                                                      move32();

        eplusnew  = L_shr(Energies[(j + temp)/2][i], shift1);

        delta_1 = L_sub( eplus23, eplus01 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_1), ithrtmp), iThresholdShift),1);

        temp2 = ptransients[j+1];                                                                    move32();
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_2 = L_add( L_sub( eplus23, eminus21 ), delta_1 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_2), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_3 = L_add( L_sub( eplusnew, eminus21 ), delta_2 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_3), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        ptransients[j+1] = temp2;                                                                    move32();

        eminus03=eminus21;                                                                           move32();
        eminus21= eplus01;                                                                           move32();
        eplus01 = eplus23;                                                                           move32();
        eplus23 = eplusnew;                                                                          move32();

      }

      for (; j < ilcnt; j+=2) {
        delta_1 = L_sub( eplus01, eminus21 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_1), ithrtmp), iThresholdShift),1);
        
        temp2 = ptransients[j];                                                                      move32();

        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_2 = L_add( L_sub( eplus23, eminus21 ), delta_1 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_2), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_3 = L_add( L_sub( eplus23, eminus03 ), delta_2 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_3), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        ptransients[j] = temp2;                                                                      move32();

        eplusnew  = L_shr(Energies[(j + temp)/2][i], shift2);

        delta_1 = L_sub( eplus23, eplus01 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_1), ithrtmp), iThresholdShift),1);

        temp2 = ptransients[j+1];                                                                    move32();
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_2 = L_add( L_sub( eplus23, eminus21 ), delta_1 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_2), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_3 = L_add( L_sub( eplusnew, eminus21 ), delta_2 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_3), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        ptransients[j+1] = temp2;                                                                    move32();

        eminus03=eminus21;                                                                           move32();
        eminus21= eplus01;                                                                           move32();
        eplus01 = eplus23;                                                                           move32();
        eplus23 = eplusnew;                                                                          move32();

      }

      {
        delta_1 = L_sub( eplus01, eminus21 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_1), ithrtmp), iThresholdShift),1);
      
        temp2 = ptransients[j];                                                                      move32();
	  
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_2 = L_add( L_sub( eplus23, eminus21 ), delta_1 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_2), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        delta_3 = L_add( L_sub( eplus23, eminus03 ), delta_2 );
        diffValue = L_sub(L_shr(L_mult(etsiopround(delta_3), ithrtmp), iThresholdShift),1);
        
        test();
        if(diffValue > 0)
          temp2 = L_add(temp2, diffValue);

        ptransients[j] = temp2;                                                                      move32();


      }

    }

  }
}


void
transientDetect (Word32 **Energies,
                 Word16 *yBufferScale,
                 HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTran,
                 Word16 *tran_vector,
                 Word16 timeStep,
                 Word16 frameMiddleBorder)
{
  
  Word16 i;
  Word32 cond;
  Word16 no_cols = h_sbrTran->no_cols;
  Word16 qmfStartSample = add(no_cols, ffr_Short_Mult(timeStep, frameMiddleBorder));
  
  calculateThresholds (Energies,
                       h_sbrTran->abs_thr,
                       yBufferScale,
                       h_sbrTran->no_rows,
                       h_sbrTran->thresholds);
  
  extractTransientCandidates (Energies,
                              yBufferScale,
                              h_sbrTran->no_cols,
                              0, 
                              h_sbrTran->no_rows,
                              h_sbrTran->thresholds,
                              h_sbrTran->transients,
                              h_sbrTran->buffer_length);
                              
  tran_vector[0] = 0;                                                               move16();
  tran_vector[1] = 0;                                                               move16();
  
  /* Check for transients in second granule (pick the last value of subsequent values)  */
  for (i = qmfStartSample; i < add(qmfStartSample, no_cols); i++){
    test(); test(); L_sub(1,1); L_sub(1,1);
    cond = (h_sbrTran->transients[i] < fixmul(0x73333333, h_sbrTran->transients[i - 1])) &&
      (h_sbrTran->transients[i - 1] > h_sbrTran->tran_thr);
    if (cond) {
      tran_vector[0] = ffr_Short_Div(sub(i, qmfStartSample), timeStep);
      tran_vector[1] = 1;                                                           move16();
      break;
    }
  }
} 





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
                            )
{
  
  Word16 temp,frame_dur;

  memset(h_sbrTransientDetector,0,sizeof(SBR_TRANSIENT_DETECTOR));       memop16(sizeof(SBR_TRANSIENT_DETECTOR)/sizeof(Word16));
 
  
  frame_dur = extract_l(ffr_Integer_Div((2048*10000), sampleFreq));

  /* The longer the frames, the more often should the FIXFIX-
     case transmit 2 envelopes instead of 1.
     Frame durations below 10 ms produce the highest threshold
     so that practically always only 1 env is transmitted. */
  temp = sub(frame_dur, 100);

  test(); sub(1, 1);
  if (temp < 1)
    temp = 1;

  temp = extract_l(ffr_Integer_Div(7500000, L_shr(L_mult(temp, temp), 1)));

  test();
  if(codecBitrate) {
    temp = extract_l(ffr_Integer_Div(ffr_Integer_Mult(temp, totalBitrate), codecBitrate));
  }                       

 
  assert(no_cols == QMF_TIME_SLOTS);
  assert(no_rows <= QMF_CHANNELS);
  
  h_sbrTransientDetector->no_cols  = no_cols;                                       move16();
  h_sbrTransientDetector->tran_thr = ffr_Integer_Div(tran_thr, no_rows);            move32();
  h_sbrTransientDetector->tran_fc = tran_fc;                                        move16();
  h_sbrTransientDetector->split_thr = temp;                                         move32();
  h_sbrTransientDetector->buffer_length = shr(extract_l(L_mult(3, h_sbrTransientDetector->no_cols)), 1);      move16();
  h_sbrTransientDetector->no_rows = no_rows;                                        move16();
  h_sbrTransientDetector->mode = mode;                                              move16();
  h_sbrTransientDetector->prevLowBandEnergy = 0;                                    move32();
  h_sbrTransientDetector->thresholds = &sbr_thresholds[chan*QMF_CHANNELS];          move32();
  h_sbrTransientDetector->abs_thr = 0x00000010;                                     move32();
  h_sbrTransientDetector->abs_thr = L_max((Word32)h_sbrTransientDetector->abs_thr,1); move32();
  
  memset(h_sbrTransientDetector->thresholds,0,sizeof(Word32)*QMF_CHANNELS);                             memop32(QMF_CHANNELS);

  h_sbrTransientDetector->transients =  &sbr_transients[chan*h_sbrTransientDetector->buffer_length];    move32();
  
  memset(h_sbrTransientDetector->transients,0,sizeof(Word32)*h_sbrTransientDetector->buffer_length);    memop32(h_sbrTransientDetector->buffer_length);
  
  return (0);
}



void
deleteSbrTransientDetector (HANDLE_SBR_TRANSIENT_DETECTOR h_sbrTransientDetector)
{
  if(h_sbrTransientDetector){
    /*
      nothing to do
    */

  }

  
}
