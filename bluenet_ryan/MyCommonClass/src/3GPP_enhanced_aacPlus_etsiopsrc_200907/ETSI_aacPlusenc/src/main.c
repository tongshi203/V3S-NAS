/*
   Sample Application
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* for MPEG-4/3GPP file format support */
#include "mp4file.h"
#include "ISOMovies.h"

/* for WAV audio file support */
#include "au_channel.h"


#include "aacenc.h"
#include "downsample_FIR.h"
#include "sbr_main.h"

/* ETSI operators types and WMOPS measurement */
#include "typedef.h"
#include "count.h"

/* Profiling */
enum {
  SBR_ENC = 0,
  RESAMPLER,
  AAC_ENC
};

#ifdef WMOPS
#define MEASURE_MAIN_WORST_CASE
/*#define MEASURE_MAIN_AVERAGE*/
#endif

/* dynamic buffer of SBR that can be reused for resampling */
extern Word16 sbr_envRBuffer[];

#define CORE_DELAY   (1600)
#define INPUT_DELAY  ((CORE_DELAY)*2 +6*64-2048+1)     /* ((1600 (core codec)*2 (multi rate) + 6*64 (sbr dec delay) - 2048 (sbr enc delay) + magic*/
#define MAX_DS_FILTER_DELAY 70                         /* the additional max resampler filter delay (source fs)*/
#define CORE_INPUT_OFFSET_PS (0)  /* (96-64) makes AAC still some 64 core samples too early wrt SBR ... maybe -32 would be even more correct, but 1024-32 would need additional SBR bitstream delay by one frame */


/*

input buffer (1ch)

|------------ 1537   -------------|-----|------ 2048 -----------------|
(core2sbr delay     )          ds     (read, core and ds area)
*/

