/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include "sys_types.h"
#include "sys_define.h"
#include "mtos_misc.h"
#include "mtos_sem.h"
#include "mtos_task.h"
#include "mtos_printk.h"
#include "mem_manager.h"
#include "mtos_mem.h"

#include "hal_base.h"
#include "hal_gpio.h"
#include "drv_dev.h"
#include "../../../drvbase/drv_dev_priv.h"
#include "i2c.h"
#include "nim.h"
#include "../../nim_priv.h"
#include "mt_fe_common.h"
#include "fe_dd3k_def.h"
#include "fe_dd3k_i2c.h"

#define NIM_DEBUG
#ifdef NIM_DEBUG
#define DS3K_PRINT    OS_PRINTF
#else
#define DS3K_PRINT DUMMY_PRINTF
#endif

#define X_TAL 28800

MT_FE_RET mt_fe_tn_set_freq_tdac3_c02a(MT_FE_DD_Device_Handle  handle,U32 freq);
MT_FE_RET mt_fe_tn_get_signal_strength_tdac3_c02a(MT_FE_DD_Device_Handle  handle,S8 *p_strength);

MT_FE_RET mt_fe_tn_set_freq_tc2800_tc(MT_FE_DD_Device_Handle  handle,U32 Freq_Hz);
MT_FE_RET mt_fe_tn_get_strength_tc2800_tc(MT_FE_DD_Device_Handle  handle,S8 *p_strength);

MT_FE_RET mt_fe_tn_MDrv_Tuner_Init(void *handle);
MT_FE_RET mt_fe_tn_get_strength_MxL603(void  *handle,S8 *p_strength);

MT_FE_RET mt_fe_tn_MDrv_Tuner_SetTuner(U32 dwFreq /*Khz*/, U8 ucBw /*MHz*/);

typedef struct
{
  u32 x_crystal;
  nim_config_t cfg;
  MT_FE_DD_DEVICE_SETTINGS device_setting;
  MT_FE_DD_Device_Handle mt_dd_handle;
}nim_dd3k_priv_t;

i2c_bus_t *g_i2c_dd3k = NULL;
os_sem_t g_i2c_dd3k_mutex = 0;

static u16 parse_QAM(u8 value)
{
  u16 n_qam = 32;

  switch (value) {
  case NIM_MODULA_QAM16:
    n_qam = 16;
    break;
  case NIM_MODULA_QAM32:
    n_qam = 32;
    break;
  case NIM_MODULA_QAM64:
    n_qam = 64;
    break;
  case NIM_MODULA_QAM128:
    n_qam = 128;
    break;
  case NIM_MODULA_QAM256:
    n_qam = 256;
    break;
  default:
    break;
  }

  return n_qam;
}

static RET_CODE nim_m88dd3k_channel_connect(
  lld_nim_t *p_lld,
  nim_channel_info_t *p_channel_info,
  BOOL for_bs)
{
  nim_dd3k_priv_t *p_priv = (nim_dd3k_priv_t *)(p_lld->p_priv);
  MT_FE_LOCK_STATE status;
  u32 lock_timeout_ms, cnt;

  DS3K_PRINT("NIM: conn, f[%d]\n",
            p_channel_info->frequency
            );

  p_channel_info->lock = 0;
  if(p_priv->cfg.tn_version == TN_MXL_603)
  {
    p_channel_info->param.dvbt.band_width = 8;
    DS3K_PRINT("%s %d frequency=%d band_width=%d \n",__FUNCTION__,__LINE__, p_channel_info->frequency, p_channel_info->param.dvbt.band_width);
    mt_fe_tn_MDrv_Tuner_SetTuner(p_channel_info->frequency, p_channel_info->param.dvbt.band_width);
  }
  p_priv->mt_dd_handle->demod_ct_settings.demod_bandwidth = MtFeBandwidth_8M;

  mt_fe_dmd_connect_dd3k(p_priv->mt_dd_handle,
    p_channel_info->frequency*1000, p_channel_info->param.dvbc.symbol_rate, parse_QAM(p_channel_info->param.dvbc.modulation), 0, p_priv->x_crystal);

  lock_timeout_ms = 2000; //2s

  for(cnt = 0; cnt < lock_timeout_ms; cnt+=100)
  {
    mt_fe_dmd_get_lock_state_dd3k(p_priv->mt_dd_handle, &status);
    if(MtFeLockState_Locked == status)
    {
     p_channel_info->lock = 1;
      break;
    }
    mtos_task_delay_ms(100);
  }
#if 0
  DS3K_PRINT("lock tp, f[%d], locked[%d]\n",
            p_channel_info->frequency,
            p_channel_info->lock);
#endif
  return SUCCESS;
}



