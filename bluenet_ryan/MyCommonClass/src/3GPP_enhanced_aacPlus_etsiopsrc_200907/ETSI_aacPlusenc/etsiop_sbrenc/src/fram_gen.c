/*
   Framing generator
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fram_gen.h"
#include "sbr_misc.h"

#include "count.h"


static SBR_FRAME_INFO frameInfo1_2048 = {1,
                                         {0 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048},
                                         {FREQ_RES_HIGH},
                                         0,
                                         1,
                                         {0 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048}};

static SBR_FRAME_INFO frameInfo2_2048 = {2,
                                         {0 + BUFFER_FRAME_START_2048,
                                          8 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048},
                                         {FREQ_RES_HIGH,
                                          FREQ_RES_HIGH},
                                         0,
                                         2,
                                         {0 + BUFFER_FRAME_START_2048,
                                          8 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048}};

static SBR_FRAME_INFO frameInfo4_2048 = {4,
                                         {0 + BUFFER_FRAME_START_2048,
                                          4 + BUFFER_FRAME_START_2048,
                                          8 + BUFFER_FRAME_START_2048,
                                          12 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048},
                                         {FREQ_RES_HIGH,
                                          FREQ_RES_HIGH,
                                          FREQ_RES_HIGH,
                                          FREQ_RES_HIGH},
                                         0,
                                         2,
                                         {0 + BUFFER_FRAME_START_2048,
                                          8 + BUFFER_FRAME_START_2048,
                                          16 + BUFFER_FRAME_START_2048}};




static void fillFrameTran (Word16 *v_bord, Word16 *length_v_bord, Word16 *v_freq,
                           Word16 *length_v_freq, Word16 *bmin, Word16 *bmax, Word16 tran,
                           Word16 *v_tuningSegm, Word16 *v_tuningFreq);
static void fillFramePre (Word16 dmax, Word16 *v_bord, Word16 *length_v_bord,
                          Word16 *v_freq, Word16 *length_v_freq, Word16 bmin,
                          Word16 rest);
static void fillFramePost (Word16 *parts, Word16 *d, Word16 dmax, Word16 *v_bord,
                           Word16 *length_v_bord, Word16 *v_freq,
                           Word16 *length_v_freq, Word16 bmax,
                           Word16 bufferFrameStart, Word16 numberTimeSlots, Word16 fmax);
static void fillFrameInter (Word16 *nL, Word16 *v_tuningSegm, Word16 *v_bord,
                            Word16 *length_v_bord, Word16 bmin, Word16 *v_freq,
                            Word16 *length_v_freq, Word16 *v_bordFollow,
                            Word16 *length_v_bordFollow, Word16 *v_freqFollow,
                            Word16 *length_v_freqFollow, Word16 i_fillFollow,
                            Word16 dmin, Word16 dmax);
static void calcFrameClass (FRAME_CLASS *frameClass, FRAME_CLASS *frameClassOld, Word16 tranFlag,
                            Word16 *spreadFlag);
static void specialCase (Word16 *spreadFlag, Word16 allowSpread, Word16 *v_bord,
                         Word16 *length_v_bord, Word16 *v_freq, Word16 *length_v_freq,
                         Word16 *parts, Word16 d);
static void calcCmonBorder (Word16 *i_cmon, Word16 *i_tran, Word16 *v_bord,
                            Word16 *length_v_bord, Word16 tran,
                            Word16 bufferFrameStart, Word16 numberTimeSlots); 
static void keepForFollowUp (Word16 *v_bordFollow, Word16 *length_v_bordFollow,
                             Word16 *v_freqFollow, Word16 *length_v_freqFollow,
                             Word16 *i_tranFollow, Word16 *i_fillFollow,
                             Word16 *v_bord, Word16 *length_v_bord, Word16 *v_freq,
                             Word16 i_cmon, Word16 i_tran, Word16 parts, Word16 numberTimeSlots);  
static void calcCtrlSignal (HANDLE_SBR_GRID hSbrGrid, FRAME_CLASS frameClass,
                            Word16 *v_bord, Word16 length_v_bord, Word16 *v_freq,
                            Word16 length_v_freq, Word16 i_cmon, Word16 i_tran,
                            Word16 spreadFlag, Word16 nL);
static void ctrlSignal2FrameInfo (HANDLE_SBR_GRID hSbrGrid,
                                  HANDLE_SBR_FRAME_INFO hFrameInfo,
                                  Word16 freq_res_fixfix);

/***************************************************************************/
/*!
  \brief    Produces the FRAME_INFO struct for the current frame

  \return   The frame info handle for the current frame

****************************************************************************/
HANDLE_SBR_FRAME_INFO
frameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                    Word16 *v_transient_info,
                    Word16 *v_tuning)
{
  Word16 numEnv, tran=0, bmin=0, bmax=0, parts, d, i_cmon, i_tran, nL;
  Word16 fmax = 0;

  Word16 *v_bord = hSbrEnvFrame->v_bord;
  Word16 *v_freq = hSbrEnvFrame->v_freq;
  Word16 *v_bordFollow = hSbrEnvFrame->v_bordFollow;
  Word16 *v_freqFollow = hSbrEnvFrame->v_freqFollow;


  Word16 *length_v_bordFollow = &hSbrEnvFrame->length_v_bordFollow;
  Word16 *length_v_freqFollow = &hSbrEnvFrame->length_v_freqFollow;
  Word16 *length_v_bord = &hSbrEnvFrame->length_v_bord;
  Word16 *length_v_freq = &hSbrEnvFrame->length_v_freq;
  Word16 *spreadFlag = &hSbrEnvFrame->spreadFlag;
  Word16 *i_tranFollow = &hSbrEnvFrame->i_tranFollow;
  Word16 *i_fillFollow = &hSbrEnvFrame->i_fillFollow;
  FRAME_CLASS *frameClassOld = &hSbrEnvFrame->frameClassOld;
  FRAME_CLASS frameClass;


  Word16 allowSpread = hSbrEnvFrame->allowSpread;
  Word16 numEnvStatic = hSbrEnvFrame->numEnvStatic;
  Word16 staticFraming = hSbrEnvFrame->staticFraming;
  Word16 dmin = hSbrEnvFrame->dmin;
  Word16 dmax = hSbrEnvFrame->dmax;

  Word16 bufferFrameStart = hSbrEnvFrame->SbrGrid.bufferFrameStart;
  Word16 numberTimeSlots = hSbrEnvFrame->SbrGrid.numberTimeSlots;
  Word16 frameMiddleSlot = hSbrEnvFrame->frameMiddleSlot;

  Word16 tranPos = v_transient_info[0];
  Word16 tranFlag = v_transient_info[1];

  Word16 *v_tuningSegm = v_tuning;
  Word16 *v_tuningFreq = (Word16*)L_add((Word32)v_tuning, 3*sizeof(*v_tuningFreq));

  Word16 freq_res_fixfix = hSbrEnvFrame->freq_res_fixfix;

  /*--------------------------------------------------------------------------
    T/F-grid and control signal generator 
    ---------------------------------------------------------------------------*/
  test();
  if (staticFraming) {
    /*--------------------------------------------------------------------------
      Ignore transient detector
      ---------------------------------------------------------------------------*/

    frameClass = FIXFIX;                                                                             move16();
    numEnv = numEnvStatic;      /* {1,2,4,8} */                                                      move16();
    *frameClassOld = FIXFIX;    /* for change to dyn */                                              move16();
    hSbrEnvFrame->SbrGrid.bs_num_env = numEnv;                                                       move16();
    hSbrEnvFrame->SbrGrid.frameClass = frameClass;                                                   move16();
  }
  else {
    /*--------------------------------------------------------------------------
      Calculate frame class to use
      ---------------------------------------------------------------------------*/
    calcFrameClass (&frameClass, frameClassOld, tranFlag, spreadFlag);

    /*--------------------------------------------------------------------------
      Design frame (or follow-up old design) 
      ---------------------------------------------------------------------------*/
    test();
    if (tranFlag) { 
      /*
        calculate transient position within envelope buffer
      */
      test(); sub(1, 1);
      if (tranPos < 4) {
        fmax = 6;     move16();
      }
      else {
        test(); test(); sub(1, 1); sub(1, 1);
        if (tranPos == 4 || tranPos == 5) {
          fmax = 4;     move16();
        }
        else {
          fmax = 8;     move16();
        }
      }

      tran = add(add(tranPos, bufferFrameStart), frameMiddleSlot);
      /*
        add mandatory borders aetsiopround transient
      */
      fillFrameTran (v_bord, length_v_bord, v_freq, length_v_freq, &bmin,
                     &bmax, tran, v_tuningSegm, v_tuningFreq);
    }

    test();
    switch (frameClass) {
    case FIXVAR:
      /*--------------------------------------------------------------------------
        Fill region before new transient:
        ---------------------------------------------------------------------------*/
      fillFramePre (dmax, v_bord, length_v_bord, v_freq, length_v_freq,
                    bmin, bmin - bufferFrameStart);


      /*--------------------------------------------------------------------------
        Fill region after new transient:
        ---------------------------------------------------------------------------*/
      fillFramePost (&parts, &d, dmax, v_bord, length_v_bord, v_freq,
                     length_v_freq, bmax, bufferFrameStart, numberTimeSlots, fmax);

      /*--------------------------------------------------------------------------
        Take care of special case:
        ---------------------------------------------------------------------------*/
      test(); test(); sub(1, 1); sub(1, 1);
      if (parts == 1 && d < dmin)       /* no fill, short last envelope */
        specialCase (spreadFlag, allowSpread, v_bord, length_v_bord,
                     v_freq, length_v_freq, &parts, d);


      /*--------------------------------------------------------------------------
        Calculate common border (split-point)
        ---------------------------------------------------------------------------*/
      calcCmonBorder (&i_cmon, &i_tran, v_bord, length_v_bord, tran,
                      bufferFrameStart, numberTimeSlots);       /* FH 00-06-26 */


      /*--------------------------------------------------------------------------
        Extract data for proper follow-up in next frame
        ---------------------------------------------------------------------------*/
      keepForFollowUp (v_bordFollow, length_v_bordFollow, v_freqFollow,
                       length_v_freqFollow, i_tranFollow, i_fillFollow,
                       v_bord, length_v_bord, v_freq, i_cmon, i_tran, parts, numberTimeSlots);  /* FH 00-06-26 */

      /*--------------------------------------------------------------------------
        Calculate control signal
        ---------------------------------------------------------------------------*/
      calcCtrlSignal (&hSbrEnvFrame->SbrGrid, frameClass,
                      v_bord, *length_v_bord, v_freq, *length_v_freq,
                      i_cmon, i_tran, *spreadFlag, DC);
      break;
    case VARFIX:
      /*--------------------------------------------------------------------------
        Follow-up old transient - calculate control signal
        ---------------------------------------------------------------------------*/
      calcCtrlSignal (&hSbrEnvFrame->SbrGrid, frameClass,
                      v_bordFollow, *length_v_bordFollow, v_freqFollow,
                      *length_v_freqFollow, DC, *i_tranFollow,
                      *spreadFlag, DC);
      break;
    case VARVAR:
      test();
      if (*spreadFlag) {
        /*--------------------------------------------------------------------------
          Follow-up old transient - calculate control signal
          ---------------------------------------------------------------------------*/
        calcCtrlSignal (&hSbrEnvFrame->SbrGrid,
                        frameClass, v_bordFollow, *length_v_bordFollow,
                        v_freqFollow, *length_v_freqFollow, DC,
                        *i_tranFollow, *spreadFlag, DC);

        *spreadFlag = 0;                                                                                       move16();

        /*--------------------------------------------------------------------------
          Extract data for proper follow-up in next frame
          ---------------------------------------------------------------------------*/
        v_bordFollow[0] = sub(hSbrEnvFrame->SbrGrid.bs_abs_bord_1, numberTimeSlots);                           move16();
        v_freqFollow[0] = 1;                                                                                   move16();
        *length_v_bordFollow = 1;                                                                              move16();
        *length_v_freqFollow = 1;                                                                              move16();

        *i_tranFollow = -DC;                                                                                   move16();
        *i_fillFollow = -DC;                                                                                   move16();
      }
      else {
        /*--------------------------------------------------------------------------
          Design remaining parts of T/F-grid (assuming next frame is VarFix)
          adapt or fill region before new transient:
          ---------------------------------------------------------------------------*/
        fillFrameInter (&nL, v_tuningSegm, v_bord, length_v_bord, bmin,
                        v_freq, length_v_freq, v_bordFollow,
                        length_v_bordFollow, v_freqFollow,
                        length_v_freqFollow, *i_fillFollow, dmin, dmax);

        /*--------------------------------------------------------------------------
          Fill after transient:
          ---------------------------------------------------------------------------*/
        fillFramePost (&parts, &d, dmax, v_bord, length_v_bord, v_freq,
                       length_v_freq, bmax, bufferFrameStart, numberTimeSlots, fmax);

        /*--------------------------------------------------------------------------
          Take care of special case:
          ---------------------------------------------------------------------------*/
        test(); test(); sub(1, 1); sub(1, 1);
        if (parts == 1 && d < dmin)     /*% no fill, short last envelope */
          specialCase (spreadFlag, allowSpread, v_bord, length_v_bord,
                       v_freq, length_v_freq, &parts, d);

        /*--------------------------------------------------------------------------
          Calculate common border (split-point)
          ---------------------------------------------------------------------------*/
        calcCmonBorder (&i_cmon, &i_tran, v_bord, length_v_bord, tran,
                        bufferFrameStart, numberTimeSlots);     

        /*--------------------------------------------------------------------------
          Extract data for proper follow-up in next frame
          ---------------------------------------------------------------------------*/
        keepForFollowUp (v_bordFollow, length_v_bordFollow,
                         v_freqFollow, length_v_freqFollow,
                         i_tranFollow, i_fillFollow, v_bord,
                         length_v_bord, v_freq, i_cmon, i_tran, parts, numberTimeSlots);       

        /*--------------------------------------------------------------------------
          Calculate control signal
          ---------------------------------------------------------------------------*/
        calcCtrlSignal (&hSbrEnvFrame->SbrGrid,
                        frameClass, v_bord, *length_v_bord, v_freq,
                        *length_v_freq, i_cmon, i_tran, 0, nL);
      }
      break;
    case FIXFIX:
      test();
      if (tranPos == 0) {
        numEnv = 1;
      }
      else {
        numEnv = 2;
      }

      hSbrEnvFrame->SbrGrid.bs_num_env = numEnv;                                                               move16();
      hSbrEnvFrame->SbrGrid.frameClass = frameClass;                                                           move16();

      break;
    default:
      assert(0);
    }
  }




  /*-------------------------------------------------------------------------
    Convert control signal to frame info struct
    ---------------------------------------------------------------------------*/
  ctrlSignal2FrameInfo (&hSbrEnvFrame->SbrGrid,
                        &hSbrEnvFrame->SbrFrameInfo,
                        freq_res_fixfix);

  return &hSbrEnvFrame->SbrFrameInfo;
}


