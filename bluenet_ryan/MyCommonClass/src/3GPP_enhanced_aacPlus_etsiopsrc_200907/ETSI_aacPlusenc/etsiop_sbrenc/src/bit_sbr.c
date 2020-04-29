/*
   SBR bit writing routines
 */
#include <stdlib.h>
#include <assert.h>
#include "sbr_def.h"
#include "FFR_bitbuffer.h"
#include "bit_sbr.h"
#include "sbr.h"
#include "code_env.h"
#include "cmondata.h"
#include "ps_bitenc.h"
#include "ps_enc.h"

#include "count.h"



typedef enum {
  SBR_ID_SCE = 1,
  SBR_ID_CPE
} SBR_ELEMENT_TYPE;


static Word16 encodeSbrData (HANDLE_SBR_ENV_DATA   sbrEnvDataLeft,
                             HANDLE_SBR_ENV_DATA   sbrEnvDataRight,
                             HANDLE_COMMON_DATA    cmonData,
                             SBR_ELEMENT_TYPE      sbrElem,
                             HANDLE_PS_ENC         hPsEnc,
                             Word32                   bHeaderActive,
                             Word32                   coupling);

static Word16 encodeSbrHeader (HANDLE_SBR_HEADER_DATA     sbrHeaderData,
                               HANDLE_SBR_BITSTREAM_DATA  sbrBitstreamData,
                               HANDLE_COMMON_DATA         cmonData);
                            

static Word16 encodeSbrHeaderData (HANDLE_SBR_HEADER_DATA sbrHeaderData,
                                   HANDLE_BIT_BUF         hBitStream);

static Word16 encodeSbrSingleChannelElement (HANDLE_SBR_ENV_DATA    sbrEnvData,
                                             HANDLE_BIT_BUF         hBitStream,
                                             HANDLE_PS_ENC          hPsEnc,
                                             Word32                    bHeaderActive);
                                         

static Word16 encodeSbrChannelPairElement (HANDLE_SBR_ENV_DATA  sbrEnvDataLeft,
                                           HANDLE_SBR_ENV_DATA  sbrEnvDataRight,
                                           HANDLE_BIT_BUF        hBitStream,
                                           Word32                   coupling);
                                        

static Word16 encodeSbrGrid (HANDLE_SBR_ENV_DATA   sbrEnvData,
                             HANDLE_BIT_BUF        hBitStream);

static Word16 encodeSbrDtdf (HANDLE_SBR_ENV_DATA   sbrEnvData,
                             HANDLE_BIT_BUF        hBitStream);

static Word16 writeNoiseLevelData (HANDLE_SBR_ENV_DATA   sbrEnvData,
                                   HANDLE_BIT_BUF        hBitStream,
                                   Word32                 coupling);

static Word16 writeEnvelopeData (HANDLE_SBR_ENV_DATA    sbrEnvData,
                                 HANDLE_BIT_BUF         hBitStream,
                                 Word32                  coupling);

static Word16 writeSyntheticCodingData (HANDLE_SBR_ENV_DATA  sbrEnvData,
                                        HANDLE_BIT_BUF       hBitStream);
                                    

static Word16 encodeExtendedData (HANDLE_PS_ENC        hPsEnc,
                                  Word32               bHeaderActive,
                                  HANDLE_BIT_BUF       hBitStream);

static Word16 getSbrExtendedDataSize (HANDLE_PS_ENC       hPsEnc,
                                      Word32              bHeaderActive);
                                

