/*
   SBR main definitions
 */
#ifndef __SBR_DEF_H
#define __SBR_DEF_H

#define SWAP(a,b)                               tempr=a, a=b, b=tempr
#define TRUE  1
#define FALSE 0


/************  Definitions ***************/
#define SBR_DOWNSAMP_FAC                        2

#define MAX_NUM_NOISE_VALUES                   10
#define MAX_NOISE_ENVELOPES                     2
#define MAX_NUM_NOISE_COEFFS                    5
#define MAX_NUM_ENVELOPE_VALUES  (MAX_ENVELOPES * MAX_FREQ_COEFFS)
#define MAX_ENVELOPES                           5

#define MAX_FREQ_COEFFS                        27


#define QMF_CHANNELS                            64
#define QMF_FILTER_LENGTH                      640
#define QMF_TIME_SLOTS                          32
#define NO_OF_ESTIMATES                          4
#define NO_SYNTHESIS_CHANNELS                   32
#define QMF_FILTER_STATE_SYN_SIZE              320
#define QMF_FILTER_STATE_ANA_SIZE              320
#define NO_POLY                               ((QMF_FILTER_LENGTH) /(2*QMF_CHANNELS)) /*! Number of polyphase filter taps */


#define NOISE_FLOOR_OFFSET                      6

#define LOW_RES                                 0
#define HIGH_RES                                1

#define LO                                      0
#define HI                                      1

#define LENGTH_SBR_FRAME_INFO                   35      /* 19 */

#define SBR_NSFB_LOW_RES                        9       /*  8 */
#define SBR_NSFB_HIGH_RES                       18      /* 16 */


#define SI_SBR_PROTOCOL_VERSION_ID              0

#define SBR_XPOS_CTRL_DEFAULT                   2

#define SBR_FREQ_SCALE_DEFAULT                  2
#define SBR_ALTER_SCALE_DEFAULT                 1
#define SBR_NOISE_BANDS_DEFAULT                 2

#define SBR_LIMITER_BANDS_DEFAULT               2
#define SBR_LIMITER_GAINS_DEFAULT               2
#define SBR_INTERPOL_FREQ_DEFAULT               1
#define SBR_SMOOTHING_LENGTH_DEFAULT            0


/* sbr_header */
#define SI_SBR_PROTOCOL_VERSION_BITS            2
#define SI_SBR_AMP_RES_BITS                     1
#define SI_SBR_COUPLING_BITS                    1
#define SI_SBR_START_FREQ_BITS                  4
#define SI_SBR_STOP_FREQ_BITS                   4
#define SI_SBR_XOVER_BAND_BITS                  3
#define SI_SBR_RESERVED_BITS                    2
#define SI_SBR_DATA_EXTRA_BITS                  1
#define SI_SBR_HEADER_EXTRA_1_BITS              1
#define SI_SBR_HEADER_EXTRA_2_BITS              1

#define SI_SBR_LC_STEREO_MODE_BITS          2

#define SI_SBR_FREQ_SCALE_BITS                  2
#define SI_SBR_ALTER_SCALE_BITS                 1
#define SI_SBR_NOISE_BANDS_BITS                 2

#define SI_SBR_LIMITER_BANDS_BITS               2
#define SI_SBR_LIMITER_GAINS_BITS               2
#define SI_SBR_INTERPOL_FREQ_BITS               1
#define SI_SBR_SMOOTHING_LENGTH_BITS            1
#define SI_SBR_RESERVED_HE2_BITS                1

/* sbr_grid */
#define SBR_CLA_BITS                            2   /*!< size of bs_frame_class */
#define SBR_ENV_BITS                            2   /*!< size of bs_num_env_raw */

#define SBR_ABS_BITS                            2   /*!< size of bs_abs_bord_raw for aacPlus */

#define SBR_NUM_BITS                            2   /*!< size of bs_num_rel */
#define SBR_REL_BITS                            2   /*!< size of bs_rel_bord_raw */
#define SBR_RES_BITS                            1   /*!< size of bs_freq_res_flag */
#define SBR_DIR_BITS                            1   /*!< size of bs_df_flag */


/* sbr_data */
#define SI_SBR_SR_MODE_BITS                     1
#define SI_SBR_ENABLE_PSEUDOSTEREO_BITS         1

#define SI_SBR_INVF_MODE_BITS                   2
#define SI_SBR_XPOS_MODE_BITS                   2
#define SI_SBR_XPOS_CTRL_BITS                   3


#define SI_SBR_START_ENV_BITS_AMP_RES_3_0           6
#define SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0   5
#define SI_SBR_START_NOISE_BITS_AMP_RES_3_0         5
#define SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0 5

#define SI_SBR_START_ENV_BITS_AMP_RES_1_5           7
#define SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5   6


#define SI_SBR_EXTENDED_DATA_BITS               1
#define SI_SBR_EXTENSION_SIZE_BITS              4
#define SI_SBR_EXTENSION_ESC_COUNT_BITS         8
#define SI_SBR_EXTENSION_ID_BITS                2

#define SBR_EXTENDED_DATA_MAX_CNT               (15+255)

#define EXTENSION_ID_PARAMETRIC_CODING          0
#define EXTENSION_ID_PS_CODING                  2

/* Envelope coding constants */
#define FREQ                      0
#define TIME                      1


/* huffman tables */
#define CODE_BOOK_SCF_LAV00         60
#define CODE_BOOK_SCF_LAV01         31
#define CODE_BOOK_SCF_LAV10         60
#define CODE_BOOK_SCF_LAV11         31
#define CODE_BOOK_SCF_LAV_BALANCE11 12
#define CODE_BOOK_SCF_LAV_BALANCE10 24

#define NO_BINS                         ( 20 )
#define NO_LOW_RES_BINS                 ( NO_IID_BINS / 2 )

#define NO_IID_BINS                     ( NO_BINS )
#define NO_ICC_BINS                     ( NO_BINS )
#define NO_IPD_BINS                     ( 11 )

#define NO_LOW_RES_IID_BINS             ( NO_LOW_RES_BINS )
#define NO_LOW_RES_ICC_BINS             ( NO_LOW_RES_BINS )

#define NEGATE_IPD_MASK                 ( 0x00001000 )

typedef enum
{
  SBR_AMP_RES_1_5=0,
  SBR_AMP_RES_3_0
}
AMP_RES;

typedef enum
{
  XPOS_MDCT,
  XPOS_MDCT_CROSS,
  XPOS_LC,
  XPOS_RESERVED,
  XPOS_SWITCHED /* not a real choice but used here to control behaviour */
}
XPOS_MODE;

typedef enum
{
  INVF_OFF = 0,
  INVF_LOW_LEVEL,
  INVF_MID_LEVEL,
  INVF_HIGH_LEVEL,
  INVF_SWITCHED /* not a real choice but used here to control behaviour */
}
INVF_MODE;

typedef enum
{
  SINGLE_RATE,
  DUAL_RATE
}
SR_MODE;

typedef enum
{
  FREQ_RES_LOW,
  FREQ_RES_HIGH
}
FREQ_RES;

#endif
