/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/*--------------------------------------------------------------------------
*
* File:				mt_fe_def.h
*
* Current version:	2.80.60
*
* Description:
*
* Log:	Description		Version		Date			Author
*---------------------------------------------------------------------------------
*		Create			1.00.10		2010.09.16		WangBingju
*		Modify			2.10.00		2011.01.28		WangBingju
*		Modify			2.20.00		2011.03.16		WangBingju
*		Modify			2.50.50		2011.07.01		WangBingju
*		Modify			2.60.10		2011.07.25		WangBingju
*		Modify			2.60.40		2011.08.10		WangBingju
*		Modify			2.60.70		2012.01.19		YZ.Huang
*		Modify			2.70.10		2012.03.19		WangBingju
*		Modify			2.70.16		2012.07.20		WangBingju
*		Modify			2.70.17		2012.11.12		WangBingju
*		Modify			2.70.18		2013.02.28		WangBingju
*		Modify			2.70.19		2013.04.25		WangBingju
*		Modify			2.70.20		2013.06.06		WangBingju
*		Modify			2.70.21		2014.03.07		WangBingju
*		Modify			2.80.00		2015.05.20		WangBingju
*		Modify			2.80.20		2015.12.10		WangBingju
*		Modify			2.80.30		2016.05.06		WangBingju
*		Modify			2.80.40		2016.06.23		WangBingju
*		Modify			2.80.50		2016.07.12		WangBingju
*		Modify			2.80.60		2016.11.28		WangBingju
*		Modify			2.80.70		2017.06.15		WangBingju
*******************************************************************************************************************/
#ifndef __MT_FE_DEF_H__
#define __MT_FE_DEF_H__

#include <stdio.h>
#include "mt_fe_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MT_FE_DRIVER_VERSION					0x28080
#define MT_FE_DMD_DD3X0X_SUPPORT				1

typedef enum _MT_FE_DD_SUPPORTED_TUNER
{
	MtFeTnId_Undef = 0,
	MtFeTN_AlpsTdac3c02a,
	//MtFeTN_TC2000,
	MtFeTN_MxL5007T,
	MtFeTN_Sharp2093,
	MtFeTN_FM2821,
	MtFeTN_DT58WT,
	MtFeTN_TC2800,
	MtFeTN_TC3800,
	MtFeTN_TC6800,		// fix102281
	MtFeTN_AlpsTdacxc0xa,
	MtFeTN_XuGuangDMT10B,
	MtFeTN_NXPTDA18273,
	MtFeTN_NXPTDA18257,
	MtFeTN_MxL601,
	MtFeTN_MxL603,
	MtFeTN_SI2158,
	MtFeTN_GeneralTuner
}MT_FE_DD_SUPPORTED_TUNER;

typedef enum _MT_FE_DD_BANDWIDTH
{
	MtFeBandwidth_Undef = 0,
	MtFeBandwidth_6M,
	MtFeBandwidth_7M,
	MtFeBandwidth_8M
} MT_FE_DD_BANDWIDTH;

typedef enum _MT_FE_DD_CARRIER_MODE
{
	MtFeCrrierMode_Undef = 0,
	MtFeCrrierMode_Multicarrier,
	MtFeCrrierMode_SingleCarrier
} MT_FE_DD_CARRIER_MODE;

typedef enum _MT_FE_DD_SPECTRUM_MODE
{
	MtFeSpectrum_Undef = 0,
	MtFeSpectrum_NoInversion,
	MtFeSpectrum_Inversion
} MT_FE_DD_SPECTRUM_MODE;

typedef enum _MT_FE_DD_FRAME_MODE
{
	MtFeFrameMode_Undef = 0,
	MtFeFrameMode_Pn420,
	MtFeFrameMode_Pn945,
	MtFeFrameMode_Pn595
} MT_FE_DD_FRAME_MODE;

typedef enum _MT_FE_DD_INTERLEAVING_DEPTH
{
	MtFeInterleavingDepth_Undef = 0,
	MtFeInterleavingDepth_240,
	MtFeInterleavingDepth_720
} MT_FE_DD_INTERLEAVING_DEPTH;

typedef enum _MT_FE_DD_FEC_CODE_RATE
{
	MtFeFecCodeRate_Undef = 0,
	MtFeFecCodeRate_0p4,
	MtFeFecCodeRate_0p6,
	MtFeFecCodeRate_0p8
} MT_FE_DD_FEC_CODE_RATE;

typedef enum _MT_FE_DD_SI_MAP_MODE
{
	MtFeSiMapMode_Undef = 0,
	MtFeSiMapMode_1,
	MtFeSiMapMode_2
} MT_FE_DD_SI_MAP_MODE;