/*****************************************************************************

    functionname: WriteEnvSingleChannelElement
    description:  writes pure SBR single channel data element
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
Word32
WriteEnvSingleChannelElement(HANDLE_SBR_HEADER_DATA     sbrHeaderData,
                             HANDLE_SBR_BITSTREAM_DATA  sbrBitstreamData,
                             HANDLE_SBR_ENV_DATA        sbrEnvData,
                             HANDLE_PS_ENC              hPsEnc,
                             HANDLE_COMMON_DATA         cmonData)
                            
{
  Word32 payloadBits = 0;

  cmonData->sbrHdrBits  = 0;                                                            move16();
  cmonData->sbrDataBits = 0;                                                            move16();

  /* write pure sbr data */
  test();
  if (sbrEnvData != NULL) {

    /* write header */
    payloadBits = L_add(payloadBits, encodeSbrHeader (sbrHeaderData,
                                                      sbrBitstreamData,
                                                      cmonData));
                                   
    /* write data */
    payloadBits = L_add(payloadBits, encodeSbrData (sbrEnvData,
                                                    NULL,
                                                    cmonData,
                                                    SBR_ID_SCE,
                                                    hPsEnc,
                                                    sbrBitstreamData->HeaderActive,
                                                    0));
  }
  return payloadBits;
}

/*****************************************************************************

    functionname: WriteEnvChannelPairElement  
    description:  writes pure SBR channel pair data element
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
Word32
WriteEnvChannelPairElement (HANDLE_SBR_HEADER_DATA     sbrHeaderData,
                            HANDLE_SBR_BITSTREAM_DATA  sbrBitstreamData,
                            HANDLE_SBR_ENV_DATA        sbrEnvDataLeft,
                            HANDLE_SBR_ENV_DATA        sbrEnvDataRight,
                            HANDLE_COMMON_DATA         cmonData)
                            
{
  Word32 payloadBits = 0;
  cmonData->sbrHdrBits  = 0;                                                            move16();
  cmonData->sbrDataBits = 0;                                                            move16();

  /* write pure sbr data */
  test(); test();
  if ((sbrEnvDataLeft != NULL) && (sbrEnvDataRight != NULL)) {

    /* write header */
    payloadBits = L_add(payloadBits, encodeSbrHeader (sbrHeaderData,
                                                      sbrBitstreamData,
                                                      cmonData));
                                    

    /* write data */
    payloadBits = L_add(payloadBits, encodeSbrData (sbrEnvDataLeft,
                                                    sbrEnvDataRight,
                                                    cmonData,
                                                    SBR_ID_CPE,
                                                    NULL, /* No parametric stereo on top of a channel pair element */
                                                    sbrBitstreamData->HeaderActive,
                                                    sbrHeaderData->coupling));
                                  
    
  }
  return payloadBits;
}

Word32
CountSbrChannelPairElement (HANDLE_SBR_HEADER_DATA     sbrHeaderData,
                            HANDLE_SBR_BITSTREAM_DATA  sbrBitstreamData,
                            HANDLE_SBR_ENV_DATA        sbrEnvDataLeft,
                            HANDLE_SBR_ENV_DATA        sbrEnvDataRight,
                            HANDLE_COMMON_DATA         cmonData)
                          
{
  Word32 payloadBits;
  struct BIT_BUF bitBufTmp = cmonData->sbrBitbuf;


  
  payloadBits = WriteEnvChannelPairElement(sbrHeaderData,
                                           sbrBitstreamData,
                                           sbrEnvDataLeft,
                                           sbrEnvDataRight,
                                           cmonData);
                                          
  cmonData->sbrBitbuf = bitBufTmp;                                                      move16();

  return payloadBits;
}