/* M88RS2000 get performance */
static RET_CODE nim_m88dd3k_channel_perf(lld_nim_t *p_lld,
                                           nim_channel_perf_t *p_perf)
{
  nim_dd3k_priv_t *p_priv = (nim_dd3k_priv_t *)(p_lld->p_priv);
  MT_FE_LOCK_STATE status;
  S8 signal_quality = 0, signal_strength = 0;
  u16 total_packets = 1;
  u16 error_packets = 0;

  mt_fe_dmd_get_lock_state_dd3k(p_priv->mt_dd_handle, &status);
  p_perf->lock = (status == MtFeLockState_Locked) ? 1 : 0;


  if(p_priv->mt_dd_handle->demod_cur_mode == MtFeType_CTTB)
    mt_fe_dmd_get_quality_dd3k_t(p_priv->mt_dd_handle, &signal_quality);
  else
    _mt_fe_dmd_get_snr_dd3k_c(p_priv->mt_dd_handle, p_priv->mt_dd_handle->dvbc_qam_config.qam[0], (u8 *)&signal_quality);

  p_perf->snr = signal_quality;

  if(p_priv->mt_dd_handle->demod_cur_mode == MtFeType_CTTB)
  {
    mt_fe_dmd_get_per_dd3k_t(p_priv->mt_dd_handle, &error_packets, &total_packets);
    if(!error_packets)
        p_perf->ber = 0.0;
    else
        p_perf->ber = (double)error_packets/(double)total_packets;
  }
  else
// fix102281 start
  {
    //_mt_fe_dmd_get_ber_dd3k_c(p_priv->mt_dd_handle, &p_perf->ber);
    unsigned int ber_error, ber_total;

    _mt_fe_dmd_get_ber_dd3k_c(p_priv->mt_dd_handle, &ber_error, &ber_total);

    p_perf->ber = ber_error * 1.0 / ber_total;
  }
// fix102281 end

  if(p_priv->mt_dd_handle->tuner_ct_settings.tuner_get_strength)
  {
    p_priv->mt_dd_handle->tuner_ct_settings.tuner_get_strength(
      p_priv->mt_dd_handle, &signal_strength);
    p_perf->agc = signal_strength;
  }
  DS3K_PRINT("%s snr[%d] ber[%d] agc[%d]\n",__FUNCTION__,p_perf->snr, p_perf->ber,p_perf->agc);
  return SUCCESS;
}


static RET_CODE nim_m88dd3k_channel_set(
  lld_nim_t *p_lld,
  nim_channel_info_t *p_channel_info,
  nim_channel_set_info_t *
  p_channel_set_info)
{
  nim_dd3k_priv_t *p_priv = (nim_dd3k_priv_t *)(p_lld->p_priv);

  if(p_priv->cfg.tn_version == TN_MXL_603)
  {
    p_channel_info->param.dvbt.band_width = 8;
    DS3K_PRINT("%s %d frequency=%d band_width=%d \n",__FUNCTION__,__LINE__, p_channel_info->frequency, p_channel_info->param.dvbt.band_width);
    mt_fe_tn_MDrv_Tuner_SetTuner(p_channel_info->frequency, p_channel_info->param.dvbt.band_width);
  }
  mt_fe_dmd_connect_dd3k(p_priv->mt_dd_handle,
    p_channel_info->frequency*1000, p_channel_info->param.dvbc.symbol_rate, parse_QAM(p_channel_info->param.dvbc.modulation), 0, p_priv->x_crystal);

  p_channel_set_info->lock_time = 2000; //2s

  return SUCCESS;
}