/*******************************************************************************
 Functionname:  createFrameInfoGenerator
 *******************************************************************************

 Description: 

 Arguments:   hSbrEnvFrame  - pointer to sbr envelope handle
              allowSpread   - commandline parameter
              numEnvStatic  - commandline parameter
              staticFraming - commandline parameter

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:     FH 00-06-26
*******************************************************************************/
void
createFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME  hSbrEnvFrame,
                          Word16 allowSpread,
                          Word16 numEnvStatic,
                          Word16 staticFraming,
                          Word16 timeSlots,
                          Word16 freq_res_fixfix)

{                               /* FH 00-06-26 */
  hSbrEnvFrame->allowSpread = allowSpread;                                              move16();
  hSbrEnvFrame->numEnvStatic = numEnvStatic;                                            move16();
  hSbrEnvFrame->staticFraming = staticFraming;                                          move16();
  hSbrEnvFrame->freq_res_fixfix = freq_res_fixfix;                                      move16();

  hSbrEnvFrame->SbrGrid.numberTimeSlots = timeSlots;                                    move16();

  hSbrEnvFrame->dmin = 4;                                                               move16();
  hSbrEnvFrame->dmax = 12;                                                              move16();
  hSbrEnvFrame->SbrGrid.bufferFrameStart = BUFFER_FRAME_START_2048;                     move16();
  hSbrEnvFrame->frameMiddleSlot = FRAME_MIDDLE_SLOT_2048;                               move16();
}


