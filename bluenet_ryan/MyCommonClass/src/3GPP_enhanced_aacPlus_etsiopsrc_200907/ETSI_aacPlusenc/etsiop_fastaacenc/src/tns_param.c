/*
   TNS parameters
 */
#include "tns.h"
#include "tns_param.h"
#include "aac_rom.h"
#include "count.h"

#ifndef NULL
#define NULL 0
#endif

/*****************************************************************************

    functionname: GetTnsParam
    description:  Get threshold calculation parameter set that best matches
                  the bit rate
    returns:      the parameter set
    input:        blockType, bitRate
    output:       the parameter set

*****************************************************************************/
void GetTnsParam(TNS_CONFIG_TABULATED *tnsConfigTab, Word32 bitRate, Word16 channels, Word16 blockType) {

  UWord16 i;
  Word16 s_temp;
  Word32 temp1,temp2;

  for(i = 0; i < sizeof(tnsInfoTab)/(Word16)sizeof(TNS_INFO_TAB); i++) {

    temp1 = L_sub( bitRate, tnsInfoTab[i].bitRateFrom );
    temp2 = L_sub( bitRate, tnsInfoTab[i].bitRateTo );
                                                                                                   test(); test();
    if ( temp1 >= 0 && temp2 <= 0 ) {
      s_temp = sub( blockType, LONG_WINDOW );                                                      test();
      if ( s_temp == 0 ) {
        s_temp = sub( channels, 1 );                                                               test();
        if ( s_temp == 0 ) {                                                                       test();
          if(tnsConfigTab != NULL)
            *tnsConfigTab=*tnsInfoTab[i].paramMono_Long;                                           move32();
        }
        else {                                                                                     test();
          if ( s_temp > 0 ) {                                                                      test();
            if(tnsConfigTab != NULL)
              *tnsConfigTab=*tnsInfoTab[i].paramStereo_Long;                                       move32();
          }
        }
      }
      else {
        s_temp = sub( blockType, SHORT_WINDOW );                                                   test();
        if ( s_temp == 0 ) {
          s_temp = sub( channels, 1 );                                                             test();
          if ( s_temp == 0 ) {                                                                     test();
            if(tnsConfigTab != NULL)
              *tnsConfigTab=*tnsInfoTab[i].paramMono_Short;                                        move32();
          }
          else {                                                                                   test();
            if ( s_temp > 0 ) {                                                                    test();
              if(tnsConfigTab != NULL)
                *tnsConfigTab=*tnsInfoTab[i].paramStereo_Short;                                    move32();
            }
          }
        }
      }
    }
  }
}

/*****************************************************************************

    functionname: GetTnsMaxBands
    description:  sets tnsMaxSfbLong, tnsMaxSfbShort according to sampling rate
    returns:
    input:        samplingRate, profile, granuleLen
    output:       tnsMaxSfbLong, tnsMaxSfbShort

*****************************************************************************/
void GetTnsMaxBands(Word32 samplingRate, Word16 blockType, Word16* tnsMaxSfb){

  UWord16 i;
  Word32 temp;

  *tnsMaxSfb = sub(*tnsMaxSfb,1);                                                                  move16();

  for (i=0; i<sizeof(tnsMaxBandsTab)/sizeof(TNS_MAX_TAB_ENTRY); i++) {
    temp = L_sub(samplingRate, tnsMaxBandsTab[i].samplingRate );                                   test();
    if ( temp == 0 ) {

      Word16 s_temp = sub(blockType,2);                                                            test();
      if ( s_temp == 0 ) {
        *tnsMaxSfb = tnsMaxBandsTab[i].maxBandShort;                                               move16();
      }
      else {
        *tnsMaxSfb = tnsMaxBandsTab[i].maxBandLong;                                                move16();
      }
      break;
    }
  }
}
