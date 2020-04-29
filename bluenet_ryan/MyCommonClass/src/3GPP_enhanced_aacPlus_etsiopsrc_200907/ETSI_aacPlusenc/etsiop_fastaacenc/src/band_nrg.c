/*
   Band/Line energy calculations
 */
#include "band_nrg.h"
#include "count.h"


void CalcBandEnergy(const Word32 *mdctSpectrum,
                    const Word16 *bandOffset,
                    const Word16  numBands,
                    Word32       *bandEnergy,
                    Word32       *bandEnergySum)
{
  Word16 i, j;
  Word32 accuSum = 0;                                           move32();

  for (i=0; i<numBands; i++) {
    Word32 accu = 0;                                            move32();
    for (j=bandOffset[i]; j<bandOffset[i+1]; j++)
      accu = L_add(accu, fixmul(mdctSpectrum[j], mdctSpectrum[j]));
    accuSum = L_add(accuSum, accu);
    bandEnergy[i] = accu;                                       move32();
  }
  *bandEnergySum = accuSum;                                     move32();
}



void CalcBandEnergyMS(const Word32 *mdctSpectrumLeft,
                      const Word32 *mdctSpectrumRight,
                      const Word16 *bandOffset,
                      const Word16  numBands,
                      Word32       *bandEnergyMid,
                      Word32       *bandEnergyMidSum,
                      Word32       *bandEnergySide,
                      Word32       *bandEnergySideSum)
{

  Word16 i, j;
  Word32 accuMidSum = 0;        
  Word32 accuSideSum = 0;                                       move32(); move32();
 

  for(i=0; i<numBands; i++) {
    Word32 accuMid = 0;
    Word32 accuSide = 0;                                        move32(); move32();
    for (j=bandOffset[i]; j<bandOffset[i+1]; j++) {
      Word32 specm, specs; 
      Word32 l, r;

      l = L_shr(mdctSpectrumLeft[j], 1);
      r = L_shr(mdctSpectrumRight[j], 1);
      specm = L_add(l, r);
      specs = L_sub(l, r);
      accuMid = L_add(accuMid, fixmul(specm, specm));
      accuSide = L_add(accuSide, fixmul(specs, specs));
    }
    bandEnergyMid[i] = accuMid;                                 move32();
    accuMidSum = L_add(accuMidSum, accuMid);
    bandEnergySide[i] = accuSide;                               move32();
    accuSideSum = L_add(accuSideSum, accuSide);
    
  }
  *bandEnergyMidSum = accuMidSum;                               move32();
  *bandEnergySideSum = accuSideSum;                             move32();
}

