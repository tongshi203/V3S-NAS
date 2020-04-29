/*
   stereo pre-processing
 */
#ifndef __STPREPRO_H
#define __STPREPRO_H

struct STEREO_PREPRO;
typedef struct STEREO_PREPRO *HANDLE_STEREO_PREPRO;

#include "interface.h"
#include "channel_map.h"

struct STEREO_PREPRO {

  Word16 normPeFac;              /*! factor to normalize input PE, depends on bitrate and bandwidth */
  Word16 stereoAttenuationInc;   /*! att. increment parameter */
  Word16 stereoAttenuationDec;   /*! att. decrement parameter */

  Word32 avrgFreqEnergyL;        /*! energy left */
  Word32 avrgFreqEnergyR;        /*! energy right */
  Word32 avrgFreqEnergyM;        /*! energy mid */
  Word32 avrgFreqEnergyS;        /*! energy side */
  Word16 smoothedPeSumSum;       /*! time-smoothed PE */
  Word16 avgStoM;                /*! time-smoothed energy ratio S/M [dB] */
  Word16 lastLtoR;               /*! previous frame energy ratio L/R [dB] */
  Word32 lastNrgLR;              /*! previous frame energy L+R */

  Word16 ImpactFactor;           /*! bitrate dependent parameter */
  Word16 stereoAttenuation;      /*! the actual attenuation of this frame */
  Word32 stereoAttFac;           /*! the actual attenuation factor of this frame */

  /* tuning parameters that are not varied from frame to frame but initialized at init */
  Flag   stereoAttenuationFlag;  /*! flag to indicate usage */
  Word16 ConstAtt;               /*! if not zero, a constant att. will be applied [dB]*/
  Word16 stereoAttMax;           /*! the max. attenuation [dB]*/

  Word16 LRMin;                  /*! tuning parameter [dB] */
  Word16 LRMax;                  /*! tuning parameter [dB] */
  Word16 SMMin;                  /*! tuning parameter [dB] */
  Word16 SMMax;                  /*! tuning parameter [dB] */
 
  Word16 PeMin;                  /*! tuning parameter */
  Word16 PeCrit;                 /*! tuning parameter */
  Word32 PeImpactMax;            /*! tuning parameter */
};

Word16 InitStereoPreProcessing(HANDLE_STEREO_PREPRO hStPrePro, 
                               Word16 nChannels, 
                               Word32 bitRate, 
                               Word32 sampleRate,
                               Word16 usedScfRatio);

void ApplyStereoPreProcess(HANDLE_STEREO_PREPRO hStPrePro,
                           Word16               nChannels, /*! total number of channels */              
                           ELEMENT_INFO        *elemInfo,
                           Word16              *timeData,
                           Word16               granuleLen);

void UpdateStereoPreProcess(PSY_OUT_CHANNEL psyOutChan[MAX_CHANNELS], 
                            QC_OUT_ELEMENT *qcOutElement,
                            HANDLE_STEREO_PREPRO hStPrePro,
                            Word16 weightPeFac);

#endif