static RET_CODE nim_m88dd3k_ioctrl(lld_nim_t *p_lld, u32 cmd, u32 param)
{
  nim_dd3k_priv_t *p_priv = (nim_dd3k_priv_t *)(p_lld->p_priv);
  MT_FE_LOCK_STATE status;

  switch(cmd)
  {
    case DEV_IOCTRL_POWER:
      switch(param)
      {
        case DEV_POWER_SLEEP:
          /* sleep demod */
          mt_fe_dmd_sleep_dd3k(p_priv->mt_dd_handle);
          break;

        case DEV_POWER_FULLSPEED:
          /* wake up demod */
          mt_fe_dmd_wake_dd3k(p_priv->mt_dd_handle);
          break;

        default:
          return ERR_PARAM;
      }
      break;

    case NIM_IOCTRL_CHANNEL_CHECK_LOCK:
      mt_fe_dmd_get_lock_state_dd3k(p_priv->mt_dd_handle, &status);
      ((nim_channel_info_t *)param)->lock =
        (status == MtFeLockState_Locked) ? 1 : 0;
      break;

    case NIM_IOCTRL_CHECK_LOCK:
      mt_fe_dmd_get_lock_state_dd3k(p_priv->mt_dd_handle, &status);
      *((u8 *)param) = (status == MtFeLockState_Locked) ? 1 : 0;
      break;
    case NIM_IOCTRL_CHANGE_TN_MODE:
      mt_fe_change_mode_dd3k(p_priv->mt_dd_handle,*((u8 *)param));
      break;

    case NIM_IOCTRL_GET_CHN_PARAM_RANGE:
      if(p_priv->cfg.tn_version == TN_TDAC3_C02A)
      {
        nim_channel_param_range_t *p_param = (nim_channel_param_range_t *)param;
        p_param->freq_max = 864000;
        p_param->freq_min = 48500;
      }
      break;

    default:
      return ERR_PARAM;
  }

  return SUCCESS;
}

RET_CODE nim_m88dd3k_open(lld_nim_t *p_lld, void *cfg)
{
  nim_dd3k_priv_t *p_priv = NULL;
  nim_config_t *p_nim_cfg = (nim_config_t *)cfg;
  DVBC_QAM_CONFIG dc_qam_cfg={{64,256,128,32,16},5,1600};

  p_priv = p_lld->p_priv = mtos_malloc(sizeof(nim_dd3k_priv_t));
  MT_ASSERT(NULL != p_priv);
  memset(p_priv, 0x00, sizeof(nim_dd3k_priv_t));

  /* config driver */
  if(p_nim_cfg)
  {
    if(p_nim_cfg->demod_band_width == 0)
      p_nim_cfg->demod_band_width = MtFeBandwidth_8M;  //default
    if(p_nim_cfg->tn_version == 0)
      p_nim_cfg->tn_version = TN_TDAC3_C02A;  //default
    if(p_nim_cfg->x_crystal== 0)
      p_nim_cfg->x_crystal = X_TAL;  //default

    memcpy(&p_priv->cfg, p_nim_cfg, sizeof(nim_config_t));
    g_i2c_dd3k = (i2c_bus_t *)p_nim_cfg->p_dem_bus;
    p_priv->x_crystal = p_nim_cfg->x_crystal;
  }
  else
  {
    p_priv->x_crystal = X_TAL;
    p_priv->cfg.tn_version = TN_TDAC3_C02A;
    p_priv->cfg.demod_band_width = MtFeBandwidth_8M;
  }

  if(NULL == g_i2c_dd3k)
  {
    g_i2c_dd3k = (i2c_bus_t *)dev_find_identifier(NULL,
                                                      DEV_IDT_TYPE,
                                                      SYS_DEV_TYPE_NIM);
  }
  MT_ASSERT(NULL != g_i2c_dd3k);

  /* create a mutex for i2c read/write */
  mtos_sem_create(&g_i2c_dd3k_mutex, TRUE);

  /* config and init dd3k driver */
  MT_FE_DD_DEMOD_SETTINGS     demod_ct_cfg;
  MT_FE_DD_DEMOD_SETTINGS     demod_dc_cfg;
  MT_FE_DD_TN_DEV_SETTINGS    tuner_ct_cfg;
  MT_FE_DD_TN_DEV_SETTINGS    tuner_dc_cfg;

  MT_FE_RET ret = MtFeErr_Ok;

  p_priv->mt_dd_handle = &p_priv->device_setting;

  demod_ct_cfg.demod_mode=MtFeType_CTTB;
  demod_ct_cfg.demod_bandwidth=p_priv->cfg.demod_band_width;
  demod_ct_cfg.demod_i2c_addr=0x20;
  demod_ct_cfg.ts_out_mode=p_priv->cfg.ts_mode ==
                NIM_TS_INTF_SERIAL ? MtFeTsOutMode_Serial : MtFeTsOutMode_Parallel;
  demod_ct_cfg.ts_out_swap=MtFeTsOut_Normal;

  demod_dc_cfg.demod_mode=MtFeType_DVBC;
  demod_dc_cfg.demod_bandwidth=p_priv->cfg.demod_band_width;
  demod_dc_cfg.demod_i2c_addr=0x20;//0x38;
  demod_dc_cfg.ts_out_mode=demod_ct_cfg.ts_out_mode;
  demod_dc_cfg.ts_out_swap=MtFeTsOut_Normal;

  if(p_priv->cfg.tn_version == TN_TDAC3_C02A)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_AlpsTdac3c02a;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=NULL;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tdac3_c02a;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_signal_strength_tdac3_c02a;

    tuner_dc_cfg.tuner_type=MtFeTN_AlpsTdac3c02a;
    tuner_dc_cfg.tuner_mode=MtFeType_DVBC;
    tuner_dc_cfg.tuner_dev_addr=0xc2;
    tuner_dc_cfg.tuner_init_ok=0;
    tuner_dc_cfg.tuner_init=NULL;
    tuner_dc_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tdac3_c02a;
    tuner_dc_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_signal_strength_tdac3_c02a;
  }