/*****************************************************************************

    functionname: encodeSbrHeader
    description:  encodes SBR Header information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrHeader (HANDLE_SBR_HEADER_DATA     sbrHeaderData,
                 HANDLE_SBR_BITSTREAM_DATA  sbrBitstreamData,
                 HANDLE_COMMON_DATA         cmonData)
                 
{
  Word16 payloadBits = 0;

  test();
  if (sbrBitstreamData->HeaderActive) {
    payloadBits = add(payloadBits, WriteBits (&cmonData->sbrBitbuf, 1, 1));
    payloadBits = add(payloadBits, encodeSbrHeaderData (sbrHeaderData,
                                                        &cmonData->sbrBitbuf));
  }
  else {
    payloadBits = add(payloadBits, WriteBits (&cmonData->sbrBitbuf, 0, 1));
  }


  cmonData->sbrHdrBits = payloadBits;                                                   move16();

  return payloadBits;
}



/*****************************************************************************

    functionname: encodeSbrHeaderData
    description:  writes sbr_header()
                  bs_protocol_version through bs_header_extra_2
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrHeaderData (HANDLE_SBR_HEADER_DATA sbrHeaderData, 
                     HANDLE_BIT_BUF hBitStream)
                     
{
  Word16 payloadBits = 0;

  test();
  if (sbrHeaderData != NULL) {
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_amp_res,
                                              SI_SBR_AMP_RES_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_start_frequency,
                                              SI_SBR_START_FREQ_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_stop_frequency,
                                              SI_SBR_STOP_FREQ_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_xover_band, 
                                              SI_SBR_XOVER_BAND_BITS));

    payloadBits = add(payloadBits, WriteBits (hBitStream, 0,
                                              SI_SBR_RESERVED_BITS));



    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->header_extra_1, 
                                              SI_SBR_HEADER_EXTRA_1_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->header_extra_2, 
                                              SI_SBR_HEADER_EXTRA_2_BITS));

    test();
    if (sbrHeaderData->header_extra_1) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->freqScale,
                                                SI_SBR_FREQ_SCALE_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->alterScale,
                                                SI_SBR_ALTER_SCALE_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_noise_bands,
                                                SI_SBR_NOISE_BANDS_BITS));
    } /* sbrHeaderData->header_extra_1 */

    if (sbrHeaderData->header_extra_2) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_limiter_bands,
                                                SI_SBR_LIMITER_BANDS_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_limiter_gains,
                                                SI_SBR_LIMITER_GAINS_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_interpol_freq,
                                                SI_SBR_INTERPOL_FREQ_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrHeaderData->sbr_smoothing_length,
                                                SI_SBR_SMOOTHING_LENGTH_BITS));

    } /* sbrHeaderData->header_extra_2 */
  } /* sbrHeaderData != NULL */

  return payloadBits;
}


/*****************************************************************************

    functionname: encodeSbrData
    description:  encodes sbr Data information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrData (HANDLE_SBR_ENV_DATA   sbrEnvDataLeft,
               HANDLE_SBR_ENV_DATA   sbrEnvDataRight,
               HANDLE_COMMON_DATA    cmonData,
               SBR_ELEMENT_TYPE      sbrElem,
               HANDLE_PS_ENC         hPsEnc,
               Word32                   bHeaderActive,
               Word32                   coupling)
{
  Word16 payloadBits = 0;

  test();
  switch (sbrElem) {
  case SBR_ID_SCE:
    payloadBits = add( payloadBits, encodeSbrSingleChannelElement (sbrEnvDataLeft, &cmonData->sbrBitbuf,
                                                                   hPsEnc,
                                                                   bHeaderActive));


    break;
  case SBR_ID_CPE:
    payloadBits = add( payloadBits, encodeSbrChannelPairElement (sbrEnvDataLeft, sbrEnvDataRight, &cmonData->sbrBitbuf,
                                                                 coupling));
    break;
  default:
    /* we never should apply SBR to any other element type */
    assert (0);
  }

  cmonData->sbrDataBits = payloadBits;                                                  move16();

  return payloadBits;
}



/*****************************************************************************

    functionname: encodeSbrSingleChannelElement
    description:  encodes sbr SCE information
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrSingleChannelElement (HANDLE_SBR_ENV_DATA   sbrEnvData,
                               HANDLE_BIT_BUF        hBitStream,
                               HANDLE_PS_ENC         hPsEnc,
                               Word32                   bHeaderActive)
                               
{
  Word16 payloadBits = 0;

  payloadBits = add(payloadBits, WriteBits (hBitStream, 0, 1)); /* no reserved bits */


  
  payloadBits = add(payloadBits, encodeSbrGrid (sbrEnvData, hBitStream));
  payloadBits = add(payloadBits, encodeSbrDtdf (sbrEnvData, hBitStream));

  {
    Word32 i;

    for (i = 0; i < sbrEnvData->noOfnoisebands; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->sbr_invf_mode_vec[i], SI_SBR_INVF_MODE_BITS));
    }
  }


  payloadBits = add(payloadBits, writeEnvelopeData (sbrEnvData, hBitStream, 0));
  payloadBits = add(payloadBits, writeNoiseLevelData (sbrEnvData, hBitStream, 0));

  payloadBits = add(payloadBits, writeSyntheticCodingData (sbrEnvData,hBitStream));



  payloadBits = add(payloadBits, encodeExtendedData(hPsEnc,
                                                      bHeaderActive,
                                                      hBitStream));

  return payloadBits;
}