/*******************************************************************************
 Functionname:  deleteFrameInfoGenerator
 *******************************************************************************

 Description:  

 Arguments:   hSbrEnvFrame  - HANDLE_SBR_ENVELOPE_FRAME
              
 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
void
deleteFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame)
{

  /*
    nothing to do
  */
  (void)hSbrEnvFrame;
}



/*******************************************************************************
 Functionname:  fillFrameTran
 *******************************************************************************

 Description:  Add mandatory borders, as described by the tuning vector 
               and the current transient position

 Arguments:   
      modified:
              v_bord        - Word16 pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - Word16 pointer to v_freq vector
              length_v_freq - length of v_freq vector
              bmin          - Word16 pointer to bmin (call by reference)
              bmax          - Word16 pointer to bmax (call by reference)
      not modified:
              tran          - position of transient 
              v_tuningSegm  - Word16 pointer to v_tuningSegm vector
              v_tuningFreq  - Word16 pointer to v_tuningFreq vector

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
fillFrameTran (Word16 *v_bord, Word16 *length_v_bord,
               Word16 *v_freq, Word16 *length_v_freq,
               Word16 *bmin, Word16 *bmax,
               Word16 tran,
               Word16 *v_tuningSegm, Word16 *v_tuningFreq)
{
  Word16 bord, i;

  *length_v_bord = 0;                                                                                move16();
  *length_v_freq = 0;                                                                                move16();

  /* add attack env leading border (optional) */
  test();
  if (v_tuningSegm[0]) {
    /* v_bord = [(Ba)] start of attack env */
    AddRight (v_bord, length_v_bord, (tran - v_tuningSegm[0]));
    
    /* v_freq = [(Fa)] res of attack env */
    AddRight (v_freq, length_v_freq, v_tuningFreq[0]);
  }

  /* add attack env trailing border/first decay env leading border */
  bord = tran;    move16();
  AddRight (v_bord, length_v_bord, tran);   /* v_bord = [(Ba),Bd1] */

  /* add first decay env trailing border/2:nd decay env leading border */
  test();
  if (v_tuningSegm[1]) {
    bord = add(bord, v_tuningSegm[1]);
    
    /* v_bord = [(Ba),Bd1,Bd2] */
    AddRight (v_bord, length_v_bord, bord);
    
    /* v_freq = [(Fa),Fd1] */
    AddRight (v_freq, length_v_freq, v_tuningFreq[1]);
  }

  /* add 2:nd decay env trailing border (optional) */
  test();
  if (v_tuningSegm[2] != 0) {
    bord = add(bord, v_tuningSegm[2]);
    
    /* v_bord = [(Ba),Bd1, Bd2,(Bd3)] */
    AddRight (v_bord, length_v_bord, bord);
    
    /* v_freq = [(Fa),Fd1,(Fd2)] */
    AddRight (v_freq, length_v_freq, v_tuningFreq[2]);
  }

  /*  v_freq = [(Fa),Fd1,(Fd2),1] what is this for ???? */
  AddRight (v_freq, length_v_freq, 1);


  /*  calc min and max values of mandatory borders */
  *bmin = v_bord[0];                                                                                 move16();
  for (i = 0; i < *length_v_bord; i++) {
    test(); sub(1, 1);
    if (v_bord[i] < *bmin) {
      *bmin = v_bord[i];                                                                             move16();
    }
  }

  *bmax = v_bord[0];                                                                                 move16();
  for (i = 0; i < *length_v_bord; i++) {
    test(); sub(1, 1);
    if (v_bord[i] > *bmax) {
      *bmax = v_bord[i];                                                                             move16();
    }
  }
}