#if 0
  else if(p_priv->cfg.tn_version == TN_TDAC7_D01)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_AlpsTdac7d01;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc0;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=NULL;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tdac7_d01;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_signal_strength_tdac7_d01;

    tuner_dc_cfg.tuner_type=MtFeTN_AlpsTdac7d01;
    tuner_dc_cfg.tuner_mode=MtFeType_DVBC;
    tuner_dc_cfg.tuner_dev_addr=0xc0;
    tuner_dc_cfg.tuner_init_ok=0;
    tuner_dc_cfg.tuner_init=NULL;
    tuner_dc_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tdac7_d01;
    tuner_dc_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_signal_strength_tdac7_d01;
  }
#endif
  else if(p_priv->cfg.tn_version == TC2800)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_TC2800;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=NULL;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc2800_tc;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc2800_tc;

    tuner_dc_cfg.tuner_type=MtFeTN_TC2800;
    tuner_dc_cfg.tuner_mode=MtFeType_DVBC;
    tuner_dc_cfg.tuner_dev_addr=0xc2;
    tuner_dc_cfg.tuner_init_ok=0;
    tuner_dc_cfg.tuner_init=NULL;
    tuner_dc_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc2800_tc;
    tuner_dc_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc2800_tc;
  }
  else if(p_priv->cfg.tn_version == TC3800)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_TC3800;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_init_tc3800_tc;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc3800_tc;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc;

    tuner_dc_cfg.tuner_type=MtFeTN_TC3800;
    tuner_dc_cfg.tuner_mode=MtFeType_DVBC;
    tuner_dc_cfg.tuner_dev_addr=0xc2;
    tuner_dc_cfg.tuner_init_ok=0;
    tuner_dc_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_init_tc3800_tc;
    tuner_dc_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc3800_tc;
    tuner_dc_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc;
  }
// fix102281 start
  else if(p_priv->cfg.tn_version == TC6800)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_TC6800;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc6;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_init_tc6800_tc;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc6800_tc;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc;

    tuner_dc_cfg.tuner_type=MtFeTN_TC6800;
    tuner_dc_cfg.tuner_mode=MtFeType_DVBC;
    tuner_dc_cfg.tuner_dev_addr=0xc6;
    tuner_dc_cfg.tuner_init_ok=0;
    tuner_dc_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_init_tc6800_tc;
    tuner_dc_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc6800_tc;
    tuner_dc_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc;
  }
// fix102281 end
  else if(p_priv->cfg.tn_version == TN_MXL_603)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_MxL603;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_MDrv_Tuner_Init;
    tuner_ct_cfg.tuner_set=NULL;//(S32 (*)(void *, U32))mt_fe_tn_set_freq_tc2800_tc;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL603;

  }
