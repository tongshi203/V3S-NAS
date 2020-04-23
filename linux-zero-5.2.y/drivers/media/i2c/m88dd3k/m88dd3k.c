
#include "m88dd3k.h"

MT_FE_DD_DEVICE_SETTINGS DD6301_setting;
//
void Tunner_Demod_Init(void)
{
  //Ó²¼ş¸´Î»DM6000

  mt_fe_dmd_hard_reset_dd3k(&DD6301_setting);


  memset(&DD6301_setting,0,sizeof(MT_FE_DD_DEVICE_SETTINGS));

  mt_fe_demod_settings_default_dd3k(&DD6301_setting);

  DD6301_setting.dmd_set_reg = (MT_FE_RET(*)(void *,U16,U8))_mt_fe_dmd_set_reg;
  DD6301_setting.dmd_get_reg = (MT_FE_RET(*)(void *,U16,U8 *))_mt_fe_dmd_get_reg;
  DD6301_setting.write_fw = (MT_FE_RET(*)(void *,U16,U8 *,U16))_mt_fe_dmd_write;
  DD6301_setting.mt_sleep = (void (*)(U32))_mt_sleep;
  DD6301_setting.tn_write = NULL;
  DD6301_setting.tn_read = NULL;

  //add by yanxuewen 20190614
  MT_FE_DD_TN_DEV_SETTINGS tuner_setting ;

  tuner_setting.tuner_type = MtFeTN_TC6800;
  tuner_setting.tuner_dev_addr = TUNER_I2C_ADDR_TC6800;
  tuner_setting.tuner_mode = MtFeType_CTTB;
  tuner_setting.tuner_init_ok = 0;
  tuner_setting.tuner_init = (S32(*)(void *))mt_fe_tn_init_tc6800_tc;
  tuner_setting.tuner_set = (S32(*)(void *, U32))mt_fe_tn_set_freq_tc6800_tc;
  tuner_setting.tuner_get_strength = (S32(*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc;
  tuner_setting.tuner_sleep = (S32(*)(void *))mt_fe_tn_sleep_tc6800_tc;
  tuner_setting.tuner_wake_up = (S32(*)(void *))mt_fe_tn_wake_up_tc6800_tc;

  mt_fe_config_tuner_settings_dd3k(&DD6301_setting, &tuner_setting);

  mt_fe_dmd_init_dd3k(&DD6301_setting);

  mt_fe_tn_wake_up_tc6800_tc(&DD6301_setting);

  _mt_sleep(1000);


  DD6301_setting.demod_ct_settings.demod_bandwidth = MtFeBandwidth_6M;
  mt_fe_dmd_connect_dd3k_t(&DD6301_setting, 738*1000000);

}

MT_FE_RET DM6301_tuner_channel(U32 Freq_KHz, MT_FE_DD_BANDWIDTH demod_bandwidth)
{
  MT_FE_RET	ret = MtFeErr_Ok;
    MT_FE_LOCK_STATE  lock_state = MtFeLockState_Undef;

    DD6301_setting.demod_ct_settings.demod_bandwidth = demod_bandwidth;

    ret = mt_fe_dmd_connect_dd3k_t(&DD6301_setting,Freq_KHz*1000);

    _mt_sleep(5000);

    if (ret == MtFeErr_Ok)
    {
      mt_fe_dmd_get_lock_state_dd3k(&DD6301_setting, &lock_state);

      if(lock_state != MtFeLockState_Locked)
        ret = MtFeErr_UnLock;
    }

//    U8 data1;
//
//    DD6301_setting.dmd_get_reg(&DD6301_setting,0x01b4,&data1);
//
//    printf("0x01b4 reg = 0x%02x",data1);
  return ret;
}

MT_FE_RET Get_lock_state(void)
{
  MT_FE_RET	ret = MtFeErr_Ok;
    MT_FE_LOCK_STATE  lock_state = MtFeLockState_Undef;

    ret = mt_fe_dmd_get_lock_state_dd3k(&DD6301_setting, &lock_state);
    if (ret == MtFeErr_Ok)
    {
      if(lock_state != MtFeLockState_Locked)
        ret = MtFeErr_UnLock;
    }
  return ret;

}
U8 Get_SNR(void)
{
  S8 snr;
  snr = 0;
  if(mt_fe_dmd_get_quality_dd3k_t(&DD6301_setting, &snr) != MtFeErr_Ok)
    return 0;
  else
    return snr;

}

void delay_ms(U16 ms)
{
    while(ms>0)
    {
    usleep(1000);
    ms--;
    }
}