typedef enum _MT_FE_DD_XCLK_MEER_SELECT
{
	MtFeAuXclk = 0,
	MtFeMerr
} MT_FE_DD_XCLK_MEER_SELECT;

typedef enum _MT_FE_DD_SWAP_SELECT
{
	MtFeTsOut_Normal = 0,
	MtFeTsOut_Swap
} MT_FE_DD_SWAP_SELECT;

typedef struct _DVBC_QAM_CONFIG
{
	U16 qam[5];
	U8 cnt;
	U32	wait_time;/*ms*/
} DVBC_QAM_CONFIG;

typedef enum _MT_FE_DD_SIGNAL_STATE
{
	MtFeSignalState_Undef = 0,
	MtFeSignalState_NoSignal,
	MtFeSignalState_Signal
} MT_FE_DD_SIGNAL_STATE;

typedef enum _MT_FE_DD_DMD_ID
{
	MtFeDmdId_Undef,
	MtFeDmdId_DD3X0X,
	MtFeDmdId_DD630X,
	MtFeDmdId_Unknown
} MT_FE_DD_DMD_ID;

typedef struct _MT_FE_DD_CHAN_INFO
{
	MT_FE_DD_CARRIER_MODE			carrier_mode;
	MT_FE_DD_SPECTRUM_MODE			spectrum_mode;
	MT_FE_DD_FRAME_MODE				frame_mode;
	MT_FE_DD_INTERLEAVING_DEPTH		interleaving_depth;
	MT_FE_MOD_MODE					constellation_pattern;
	MT_FE_DD_FEC_CODE_RATE			fec_code_rate;
	MT_FE_DD_SI_MAP_MODE			si_map_mode;
} MT_FE_DD_CHAN_INFO;

typedef struct _MT_FE_DD_DEMOD_SETTINGS
{
	MT_FE_TYPE						demod_mode;
	U8								demod_i2c_addr;
	MT_FE_TS_OUT_MODE				ts_out_mode;
	MT_FE_DD_BANDWIDTH				demod_bandwidth;
	MT_FE_DD_SWAP_SELECT			ts_out_swap;
} MT_FE_DD_DEMOD_SETTINGS,*MT_FE_DD_Demod_Handle;

typedef struct _MT_FE_DD_TN_DEV_SETTINGS
{
	MT_FE_DD_SUPPORTED_TUNER		tuner_type;
	MT_FE_TYPE						tuner_mode;
	U8								tuner_dev_addr;				/* tuner device i2c addres*/
	U8								tuner_init_ok;				/*tuner init yes or no,0 :no 1:yes*/							
	S32 (*tuner_init)(void *handle);							/* tuner init function */
	S32 (*tuner_set)(void *handle, U32 freq);					/* set tuner function */
	S32 (*tuner_get_strength)(void *handle, S8 *p_strength );	/* get signal strength function */
	S32 (*tuner_sleep)(void *handle);							/* tuner sleep function */		// fix102281
	S32 (*tuner_wake_up)(void *handle);							/* tuner wake up function */	// fix102281
} MT_FE_DD_TN_DEV_SETTINGS, *MT_FE_DD_Tuner_Handle;

typedef struct _MT_FE_DD_DEVICE_SETTINGS
{
	U8								device_index;
	MT_FE_DD_DMD_ID					demod_id;
	MT_FE_SUPPORTED_DEMOD			current_demod_type;
	MT_FE_TYPE						demod_cur_mode;
	MT_FE_DD_DEMOD_SETTINGS			demod_ct_settings;
	MT_FE_DD_DEMOD_SETTINGS			demod_dc_settings;
	MT_FE_DD_TN_DEV_SETTINGS		tuner_ct_settings;
	MT_FE_DD_TN_DEV_SETTINGS		tuner_dc_settings;
	DVBC_QAM_CONFIG					dvbc_qam_config;
	MT_FE_RET	(*dmd_set_reg)(void *handle, U16 reg_index, U8 reg_value);
	MT_FE_RET	(*dmd_get_reg)(void *handle, U16 reg_index, U8 *p_buf);
	MT_FE_RET	(*write_fw)(void *handle, U16 reg_index, U8 *p_buf, U16 n_byte);
	void		(*mt_sleep)(U32 ticks_ms);
	//MT_FE_RET	(*tn_set_reg)(void *handle, U8 reg_index, U8 reg_value);
	//MT_FE_RET	(*tn_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
	MT_FE_RET	(*tn_write)(void *handle, U8 *p_buf, U16 n_byte);
	MT_FE_RET	(*tn_read)(void *handle, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);
}MT_FE_DD_DEVICE_SETTINGS,*MT_FE_DD_Device_Handle;