/*******************************************************************************
 Functionname:  fillFramePre
 *******************************************************************************

 Description: Add borders before mandatory borders, if needed

 Arguments:   
       modified:
              v_bord        - Word16 pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - Word16 pointer to v_freq vector
              length_v_freq - length of v_freq vector
       not modified:
              dmax          - Word16 value
              bmin          - Word16 value
              rest          - Word16 value

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
fillFramePre (Word16 dmax, 
              Word16 *v_bord, Word16 *length_v_bord,
              Word16 *v_freq, Word16 *length_v_freq,
              Word16 bmin, Word16 rest)
{
  Word16 parts, d, j, S, s = 0, segm, bord;

  /*
    start with one envelope 
  */

  parts = 1;
  d = rest;

  /*
    calc # of additional envelopes and corresponding lengths
  */
  test();
  while (d > dmax) {
    parts = add(parts, 1);

    segm = extract_l(ffr_Short_Div(rest, parts));
    S = shr(sub(segm, 2), 1);
    s = S_min (8, add(shl(S, 1), 2));
    d = sub(rest,ffr_Short_Mult(sub(parts, 1), s));
    test();
  }

  /*
    add borders before mandatory borders
  */

  bord = bmin;

  for (j = 0; j <= parts - 2; j++) {
    bord = sub(bord, s);

    /* v_bord = [...,(Bf),(Ba),Bd1, Bd2 ,(Bd3)] */
    AddLeft (v_bord, length_v_bord, bord);
    
    /* v_freq = [...,(1 ),(Fa),Fd1,(Fd2), 1   ] */
    AddLeft (v_freq, length_v_freq, 1);
  }
}


/*******************************************************************************
 Functionname:  fillFramePost
 *******************************************************************************

 Description: -Add borders after mandatory borders, if needed
               Make a preliminary design of next frame, 
               assuming no transient is present there

 Arguments:   
       modified:
              parts         - Word16 pointer to parts (call by reference)
              d             - Word16 pointer to d (call by reference)
              v_bord        - Word16 pointer to v_bord vector
              length_v_bord - length of v_bord vector
              v_freq        - Word16 pointer to v_freq vector
              length_v_freq - length of v_freq vector
        not modified:
              bmax          - Word16 value
              dmax          - Word16 value

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:     FH 00-06-26, FH 01-06-21
*******************************************************************************/
static void
fillFramePost (Word16 *parts, Word16 *d, Word16 dmax, Word16 *v_bord, Word16 *length_v_bord,
               Word16 *v_freq, Word16 *length_v_freq, Word16 bmax,
               Word16 bufferFrameStart, Word16 numberTimeSlots, Word16 fmax)
{
  Word16 j, rest, segm, S, s = 0, bord, dTmp, partsTmp;

  rest = sub(add(bufferFrameStart, shl(numberTimeSlots, 1)), bmax);
  dTmp = rest;                                                               move16();

  test();
  if (dTmp > 0) {
    partsTmp = 1;           /* start with one envelope */                    move16();

    /* calc # of additional envelopes and corresponding lengths */

    test();
    while (dTmp > dmax) {
      partsTmp = add(partsTmp, 1);

      segm = ffr_Short_Div(rest, partsTmp);
      S = shr(sub(segm, 2), 1);
      s = S_min (fmax, add(shl(S, 1), 2));
      dTmp = sub(rest, ffr_Short_Mult(sub(partsTmp, 1), s));
      test();
    }

    /* add borders after mandatory borders */

    bord = bmax;
    for (j = 0; j <= partsTmp - 2; j++) {
      bord = add(bord, s);

      AddRight (v_bord, length_v_bord, bord);

      AddRight (v_freq, length_v_freq, 1);
    }
  }
  else {
    partsTmp = 1;                                                            move16();

    /* remove last element from v_bord and v_freq */

    *length_v_bord = sub(*length_v_bord, 1);                                 move16();
    *length_v_freq = sub(*length_v_freq, 1);                                 move16();

  }
  *parts = partsTmp;                                                         move16();
  *d = dTmp;                                                                 move16();
}