#if 0
  else if(p_priv->cfg.tn_version == TN_NXP_TDA18273)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_NXPTDA18273;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;   //real address?
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_nxp_tda18273_init;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_nxp_tda18273;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_signal_strength_nxp_tda18273;
  }
  else if(p_priv->cfg.tn_version == TN_MXL_5007T)
  {
    tuner_ct_cfg.tuner_type=MtFeTN_MxL5007T;
    tuner_ct_cfg.tuner_mode=MtFeType_CTTB;
    tuner_ct_cfg.tuner_dev_addr=0xc2;    //real address?
    tuner_ct_cfg.tuner_init_ok=0;
    tuner_ct_cfg.tuner_init=(S32 (*)(void *))mt_fe_tn_MxL5007T_Init;
    tuner_ct_cfg.tuner_set=(S32 (*)(void *, U32))mt_fe_tn_set_freq_MxL5007T;
    tuner_ct_cfg.tuner_get_strength=(S32 (*)(void *, S8 *))mt_fe_tn_get_strength_MxL5007T;
  }
#endif
  p_priv->mt_dd_handle->dmd_set_reg=(MT_FE_RET (*)(void *, U16, U8))_mt_fe_dmd_set_reg;
  p_priv->mt_dd_handle->dmd_get_reg =(MT_FE_RET (*)(void *, U16, U8 *))_mt_fe_dmd_get_reg;
  p_priv->mt_dd_handle->write_fw =(MT_FE_RET (*)(void *, U16, U8 *, U16))_mt_fe_dmd_write;
  //p_priv->mt_dd_handle->tn_set_reg = (S32 (*)(void *, U8, U8))fe_dd3k_tn_set_reg;
  //p_priv->mt_dd_handle->tn_get_reg = (S32 (*)(void *, U8, U8 *))fe_dd3k_tn_get_reg;
  p_priv->mt_dd_handle->tn_write=(MT_FE_RET (*)(void *, U8 *, U16))_mt_fe_tn_write;
  p_priv->mt_dd_handle->tn_read=(MT_FE_RET (*)(void *, U8 *, U16, U8 *, U16))_mt_fe_tn_read;
  p_priv->mt_dd_handle->mt_sleep=(void (*)(U32))_mt_sleep;

  mt_fe_config_demod_settings_dd3k(p_priv->mt_dd_handle,&demod_ct_cfg);
  mt_fe_config_demod_settings_dd3k(p_priv->mt_dd_handle,&demod_dc_cfg);
  mt_fe_config_tuner_settings_dd3k(p_priv->mt_dd_handle,&tuner_ct_cfg);
  mt_fe_config_tuner_settings_dd3k(p_priv->mt_dd_handle,&tuner_dc_cfg);

  mt_fe_config_scan_settings_dd3k_c(p_priv->mt_dd_handle,&dc_qam_cfg);

  ret=mt_fe_dmd_init_dd3k(p_priv->mt_dd_handle);
  if(ret!=MtFeErr_Ok)
  {
    return ERR_FAILURE;
  }

  return SUCCESS;
}


RET_CODE nim_m88dd3k_close(lld_nim_t *p_lld)
{
  nim_dd3k_priv_t *priv = (nim_dd3k_priv_t *)(p_lld->p_priv);

  mtos_free(priv);

  return SUCCESS;
}


static void nim_m88dd3k_detach(lld_nim_t *p_lld)
{

}


RET_CODE nim_m88dd3k_attach(char *name)
{
  nim_device_t *dev;
  device_base_t *dev_base;
  lld_nim_t *lld_dev;

  /* allocate driver memory resource */
  dev = (nim_device_t *)dev_allocate(name, SYS_DEV_TYPE_NIM,
                                     sizeof(nim_device_t), sizeof(lld_nim_t));
  if(NULL == dev)
  {
    return ERR_FAILURE;
  }

  /* link base function */
  dev_base = (device_base_t *)dev->p_base;
  dev_base->open = (RET_CODE (*)(void *, void *))nim_m88dd3k_open;
  dev_base->close = (RET_CODE (*)(void *))nim_m88dd3k_close;
  dev_base->detach = (void (*)(void *))nim_m88dd3k_detach;
  dev_base->io_ctrl = (RET_CODE (*)(void *, u32, u32))nim_m88dd3k_ioctrl;

  /* attach lld driver */
  lld_dev = (lld_nim_t *)dev->p_priv;
  lld_dev->channel_scan = NULL;
  lld_dev->channel_connect = nim_m88dd3k_channel_connect;
  lld_dev->channel_set = nim_m88dd3k_channel_set;
  lld_dev->channel_perf = nim_m88dd3k_channel_perf;
  lld_dev->diseqc_ctrl = NULL;

  return SUCCESS;
}

