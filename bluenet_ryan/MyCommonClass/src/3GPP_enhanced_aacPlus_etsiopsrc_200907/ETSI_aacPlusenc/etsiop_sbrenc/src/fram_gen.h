/*
   Framing generator prototypes and structs 
 */
#ifndef _FRAM_GEN_H
#define _FRAM_GEN_H

#include "sbr_def.h" /* for MAX_ENVELOPES and MAX_NOISE_ENVELOPES in struct FRAME_INFO */
#include "sbr_main.h" /* CODEC_TYPE */

#define MAX_ENVELOPES_VARVAR MAX_ENVELOPES /*!< worst case number of envelopes in a VARVAR frame */
#define MAX_ENVELOPES_FIXVAR_VARFIX 4 /*!< worst case number of envelopes in VARFIX and FIXVAR frames */
#define MAX_NUM_REL 3 /*!< maximum number of relative borders in any VAR frame */

/* SBR frame class definitions */
typedef enum {
  FIXFIX = 0, /*!< bs_frame_class: leading and trailing frame borders are fixed */
  FIXVAR,     /*!< bs_frame_class: leading frame border is fixed, trailing frame border is variable */
  VARFIX,     /*!< bs_frame_class: leading frame border is variable, trailing frame border is fixed */
  VARVAR      /*!< bs_frame_class: leading and trailing frame borders are variable */
}FRAME_CLASS;


/* helper constants */
#define DC      4711
#define EMPTY   (-99)


#define BUFFER_FRAME_START_2048 0
#define FRAME_MIDDLE_SLOT_2048  4
#define NUMBER_TIME_SLOTS_2048  16


/*!
  \struct SBR_GRID
  \brief  sbr_grid() signals to be converted to bitstream elements  

*/
typedef struct
{
  Word16 bufferFrameStart;     /*!< frame generator vs analysis buffer time alignment (currently set to 0, offset added elsewhere) */   
  Word16 numberTimeSlots;      /*!< number of SBR timeslots per frame */

  FRAME_CLASS frameClass;      /*!< SBR frame class  */ 
  Word16 bs_num_env;           /*!< bs_num_env, number of envelopes for FIXFIX */       
  Word16 bs_abs_bord;          /*!< bs_abs_bord, absolute border for VARFIX and FIXVAR */
  Word16 n;                    /*!< number of relative borders for VARFIX and FIXVAR   */
  Word16 p;                    /*!< pointer-to-transient-border  */
  Word16 bs_rel_bord[MAX_NUM_REL];         /*!< bs_rel_bord, relative borders for all VAR */
  Word16 v_f[MAX_ENVELOPES_FIXVAR_VARFIX]; /*!< envelope frequency resolutions for FIXVAR and VARFIX  */

  Word16 bs_abs_bord_0;        /*!< bs_abs_bord_0, leading absolute border for VARVAR */
  Word16 bs_abs_bord_1;        /*!< bs_abs_bord_1, trailing absolute border for VARVAR */
  Word16 bs_num_rel_0;         /*!< bs_num_rel_0, number of relative borders associated with leading absolute border for VARVAR */
  Word16 bs_num_rel_1;         /*!< bs_num_rel_1, number of relative borders associated with trailing absolute border for VARVAR */
  Word16 bs_rel_bord_0[MAX_NUM_REL];       /*!< bs_rel_bord_0, relative borders associated with leading absolute border for VARVAR */
  Word16 bs_rel_bord_1[MAX_NUM_REL];       /*!< bs_rel_bord_1, relative borders associated with trailing absolute border for VARVAR */
  Word16 v_fLR[MAX_ENVELOPES_VARVAR];      /*!< envelope frequency resolutions for VARVAR  */

}
SBR_GRID; /* size Word16: 28 */
typedef SBR_GRID *HANDLE_SBR_GRID;