/*****************************************************************************

    functionname: encodeSbrChannelPairElement
    description:  encodes sbr CPE information
    returns:
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrChannelPairElement (HANDLE_SBR_ENV_DATA   sbrEnvDataLeft,
                             HANDLE_SBR_ENV_DATA   sbrEnvDataRight,
                             HANDLE_BIT_BUF        hBitStream,
                             Word32                coupling)
{
  Word16 payloadBits = 0;
  Word16 i = 0;

  payloadBits = add(payloadBits, WriteBits (hBitStream, 0, 1)); /* no reserved bits */

  payloadBits = add(payloadBits, WriteBits (hBitStream, coupling, SI_SBR_COUPLING_BITS));

  test();
  if (coupling) {
    payloadBits = add(payloadBits, encodeSbrGrid (sbrEnvDataLeft, hBitStream));
    payloadBits = add(payloadBits, encodeSbrDtdf (sbrEnvDataLeft, hBitStream));
    payloadBits = add(payloadBits, encodeSbrDtdf (sbrEnvDataRight, hBitStream));

    for (i = 0; i < sbrEnvDataLeft->noOfnoisebands; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvDataLeft->sbr_invf_mode_vec[i], SI_SBR_INVF_MODE_BITS));
    }




    payloadBits = add(payloadBits, writeEnvelopeData  (sbrEnvDataLeft,  hBitStream,1));
    payloadBits = add(payloadBits, writeNoiseLevelData (sbrEnvDataLeft,  hBitStream,1));
    payloadBits = add(payloadBits, writeEnvelopeData  (sbrEnvDataRight, hBitStream,1));
    payloadBits = add(payloadBits, writeNoiseLevelData (sbrEnvDataRight, hBitStream,1));




    payloadBits = add(payloadBits, writeSyntheticCodingData (sbrEnvDataLeft,hBitStream));
    payloadBits = add(payloadBits, writeSyntheticCodingData (sbrEnvDataRight,hBitStream));


  } else { /* no coupling */
    payloadBits = add(payloadBits, encodeSbrGrid (sbrEnvDataLeft,  hBitStream));
    payloadBits = add(payloadBits, encodeSbrGrid (sbrEnvDataRight, hBitStream));
    payloadBits = add(payloadBits, encodeSbrDtdf (sbrEnvDataLeft,  hBitStream));
    payloadBits = add(payloadBits, encodeSbrDtdf (sbrEnvDataRight, hBitStream));

    for (i = 0; i < sbrEnvDataLeft->noOfnoisebands; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvDataLeft->sbr_invf_mode_vec[i], 
                                                  SI_SBR_INVF_MODE_BITS));
    }
    for (i = 0; i < sbrEnvDataRight->noOfnoisebands; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvDataRight->sbr_invf_mode_vec[i], 
                                                  SI_SBR_INVF_MODE_BITS));
    }

    

    payloadBits = add(payloadBits, writeEnvelopeData  (sbrEnvDataLeft,  hBitStream,0));
    payloadBits = add(payloadBits, writeEnvelopeData  (sbrEnvDataRight, hBitStream,0));
    payloadBits = add(payloadBits, writeNoiseLevelData (sbrEnvDataLeft,  hBitStream,0));
    payloadBits = add(payloadBits, writeNoiseLevelData (sbrEnvDataRight, hBitStream,0));


    payloadBits = add(payloadBits, writeSyntheticCodingData (sbrEnvDataLeft,hBitStream));
    payloadBits = add(payloadBits, writeSyntheticCodingData (sbrEnvDataRight,hBitStream));

  } /* coupling */

  payloadBits = add(payloadBits, encodeExtendedData(NULL,
                                                    0,
                                                    hBitStream));


  return payloadBits;
}