/*******************************************************************************
 Functionname:  fillFrameInter
 *******************************************************************************

 Description: 

 Arguments:   nL                  -
              v_tuningSegm        -
              v_bord              -
              length_v_bord       -
              bmin                -
              v_freq              -
              length_v_freq       -
              v_bordFollow        -
              length_v_bordFollow -
              v_freqFollow        -
              length_v_freqFollow -
              i_fillFollow        -
              dmin                -
              dmax                -

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
fillFrameInter (Word16 *nL, Word16 *v_tuningSegm, Word16 *v_bord, Word16 *length_v_bord,
                Word16 bmin, Word16 *v_freq, Word16 *length_v_freq, Word16 *v_bordFollow,
                Word16 *length_v_bordFollow, Word16 *v_freqFollow,
                Word16 *length_v_freqFollow, Word16 i_fillFollow, Word16 dmin,
                Word16 dmax)
{
  Word16 middle, b_new, numBordFollow, bordMaxFollow, i;

  /* % remove fill borders: */
  test(); sub(1, 1);
  if (i_fillFollow >= 1) {
    *length_v_bordFollow = i_fillFollow;                                                             move16();
    *length_v_freqFollow = i_fillFollow;                                                             move16();
  }

  numBordFollow = *length_v_bordFollow;                                                              move16();
  bordMaxFollow = v_bordFollow[numBordFollow - 1];                                                   move16();

  /* remove even more borders if needed */
  middle = sub(bmin, bordMaxFollow);     
  test();
  while (middle < 0) {
    numBordFollow = sub(numBordFollow, 1);
    bordMaxFollow = v_bordFollow[numBordFollow - 1];
    middle = sub(bmin, bordMaxFollow);
    test();
  }

  *length_v_bordFollow = numBordFollow;                                                              move16();
  *length_v_freqFollow = numBordFollow;                                                              move16();
  *nL = sub(numBordFollow, 1);                                                                       move16();

  b_new = *length_v_bord;                                                                            move16();

  test(); sub(1, 1);
  if (middle <= dmax) {
    test(); sub(1, 1);
    if (middle >= dmin) {       /* concatenate */
      AddVecLeft (v_bord, length_v_bord, v_bordFollow, *length_v_bordFollow);
      AddVecLeft (v_freq, length_v_freq, v_freqFollow, *length_v_freqFollow);
    }

    else {
      test();
      if (v_tuningSegm[0] != 0) {       /* remove one new border and concatenate */
        *length_v_bord = sub(b_new, 1);                                                            move16();
        AddVecLeft (v_bord, length_v_bord, v_bordFollow,
                    *length_v_bordFollow);

        *length_v_freq = sub(b_new, 1);                                                            move16();
        AddVecLeft (v_freq + 1, length_v_freq, v_freqFollow,
                    *length_v_freqFollow);

      }
      else {
        test(); sub(1, 1);
        if (*length_v_bordFollow > 1) { /* remove one old border and concatenate */
          AddVecLeft (v_bord, length_v_bord, v_bordFollow,
                      *length_v_bordFollow - 1);
          AddVecLeft (v_freq, length_v_freq, v_freqFollow,
                      *length_v_bordFollow - 1);

          *nL = sub(*nL, 1);                                                                         move16();
        }
        else {                  /* remove new "transient" border and concatenate */


          for (i = 0; i < *length_v_bord - 1; i++) {
            v_bord[i] = v_bord[i + 1];                                                               move16();
          }

          for (i = 0; i < *length_v_freq - 1; i++) {
            v_freq[i] = v_freq[i + 1];                                                               move16();
          }

          *length_v_bord = sub(b_new, 1);                                                            move16();
          *length_v_freq = sub(b_new, 1);                                                            move16();

          AddVecLeft (v_bord, length_v_bord, v_bordFollow,
                      *length_v_bordFollow);
          AddVecLeft (v_freq, length_v_freq, v_freqFollow,
                      *length_v_freqFollow);
        }
      }
    }
  }
  else {                        /* middle > dmax */

    fillFramePre (dmax, v_bord, length_v_bord, v_freq, length_v_freq, bmin,
                  middle);
    AddVecLeft (v_bord, length_v_bord, v_bordFollow, *length_v_bordFollow);
    AddVecLeft (v_freq, length_v_freq, v_freqFollow, *length_v_freqFollow);
  }
}



/*******************************************************************************
 Functionname:  calcFrameClass
 *******************************************************************************

 Description: 

 Arguments:  Word16* frameClass, Word16* frameClassOld, Word16 tranFlag, Word16* spreadFlag)

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
calcFrameClass (FRAME_CLASS *frameClass, FRAME_CLASS *frameClassOld, Word16 tranFlag,
                Word16 *spreadFlag)
{
  test();
  switch (*frameClassOld) {
  case FIXFIX:
    test(); move16();
    if (tranFlag)
      *frameClass = FIXVAR;
    else
      *frameClass = FIXFIX;
    break;
  case FIXVAR:
    test();
    if (tranFlag) {
      *frameClass = VARVAR;                                   move16();
      *spreadFlag = 0;                                        move16();
    }
    else {
      test(); move16();
      if (*spreadFlag)
        *frameClass = VARVAR;
      else
        *frameClass = VARFIX;
    }
    break;
  case VARFIX:
    test(); move16();
    if (tranFlag)
      *frameClass = FIXVAR;
    else
      *frameClass = FIXFIX;
    break;
  case VARVAR:
    test();
    if (tranFlag) {
      *frameClass = VARVAR;                                   move16();
      *spreadFlag = 0;                                        move16();
    }
    else {
      test(); move16();
      if (*spreadFlag)
        *frameClass = VARVAR;
      else
        *frameClass = VARFIX;
    }
    break;
  };

  *frameClassOld = *frameClass;                               move16();
}



/*******************************************************************************
 Functionname:  specialCase
 *******************************************************************************

 Description: 

 Arguments:   spreadFlag
              allowSpread
              v_bord
              length_v_bord
              v_freq
              length_v_freq
              parts
              d

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
specialCase (Word16 *spreadFlag, Word16 allowSpread, Word16 *v_bord,
             Word16 *length_v_bord, Word16 *v_freq, Word16 *length_v_freq, Word16 *parts,
             Word16 d)
{
  Word16 L;

  L = *length_v_bord;

  test();
  if (allowSpread) {        
    *spreadFlag = 1;                                                       move16();
    AddRight (v_bord, length_v_bord, v_bord[L - 1] + 8);
    AddRight (v_freq, length_v_freq, 1);
    *parts = add(*parts, 1);                                               move16();
  }
  else {
    test(); sub(1, 1);
    if (d == 1) {              
      *length_v_bord = sub(L, 1);                                          move16();
      *length_v_freq = sub(L, 1);                                          move16();
    }
    else {
      test(); sub(1, 1);
      if (sub(v_bord[L - 1], v_bord[L - 2]) > 2) {
        v_bord[L - 1] = sub(v_bord[L - 1], 2);                             move16();
        v_freq[*length_v_freq - 1] = 0;                                    move16();
      }
    }
  }
}



/*******************************************************************************
 Functionname:  calcCmonBorder
 *******************************************************************************

 Description: 

 Arguments:   i_cmon
              i_tran
              v_bord
              length_v_bord
              tran

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:     FH 00-06-26
*******************************************************************************/
static void
calcCmonBorder (Word16 *i_cmon, Word16 *i_tran, Word16 *v_bord, Word16 *length_v_bord,
                Word16 tran, Word16 bufferFrameStart, Word16 numberTimeSlots)
{ 
  Word16 i;

  for (i = 0; i < *length_v_bord; i++) {
    test(); sub(1, 1);
    if (v_bord[i] >= add(bufferFrameStart, numberTimeSlots)) {   
      *i_cmon = i;                                                                                   move16();
      break;
    }
  }

  /* keep track of transient: */
  for (i = 0; i < *length_v_bord; i++) {
    test(); sub(1, 1);
    if (v_bord[i] >= tran) {
      *i_tran = i;                                                                                   move16();
      break;
    }
    else {
      *i_tran = EMPTY;                                                                               move16();
    }
  }
}