/*!
  \struct SBR_FRAME_INFO
  \brief  time/frequency grid description for one frame  
*/
typedef struct
{  
  Word16 nEnvelopes;                         /*!< number of envelopes */
  Word16 borders[MAX_ENVELOPES+1];           /*!< envelope borders in SBR timeslots */
  FREQ_RES freqRes[MAX_ENVELOPES];           /*!< frequency resolution of each envelope */  
  Word16 shortEnv;                           /*!< number of an envelope to be shortened (starting at 1) or 0 for no shortened envelope */  
  Word16 nNoiseEnvelopes;                    /*!< number of noise floors */  
  Word16 bordersNoise[MAX_NOISE_ENVELOPES+1];/*!< noise floor borders in SBR timeslots */
}
SBR_FRAME_INFO; /* size Word16: 17 */

typedef SBR_FRAME_INFO *HANDLE_SBR_FRAME_INFO;


/*!
  \struct SBR_ENVELOPE_FRAME
  \brief  frame generator main struct
*/
typedef struct
{ 
  Word16 frameMiddleSlot;      /*!< transient detector offset in SBR timeslots */

  /* basic tuning parameters */
  Word16 staticFraming;        /*!< 1: run static framing in time, i.e. exclusive use of bs_frame_class = FIXFIX */
  Word16 numEnvStatic;         /*!< number of envelopes per frame for static framing */
  Word16 freq_res_fixfix;      /*!< envelope frequency resolution to use for bs_frame_class = FIXFIX */

  Word16 *v_tuningSegm;        /*!< segment lengths to use around transient */
  Word16 *v_tuningFreq;        /*!< frequency resolutions to use around transient */
  Word16 dmin;                 /*!< minimum length of dependent segments */
  Word16 dmax;                 /*!< maximum length of dependent segments */
  Word16 allowSpread;          /*!< 1: allow isolated transient to influence grid of 3 consecutive frames */
 
  FRAME_CLASS frameClassOld;                   /*!< frame class used for previous frame */
  Word16 spreadFlag;                           /*!< 1: use VARVAR instead of VARFIX to follow up old transient */

  Word16 v_bord[2 * MAX_ENVELOPES_VARVAR + 1]; /*!< borders for current frame and preliminary borders for next frame (fixed borders excluded) */
  Word16 length_v_bord;                        /*!< helper variable: length of v_bord */
  Word16 v_freq[2 * MAX_ENVELOPES_VARVAR + 1]; /*!< frequency resolutions for current frame and preliminary resolutions for next frame */
  Word16 length_v_freq;                        /*!< helper variable: length of v_freq */

  Word16 v_bordFollow[MAX_ENVELOPES_VARVAR];   /*!< preliminary borders for current frame (calculated during previous frame) */ 
  Word16 length_v_bordFollow;                  /*!< helper variable: length of v_bordFollow */
  Word16 i_tranFollow;                         /*!< poWord16s to transient border in v_bordFollow (may be negative, see keepForFollowUp()) */ 
  Word16 i_fillFollow;                         /*!< points to first fill border in v_bordFollow */
  Word16 v_freqFollow[MAX_ENVELOPES_VARVAR];   /*!< preliminary frequency resolutions for current frame (calculated during previous frame) */ 
  Word16 length_v_freqFollow;                  /*!< helper variable: length of v_freqFollow */

  SBR_GRID         SbrGrid;         /*!< sbr_grid() signals to be converted to bitstream elements */
  SBR_FRAME_INFO   SbrFrameInfo;    /*!< time/frequency grid description for one frame */
}
SBR_ENVELOPE_FRAME; /* size Word16: 88 */

typedef SBR_ENVELOPE_FRAME *HANDLE_SBR_ENVELOPE_FRAME;


void
createFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME  hSbrEnvFrame,
                          Word16 allowSpread,
                          Word16 numEnvStatic,
                          Word16 staticFraming,
                          Word16 timeSlots,
                          Word16 freq_res_fixfix); 

void
deleteFrameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame);

HANDLE_SBR_FRAME_INFO
frameInfoGenerator (HANDLE_SBR_ENVELOPE_FRAME hSbrEnvFrame,
                    Word16 *v_transient_info,
                    Word16 *v_tuning);

#endif