static Word16 ceil_ln2(Word32 x)
{
  Word16 tmp=-1;
  test();
  while(L_shl(1, tmp = add(tmp, 1)) < x) {
    test();
  };
  return(tmp);
}


/*****************************************************************************

    functionname: encodeSbrGrid
    description:  if hBitStream != NULL writes bits that describes the
                  time/frequency grouping of a frame; else counts them only
    returns:      number of bits written or counted
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrGrid (HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_BIT_BUF hBitStream)
{
  Word16 payloadBits = 0;
  Word16 i, temp;
  Word16 bufferFrameStart = sbrEnvData->hSbrBSGrid->bufferFrameStart;
  Word16 numberTimeSlots  = sbrEnvData->hSbrBSGrid->numberTimeSlots;
  

  payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->frameClass,
                                            SBR_CLA_BITS));

  test();
  switch (sbrEnvData->hSbrBSGrid->frameClass) {
  case FIXFIX:
    temp = ceil_ln2(sbrEnvData->hSbrBSGrid->bs_num_env);
    payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_ENV_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->freq_res_fixfix, SBR_RES_BITS));
    break;

  case FIXVAR:
  case VARFIX:
    test(); sub(1, 1);
    if (sbrEnvData->hSbrBSGrid->frameClass == FIXVAR)
      temp = sub(sbrEnvData->hSbrBSGrid->bs_abs_bord, add(bufferFrameStart, numberTimeSlots));
    else
      temp = sub(sbrEnvData->hSbrBSGrid->bs_abs_bord, bufferFrameStart);

    payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_ABS_BITS));

    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->n, SBR_NUM_BITS));
    
    for (i = 0; i < sbrEnvData->hSbrBSGrid->n; i++) {
      temp = shr(sub(sbrEnvData->hSbrBSGrid->bs_rel_bord[i], 2), 1);
      payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_REL_BITS));
    }

    temp = ceil_ln2(L_add(sbrEnvData->hSbrBSGrid->n, 2));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->p, temp));

    for (i = 0; i < sbrEnvData->hSbrBSGrid->n + 1; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->v_f[i],
                                                SBR_RES_BITS));
    }
    break;

  case VARVAR:    
    temp = sub(sbrEnvData->hSbrBSGrid->bs_abs_bord_0, bufferFrameStart);
    payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_ABS_BITS));
    temp = sub(sbrEnvData->hSbrBSGrid->bs_abs_bord_1, add(bufferFrameStart, numberTimeSlots));
    payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_ABS_BITS));

    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->bs_num_rel_0, SBR_NUM_BITS));
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->bs_num_rel_1, SBR_NUM_BITS));

    for (i = 0; i < sbrEnvData->hSbrBSGrid->bs_num_rel_0; i++) {
      temp = shr(sub(sbrEnvData->hSbrBSGrid->bs_rel_bord_0[i], 2), 1);
      payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_REL_BITS));
    }

    for (i = 0; i < sbrEnvData->hSbrBSGrid->bs_num_rel_1; i++) {
      temp = shr(sub(sbrEnvData->hSbrBSGrid->bs_rel_bord_1[i], 2), 1);
      payloadBits = add(payloadBits, WriteBits (hBitStream, temp, SBR_REL_BITS));
    }

    temp = ceil_ln2(L_add(L_add(sbrEnvData->hSbrBSGrid->bs_num_rel_0, 
                    sbrEnvData->hSbrBSGrid->bs_num_rel_1), 2));
    payloadBits =  add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->p, temp));

    temp = add(add(sbrEnvData->hSbrBSGrid->bs_num_rel_0,
      sbrEnvData->hSbrBSGrid->bs_num_rel_1), 1);

    for (i = 0; i < temp; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->hSbrBSGrid->v_fLR[i],
                                                  SBR_RES_BITS));
    }
    break;
  }

  return payloadBits;
}


/*****************************************************************************

    functionname: encodeSbrDtdf
    description:  writes bits that describes the direction of the envelopes of a frame
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
encodeSbrDtdf (HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_BIT_BUF hBitStream)
{
  Word16 i, payloadBits = 0, noOfNoiseEnvelopes;

  test();
  noOfNoiseEnvelopes = (sbrEnvData->noOfEnvelopes > 1) ? 2 : 1;

  for (i = 0; i < sbrEnvData->noOfEnvelopes; ++i) {
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->domain_vec[i], SBR_DIR_BITS));
  }
  for (i = 0; i < noOfNoiseEnvelopes; ++i) {
    payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->domain_vec_noise[i], SBR_DIR_BITS));
  }

  return payloadBits;
}


/*****************************************************************************

    functionname: writeNoiseLevelData
    description:  writes bits corresponding to the noise-floor-level
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
writeNoiseLevelData (HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_BIT_BUF hBitStream, Word32 coupling)
{
  Word16 j, i, payloadBits = 0;
  Word16 nNoiseEnvelopes = ((sbrEnvData->noOfEnvelopes > 1) ? 2 : 1);
  test();

  for (i = 0; i < nNoiseEnvelopes; i++) {
    test();
    switch (sbrEnvData->domain_vec_noise[i]) {
    case FREQ:
      test(); test();
      if (coupling && sbrEnvData->balance) {
        payloadBits =  add(payloadBits, WriteBits (hBitStream,
                                                   sbrEnvData->sbr_noise_levels[i * sbrEnvData->noOfnoisebands],
                                                   sbrEnvData->si_sbr_start_noise_bits_balance));
      } else {
        payloadBits =  add(payloadBits, WriteBits (hBitStream,
                                                   sbrEnvData->sbr_noise_levels[i * sbrEnvData->noOfnoisebands],
                                                   sbrEnvData->si_sbr_start_noise_bits));
      }
      
      for (j = 1 + i * sbrEnvData->noOfnoisebands; j < (sbrEnvData->noOfnoisebands * (1 + i)); j++) {
        test();
        if (coupling) {
          test();
          if (sbrEnvData->balance) {
            /* coupling && balance */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableNoiseBalanceFreqC[sbrEnvData->sbr_noise_levels[j] +
                                                                                             CODE_BOOK_SCF_LAV_BALANCE11],
                                                      sbrEnvData->hufftableNoiseBalanceFreqL[sbrEnvData->sbr_noise_levels[j] +
                                                                                             CODE_BOOK_SCF_LAV_BALANCE11]));
          } else {
            /* coupling && !balance */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableNoiseLevelFreqC[sbrEnvData->sbr_noise_levels[j] +
                                                                                           CODE_BOOK_SCF_LAV11],
                                                      sbrEnvData->hufftableNoiseLevelFreqL[sbrEnvData->sbr_noise_levels[j] +
                                                                                           CODE_BOOK_SCF_LAV11]));
          }
        } else {
          /* !coupling */
          logic16(); logic16(); /* Word8 read access */ 
          payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                    sbrEnvData->hufftableNoiseFreqC[sbrEnvData->sbr_noise_levels[j] +
                                                                                    CODE_BOOK_SCF_LAV11],
                                                    sbrEnvData->hufftableNoiseFreqL[sbrEnvData->sbr_noise_levels[j] +
                                                                                    CODE_BOOK_SCF_LAV11]));
        }
      }
      break;
      
    case TIME:
      for (j = i * sbrEnvData->noOfnoisebands; j < (sbrEnvData->noOfnoisebands * (1 + i)); j++) {
        test();
        if (coupling) {
          test();
          if (sbrEnvData->balance) {
            /* coupling && balance */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableNoiseBalanceTimeC[sbrEnvData->sbr_noise_levels[j] +
                                                                                             CODE_BOOK_SCF_LAV_BALANCE11],
                                                      sbrEnvData->hufftableNoiseBalanceTimeL[sbrEnvData->sbr_noise_levels[j] +
                                                                                                CODE_BOOK_SCF_LAV_BALANCE11]));
          } else {
            /* coupling && !balance */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableNoiseLevelTimeC[sbrEnvData->sbr_noise_levels[j] +
                                                                                           CODE_BOOK_SCF_LAV11],
                                                      sbrEnvData->hufftableNoiseLevelTimeL[sbrEnvData->sbr_noise_levels[j] +
                                                                                           CODE_BOOK_SCF_LAV11]));
          }
        } else {
          /* !coupling */
          logic16(); logic16(); /* Word8 read access */ 
          payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                    sbrEnvData->hufftableNoiseLevelTimeC[sbrEnvData->sbr_noise_levels[j] +
                                                                                         CODE_BOOK_SCF_LAV11],
                                                    sbrEnvData->hufftableNoiseLevelTimeL[sbrEnvData->sbr_noise_levels[j] +
                                                                                         CODE_BOOK_SCF_LAV11]));
        }
      }
      break;
    }
  }
  return payloadBits;
}