static Word16 inputBuffer[(AACENC_BLOCKSIZE*2 + MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS];
static UWord32 outputBuffer[(6144/8)*MAX_CHANNELS/(sizeof(Word32))];

static RESAMPLER_FIR_2_1 down2Sampler[MAX_CHANNELS];
static RESAMPLER_FIR_2_1 up2Sampler[MAX_CHANNELS];
static RESAMPLER_FIR_3_2 down3Sampler[MAX_CHANNELS];

Word32 cntId[MAXCOUNTERS]; /* global counters, may be used for profiling */


int main( int argc, char **argv )
{
  AACENC_CONFIG config;

  AuChanInfo inputInfo;
  hAudioChannel inputFile = NULL;
  AuChanType auType = TYPE_AUTODETECT ; /* must be set */
  AuChanMode auFlags = AU_CHAN_READ;
  AuChanError auError;

  HANDLE_MP4_FILE hMp4File;

  Word16 error;
  Flag   bEncodeMono = 0;
  Word16 ch;
  Word16 dummy;

  Word32 bitrate;
  Word16 nChannelsAAC, nChannelsSBR;
  Word32 sampleRateAAC;
  Word32 frmCnt = 0;
  Word16 bandwidth = 0;

  Word16 numAncDataBytes=0;
  UWord8 ancDataBytes[MAX_PAYLOAD_SIZE];

  Word32 numSamplesRead;
  Flag   bDingleRate = 0;
  Flag   useParametricStereo = 0;
  Word16 coreWriteOffset = 0;
  Word16 coreReadOffset = 0;
  Word16 envWriteOffset = 0;
  Word16 envReadOffset = 0;
  Word16 writeOffset=INPUT_DELAY*MAX_CHANNELS;

  struct AAC_ENCODER *hAacEnc = NULL;
  struct SBR_ENCODER *hEnvEnc = NULL;

  Flag   bDoUpsample = 0;
  Flag   upsampleReadOffset = 0;

  Word16 inSamples;
  Flag   bDo32Resample = 0;
  Word16 nRuns;
  Word16 *resamplerScratch = sbr_envRBuffer;

  UWord32 numOutBytes = 0;
  Word32 i = 0;
  UWord8 *bitstreamOut = (UWord8*)outputBuffer;
  Word32 *nOutBytes = (Word32*)&numOutBytes;                         
  Word16 numEncSamples;
    
  Flag   downsample = 0;
    

  fprintf(stderr,"\n");
  fprintf(stderr,"*************************************************************\n");
  fprintf(stderr,"* Enhanced aacPlus 3GPP ETSI-op Reference Encoder\n");
  fprintf(stderr,"* Build %s, %s\n", __DATE__, __TIME__);
  fprintf(stderr,"*\n");
  fprintf(stderr,"*************************************************************\n\n");


  /*
   * parse command line arguments
   */
  if (argc != 5) {
    fprintf(stderr, "\nUsage:   %s <wav_file> <bitstream_file> <bitrate> <(m)ono/(s)tereo>\n", argv[0]);
    fprintf(stderr, "\nExample: %s input.wav out.3gp 24000 s\n", argv[0]);
    return 0;
  }

  bitrate = atoi(argv[3]);

  if ( strcmp (argv[4],"m") == 0 ) {
    bEncodeMono = 1;
  }
  else {
    if ( strcmp (argv[4],"s") != 0 ) {
      fprintf(stderr, "\nWrong mode %s, use either (m)ono or (s)tereo\n", argv[4]);
      return 0;
    }
  }
  fflush(stdout);
  
  /*
    open audio input file
  */

  inputInfo.bitsPerSample  = 16 ;       /* only relevant if valid == 1 */
  inputInfo.sampleRate     = 44100 ;    /* only relevant if valid == 1 */
  inputInfo.nChannels      = 2 ;        /* only relevant if valid == 1 */
  inputInfo.nSamples       = 0 ;        /* only relevant if valid == 1 */
  inputInfo.isLittleEndian = 1;

  inputInfo.fpScaleFactor  = 1.0 ;      /* must be set */
  inputInfo.valid          = 1 ;        /* must be set */
  inputInfo.useWaveExt     = 0;


  auError = AuChannelOpen (&inputFile, argv[1], auFlags, &auType, &inputInfo);

  if(auError != AU_CHAN_OK){
    fprintf(stderr,"could not open %s\n", argv[1]);
    exit(10);
  }

  if ( (!bEncodeMono) && (inputInfo.nChannels!=2) ) {
    fprintf(stderr,"Need stereo input for stereo coding mode !\n");
    exit(10);
  }

#if (MAX_CHANNELS==1)
  if (!bEncodeMono) {
    fprintf(stderr,"Mono encoder cannot encode stereo coding mode !\n");
    exit(10);
  }

  if (inputInfo.nChannels!=1) {
    fprintf(stderr,"Mono encoder can only handle mono input files !\n");
    exit(10);
  }
#endif

  sampleRateAAC = inputInfo.sampleRate;

  /* open encoder */
  /* make reasonable default settings */
  AacInitDefaultConfig(&config);
    
  nChannelsAAC = nChannelsSBR = bEncodeMono ? 1:inputInfo.nChannels;

  if ( (inputInfo.nChannels == 2) && (!bEncodeMono) && (bitrate >= 16000) && (bitrate < 44001) ) {
    useParametricStereo = 1;
  }

  if (useParametricStereo) {
    nChannelsAAC = 1;
    nChannelsSBR = 2;
  }

  if ( ( (inputInfo.sampleRate == 48000) && (nChannelsAAC == 2) && (bitrate < 24000) )
       || ( (inputInfo.sampleRate == 48000) && (nChannelsAAC == 1) && (bitrate < 12000) ) ) {
    bDo32Resample  = 1;
  }

  if (inputInfo.sampleRate == 16000) {
    bDoUpsample = 1;
    inputInfo.sampleRate = 32000;
    bDingleRate = 1;
  }

  sampleRateAAC = inputInfo.sampleRate;

  if (bDo32Resample) {
    sampleRateAAC = 32000;
  }

  config.bitRate = bitrate;
  config.nChannelsIn = nChannelsSBR;
  config.nChannelsOut = nChannelsAAC;
    
  config.bandWidth=bandwidth;
    
  if ( IsSbrSettingAvail(bitrate, nChannelsAAC, sampleRateAAC, (UWord32*)&sampleRateAAC) ) {
    sbrConfiguration sbrConfig;
        
    InitializeSbrDefaults( &sbrConfig );
        
    sbrConfig.usePs = useParametricStereo;

    AdjustSbrSettings( &sbrConfig,
                       bitrate,
                       nChannelsAAC,
                       sampleRateAAC,
                       AACENC_TRANS_FAC,
                       24000 );      
        
    EnvOpen( &hEnvEnc,
             &sbrConfig,
             &config.bandWidth );
        
    /* set FIR resampler */
    if ( bDoUpsample ) {
      downsample = 0;
      InitResampler_firUp2( &(up2Sampler[0]), sampleRateAAC, sampleRateAAC * 2 );
#if (MAX_CHANNELS>1)
      InitResampler_firUp2( &(up2Sampler[1]), sampleRateAAC, sampleRateAAC * 2 );
#endif
      if (useParametricStereo) {
        writeOffset   += AACENC_BLOCKSIZE * MAX_CHANNELS;
        upsampleReadOffset  = writeOffset;
        envWriteOffset  = envReadOffset;
      }
      else {
        writeOffset        += AACENC_BLOCKSIZE * MAX_CHANNELS;
        coreReadOffset      = writeOffset;
        upsampleReadOffset  = writeOffset - (((INPUT_DELAY-up2Sampler[0].delay) >> 1) * MAX_CHANNELS);
        envWriteOffset      = ((INPUT_DELAY-up2Sampler[0].delay) &  0x1) * MAX_CHANNELS;
        envReadOffset       = 0;
      }
    }
    else {
      if ( bDo32Resample ) {
        InitResampler_firDown32( &(up2Sampler[0]), &(down3Sampler[0]), sampleRateAAC * 3, sampleRateAAC * 2 );
#if (MAX_CHANNELS>1)
        InitResampler_firDown32( &(up2Sampler[1]), &(down3Sampler[1]), sampleRateAAC * 3, sampleRateAAC * 2 );
#endif
      }
      if (useParametricStereo) {
        downsample = 0;
        envReadOffset = (MAX_DS_FILTER_DELAY + INPUT_DELAY)*MAX_CHANNELS;
        coreWriteOffset = CORE_INPUT_OFFSET_PS;
        writeOffset = envReadOffset;
      }
      else {
        downsample = 1;
        writeOffset = INPUT_DELAY * MAX_CHANNELS;
      }
    }
  }
  else {
    fprintf(stderr,"\nconfiguration not supported\n" );
    return 1;
  }
    
  if ( downsample ) {
    Word32 transitionFac;
    transitionFac = (sampleRateAAC-config.bandWidth*2) * 1000 / sampleRateAAC;    
    InitResampler_firDown2( &(down2Sampler[0]), sampleRateAAC * 2, sampleRateAAC );
#if (MAX_CHANNELS>1)
    InitResampler_firDown2( &(down2Sampler[1]), sampleRateAAC * 2, sampleRateAAC );
#endif
    if (!useParametricStereo)
      writeOffset += down2Sampler[0].delay * MAX_CHANNELS;
  }
    
  config.sampleRate = sampleRateAAC;

  error = AacEncOpen(&hAacEnc, config);
    
  if ( error ) {
    fprintf(stderr,"\nAacEncOpen failed\n" );
    return 1;
  }
  
  for ( i = 0; i < (Word32)sizeof(inputBuffer)/2; i++ ) {
    inputBuffer[i] = 0;
  }


  /*
    set up MPEG-4/3GPP file format library (not instrumented nor accounted for RAM requirements)
  */

  {
    UWord8 ASConfigBuffer[80];
    UWord32  nConfigBits;
    UWord32  nConfigBytes;

    memset (ASConfigBuffer, 0, 80);

    if ( GetMPEG4ASConfig( (unsigned int)sampleRateAAC,
                           (unsigned int)nChannelsAAC,
                           (unsigned  char*)ASConfigBuffer,
                           (unsigned int*)&nConfigBits,
                           (int)1,
                           (int)bDingleRate) ) {
      fprintf(stderr, "\nCould not initialize Audio Specific Config\n");
      exit(10);
    }


    nConfigBytes = (nConfigBits+7)>>3;

    if (OpenMP4File(&hMp4File,
                    ASConfigBuffer,
                    nConfigBytes,
                    argv[2],
                    (!bDingleRate) ? sampleRateAAC*2 : sampleRateAAC, /* output sampleRate */
                    bitrate,
                    nChannelsAAC,
                    1,
                    1) ) {
      fprintf(stderr, "\nFailed to create 3GPP file\n") ;
      exit(10);
    }
  }

  /*
    Be verbose
  */


  fprintf(stdout,"input file %s: \nsr = %d, nc = %d\n\n",
          argv[1], (bDoUpsample) ? inputInfo.sampleRate/2 : inputInfo.sampleRate, inputInfo.nChannels);
  fprintf(stdout,"output file %s: \nbr = %ld sr-OUT = %ld  nc-OUT = %d\n\n",
          argv[2], bitrate, (!bDingleRate) ? sampleRateAAC*2 : sampleRateAAC, nChannelsSBR);
  fflush(stdout);
  


  Init_WMOPS_counter();

#ifdef MEASURE_MAIN_AVERAGE
  cntId[SBR_ENC]        = getCounterId("SBR Encoder");
  cntId[RESAMPLER]      = getCounterId("Resampler");
  cntId[AAC_ENC]        = getCounterId("AAC Encoder");
#endif

#ifdef MEASURE_MAIN_WORST_CASE
  cntId[0]        = getCounterId("Enhanced aacPlus Encoder");
#endif
  /*
    set up input samples block size feed
  */
  inSamples = AACENC_BLOCKSIZE * inputInfo.nChannels * 2 ;

  if (bDo32Resample) {
    inSamples = (AACENC_BLOCKSIZE + AACENC_BLOCKSIZE / 2) * inputInfo.nChannels * 2;
  }
  else {
    if (bDoUpsample) {
      inSamples = AACENC_BLOCKSIZE * inputInfo.nChannels;
    }
    else {
      inSamples = AACENC_BLOCKSIZE * inputInfo.nChannels * 2;
    }
  }


  while ( 1 ) { /* encode loop */
    
#if defined(MEASURE_MAIN_AVERAGE) || defined(MEASURE_MAIN_WORST_CASE)
    /* initiate counter */
    setCounter(cntId[SBR_ENC]); 
    Reset_WMOPS_counter ();
#endif

    /* encode one frame */

    if (bDo32Resample) {
      Word16 outSamplesPerCh;
      Word16 idx = writeOffset;

      for (nRuns = 0; nRuns<2; nRuns++) {

        AuChannelReadShort(inputFile, resamplerScratch, inSamples/2, (int*)&numSamplesRead);
        
        if ( !numSamplesRead ) {
          fprintf( stderr, "\nend of %s reached\n", argv[2] );
          goto close_encoder;
        }
        
        /* downmix stereo input signal to mono */
        if ( inputInfo.nChannels == 2 && bEncodeMono) {
          for (i=0; i<numSamplesRead/2; i++)
            resamplerScratch[MAX_CHANNELS*i] = (resamplerScratch[2*i] + resamplerScratch[2*i+1])/2;
        }
        
        if ( config.nChannelsIn == 1 && inputInfo.nChannels == 1) {
          for(i=numSamplesRead-1;i>=0;i--) {
            resamplerScratch[MAX_CHANNELS*i] = resamplerScratch[i];
          }
          numSamplesRead *= 2;
        }
        
        /* 3:2 resampler */
        for ( ch = 0; ch < config.nChannelsIn; ch++ ) {
          Resample_firDown32( &(up2Sampler[ch]), 
                              &(down3Sampler[ch]), 
                              resamplerScratch+ch,
                              numSamplesRead/MAX_CHANNELS,
                              MAX_CHANNELS,
                              &inputBuffer[idx+ch],
                              &outSamplesPerCh,
                              MAX_CHANNELS );
        }
        idx += outSamplesPerCh * MAX_CHANNELS;
      }
      numSamplesRead = 2 * outSamplesPerCh * MAX_CHANNELS;
    }
    else {

      AuChannelReadShort(inputFile, inputBuffer + writeOffset, inSamples, (int*)&numSamplesRead);
      
      if ( !numSamplesRead ) {
        fprintf( stderr, "\nend of %s reached\n", argv[2] );
        goto close_encoder;
      }

      /* downmix stereo input signal to mono */
      if ( inputInfo.nChannels == 2 && bEncodeMono) {
        for (i=0; i<numSamplesRead/2; i++)
          inputBuffer[writeOffset+MAX_CHANNELS*i] = (inputBuffer[writeOffset+2*i] + inputBuffer[writeOffset+2*i+1])/2;
      }

#ifndef MONO_ONLY
      /* a stereo encoder always needs two channel input */
      if ( config.nChannelsIn == 1 && inputInfo.nChannels == 1) {
        for(i=numSamplesRead-1;i>=0;i--) {
          inputBuffer[writeOffset+MAX_CHANNELS*i] = inputBuffer[writeOffset+i];
        }
        numSamplesRead *= 2;
      }
#endif
    }

    numEncSamples = 0;

    if (bDoUpsample) {
      for(ch=0;ch < inputInfo.nChannels;ch++) {
        Resample_firUp2( &(up2Sampler[ch]), 
                         inputBuffer+upsampleReadOffset+ch,
                         numSamplesRead/MAX_CHANNELS,
                         MAX_CHANNELS,
                         inputBuffer+envWriteOffset+ch,
                         &dummy,
                         MAX_CHANNELS );
      }
    }
    
    EnvEncodeFrame( hEnvEnc,
                    inputBuffer+envReadOffset,
                    inputBuffer+coreWriteOffset,             /*!< Downmixed time samples (output) */
                    MAX_CHANNELS,
                    &numAncDataBytes,
                    ancDataBytes );

#ifdef MEASURE_MAIN_AVERAGE
    /* setup counters for resampler */
    fwc();
    setCounter(cntId[RESAMPLER]); 
    Reset_WMOPS_counter();
#endif
    if ( downsample ) {
      for ( ch = 0; ch < nChannelsAAC; ch++ ) {
        Word16 outSamples;
            
        Resample_firDown2( &(down2Sampler[ch]), 
                           inputBuffer+writeOffset+ch,
                           numSamplesRead/MAX_CHANNELS,
                           MAX_CHANNELS,
                           inputBuffer+ch,
                           &outSamples,
                           MAX_CHANNELS );
          
        numEncSamples += outSamples; 
      }
    } 
    else {
      if (useParametricStereo) {
        for ( i = AACENC_BLOCKSIZE-1; i >= 0; i-- ) {
          inputBuffer[coreReadOffset+i*MAX_CHANNELS] = inputBuffer[coreWriteOffset+i];
        }
      }
    }

#ifdef MEASURE_MAIN_AVERAGE
    /* setup counters for AAC encoder */
    fwc();
    setCounter(cntId[AAC_ENC]); 
    Reset_WMOPS_counter();
#endif


    AacEncEncode( hAacEnc,
                  inputBuffer+coreReadOffset,
                  ancDataBytes,
                  &numAncDataBytes,
                  bitstreamOut,
                  nOutBytes );

    if (bDoUpsample) {
      memmove( &inputBuffer[envReadOffset],
               &inputBuffer[envReadOffset+AACENC_BLOCKSIZE*MAX_CHANNELS*2],
               (envWriteOffset-envReadOffset)*sizeof(Word16));
      
      memmove( &inputBuffer[upsampleReadOffset],
               &inputBuffer[upsampleReadOffset+AACENC_BLOCKSIZE*MAX_CHANNELS],
               (writeOffset-upsampleReadOffset)*sizeof(Word16));
    }
    else {
      memmove( inputBuffer,inputBuffer+AACENC_BLOCKSIZE*2*MAX_CHANNELS,writeOffset*sizeof(Word16));
    }
    
    /*
      Write one frame of encoded audio to file
    */

    if (numOutBytes) {
      MP4FileAddFrame( hMp4File,
                       (unsigned int*)outputBuffer,
                       numOutBytes );
    }

    frmCnt++;

    fprintf( stderr, "\r[%ld]", frmCnt );

    fwc();
#ifdef MEASURE_MAIN_WORST_CASE
    WMOPS_output(0);
#endif

  }


 close_encoder:

  /*
    Close encoder
  */

#ifdef MEASURE_MAIN_AVERAGE
  WMOPS_summary(0);
#endif

  fprintf( stderr,"\n" ); 
  fflush( stderr );

  AuChannelClose (inputFile);

  if (WriteMP4File( hMp4File)) {
    fprintf(stderr, "Writing of 3GPP file failed.");
    exit(10);
  }
  CloseMP4File( hMp4File);

  printf("\nencoding finished\n");

  return 0;
}