/*******************************************************************************
 Functionname:  keepForFollowUp
 *******************************************************************************

 Description: 

 Arguments:   v_bordFollow
              length_v_bordFollow
              v_freqFollow
              length_v_freqFollow
              i_tranFollow
              i_fillFollow
              v_bord
              length_v_bord
              v_freq
              i_cmon
              i_tran
              parts)

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:     FH 00-06-26
*******************************************************************************/
static void
keepForFollowUp (Word16 *v_bordFollow, Word16 *length_v_bordFollow,
                 Word16 *v_freqFollow, Word16 *length_v_freqFollow,
                 Word16 *i_tranFollow, Word16 *i_fillFollow, Word16 *v_bord,
                 Word16 *length_v_bord, Word16 *v_freq, Word16 i_cmon, Word16 i_tran,
                 Word16 parts, Word16 numberTimeSlots)
{
  Word16 L, i, j;

  L = *length_v_bord;

  (*length_v_bordFollow) = 0;                                                                        move16();
  (*length_v_freqFollow) = 0;                                                                        move16();

  for (j = 0, i = i_cmon; i < L; i++, j++) {
    v_bordFollow[j] = sub(v_bord[i], numberTimeSlots);                                               move16();
    v_freqFollow[j] = v_freq[i];                                                                     move16();
    *length_v_bordFollow = add(*length_v_bordFollow, 1);                                             move16();
    *length_v_freqFollow = add(*length_v_freqFollow, 1);                                             move16();
  }
  test(); sub(1, 1);
  if (i_tran != EMPTY) {
    *i_tranFollow = sub(i_tran, i_cmon);                                                             move16();
  }
  else {
    *i_tranFollow = EMPTY;                                                                           move16();
  }
  *i_fillFollow = sub(sub(L, sub(parts, 1)), i_cmon);                                                move16();
}


/*******************************************************************************
 Functionname:  calcCtrlSignal
 *******************************************************************************

 Description: 

 Arguments:   hSbrGrid
              frameClass
              v_bord
              length_v_bord
              v_freq
              length_v_freq
              i_cmon
              i_tran
              spreadFlag
              nL

 Return:      none

 Written:     Kristofer Kjoerling, CTS AB
 Revised:
*******************************************************************************/
static void
calcCtrlSignal (HANDLE_SBR_GRID hSbrGrid,
                FRAME_CLASS frameClass, Word16 *v_bord, Word16 length_v_bord, Word16 *v_freq,
                Word16 length_v_freq, Word16 i_cmon, Word16 i_tran, Word16 spreadFlag,
                Word16 nL)
{


  Word16 i, r, a, n, p, b, aL, aR, ntot, nmax, nR;

  Word16 *v_f = hSbrGrid->v_f;
  Word16 *v_fLR = hSbrGrid->v_fLR;
  Word16 *v_r = hSbrGrid->bs_rel_bord;
  Word16 *v_rL = hSbrGrid->bs_rel_bord_0;
  Word16 *v_rR = hSbrGrid->bs_rel_bord_1;

  Word16 length_v_r = 0;
  Word16 length_v_rR = 0;
  Word16 length_v_rL = 0;
  move16();move16(); move16();

  test();
  switch (frameClass) {
  case FIXVAR:
    /* absolute border: */

    a = v_bord[i_cmon];                                                      move16();

    /* relative borders: */
    length_v_r = 0;                                                          move16();
    i = i_cmon;                                                              move16();

    test();
    while (i >= 1) {
      r = sub(v_bord[i], v_bord[i - 1]);
      AddRight (v_r, &length_v_r, r);
      i = sub(i, 1);
      test();
    }


    /*  number of relative borders: */
    n = length_v_r;                                                                                 move16();


    /* freq res: */
    for (i = 0; i < i_cmon; i++) {
      v_f[i] = v_freq[i_cmon - 1 - i];                                                               move16();
    }
    v_f[i_cmon] = 1;                                                                                 move16();

    /* pointer: */
    test(); test(); sub(1, 1); sub(1, 1);
    if (i_cmon >= i_tran && i_tran != EMPTY) {
      p = add(sub(i_cmon, i_tran), 1);
    }
    else {
      p = 0;                                                                                         move16();
    }

    hSbrGrid->frameClass = frameClass;                                                               move16();
    hSbrGrid->bs_abs_bord = a;                                                                       move16();
    hSbrGrid->n = n;                                                                                 move16();
    hSbrGrid->p = p;                                                                                 move16();

    break;
  case VARFIX:
    /* absolute border: */
    a = v_bord[0];                                                                                   move16();

    /* relative borders: */
    length_v_r = 0;                                                                                  move16();

    for (i = 1; i < length_v_bord; i++) {
      r = sub(v_bord[i], v_bord[i - 1]);
      AddRight (v_r, &length_v_r, r);
    }


    /* number of relative borders: */
    n = length_v_r;                                                                                  move16();

    /* freq res: */
    memcpy (v_f, v_freq, length_v_freq * sizeof (Word16));                                           memop16(length_v_freq);


    /* pointer: */
    test(); test(); sub(1, 1);
    if (i_tran >= 0 && i_tran != EMPTY) {
      p = add(i_tran, 1);
    }
    else {
      p = 0;                                                                                         move16();
    }

    hSbrGrid->frameClass = frameClass;                                                               move16();
    hSbrGrid->bs_abs_bord = a;                                                                       move16();
    hSbrGrid->n = n;                                                                                 move16();
    hSbrGrid->p = p;                                                                                 move16();

    break;
  case VARVAR:
    test();
    if (spreadFlag) {
      /* absolute borders: */
      b = length_v_bord;                                                                             move16();

      aL = v_bord[0];                                                                                move16();
      aR = v_bord[b - 1];                                                                            move16();


      /* number of relative borders:    */
      ntot = sub(b, 2);

      nmax = 2;                                                                                      move16();
      test(); sub(1, 1);
      if (ntot > nmax) {
        nL = nmax;                                                                                   move16();
        nR = sub(ntot, nmax);
      }
      else {
        nL = ntot;                                                                                   move16();
        nR = 0;                                                                                      move16();
      }

      /* relative borders: */
      length_v_rL = 0;
      for (i = 1; i <= nL; i++) {
        r = sub(v_bord[i], v_bord[i - 1]);
        AddRight (v_rL, &length_v_rL, r);
      }

      length_v_rR = 0;                                                                               move16();
      i = sub(b, 1);
      test();
      while (i >= sub(b, nR)) {
        r = sub(v_bord[i], v_bord[i - 1]);
        AddRight (v_rR, &length_v_rR, r);
        i = sub(i, 1);
        test();
      }

      /* pointer (only one due to constraint in frame info): */
      test(); test(); sub(1, 1);
      if (i_tran > 0 && i_tran != EMPTY) {
        p = sub(b, i_tran);
      }
      else {
        p = 0;                                                                                       move16();
      }

      /* freq res: */

      for (i = 0; i <  sub(b, 1); i++) {
        v_fLR[i] = v_freq[i];                                                                        move16();
      }
    }
    else {

      length_v_bord = add(i_cmon, 1);
      length_v_freq = add(i_cmon, 1);


      /* absolute borders: */
      b = length_v_bord;                                                                             move16();

      aL = v_bord[0];                                                                                move16();
      aR = v_bord[b - 1];                                                                            move16();

      /* number of relative borders:   */
      ntot = sub(b, 2);
      nR = sub(ntot, nL);


      /* relative borders: */
      length_v_rL = 0;
      for (i = 1; i <= nL; i++) {
        r = sub(v_bord[i], v_bord[i - 1]);
        AddRight (v_rL, &length_v_rL, r);
      }



      length_v_rR = 0;                                                                               move16();
      i = sub(b, 1);
      test();
      while (i >= sub(b, nR)) {
        r = sub(v_bord[i], v_bord[i - 1]);
        AddRight (v_rR, &length_v_rR, r);
        i = sub(i, 1);
        test();
      }



      /* pointer (only one due to constraint in frame info): */
      test(); test(); sub(1, 1); sub(1, 1);
      if (i_cmon >= i_tran && i_tran != EMPTY) {
        p = add(sub(i_cmon, i_tran), 1);
      }
      else {
        p = 0;                                                                                       move16();
      }

      /* freq res: */
      for (i = 0; i < sub(b, 1); i++) {
        v_fLR[i] = v_freq[i];                                                                        move16();
      }
    }


    hSbrGrid->frameClass = frameClass;                                                               move16();
    hSbrGrid->bs_abs_bord_0 = aL;                                                                    move16();
    hSbrGrid->bs_abs_bord_1 = aR;                                                                    move16();
    hSbrGrid->bs_num_rel_0 = nL;                                                                     move16();
    hSbrGrid->bs_num_rel_1 = nR;                                                                     move16();
    hSbrGrid->p = p;                                                                                 move16();

    break;

  default:
    /* do nothing */
    break;
  }
}