/*****************************************************************************

    functionname: writeEnvelopeData
    description:  writes bits corresponding to the envelope
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16
writeEnvelopeData (HANDLE_SBR_ENV_DATA sbrEnvData, HANDLE_BIT_BUF hBitStream, Word32 coupling)
{
  Word16 payloadBits = 0, j, i;
  Word32 delta;

  for (j = 0; j < sbrEnvData->noOfEnvelopes; j++) { /* loop over all envelopes */
    test(); L_sub(1, 1);
    if (sbrEnvData->domain_vec[j] == FREQ) {
      test(); test();
      if (coupling && sbrEnvData->balance) {
        payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->ienvelope[j][0], sbrEnvData->si_sbr_start_env_bits_balance));
      } else {
        payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->ienvelope[j][0], sbrEnvData->si_sbr_start_env_bits));
      }
    }

    for (i = 1 - sbrEnvData->domain_vec[j]; i < sbrEnvData->noScfBands[j]; i++) {
      delta = sbrEnvData->ienvelope[j][i];
      if (coupling && sbrEnvData->balance) {
        assert (abs (delta) <= sbrEnvData->codeBookScfLavBalance);
      } else {
        assert (abs (delta) <= sbrEnvData->codeBookScfLav);
      }
      test();
      if (coupling) {
        test();
        if (sbrEnvData->balance) {
          test();
          if (sbrEnvData->domain_vec[j]) {
            /* coupling && balance && TIME */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableBalanceTimeC[delta + sbrEnvData->codeBookScfLavBalance],
                                                      sbrEnvData->hufftableBalanceTimeL[delta + sbrEnvData->codeBookScfLavBalance]));
          } else {
            /* coupling && balance && FREQ */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableBalanceFreqC[delta + sbrEnvData->codeBookScfLavBalance],
                                                      sbrEnvData->hufftableBalanceFreqL[delta + sbrEnvData->codeBookScfLavBalance]));
          }
        } else {
          test();
          if (sbrEnvData->domain_vec[j]) {
            /* coupling && !balance && TIME */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableLevelTimeC[delta + sbrEnvData->codeBookScfLav],
                                                      sbrEnvData->hufftableLevelTimeL[delta + sbrEnvData->codeBookScfLav]));
          } else {
            /* coupling && !balance && FREQ */
            logic16(); logic16(); /* Word8 read access */ 
            payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                      sbrEnvData->hufftableLevelFreqC[delta + sbrEnvData->codeBookScfLav],
                                                      sbrEnvData->hufftableLevelFreqL[delta + sbrEnvData->codeBookScfLav]));
          }
        }
      } else {
        test();
        if (sbrEnvData->domain_vec[j]) {
          /* !coupling && TIME */
          logic16(); logic16(); /* Word8 read access */ 
          payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                    sbrEnvData->hufftableTimeC[delta + sbrEnvData->codeBookScfLav],
                                                    sbrEnvData->hufftableTimeL[delta + sbrEnvData->codeBookScfLav]));
        } else {
          /* !coupling && FREQ */
          logic16(); logic16(); /* Word8 read access */ 
          payloadBits = add(payloadBits, WriteBits (hBitStream,
                                                    sbrEnvData->hufftableFreqC[delta + sbrEnvData->codeBookScfLav],
                                                    sbrEnvData->hufftableFreqL[delta + sbrEnvData->codeBookScfLav]));
        }
      }
    }
  }
  return payloadBits;
}