MT_FE_RET mt_fe_get_chip_version_dd3k(MT_FE_DD_Device_Handle  handle, U32 *chip_ver);
MT_FE_RET _mt_fe_dmd_reg_init_dd3k_t(MT_FE_DD_Device_Handle  handle);
MT_FE_RET _mt_fe_dmd_download_fw_dd3k_t(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_init_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_scan_settings_default_dd3k_c(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_config_scan_settings_dd3k_c(MT_FE_DD_Device_Handle handle, DVBC_QAM_CONFIG *config_handle);
MT_FE_RET mt_fe_demod_settings_default_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_config_demod_settings_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_DD_DEMOD_SETTINGS *demod_hadle);
MT_FE_RET mt_fe_tuner_settings_default_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_config_tuner_settings_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_DD_TN_DEV_SETTINGS *tuner_hadle);
MT_FE_RET mt_fe_change_mode_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_TYPE set_mode);
MT_FE_RET mt_fe_dmd_connect_dd3k_t(MT_FE_DD_Device_Handle  handle, U32 feq);
MT_FE_RET mt_fe_dmd_connect_dd3k_c(MT_FE_DD_Device_Handle  handle, U32 freq, U32 sym, U16 qam, U8 inverted, U32 xtal);
MT_FE_RET mt_fe_dmd_hard_reset_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_soft_reset_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_set_bandwidth_t(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_close_ts_output_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_open_ts_output_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_set_output_mode_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_get_lock_state_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_signal_state_dd3k_t(MT_FE_DD_Device_Handle  handle, MT_FE_DD_SIGNAL_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_per_dd3k_t(MT_FE_DD_Device_Handle  handle, U16 *p_err_cnt, U16 *p_pkt_cnt);
MT_FE_RET mt_fe_dmd_get_noise_power_dd3k_t(MT_FE_DD_Device_Handle  handle, S32 *p_noise, U32 *p_acc);
MT_FE_RET mt_fe_dmd_get_quality_dd3k_t(MT_FE_DD_Device_Handle  handle, S8 *p_snr);
MT_FE_RET mt_fe_dmd_chan_info_code_parse_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_DD_CHAN_INFO* p_info);
MT_FE_RET mt_fe_dmd_connect_dd3k(MT_FE_DD_Device_Handle  handle, U32 freq, U32 sym, U16 qam, U8 inverted, U32 xtal);
MT_FE_RET _mt_fe_dmd_get_snr_dd3k_c(MT_FE_DD_Device_Handle  handle, U16 qam, U8 *dvbc_snr);
MT_FE_RET _mt_fe_dmd_get_ber_dd3k_c(MT_FE_DD_Device_Handle  handle, U32 *dvbc_ber, U32 *dvbc_total);// fix102281
MT_FE_RET mt_fe_dmd_xclk_merr_select_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_DD_XCLK_MEER_SELECT select );
MT_FE_RET mt_fe_dmd_auto_scan_dd3k_c(MT_FE_DD_Device_Handle  handle, U32 freq, U32 xtal, U32 *sym, U16 *qam, U8 *inverted, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_sleep_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_wake_dd3k(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_dmd_agc_hi_z__dd3k(MT_FE_DD_Device_Handle  handle, BOOL agc_hiz);
MT_FE_RET mt_fe_dmd_lab_enable_dd3k(MT_FE_DD_Device_Handle  handle, BOOL lab_enble);				// fix102281
MT_FE_RET mt_fe_dmd_optimize_nonstandard_mode_dd3k(MT_FE_DD_Device_Handle  handle, U8 mode_cfg);	// fix102281

MT_FE_RET mt_fe_tn_init_tc3800_tc(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_tn_set_freq_tc3800_tc(MT_FE_DD_Device_Handle  handle,U32 Freq_Hz);
MT_FE_RET mt_fe_tn_get_strength_tc3800_tc(MT_FE_DD_Device_Handle  handle,S8 *p_strength);
// fix102281 start
MT_FE_RET mt_fe_tn_sleep_tc3800_tc(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_tn_wake_up_tc3800_tc(MT_FE_DD_Device_Handle  handle);

MT_FE_RET mt_fe_tn_init_tc6800_tc(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_tn_set_freq_tc6800_tc(MT_FE_DD_Device_Handle  handle,U32 Freq_Hz);
MT_FE_RET mt_fe_tn_get_strength_tc6800_tc(MT_FE_DD_Device_Handle  handle,S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc6800_tc(MT_FE_DD_Device_Handle  handle);
MT_FE_RET mt_fe_tn_wake_up_tc6800_tc(MT_FE_DD_Device_Handle  handle);
// fix102281 end

#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_H__ */