/*******************************************************************************
 Functionname:  createDefFrameInfo
 *******************************************************************************

 Description: Copies the default (static) frameInfo structs to the frameInfo
              passed by reference; only used for FIXFIX frames

 Arguments:   hFrameInfo             - HANLDE_SBR_FRAME_INFO
              nEnv                   - Word16
              nTimeSlots             - Word16

 Return:      none; hSbrFrameInfo contains a copy of the default frameInfo

 Written:     2002/02/05 Andreas Schneider, CT
 Revised:
*******************************************************************************/
static void
createDefFrameInfo(HANDLE_SBR_FRAME_INFO hSbrFrameInfo, Word16 nEnv)
{
  test();
  switch (nEnv) {
  case 1:
    memcpy (hSbrFrameInfo, &frameInfo1_2048, sizeof (SBR_FRAME_INFO));        memop16((sizeof(SBR_FRAME_INFO)+1)/sizeof(Word16));
    break;
  case 2:
    memcpy (hSbrFrameInfo, &frameInfo2_2048, sizeof (SBR_FRAME_INFO));        memop16((sizeof(SBR_FRAME_INFO)+1)/sizeof(Word16));
    break;
  case 4:
    memcpy (hSbrFrameInfo, &frameInfo4_2048, sizeof (SBR_FRAME_INFO));        memop16((sizeof(SBR_FRAME_INFO)+1)/sizeof(Word16));
    break;
  default:
    assert(0);
  }
}