/*****************************************************************************

    functionname: encodeExtendedData
    description:  writes bits corresponding to the extended data
    returns:      number of bits written
    input:
    output:

*****************************************************************************/
static Word16 encodeExtendedData (HANDLE_PS_ENC        hPsEnc,
                                  Word32               bHeaderActive,
                                  HANDLE_BIT_BUF       hBitStream)
{
  Word16 extDataSize;
  Word16 payloadBits = 0;
  Word16 writtenNoBits = 0; /* Number of actual data bits written after the length information */


  extDataSize = getSbrExtendedDataSize(hPsEnc, bHeaderActive);

  test();
  if (extDataSize != 0) {
    Word32 maxExtSize = (1<<SI_SBR_EXTENSION_SIZE_BITS) - 1;
    payloadBits = add(payloadBits, WriteBits (hBitStream, 1, SI_SBR_EXTENDED_DATA_BITS));
    assert(extDataSize <= SBR_EXTENDED_DATA_MAX_CNT);

    test(); sub(1, 1);
    if (extDataSize < maxExtSize) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, extDataSize, SI_SBR_EXTENSION_SIZE_BITS));
    } else {
      payloadBits = add(payloadBits, WriteBits (hBitStream, maxExtSize, SI_SBR_EXTENSION_SIZE_BITS));
      payloadBits = add(payloadBits, WriteBits (hBitStream, L_sub(extDataSize, maxExtSize), SI_SBR_EXTENSION_ESC_COUNT_BITS));
    }

    /* parametric coding signalled here? */

    writtenNoBits = add(writtenNoBits, hPsEnc->psBitBuf.cntBits);
    payloadBits = add(payloadBits, writtenNoBits);

    {
      Word32 i, bit;
      for (i=0; i<writtenNoBits; i++) {
        bit = ReadBits (&hPsEnc->psBitBuf, 1);
        bit = WriteBits (hBitStream, bit, 1);
      }
    }

    /* Insert fillbits after the last extension */
    writtenNoBits = writtenNoBits%8;                                                    logic32();
    test();
    if (writtenNoBits) {
      payloadBits = add(payloadBits, WriteBits(hBitStream, 0, sub(8, writtenNoBits)));
    }


  } else {
    payloadBits = add(payloadBits, WriteBits (hBitStream, 0, SI_SBR_EXTENDED_DATA_BITS));
  }

  return payloadBits;
}


