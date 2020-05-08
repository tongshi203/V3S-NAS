/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
 * Filename:		mt_fe_tn_tc6800.h
 *
 * Description:		Interface for the tc6800
 *
 *
 * Author:			Lei.Zhu
 *
 * Version:			0.01.01
 * Date:			2016-03-25

 * History:
 * Description		Version		Date					Author
 *---------------------------------------------------------------------------
 * File Create		0.01.00		2016.03.22				Kenn.Wang
 *---------------------------------------------------------------------------
 ****************************************************************************/


#ifndef  _MT_FE_TC6800_H
#define  _MT_FE_TC6800_H

#include "fe_dd3k_def.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TC6800_ENABLE_PRINT		0

#if (TC6800_ENABLE_PRINT == 1)
	#define tc6800_dbg_print(str)				printf str
#else
	#define tc6800_dbg_print(str)
#endif

#if 0
#if 1
#define	U8	unsigned char								/* 8bit unsigned	*/
#define	S8	signed char									/* 8bit unsigned	*/
#define	U16	unsigned short								/* 16bit unsigned	*/
#define	S16	signed short								/* 16bit unsigned	*/
#define	U32	unsigned int								/* 32bit unsigned	*/
#define	S32	signed int									/* 16bit unsigned	*/
#define	DB	double
#else
typedef	unsigned char		U8;							/* 8bit unsigned	*/
typedef	unsigned char		S8;							/* 8bit unsigned	*/
typedef	unsigned short		U16;						/* 16bit unsigned	*/
typedef	signed short		S16;						/* 16bit unsigned	*/
typedef	unsigned int		U32;						/* 32bit unsigned	*/
typedef	signed int			S32;						/* 16bit unsigned	*/
typedef	double				DB;
#endif
#endif

#define  MT_TC6800_CUSTOM_CONFIG	0			// 0: General customers		1: Jiuzhou

#define  TUNER_I2C_ADDR_TC6800		0xc6

typedef struct _MT_FE_TN_Device_Settings_TC6800
{
	U8			tuner_init_OK;			// Tuner initialize status
	U8			tuner_mcu_status;		// Tuner mcu status
	U8			tuner_dev_addr;			// Tuner device address

	U8			tuner_application;		// 0: Cable, 1: DTMB, 2: Analog TV, 3: DVB-T2,4: DVB-T

	U16			tuner_qam;
	U16			tuner_mtt;
	U16			tuner_custom_cfg;
	U32			tuner_version;
	U8			tuner_fw_version;
	U32			tuner_time;

	U8			tuner_crystal_cap;		// Crystal loading capacitor, from 0x00 to 0x1f,default 0x18;
	U8			tuner_clock_out;		// 0: Clock Out Off, 1: Clock Out On
	S16			tuner_mode;				// 0: Normal (LT Off), 1: Normal (LT On), 2: LT On both in work and sleep modes.
	U8			tuner_int_lt;
	U8			tuner_gpio_out;			// 0: GPIO out disable, 1: GPIO out enable
	U32			tuner_freq;				// RF frequency to be set, unit: KHz
	U8			tuner_int_im;
	U16			tuner_mixer_type;
	U32			tuner_crystal;			// Tuner crystal frequency, unit: KHz
	U32			tuner_int_ldiv;
	U32			tuner_int_fvco_tg;
	double		tuner_Fvco_val;				//output
	U32			tuner_dac;				// Tuner DAC frequency, unit: KHz
	U8			tuner_dac_gain;			// Tuner DAC gain:1~6;
	U32			tuner_bandwidth;		// Bandwidth of the channel, unit: KHz, 1700/6000/7000/8000
	U8			tuner_signal_delay_en;
	U32			tuner_signal_delay_ns;
	U8			tuner_harmonic_imp;

} MT_FE_TN_DEVICE_SETTINGS_TC6800, *MT_FE_Tuner_Handle_TC6800;

//external function
//extern S32 _mt_fe_tn_get_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data);
//extern S32 _mt_fe_tn_set_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 reg_data);
//extern S32 _mt_fe_tn_write_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data, U8 data_len);
//extern void _mt_sleep_tc6800(U32 ticks_ms);

//internal function for register read write

S32 _mt_fe_tn_set_reg_bit_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit);
S32 _mt_fe_tn_get_reg_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 *reg_data);   //reg_addr_expand: 0xfe, 0xb4, 0xb9
S32 _mt_fe_tn_set_reg_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 reg_data);    //reg_addr_expand: 0xfe, 0xb4, 0xb9
S32 _mt_fe_tn_set_reg_bit_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit);


//internal function
void _mt_fe_tn_set_RF_front_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 Freq_KHz);
void _mt_fe_tn_set_Mixer_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 Freq_KHz, U8 mixer_sel);
void _mt_fe_tn_set_LO_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 Freq_KHz, U8 mixer_sel);
void _mt_fe_tn_set_PLL_freq_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 Delta_sign, U8 Num_step);
void _mt_fe_tn_freq_fine_tune_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 Delta_sign, U8 Num_step);
void _mt_fe_tn_set_BB_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void _mt_fe_tn_set_BB_Cal_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 bandwidth);
void _mt_fe_tn_set_DAC_tc6800(MT_FE_Tuner_Handle_TC6800 handle,U32 Freq_KHz);
void _mt_fe_tn_preset_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void _mt_fe_tn_set_tune_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 Freq_KHz, U8 mixer_sel, U32 WaitTime);
void _mt_fe_tn_set_sysmon_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void _mt_fe_tn_poweron_set_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void _mt_fe_tn_set_appendix_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

void _mt_fe_tn_set_mch_cal_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void _mt_fe_tn_set_mch_app_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

//APIs
void mt_fe_tn_wakeup_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void mt_fe_tn_sleep_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
S32 mt_fe_tn_download_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

void mt_fe_tn_init_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void mt_fe_tn_set_freq_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 Freq_KHz);

void mt_fe_tn_set_GPIOout_high_tc6800(MT_FE_Tuner_Handle_TC6800 handle);
void mt_fe_tn_set_GPIOout_low_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

void mt_fe_tn_get_diagnose_info_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32* data1, U32* data2);
S32	 mt_fe_tn_get_signal_strength_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

S32 mt_fe_tn_self_check_tc6800(MT_FE_Tuner_Handle_TC6800 handle);

#ifdef __cplusplus
}
#endif

#endif