/*******************************************************************************
 Functionname:  ctrlSignal2FrameInfo
 *******************************************************************************

 Description: Calculates frame_info struct from control signal.

 Arguments:   hSbrGrid - source
              hSbrFrameInfo - destination

 Return:      void; hSbrFrameInfo contains the updated FRAME_INFO struct

 Written:     Kristofer Kjoerling, CTS AB
 Revised:     FH 00-06-26
              KK 00-09-01
              02-02-05 Andreas Schneider, CT
*******************************************************************************/
static void
ctrlSignal2FrameInfo (HANDLE_SBR_GRID hSbrGrid,
                      HANDLE_SBR_FRAME_INFO hSbrFrameInfo,
                      Word16 freq_res_fixfix)
{
  Word16 nEnv = 0, border = 0;
  Word16 i, k, p;
  Word16 *v_r = hSbrGrid->bs_rel_bord;
  Word16 *v_f = hSbrGrid->v_f;

  FRAME_CLASS frameClass    = hSbrGrid->frameClass;
  Word16 bufferFrameStart   = hSbrGrid->bufferFrameStart;
  Word16 numberTimeSlots    = hSbrGrid->numberTimeSlots;

  test();
  switch (frameClass) {
  case FIXFIX:
    createDefFrameInfo(hSbrFrameInfo, hSbrGrid->bs_num_env);

    test(); sub(1, 1);
    if (freq_res_fixfix == FREQ_RES_LOW) {
      for (i = 0; i < hSbrFrameInfo->nEnvelopes; i++) {
        hSbrFrameInfo->freqRes[i] = FREQ_RES_LOW;                                                    move16();
      }
    }
    break;

  case FIXVAR:
  case VARFIX:
    nEnv = add(hSbrGrid->n, 1);    
    assert(nEnv <= MAX_ENVELOPES_FIXVAR_VARFIX);

    hSbrFrameInfo->nEnvelopes = nEnv;                                                                move16();

    border = hSbrGrid->bs_abs_bord; /* read the absolute border */

    test(); sub(1, 1);
    if (nEnv == 1) {
      hSbrFrameInfo->nNoiseEnvelopes = 1;                                                            move16();
    }
    else {
      hSbrFrameInfo->nNoiseEnvelopes = 2;                                                            move16();
    }

    break;

  default:
    /* do nothing */
    break;
  }

  test();
  switch (frameClass) {
  case FIXVAR:
    hSbrFrameInfo->borders[0] = bufferFrameStart; /* start-position of 1st envelope */               move16();

    hSbrFrameInfo->borders[nEnv] = border;                                                           move16();

    for (k = 0, i = sub(nEnv, 1); k < nEnv - 1; k++, i--) {
      border = sub(border, v_r[k]);
      hSbrFrameInfo->borders[i] = border;                                                            move16();
    }

    /* make either envelope nr. nEnv + 1 - p short; or don't shorten if p == 0 */
    p = hSbrGrid->p;
    test();
    if (p == 0) {
      hSbrFrameInfo->shortEnv = 0;                                                                   move16();
    } else {
      hSbrFrameInfo->shortEnv = sub(add(nEnv, 1), p);                                                move16();
    }

    for (k = 0, i = nEnv - 1; k < nEnv; k++, i--) {
      hSbrFrameInfo->freqRes[i] = (FREQ_RES)v_f[k];                                                  move16();
    }

    /* if either there is no short envelope or the last envelope is short... */
    test(); test();
    if (p == 0 || sub(p,1) == 0) {
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];                             move16();
    } else {
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];              move16();
    }
    
    break;

  case VARFIX:
    /* in this case 'border' indicates the start of the 1st envelope */
    hSbrFrameInfo->borders[0] = border;                                                              move16();

    for (k = 0; k < nEnv - 1; k++) {
      border = add(border, v_r[k]);
      hSbrFrameInfo->borders[k + 1] = border;                                                        move16();
    }

    hSbrFrameInfo->borders[nEnv] = add(bufferFrameStart, numberTimeSlots);                           move16();

    p = hSbrGrid->p;
    test();
    if (p == 0 || sub(p,1) == 0) {
      hSbrFrameInfo->shortEnv = 0;                                                                   move16();
    } else {
      hSbrFrameInfo->shortEnv = sub(p, 1);                                                           move16();
    }

    for (k = 0; k < nEnv; k++) {
      hSbrFrameInfo->freqRes[k] = (FREQ_RES)v_f[k];                                                  move16();
    }

    test();
    switch (p) {
    case 0:
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[1];                                    move16();
      break;
    case 1:
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];                             move16();
      break;
    default:
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];              move16();
      break;
    }
    break;

  case VARVAR:
    nEnv = add(add(hSbrGrid->bs_num_rel_0, hSbrGrid->bs_num_rel_1), 1);
    assert(nEnv <= MAX_ENVELOPES_VARVAR); /* just to be sure */
    hSbrFrameInfo->nEnvelopes = nEnv;                                                                move16();

    hSbrFrameInfo->borders[0] = border = hSbrGrid->bs_abs_bord_0;                                    move16();

    for (k = 0, i = 1; k < hSbrGrid->bs_num_rel_0; k++, i++) {
      border = add(border, hSbrGrid->bs_rel_bord_0[k]);
      hSbrFrameInfo->borders[i] = border;                                                            move16();
    }

    border = hSbrGrid->bs_abs_bord_1;
    hSbrFrameInfo->borders[nEnv] = border;                                                           move16();

    for (k = 0, i = nEnv - 1; k < hSbrGrid->bs_num_rel_1; k++, i--) {
      border = sub(border, hSbrGrid->bs_rel_bord_1[k]);
      hSbrFrameInfo->borders[i] = border;                                                            move16();
    }

    p = hSbrGrid->p;
    test();
    if (p == 0) {
      hSbrFrameInfo->shortEnv = 0;                                                                   move16();
    } else {
      hSbrFrameInfo->shortEnv = sub(add(nEnv, 1), p);                                                move16();
    }

    for (k = 0; k < nEnv; k++) {
      hSbrFrameInfo->freqRes[k] = (FREQ_RES)hSbrGrid->v_fLR[k];                                      move16();
    }

    test(); sub(1, 1);
    if (nEnv == 1) {
      hSbrFrameInfo->nNoiseEnvelopes = 1;                                                            move16();
      hSbrFrameInfo->bordersNoise[0] = hSbrGrid->bs_abs_bord_0;                                      move16();
      hSbrFrameInfo->bordersNoise[1] = hSbrGrid->bs_abs_bord_1;                                      move16();
    } else {
      hSbrFrameInfo->nNoiseEnvelopes = 2;                                                            move16();
      hSbrFrameInfo->bordersNoise[0] = hSbrGrid->bs_abs_bord_0;                                      move16();

      test();
      if (p == 0 || sub(p,1) == 0) {
        hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv - 1];                           move16();
      } else {
        hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[hSbrFrameInfo->shortEnv];            move16();
      }
      hSbrFrameInfo->bordersNoise[2] = hSbrGrid->bs_abs_bord_1;                                      move16();
    }
    break;

  default:
    /* do nothing */
    break;
  }

  test(); sub(1, 1); sub(1, 1);
  if (frameClass == VARFIX || frameClass == FIXVAR) {
    hSbrFrameInfo->bordersNoise[0] = hSbrFrameInfo->borders[0];                                      move16();
     test(); sub(1, 1);
    if (nEnv == 1) {
      hSbrFrameInfo->bordersNoise[1] = hSbrFrameInfo->borders[nEnv];                                 move16();
    } else {
      hSbrFrameInfo->bordersNoise[2] = hSbrFrameInfo->borders[nEnv];                                 move16();
    }
  }
}