/*****************************************************************************

    functionname: writeSyntheticCodingData
    description:  writes bits corresponding to the "synthetic-coding"-extension
    returns:      number of bits written
    input:        
    output:        

*****************************************************************************/
static Word16 writeSyntheticCodingData (HANDLE_SBR_ENV_DATA  sbrEnvData,
                                        HANDLE_BIT_BUF       hBitStream)
                                     
{
  Word16 i;
  Word16 payloadBits = 0;

  payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->addHarmonicFlag, 1));

  test();
  if (sbrEnvData->addHarmonicFlag) {
    for (i = 0; i < sbrEnvData->noHarmonics; i++) {
      payloadBits = add(payloadBits, WriteBits (hBitStream, sbrEnvData->addHarmonic[i], 1));
    }
  }

  return payloadBits;
}


/*****************************************************************************

    functionname: getSbrExtendedDataSize
    description:  counts the number of bits needed for encoding the
                  extended data (including extension id)

    returns:      number of bits needed for the extended data
    input:        
    output:

*****************************************************************************/
static Word16
getSbrExtendedDataSize (HANDLE_PS_ENC      hPsEnc,
                        Word32             bHeaderActive
                        )
{
  Word16 extDataBits = 0;

  /* add your new extended data counting methods here */

#ifndef MONO_ONLY
  test();
  if (hPsEnc) {
    WritePsData(hPsEnc, bHeaderActive);
    extDataBits = add(extDataBits, hPsEnc->psBitBuf.cntBits);
  }
#endif

  return shr(add(extDataBits, 7), 3);
}
