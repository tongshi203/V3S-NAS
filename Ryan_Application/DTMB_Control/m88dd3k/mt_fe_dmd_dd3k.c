/****************************************************************************
*Montage Proprietary and Confidential
* Montage Technology (Shanghai) Co., Ltd.
*@2015 Montage Technology Group Limited and/or its affiliated companies
* --------------------------------------------------------------------------
*
* File:				mt_fe_dmd_dd3k.c
*
* Current version:	2.80.80
*
* Description:		M88DD630x/M88DD3X0X IC Driver.
*
* Log:	Description			Version		Date			  Author
*------------------------------------------------------------------------
*	Create			1.00.10		2010.09.16		WangBingju
*	Modify			2.00.00		2011.01.28		WangBingju
*	Modify			2.20.00		2011.03.16		WangBingju
*	Modify			2.50.00		2011.05.18		WangBingju
*	Modify			2.50.40		2011.06.29		WangBingju
*	Modify			2.60.00		2011.07.20		WangBingju
*	Modify			2.60.40		2011.08.10		WangBingju
*	Modify			2.60.50		2011.09.01		WangBingju
*	Modify			2.60.60		2011.11.18		WangBingju
*	Modify			2.60.70		2012.01.09		WangBingju
*	Modify			2.60.90		2012.02.16		WangBingju
*	Modify			2.70.00		2012.02.29		WangBingju
*	Modify			2.70.10		2012.03.19		WangBingju
*	Modify			2.70.16		2012.07.20		WangBingju
*	Modify			2.70.17		2012.11.12		WangBingju
*	Modify			2.70.18		2013.02.28		WangBingju
*	Modify			2.70.19		2013.04.25		WangBingju
*	Modify			2.70.20		2013.06.06		WangBingju
*	Modify			2.70.21		2014.03.07		WangBingju
*	Modify			2.80.00		2015.05.20		WangBingju
*	Modify			2.80.10		2015.10.29		WangBingju
*	Modify			2.80.20		2016.03.25		WangBingju
*	Modify			2.80.30		2016.05.06		WangBingju
*	Modify			2.80.40		2016.07.01		WangBingju
*	Modify			2.80.50		2016.07.12		WangBingju
*	Modify			2.80.60		2016.11.28		WangBingju
*	Modify			2.80.70		2017.06.26		WangBingju
*	Modify			2.80.80		2017.12.12		WangBingju
********************************************************************************************************/
#include <stdio.h>	/*标准的输入输出函数库*/
#include <stdlib.h>	/*标准的输入输出函数库*/
#include <unistd.h> /*对于read和write等函数的支持*/
#include <sys/ioctl.h> /*IO指令流函数，如cmd等，除了打开函数之外，其他的函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fe_dd3k_def.h"
#include "fe_dd3k_i2c.h"
#include <math.h>
#include "mt_fe_dmd_fw_dd3k.h"

#define LED_MAGIC 'k'
#define IOCTL_LED_ON  _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_RST_H  _IOW (LED_MAGIC, 4, int)
#define IOCTL_RST_L  _IOW (LED_MAGIC, 5, int)

#define QAM_CODE_RATE_MASK				0x0000000f	/*	bit0 ~ bit 3	*/
#define QAM_CODE_RATE_POS				0
#define INTERLEACING_DEPTH_MASK			0x00000001	/*	bit 4			*/
#define INTERLEACING_DEPTH_POS			4
#define FRAME_MODE_MASK					0x00000003	/*	bit5 ~ bit 6	*/
#define FRAME_MODE_POS					5
#define SPEC_INVC_MASK					0x00000001	/*	bit7			*/
#define SPEC_INVC_POS					7
#define CARRIER_MODE_MASK				0x00000001	/*	bit8			*/
#define CARRIER_MODE_POS				8
#define SI_MAP_MODE_MASK				0x00000003	/*	bit9 ~ bit10	*/
#define SI_MAP_MODE_POS					9
#define VERIFICATION_MASK				0x0000ffff	/*	bit16 ~ bit31	*/
#define VERIFICATION_POS				16

#define VERIFICATION_CODE				0x00001234




#define SET_QAM_CODE_RATE(code, value)		code = (code & (~(QAM_CODE_RATE_MASK << QAM_CODE_RATE_POS)))\
												| (((value) & QAM_CODE_RATE_MASK) << QAM_CODE_RATE_POS)
#define SET_INTERLEACING_DEPTH(code, value)	code = (code & (~(INTERLEACING_DEPTH_MASK << INTERLEACING_DEPTH_POS)))\
												| (((value) & INTERLEACING_DEPTH_MASK) << INTERLEACING_DEPTH_POS)
#define SET_FRAME_MODE(code, value)			code = (code & ((~FRAME_MODE_MASK << FRAME_MODE_POS)))\
												| (((value) & FRAME_MODE_MASK) << FRAME_MODE_POS)
#define SET_SPEC_INVC(code, value)				code = (code & ((~SPEC_INVC_MASK << SPEC_INVC_POS)))\
												| (((value) & SPEC_INVC_MASK) << SPEC_INVC_POS)
#define SET_CARRIER_MODE(code, value)		code = (code & ((~CARRIER_MODE_MASK << CARRIER_MODE_POS)))\
												| (((value) & CARRIER_MODE_MASK) << CARRIER_MODE_POS)
#define SET_SI_MAP_MODE(code, value)			code = (code & ((~(SI_MAP_MODE_MASK << SI_MAP_MODE_POS))))\
												| (((value) & SI_MAP_MODE_MASK) << SI_MAP_MODE_POS)
#define SET_VERIFICATION_CODE(code, value)	code = (code & (~VERIFICATION_MASK << VERIFICATION_POS))\
												| (((value) & VERIFICATION_MASK) << VERIFICATION_POS)


#define GET_QAM_CODE_RATE(code)						((code >> QAM_CODE_RATE_POS     ) & QAM_CODE_RATE_MASK     )
#define GET_INTERLEACING_DEPTH(code)			((code >> INTERLEACING_DEPTH_POS) & INTERLEACING_DEPTH_MASK)
#define GET_FRAME_MODE(code)							((code >> FRAME_MODE_POS        ) & FRAME_MODE_MASK        )
#define GET_SPEC_INVC(code)								((code >> SPEC_INVC_POS         ) & SPEC_INVC_MASK         )
#define GET_CARRIER_MODE(code)						((code >> CARRIER_MODE_POS      ) & CARRIER_MODE_MASK      )
#define GET_SI_MAP_MODE(code)							((code >> SI_MAP_MODE_POS       ) & SI_MAP_MODE_MASK       )
#define GET_VERIFICATION_CODE(code)				((code >> VERIFICATION_POS      ) & VERIFICATION_MASK      )


// fix102281 start
const U32 log10_table[127][2]=
{
	{1,0},{2,3010},{3,4771},{4,6021},{5,6990},{6,7782},{7,8451},{8,9031},{9,9542},{10,10000},
	{11,10414},{12,10792},{13,11139},{14,11461},{15,11761},{16,12041},{17,12304},{18,12553},{19,12788},{20,13010},
	{21,13222},{22,13424},{24,13802},{26,14150},{28,14472},{30,14771},{32,15051},{34,15315},{36,15563},{38,15798},{40,16021},
	{42,16232},{44,16435},{47,16721},{50,16990},{53,17243},{56,17482},{59,17709},{62,17924},{65,18129},{69,18388},{73,18633},
	{77,18865},{81,19085},{85,19294},{90,19542},{95,19777},{100,20000},{105,20212},{110,20414},{116,20645},{122,20864},{128,21072},
	{135,21303},{142,21523},{149,21732},{157,21959},{165,22175},{173,22380},{182,22601},{191,22810},{201,23032},{211,23243},{221,23444},
	{232,23655},{243,23856},{255,24065},{268,24281},{281,24487},{295,24698},{309,24900},{324,25105},{340,25315},{357,25527},{374,25729},
	{392,25933},{411,26138},{431,26345},{452,26551},{474,26758},{497,26964},{521,27168},{546,27372},{572,27574},{599,27774},{628,27980},
	{658,28182},{690,28388},{723,28591},{758,28797},{794,28998},{832,29201},{872,29405},{914,29609},{958,29814},{1004,30017},{1052,30220},
	{1102,30422},{1154,30622},{1209,30824},{1266,31024},{1326,31225},{1389,31427},{1455,31629},{1524,31830},{1596,32030},{1672,32232},
	{1751,32433},{1834,32634},{1921,32835},{2012,33036},{2107,33237},{2207,33438},{2312,33640},{2421,33840},{2536,34041},{2656,34242},
	{2782,34444},{2914,34645},{3052,34846},{3196,35046},{3347,35247},{3505,35447},{3671,35648},{3845,35849},{4027,36050},{4095,36123}
};

U32 log10table(U32 input_value)
{
	U8 i = 0;
	U32 log10_result = 0;

	if ((input_value<1) || (input_value>=4096))
	{
		return 0;
	}

	for (i=0;i<127;i++)
	{
		if (input_value==log10_table[i][0])
		{
			log10_result = (U32)(log10_table[i][1]/100);
			break;
		}
		else if ((input_value>log10_table[i][0])&&(input_value<log10_table[i+1][0]))
		{
			log10_result = (U32)((log10_table[i][1]+log10_table[i+1][1])/200);
			break;
		}
	}

	return log10_result;
}
// fix102281 end

/*	FUNCTIN:
**		mt_fe_get_driver_version_dd3k
**
**	DESCRIPTION:
**		get version of this driver
**
**	IN:
**		none
**
**	OUT:
**		*pd_ver
**
**	RETURN:
*/
MT_FE_RET mt_fe_get_driver_version_dd3k(U32 *pd_ver)
{
	*pd_ver = MT_FE_DRIVER_VERSION;/*driver software's version*/

	return MtFeErr_Ok;
}
/*	FUNCTIN:
**		mt_fe_get_chip_version_dd3k
**
**	DESCRIPTION:
**		get version of this chip
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		*pd_ver
**
**	RETURN:
*/
MT_FE_RET mt_fe_get_chip_version_dd3k(MT_FE_DD_Device_Handle  handle, U32 *chip_ver)
{
	U8	reg_0x08 = 0, reg_0x02 = 0, reg_0x01 = 0, reg_0x00 = 0;

	handle->dmd_get_reg(handle, 0x02, &reg_0x02);
	handle->dmd_get_reg(handle, 0x01, &reg_0x01);
	handle->dmd_get_reg(handle, 0x08, &reg_0x08);
	handle->dmd_get_reg(handle, 0x00, &reg_0x00);

	*chip_ver = ((U32)reg_0x02 << 24) + ((U32)reg_0x01 << 16) + ((U32)reg_0x08 << 8) + ((U32)reg_0x00);/*chip version*/

	return MtFeErr_Ok;
}



/*	FUNCTIN:
**		mt_fe_change_mode_dd3k
**
**	DESCRIPTION:
**		set mode of this demodulator
**
**	IN:
**		MT_FE_DD_Device_Handle  handle,MT_FE_DD_DEMOD_MODE set_mode
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_change_mode_dd3k(MT_FE_DD_Device_Handle  handle, MT_FE_TYPE set_mode)
{
	U8	Value = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		if (set_mode == MtFeType_CTTB)
		{
			handle->demod_cur_mode = MtFeType_CTTB;

			handle->dmd_set_reg(handle, 0x001d, 0x8d);
			handle->dmd_set_reg(handle, 0x001e, 0x8a);
			handle->dmd_set_reg(handle, 0x001d, 0x8e);
			handle->dmd_set_reg(handle, 0x001e, 0xf5);
			handle->dmd_set_reg(handle, 0x001d, 0x8f);
			handle->dmd_set_reg(handle, 0x001e, 0x01);

			handle->mt_sleep(1);

			handle->dmd_get_reg(handle, 0x0003, &Value);
			Value = (U8)(Value | 0x41);
			handle->dmd_set_reg(handle, 0x0003, Value);
			handle->dmd_set_reg(handle, 0x0006, 0x02);

			mt_fe_dmd_soft_reset_dd3k(handle);

			if (handle->tuner_ct_settings.tuner_init != NULL)
			{
				if (handle->tuner_ct_settings.tuner_init_ok == 0)
				{
					handle->tuner_ct_settings.tuner_init(handle);
					handle->tuner_ct_settings.tuner_init_ok = 1;
				}
			}
		}
		else	/*DVB-C*/
		{
			handle->demod_cur_mode = MtFeType_CTTB;

			handle->dmd_set_reg(handle, 0x001d, 0x8d);
			handle->dmd_set_reg(handle, 0x001e, 0xba);
			handle->dmd_set_reg(handle, 0x001d, 0x8e);
			handle->dmd_set_reg(handle, 0x001e, 0x95);
			handle->dmd_set_reg(handle, 0x001d, 0x8f);
			handle->dmd_set_reg(handle, 0x001e, 0x05);

			handle->mt_sleep(1);

			handle->dmd_get_reg(handle, 0x0003, &Value);
			Value = (U8)(Value | 0x41);
			handle->dmd_set_reg(handle, 0x0003, Value);
			handle->dmd_set_reg(handle, 0x0006, 0x0c);

			handle->demod_cur_mode = MtFeType_DVBC;
			if(handle->tuner_dc_settings.tuner_init != NULL)
			{
				if(handle->tuner_dc_settings.tuner_init_ok == 0)
				{
					handle->tuner_dc_settings.tuner_init(handle);
					handle->tuner_dc_settings.tuner_init_ok = 1;
				}
			}
		}
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		if (set_mode == MtFeType_CTTB)
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x00);
			handle->mt_sleep(1);
			handle->demod_cur_mode = MtFeType_CTTB;

			mt_fe_dmd_soft_reset_dd3k(handle);

			if (handle->tuner_ct_settings.tuner_init != NULL)
			{
				if (handle->tuner_ct_settings.tuner_init_ok == 0)
				{
					handle->tuner_ct_settings.tuner_init(handle);
					handle->tuner_ct_settings.tuner_init_ok = 1;
				}
			}
		}
		else	/*DVB-C*/
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x80);
			handle->mt_sleep(1);
			handle->demod_cur_mode = MtFeType_DVBC;

			mt_fe_dmd_soft_reset_dd3k(handle);

			if(handle->tuner_dc_settings.tuner_init != NULL)
			{
				if(handle->tuner_dc_settings.tuner_init_ok == 0)
				{
					handle->tuner_dc_settings.tuner_init(handle);
					handle->tuner_dc_settings.tuner_init_ok = 1;
				}
			}
		}
	}

	return MtFeErr_Ok;
}

/**		mt_fe_scan_settings_default_dd3k_c
**
**	DESCRIPTION:
**		config auto scan in dvbc mode by default settings
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_scan_settings_default_dd3k_c(MT_FE_DD_Device_Handle  handle)
{
	handle->dvbc_qam_config.qam[0] = 64;
	handle->dvbc_qam_config.qam[1] = 256;
	handle->dvbc_qam_config.qam[2] = 128;
	handle->dvbc_qam_config.qam[3] = 32;
	handle->dvbc_qam_config.qam[4] = 16;
	handle->dvbc_qam_config.cnt = 5;
	handle->dvbc_qam_config.wait_time = 1600;

	return MtFeErr_Ok;
}
/*	FUNCTIN:
**		mt_fe_config_scan_settings_dd3k_c
**
**	DESCRIPTION:
**		cofig scan settings in dvbc mode
**
**	IN:
**		MT_FE_DD_Device_Handle handle
**		DVBC_QAM_CONFIG *config_handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_config_scan_settings_dd3k_c(MT_FE_DD_Device_Handle handle, DVBC_QAM_CONFIG *config_handle)
{
	handle->dvbc_qam_config.qam[0] = config_handle->qam[0];
	handle->dvbc_qam_config.qam[1] = config_handle->qam[1];
	handle->dvbc_qam_config.qam[2] = config_handle->qam[2];
	handle->dvbc_qam_config.qam[3] = config_handle->qam[3];
	handle->dvbc_qam_config.qam[4] = config_handle->qam[4];
	handle->dvbc_qam_config.cnt = config_handle->cnt;
	handle->dvbc_qam_config.wait_time = config_handle->wait_time;

	return MtFeErr_Ok;
}
/*	FUNCTIN:
**		mt_fe_demod_settings_default_dd3k
**
**	DESCRIPTION:
**		config demod by default settings
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_demod_settings_default_dd3k(MT_FE_DD_Device_Handle  handle)
{
	/*CTTB default setting*/
	handle->demod_ct_settings.demod_mode = MtFeType_CTTB;
	handle->demod_ct_settings.demod_bandwidth = MtFeBandwidth_8M;
	handle->demod_ct_settings.demod_i2c_addr = 0x20;
	handle->demod_ct_settings.ts_out_mode = MtFeTsOutMode_Common;
	handle->demod_ct_settings.ts_out_swap = MtFeTsOut_Normal;

	/*DVBC default setting*/
	handle->demod_dc_settings.demod_mode = MtFeType_DVBC;
	handle->demod_dc_settings.demod_bandwidth = 0;
	handle->demod_dc_settings.demod_i2c_addr = 0x38;
	handle->demod_dc_settings.ts_out_mode = MtFeTsOutMode_Common;
	handle->demod_dc_settings.ts_out_swap = MtFeTsOut_Normal;

	mt_fe_scan_settings_default_dd3k_c(handle);

	handle->demod_cur_mode = MtFeType_CTTB;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_config_demod_settings_dd3k
**
**	DESCRIPTION:
**		cofig demod settings
**
**	IN:
**		MT_FE_DD_Device_Handle handle
**		MT_FE_DD_DEMOD_SETTINGS *demod_handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_config_demod_settings_dd3k(MT_FE_DD_Device_Handle handle, MT_FE_DD_DEMOD_SETTINGS *demod_handle)
{
	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if (demod_handle->demod_mode == MtFeType_CTTB)
	{
		handle->demod_ct_settings.demod_mode = demod_handle->demod_mode;
		handle->demod_ct_settings.demod_bandwidth = demod_handle->demod_bandwidth;
		handle->demod_ct_settings.demod_i2c_addr = demod_handle->demod_i2c_addr;
		handle->demod_ct_settings.ts_out_mode = demod_handle->ts_out_mode;
		handle->demod_ct_settings.ts_out_swap = demod_handle->ts_out_swap;
	}
	else/*DVBC*/
	{
		handle->demod_dc_settings.demod_mode = demod_handle->demod_mode;
		handle->demod_dc_settings.demod_bandwidth = demod_handle->demod_bandwidth;
		handle->demod_dc_settings.demod_i2c_addr = demod_handle->demod_i2c_addr;
		handle->demod_dc_settings.ts_out_mode = demod_handle->ts_out_mode;
		handle->demod_dc_settings.ts_out_swap = demod_handle->ts_out_swap;
	}

	return MtFeErr_Ok;
}

#if 0
/*	FUNCTIN:
**		mt_fe_tuner_settings_default_dd3k
**
**	DESCRIPTION:
**		config tuner by default settings
**
**	IN:
**		MT_FE_Tuner_Handle tuner_handle
**
**	OUT:
**
**
**	RETURN:
*/
MT_FE_RET mt_fe_tuner_settings_default_dd3k(MT_FE_DD_Device_Handle  handle)
{
	if	(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	/*CTTB Setting*/
	handle->tuner_ct_settings.tuner_type = MtFeTN_AlpsTdac3c02a;
	handle->tuner_ct_settings.tuner_mode = MtFeType_CTTB;
	handle->tuner_ct_settings.tuner_dev_addr = 0xc2;
	handle->tuner_ct_settings.tuner_init_ok = 0;
	handle->tuner_ct_settings.tuner_init = NULL;
	handle->tuner_ct_settings.tuner_set = mt_fe_tn_set_freq_tdac3_c02a;
	handle->tuner_ct_settings.tuner_get_strength = mt_fe_tn_get_signal_strength_tdac3_c02a;

	/*DVBC Setting*/
	handle->tuner_dc_settings.tuner_type = MtFeTN_AlpsTdac3c02a;
	handle->tuner_dc_settings.tuner_mode = MtFeType_DVBC;
	handle->tuner_dc_settings.tuner_dev_addr = 0xc2;
	handle->tuner_dc_settings.tuner_init_ok = 0;
	handle->tuner_dc_settings.tuner_init = NULL;
	handle->tuner_dc_settings.tuner_set = mt_fe_tn_set_freq_tdac3_c02a;
	handle->tuner_dc_settings.tuner_get_strength = mt_fe_tn_get_signal_strength_tdac3_c02a;

	return MtFeErr_Ok;
}
#endif

/*	FUNCTIN:
**		mt_fe_config_tuner_settings_dd3k
**
**	DESCRIPTION:
**		config tuner settings
**
**	IN:
**		MT_FE_DD_Device_Handle handle,
**		MT_FE_DD_TN_DEV_SETTINGS *tuner_handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_config_tuner_settings_dd3k(MT_FE_DD_Device_Handle handle, MT_FE_DD_TN_DEV_SETTINGS *tuner_handle)
{
	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if (tuner_handle->tuner_mode == MtFeType_CTTB)
	{
		handle->tuner_ct_settings.tuner_type = tuner_handle->tuner_type;
		handle->tuner_ct_settings.tuner_mode = tuner_handle->tuner_mode;
		handle->tuner_ct_settings.tuner_dev_addr = tuner_handle->tuner_dev_addr;
		handle->tuner_ct_settings.tuner_init_ok = tuner_handle->tuner_init_ok;
		handle->tuner_ct_settings.tuner_init = tuner_handle->tuner_init;
		handle->tuner_ct_settings.tuner_set = tuner_handle->tuner_set;
		handle->tuner_ct_settings.tuner_get_strength = tuner_handle->tuner_get_strength;
		handle->tuner_ct_settings.tuner_sleep = tuner_handle->tuner_sleep;		// fix102281
		handle->tuner_ct_settings.tuner_wake_up = tuner_handle->tuner_wake_up;	// fix102281
	}
	else
	{
		handle->tuner_dc_settings.tuner_type = tuner_handle->tuner_type;
		handle->tuner_dc_settings.tuner_mode = tuner_handle->tuner_mode;
		handle->tuner_dc_settings.tuner_dev_addr = tuner_handle->tuner_dev_addr;
		handle->tuner_dc_settings.tuner_init_ok = tuner_handle->tuner_init_ok;
		handle->tuner_dc_settings.tuner_init = tuner_handle->tuner_init;
		handle->tuner_dc_settings.tuner_set = tuner_handle->tuner_set;
		handle->tuner_dc_settings.tuner_get_strength = tuner_handle->tuner_get_strength;
		handle->tuner_dc_settings.tuner_sleep = tuner_handle->tuner_sleep;		// fix102281
		handle->tuner_dc_settings.tuner_wake_up = tuner_handle->tuner_wake_up;	// fix102281
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_reg_init_dd3k_t
**
**	DESCRIPTION:
**		init CTTB module register
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**		none
**	RETURN:
*
*/
MT_FE_RET _mt_fe_dmd_reg_init_dd3k_t(MT_FE_DD_Device_Handle  handle)
{
	U8	Value = 0;
	U8	data1 = 0,data2 = 0;
	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;
	}
	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT
		handle->dmd_set_reg(handle, 0x0005, 0x01);
		handle->dmd_set_reg(handle, 0x0005, 0x00);
		handle->dmd_set_reg(handle, 0x0000, 0x01);
		//analog regulator
		handle->dmd_set_reg(handle, 0x001d, 0x82);
		handle->dmd_set_reg(handle, 0x001e, 0x80);
		handle->dmd_set_reg(handle, 0x001d, 0x80);
		handle->dmd_set_reg(handle, 0x001e, 0xd4);
		handle->dmd_set_reg(handle, 0x001d, 0x81);
		handle->dmd_set_reg(handle, 0x001e, 0x68);
		handle->dmd_set_reg(handle, 0x001d, 0x83);
		handle->dmd_set_reg(handle, 0x001e, 0xaa);
		handle->dmd_set_reg(handle, 0x001d, 0x85);
		handle->dmd_set_reg(handle, 0x001e, 0x92);
		handle->dmd_set_reg(handle, 0x001d, 0x86);
		handle->dmd_set_reg(handle, 0x001e, 0x64);

		handle->dmd_set_reg(handle, 0x001d, 0x3d);
		handle->dmd_set_reg(handle, 0x001e, 0xba);
		handle->dmd_set_reg(handle, 0x001d, 0x8e);
		handle->dmd_set_reg(handle, 0x001e, 0x95);
		handle->dmd_set_reg(handle, 0x001d, 0x8f);
		handle->dmd_set_reg(handle, 0x001e, 0x05);
		handle->dmd_set_reg(handle, 0x001d, 0x89);
		handle->dmd_set_reg(handle, 0x001e, 0xa9);
		handle->dmd_set_reg(handle, 0x001d, 0x8a);
		handle->dmd_set_reg(handle, 0x001e, 0x0e);
		handle->dmd_set_reg(handle, 0x001d, 0x82);
		handle->dmd_set_reg(handle, 0x001e, 0x00);
		handle->dmd_set_reg(handle, 0x001d, 0x8d);
		handle->dmd_set_reg(handle, 0x001e, 0x8a);
		handle->dmd_set_reg(handle, 0x001d, 0x8e);
		handle->dmd_set_reg(handle, 0x001e, 0xf5);
		handle->dmd_set_reg(handle, 0x001d, 0x8f);
		handle->dmd_set_reg(handle, 0x001e, 0x01);
		handle->dmd_set_reg(handle, 0x001d, 0x89);
		handle->dmd_set_reg(handle, 0x001e, 0xad);
		handle->dmd_set_reg(handle, 0x001d, 0x82);
		handle->dmd_set_reg(handle, 0x001e, 0x05);
		//agc output from OE to D0 in OD pad
		handle->dmd_set_reg(handle, 0x0003, 0xc1);
		handle->dmd_set_reg(handle, 0x001f, 0x05);

		handle->dmd_set_reg(handle, 0x0007, 0x03);
		handle->dmd_set_reg(handle, 0x00ae, 0x05);
		handle->dmd_set_reg(handle, 0x0004, 0x06);
		handle->dmd_set_reg(handle, 0x0031, 0xdd);
		handle->dmd_set_reg(handle, 0x0034, 0x85);
		handle->dmd_set_reg(handle, 0x007d, 0x06);
		handle->dmd_set_reg(handle, 0x0078, 0xc9);
		handle->dmd_set_reg(handle, 0x00ed, 0x40);
		handle->dmd_set_reg(handle, 0x00ef, 0x0b);
		handle->dmd_set_reg(handle, 0x0073, 0x70);

		handle->dmd_set_reg(handle, 0x00ed, 0x44);
		handle->dmd_set_reg(handle, 0x00ef, 0x90);
		handle->dmd_set_reg(handle, 0x0064, 0x68);
		handle->dmd_set_reg(handle, 0x0069, 0x0b);
		handle->dmd_set_reg(handle, 0x0075, 0x44);

		handle->dmd_set_reg(handle, 0x007d, 0x28);
		handle->dmd_set_reg(handle, 0x007e, 0x00);
		handle->dmd_set_reg(handle, 0x007d, 0x29);
		handle->dmd_set_reg(handle, 0x007e, 0x00);
		handle->dmd_set_reg(handle, 0x007d, 0x2a);
		handle->dmd_set_reg(handle, 0x007e, 0x1a);
		handle->dmd_set_reg(handle, 0x007d, 0x2b);
		handle->dmd_set_reg(handle, 0x007e, 0x4c);
		handle->dmd_set_reg(handle, 0x007d, 0x2c);
		handle->dmd_set_reg(handle, 0x007e, 0x1a);
		handle->dmd_set_reg(handle, 0x007d, 0x2d);
		handle->dmd_set_reg(handle, 0x007e, 0x00);
		handle->dmd_set_reg(handle, 0x007d, 0x2e);
		handle->dmd_set_reg(handle, 0x007e, 0x00);

		if (handle->demod_ct_settings.ts_out_swap != MtFeTsOut_Normal)
		{
			handle->dmd_get_reg(handle, 0x001f, &Value);
			Value = (U8)(Value | 0x02);
			handle->dmd_set_reg(handle, 0x001f, Value);
		}

		if(handle->tuner_ct_settings.tuner_type == MtFeTN_MxL5007T)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x0b);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0x87);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x03);

			handle->dmd_set_reg(handle, 0x0028, 0x02);
		}
		else if(handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18273)
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x0e);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0x38);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0xe4);

			handle->dmd_set_reg(handle, 0x007d, 0x06);
			handle->dmd_set_reg(handle, 0x0078, 0xa9);
			handle->dmd_set_reg(handle, 0x0028, 0x01);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_TC2800)
		{
			handle->dmd_set_reg(handle, 0x0028, 0x04);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL601)
		{
			handle->dmd_set_reg(handle, 0x0028, 0x05);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_SI2158)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x09);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0xc3);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x0a);
			handle->dmd_set_reg(handle, 0x0028, 0x06);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18257)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x09);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0xc3);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x0a);

			handle->dmd_set_reg(handle, 0x007d, 0x06);
			handle->dmd_set_reg(handle, 0x0078, 0xa9);
			handle->dmd_set_reg(handle, 0x0028, 0x01);
		}
#if 0
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18275)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x0c);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0x07);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x62);

			handle->dmd_set_reg(handle, 0x007d, 0x06);
			handle->dmd_set_reg(handle, 0x0078, 0xa9);
			handle->dmd_set_reg(handle, 0x0028, 0x01);
		}
#endif
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL603)
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x0d);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0xbb);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x1c);

		}
#if 0
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL608)
		{
			handle->dmd_set_reg(handle, 0x0044, 0x00);
			handle->dmd_set_reg(handle, 0x0045, 0x0d);
			handle->dmd_set_reg(handle, 0x0044, 0x01);
			handle->dmd_set_reg(handle, 0x0045, 0xbb);
			handle->dmd_set_reg(handle, 0x0044, 0x02);
			handle->dmd_set_reg(handle, 0x0045, 0x1c);
		}
#endif
		//SOFT reset
		handle->dmd_set_reg(handle, 0x0000, 0x00);
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		handle->dmd_set_reg(handle, 0x0000, 0x01);
		handle->dmd_set_reg(handle, 0x0000, 0x00);
		handle->dmd_set_reg(handle, 0x0243, 0x80);
		handle->dmd_set_reg(handle, 0x0241, 0x01);
		handle->dmd_set_reg(handle, 0x0240, 0x80);
		handle->dmd_set_reg(handle, 0x0032, 0x01);
		handle->dmd_set_reg(handle, 0x0008, 0x40);
		handle->dmd_set_reg(handle, 0x0004, 0x01);

// fix102281 start
		handle->dmd_set_reg(handle, 0x0049, 0x02);
		handle->dmd_set_reg(handle, 0x0061, 0x50);
		handle->dmd_set_reg(handle, 0x007c, 0x85);
		handle->dmd_set_reg(handle, 0x007d, 0x0a);
		handle->dmd_set_reg(handle, 0x00c1, 0x0c);
		handle->dmd_set_reg(handle, 0x00c2, 0x3f);
		handle->dmd_set_reg(handle, 0x00c5, 0x3f);
		handle->dmd_set_reg(handle, 0x00c9, 0x18);
		handle->dmd_set_reg(handle, 0x00cd, 0x06);
		handle->dmd_set_reg(handle, 0x00e4, 0x33);
		handle->dmd_set_reg(handle, 0x00e5, 0x33);
		handle->dmd_set_reg(handle, 0x00e6, 0x33);

		handle->dmd_set_reg(handle, 0x0143, 0x6e);
		handle->dmd_set_reg(handle, 0x0151, 0x86);
		handle->dmd_set_reg(handle, 0x0156, 0x62);
		handle->dmd_set_reg(handle, 0x0157, 0x76);
		handle->dmd_set_reg(handle, 0x0158, 0x66);
		handle->dmd_set_reg(handle, 0x0159, 0x63);
		handle->dmd_set_reg(handle, 0x015a, 0x76);
		handle->dmd_set_reg(handle, 0x015b, 0x66);
		handle->dmd_set_reg(handle, 0x015d, 0x76);
		handle->dmd_set_reg(handle, 0x015e, 0xf4);
		handle->dmd_set_reg(handle, 0x015f, 0x8d);
		handle->dmd_set_reg(handle, 0x0178, 0x04);
		handle->dmd_set_reg(handle, 0x017a, 0x01);

		handle->dmd_set_reg(handle, 0x0186, 0x3a);
		handle->dmd_set_reg(handle, 0x0187, 0x17);
		handle->dmd_set_reg(handle, 0x0189, 0x24);
		handle->dmd_set_reg(handle, 0x018b, 0x3f);
		handle->dmd_set_reg(handle, 0x018c, 0x17);
		handle->dmd_set_reg(handle, 0x0191, 0x3e);
		handle->dmd_set_reg(handle, 0x0194, 0x32);
		handle->dmd_set_reg(handle, 0x0196, 0x64);
		handle->dmd_set_reg(handle, 0x01a6, 0x00);

		handle->dmd_set_reg(handle, 0x0290, 0x80);
		handle->dmd_set_reg(handle, 0x0291, 0x83);

		////adc
		data1 = 0;
		handle->dmd_get_reg(handle,0x0022,&data1);
		handle->dmd_set_reg(handle,0x0022,(U8)(data1 & 0xfd));
// fix102281 end

		handle->dmd_set_reg(handle,0x0013,0x0c);  // cal clock div 4 =5.8MHz
		handle->dmd_set_reg(handle,0x037a,0xFF);  // high threshold
		handle->dmd_set_reg(handle,0x037b,0x00);  // low threshold

		handle->dmd_set_reg(handle,0x001e,0x77);  // turn on cal clock
		handle->dmd_set_reg(handle,0x001f,0x50);  // reset cal clock
		handle->dmd_set_reg(handle,0x001f,0x54);  // release cal clock; set freq

		handle->dmd_set_reg(handle,0x0377,0x40);  // bypass ADC IQ balance
		handle->dmd_set_reg(handle,0x0363,0x5a);  // sample freq div4
		handle->dmd_set_reg(handle,0x0364,0xC4);  // IQ cal path enable, pse

		Value = 0;
		do
		{
			data1=0;
			data2 = 0;
			Value++;
			handle->dmd_set_reg(handle,0x0361,0x00); // clear

			handle->dmd_set_reg(handle,0x0360,0xff);
			handle->mt_sleep(2);
			handle->dmd_set_reg(handle,0x0360,0x00); // soft reset

			handle->dmd_set_reg(handle,0x0362,0x63);
			handle->mt_sleep(2);
			handle->dmd_set_reg(handle,0x0362,0x23); // clear log

			handle->dmd_set_reg(handle,0x0361,0x3a); // start calibration
			handle->mt_sleep(20);

			handle->dmd_get_reg(handle,0x0369,&data1); // read cal log
			handle->dmd_get_reg(handle,0x036b,&data2); // read cal log
		}while((((data1 != 0x2d) && (data1 != 0x2e) && (data1 != 0x2c)) || ((data2 != 0x2d) && (data2 != 0x2e) && (data2 != 0x2c))) && (Value < 4));
// fix102281

		handle->dmd_set_reg(handle,0x0361,0x00);  // disable cal setting
		handle->dmd_set_reg(handle,0x001e,0x07);  // turn off cal clock

// fix102281 start
		data1 = 0;
		handle->dmd_get_reg(handle,0x0022,&data1);
		handle->dmd_set_reg(handle,0x0022,(U8)(data1 | 0x02));
// fix102281 end

		///////////
		Value = 0;
		handle->dmd_get_reg(handle, 0x0009, &Value);
		Value = (U8)(Value | 0x10);
		handle->dmd_set_reg(handle, 0x0009, Value);

		handle->dmd_set_reg(handle, 0x0008, 0x10);
		handle->dmd_set_reg(handle, 0x0004, 0x00);
		handle->dmd_set_reg(handle, 0x0032, 0x00);

		handle->dmd_set_reg(handle, 0x0242, 0x00);	// fix102281
		//if (handle->demod_ct_settings.ts_out_swap != MtFeTsOut_Normal)
		//{
		//	handle->dmd_get_reg(handle, 0x001f, &Value);
		//	Value = (U8)(Value | 0x02);
		//	handle->dmd_set_reg(handle, 0x001f, Value);
		//}

		if(handle->tuner_ct_settings.tuner_type == MtFeTN_MxL5007T)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0086, 0x0b);
			handle->dmd_set_reg(handle, 0x0085, 0x87);
			handle->dmd_set_reg(handle, 0x0084, 0x03);
		}
		else if(handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18273)
		{
			handle->dmd_set_reg(handle, 0x0086, 0x0e);
			handle->dmd_set_reg(handle, 0x0085, 0x38);
			handle->dmd_set_reg(handle, 0x0084, 0xe4);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_TC2800)
		{
			//handle->dmd_set_reg(handle, 0x0028, 0x04);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL601)
		{
		//	handle->dmd_set_reg(handle, 0x0028, 0x05);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_SI2158)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0086, 0x09);
			handle->dmd_set_reg(handle, 0x0085, 0xc3);
			handle->dmd_set_reg(handle, 0x0084, 0x0a);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18257)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0086, 0x09);
			handle->dmd_set_reg(handle, 0x0085, 0xc3);
			handle->dmd_set_reg(handle, 0x0084, 0x0a);
		}
#if 0
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18275)//for low IF
		{
			handle->dmd_set_reg(handle, 0x0086, 0x0c);
			handle->dmd_set_reg(handle, 0x0085, 0x07);
			handle->dmd_set_reg(handle, 0x0084, 0x62);
		}
#endif
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL603)
		{
			handle->dmd_set_reg(handle, 0x0086, 0x09);//0x0d);
			handle->dmd_set_reg(handle, 0x0085, 0xc3);//0xbb);
			handle->dmd_set_reg(handle, 0x0084, 0x0a);//0x1c);

		}
#if 0
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL608)////IF 5M
		{
			handle->dmd_set_reg(handle, 0x0086, 0x09);
			handle->dmd_set_reg(handle, 0x0085, 0xc3);
			handle->dmd_set_reg(handle, 0x0084, 0x0a);
		}
#endif
// fix102281 start
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_TC3800)
		{
			handle->dmd_get_reg(handle,0x0240,&data1);
			data1 = (U8)(data1 | 0x01);
			handle->dmd_set_reg(handle, 0x0240, data1);
			handle->dmd_get_reg(handle,0x036d,&data1);
			data1 = (U8)((data1 & 0xcf)| 0x40);
			handle->dmd_set_reg(handle, 0x036d, data1);
		}
		else if (handle->tuner_ct_settings.tuner_type == MtFeTN_TC6800)
		{
			handle->dmd_get_reg(handle,0x0240,&data1);
			data1 = (U8)(data1 | 0x01);
			handle->dmd_set_reg(handle, 0x0240, data1);
			handle->dmd_get_reg(handle,0x036d,&data1);
			data1 = (U8)((data1 & 0xcf)| 0x40);
			handle->dmd_set_reg(handle, 0x036d, data1);
		}
// fix102281 end
	}

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		_mt_fe_dmd_reg_init_dd3k_c
**
**	DESCRIPTION:
**		init DVB-C module register
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**		none
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_reg_init_dd3k_c(MT_FE_DD_Device_Handle  handle)
{
	if (handle->demod_cur_mode != MtFeType_DVBC)
	{
		return MtFeErr_NoMatch;
	}

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT
		handle->dmd_set_reg(handle, 0x0000, 0x48);
		handle->dmd_set_reg(handle, 0x0001, 0x09);
		handle->dmd_set_reg(handle, 0x00fb, 0x0a);
		handle->dmd_set_reg(handle, 0x00fc, 0x0b);
		handle->dmd_set_reg(handle, 0x0002, 0x0b);
		handle->dmd_set_reg(handle, 0x0003, 0x18);
		handle->dmd_set_reg(handle, 0x0005, 0x0d);
		handle->dmd_set_reg(handle, 0x0036, 0x80);
		handle->dmd_set_reg(handle, 0x0043, 0x40);
		handle->dmd_set_reg(handle, 0x0055, 0x7a);

		handle->dmd_set_reg(handle, 0x0056, 0xd9);
		handle->dmd_set_reg(handle, 0x0057, 0xdF);
		handle->dmd_set_reg(handle, 0x0058, 0x39);
		handle->dmd_set_reg(handle, 0x005a, 0x00);
		handle->dmd_set_reg(handle, 0x005c, 0x71);
		handle->dmd_set_reg(handle, 0x005d, 0x23);
		handle->dmd_set_reg(handle, 0x0086, 0x40);
		handle->dmd_set_reg(handle, 0x00f9, 0x08);
		handle->dmd_set_reg(handle, 0x0061, 0x40);
		handle->dmd_set_reg(handle, 0x0062, 0x0a);

		handle->dmd_set_reg(handle, 0x0090, 0x06);
		handle->dmd_set_reg(handle, 0x00de, 0x00);
		handle->dmd_set_reg(handle, 0x00a0, 0x03);
		handle->dmd_set_reg(handle, 0x00df, 0x81);
		handle->dmd_set_reg(handle, 0x00fa, 0x40);
		handle->dmd_set_reg(handle, 0x0037, 0x10);
		handle->dmd_set_reg(handle, 0x00f0, 0x40);
		handle->dmd_set_reg(handle, 0x00f2, 0x9c);
		handle->dmd_set_reg(handle, 0x00f3, 0x40);
		handle->dmd_set_reg(handle, 0x0030, 0xff);

		handle->dmd_set_reg(handle, 0x0031, 0x00);
		handle->dmd_set_reg(handle, 0x0032, 0x00);
		handle->dmd_set_reg(handle, 0x0033, 0x00);
		handle->dmd_set_reg(handle, 0x0035, 0x32);
		handle->dmd_set_reg(handle, 0x0039, 0x00);
		handle->dmd_set_reg(handle, 0x003a, 0x00);
		handle->dmd_set_reg(handle, 0x00f1, 0x00);
		handle->dmd_set_reg(handle, 0x00f4, 0x00);
		handle->dmd_set_reg(handle, 0x00f5, 0x40);
		handle->dmd_set_reg(handle, 0x0042, 0x24);

		handle->dmd_set_reg(handle, 0x00e1, 0x27);
		handle->dmd_set_reg(handle, 0x0092, 0x7f);
		handle->dmd_set_reg(handle, 0x0093, 0x91);
		handle->dmd_set_reg(handle, 0x0095, 0x00);
		handle->dmd_set_reg(handle, 0x002b, 0x33);
		handle->dmd_set_reg(handle, 0x002a, 0x2a);
		handle->dmd_set_reg(handle, 0x002e, 0x80);
		handle->dmd_set_reg(handle, 0x0025, 0x25);
		handle->dmd_set_reg(handle, 0x002d, 0xff);
		handle->dmd_set_reg(handle, 0x0026, 0xff);

		handle->dmd_set_reg(handle, 0x0027, 0x00);
		handle->dmd_set_reg(handle, 0x0024, 0x25);
		handle->dmd_set_reg(handle, 0x00a4, 0xff);
		handle->dmd_set_reg(handle, 0x00a3, 0x10);
		handle->dmd_set_reg(handle, 0x00f6, 0x4e);
		handle->dmd_set_reg(handle, 0x00f7, 0x20);
		handle->dmd_set_reg(handle, 0x0089, 0x02);
		handle->dmd_set_reg(handle, 0x0014, 0x08);
		handle->dmd_set_reg(handle, 0x006f, 0x0d);
		handle->dmd_set_reg(handle, 0x0010, 0xff);

		handle->dmd_set_reg(handle, 0x0011, 0x00);
		handle->dmd_set_reg(handle, 0x0012, 0x30);
		handle->dmd_set_reg(handle, 0x0013, 0x23);
		handle->dmd_set_reg(handle, 0x0060, 0x00);
		handle->dmd_set_reg(handle, 0x0069, 0x00);
		handle->dmd_set_reg(handle, 0x006a, 0x03);
		handle->dmd_set_reg(handle, 0x00e0, 0x75);
		handle->dmd_set_reg(handle, 0x008d, 0x29);
		handle->dmd_set_reg(handle, 0x004e, 0xd8);
		handle->dmd_set_reg(handle, 0x0088, 0x80);

		handle->dmd_set_reg(handle, 0x0052, 0x79);
		handle->dmd_set_reg(handle, 0x0053, 0x03);
		handle->dmd_set_reg(handle, 0x0059, 0x30);
		handle->dmd_set_reg(handle, 0x005e, 0x02);
		handle->dmd_set_reg(handle, 0x005f, 0x0f);
		handle->dmd_set_reg(handle, 0x0071, 0x03);
		handle->dmd_set_reg(handle, 0x0072, 0x12);
		handle->dmd_set_reg(handle, 0x0073, 0x12);

		if (handle->demod_dc_settings.ts_out_swap != MtFeTsOut_Normal)
		{
			handle->dmd_set_reg(handle, 0x009a, 0x50);
			handle->dmd_set_reg(handle, 0x009b, 0x34);
			handle->dmd_set_reg(handle, 0x009c, 0x12);
		}

		if (handle->tuner_dc_settings.tuner_type == MtFeTN_NXPTDA18273)
		{
			handle->dmd_set_reg(handle, 0x0016, 0x13);
			handle->dmd_set_reg(handle, 0x0017, 0x8E);
		}
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		handle->dmd_set_reg(handle, 0x0000, 0x48);
		handle->dmd_set_reg(handle, 0x0001, 0x09);
		handle->dmd_set_reg(handle, 0x00fb, 0x0a);
		handle->dmd_set_reg(handle, 0x00fc, 0x0b);
		handle->dmd_set_reg(handle, 0x0002, 0x0b);
		handle->dmd_set_reg(handle, 0x0003, 0x18);
		handle->dmd_set_reg(handle, 0x0005, 0x0d);
		handle->dmd_set_reg(handle, 0x0036, 0x80);
		handle->dmd_set_reg(handle, 0x0043, 0x40);
		handle->dmd_set_reg(handle, 0x0055, 0x7a);
		handle->dmd_set_reg(handle, 0x0056, 0xd9);
		handle->dmd_set_reg(handle, 0x0057, 0xdF);
		handle->dmd_set_reg(handle, 0x0058, 0x39);
		handle->dmd_set_reg(handle, 0x005a, 0x00);
		handle->dmd_set_reg(handle, 0x005c, 0x71);
		handle->dmd_set_reg(handle, 0x005d, 0x23);

		handle->dmd_set_reg(handle, 0x00f9, 0x08);
		handle->dmd_set_reg(handle, 0x0061, 0x40);
		handle->dmd_set_reg(handle, 0x0062, 0x0a);
		handle->dmd_set_reg(handle, 0x0090, 0x06);
		handle->dmd_set_reg(handle, 0x00de, 0x00);
		handle->dmd_set_reg(handle, 0x00a0, 0x03);
		handle->dmd_set_reg(handle, 0x00df, 0x81);
		handle->dmd_set_reg(handle, 0x00fa, 0x40);
		handle->dmd_set_reg(handle, 0x0037, 0x10);
		handle->dmd_set_reg(handle, 0x00f0, 0x40);
		handle->dmd_set_reg(handle, 0x00f2, 0x9c);
		handle->dmd_set_reg(handle, 0x00f3, 0x40);
		if ((handle->tuner_dc_settings.tuner_type == MtFeTN_TC3800) )
		{
			handle->dmd_set_reg(handle, 0x0030,  0xff);
		}
		else
			handle->dmd_set_reg(handle, 0x0030, 0xff);

		handle->dmd_set_reg(handle, 0x0031, 0x00);
		handle->dmd_set_reg(handle, 0x0032, 0x00);
		handle->dmd_set_reg(handle, 0x0033, 0x00);
		handle->dmd_set_reg(handle, 0x0035, 0x32);
		handle->dmd_set_reg(handle, 0x0039, 0x00);

		if ((handle->tuner_dc_settings.tuner_type == MtFeTN_TC3800) )
		{
			handle->dmd_set_reg(handle, 0x003a, 0x60);
		}
		else
			handle->dmd_set_reg(handle, 0x003a, 0x00);

		handle->dmd_set_reg(handle, 0x00f1, 0xff);
		handle->dmd_set_reg(handle, 0x00f4, 0x00);
		handle->dmd_set_reg(handle, 0x00f5, 0x40);
		handle->dmd_set_reg(handle, 0x0042, 0x24);
		//handle->dmd_set_reg(handle, 0x00e1, 0x27);

		handle->dmd_set_reg(handle, 0x0092, 0x7f);
		handle->dmd_set_reg(handle, 0x0093, 0x91);
		handle->dmd_set_reg(handle, 0x0095, 0x00);
		handle->dmd_set_reg(handle, 0x002b, 0x33);
		handle->dmd_set_reg(handle, 0x002a, 0x2a);
		handle->dmd_set_reg(handle, 0x002e, 0x80);
		handle->dmd_set_reg(handle, 0x0025, 0x25);
		handle->dmd_set_reg(handle, 0x002d, 0xff);
		handle->dmd_set_reg(handle, 0x0026, 0xff);
		handle->dmd_set_reg(handle, 0x0027, 0x00);
		handle->dmd_set_reg(handle, 0x0024, 0x25);
		handle->dmd_set_reg(handle, 0x00a4, 0xff);
		handle->dmd_set_reg(handle, 0x00a3, 0x0d);

		handle->dmd_set_reg(handle, 0x00f6, 0x4e);
		handle->dmd_set_reg(handle, 0x00f7, 0x20);
		handle->dmd_set_reg(handle, 0x0089, 0x02);
		handle->dmd_set_reg(handle, 0x0014, 0x08);
		handle->dmd_set_reg(handle, 0x006f, 0x0d);
		handle->dmd_set_reg(handle, 0x0010, 0xff);
		handle->dmd_set_reg(handle, 0x0011, 0x00);
		handle->dmd_set_reg(handle, 0x0012, 0x30);
		handle->dmd_set_reg(handle, 0x0013, 0x23);
		handle->dmd_set_reg(handle, 0x0060, 0x00);
		handle->dmd_set_reg(handle, 0x0069, 0x00);
		handle->dmd_set_reg(handle, 0x006a, 0x03);
		handle->dmd_set_reg(handle, 0x00e0, 0x75);

		//handle->dmd_set_reg(handle, 0x008d, 0x29);
		handle->dmd_set_reg(handle, 0x004e, 0xd8);
		handle->dmd_set_reg(handle, 0x0088, 0x80);
		handle->dmd_set_reg(handle, 0x0052, 0x79);
		handle->dmd_set_reg(handle, 0x0053, 0x03);
		handle->dmd_set_reg(handle, 0x0059, 0x30);
		handle->dmd_set_reg(handle, 0x005e, 0x02);
		handle->dmd_set_reg(handle, 0x005f, 0x0f);
		handle->dmd_set_reg(handle, 0x0071, 0x03);
		handle->dmd_set_reg(handle, 0x0072, 0x12);
		handle->dmd_set_reg(handle, 0x0073, 0x12);
/*
		if (handle->demod_dc_settings.ts_out_swap != MtFeTsOut_Normal)
		{
			handle->dmd_set_reg(handle, 0x009a, 0x50);
			handle->dmd_set_reg(handle, 0x009b, 0x34);
			handle->dmd_set_reg(handle, 0x009c, 0x12);
		}

		if (handle->tuner_dc_settings.tuner_type == MtFeTN_NXPTDA18273)
		{
			handle->dmd_set_reg(handle, 0x0016, 0x13);
			handle->dmd_set_reg(handle, 0x0017, 0x8E);
		}
		*/
	}


	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		_mt_fe_dmd_download_fw_dd3k
**
**	DESCRIPTION:
**		download CTTB module firmware
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_download_fw_dd3k_t(MT_FE_DD_Device_Handle  handle)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U32	i = 0;
	U32	firm_size = 0;
	U16	firm_addr = 0;
	U8	tmp[3] = {0,0,0};


	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;
	}
	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT

		const U8* p_fw=m88dd3k_fm;
		firm_size = sizeof(m88dd3k_fm)/2;

		handle->dmd_set_reg(handle, 0x00c6, 0x09);
		handle->dmd_set_reg(handle, 0x00b7, 0x01);
		handle->dmd_set_reg(handle, 0x0000, 0x01);
		handle->dmd_set_reg(handle, 0x0000, 0x00);
		handle->dmd_set_reg(handle, 0x00bd, 0x80);

		firm_addr = 0x00b5;
		for (i = 0; i < firm_size; i++)
		{
			tmp[0] = *(p_fw++);
			tmp[1] = *(p_fw++);

			ret = handle->write_fw(handle, firm_addr, tmp, 2);
			if (ret != MtFeErr_Ok)
				break;
		}
		handle->dmd_get_reg(handle, 0x00b5, &tmp[1]);
		handle->dmd_get_reg(handle, 0x00b6, &tmp[2]);
		if((tmp[1] != 0xfd) || (tmp[2] != 0x83))			// fix102281
		{
			handle->dmd_set_reg(handle, 0x00b7, 0x00);
			return MtFeErr_FirmwareErr;
		}
		handle->dmd_set_reg(handle, 0x00b7, 0x00);

		if (ret != MtFeErr_Ok)
		{
			mt_fe_print(("MT: _mt_fe_dmd_download_fw_dd3k FAILED!    [ret = %d]\n", ret));
			return MtFeErr_I2cErr;
		}
	#endif
	}
	else
	{
		const U8* p_fwc0 = m88dd3k_fmc0;
		firm_size = sizeof(m88dd3k_fmc0)/2;

		handle->dmd_set_reg(handle, 0x0032, 0x01);
		handle->dmd_set_reg(handle, 0x0008, 0x40);
		handle->dmd_set_reg(handle, 0x0008, 0x00);
		handle->dmd_set_reg(handle, 0x0038, 0x80);

		firm_addr = 0x0030;

		for (i = 0; i < firm_size; i++)
		{
			tmp[0] = *(p_fwc0++);
			tmp[1] = *(p_fwc0++);

			ret = handle->write_fw(handle, firm_addr, tmp, 2);
			if (ret != MtFeErr_Ok)
				break;
		}

		handle->dmd_get_reg(handle, 0x0030, &tmp[1]);
		handle->dmd_get_reg(handle, 0x0031, &tmp[2]);
		handle->dmd_set_reg(handle, 0x0032, 0x00);

		if((tmp[2] != 0xdc) || (tmp[1] != 0x9f))			// fix102281
		{
			return MtFeErr_FirmwareErr;
		}

		if (ret != MtFeErr_Ok)
		{
			mt_fe_print(("MT: _mt_fe_dmd_download_fw_dd3k FAILED!    [ret = %d]\n", ret));
			return MtFeErr_I2cErr;
		}
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_init_dd3k
**
**	DESCRIPTION:
**		initialize M88DD3000
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_init_dd3k(MT_FE_DD_Device_Handle  handle)
{
	S32 ret = MtFeErr_Ok;
	U8	data = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle->demod_cur_mode = MtFeType_CTTB;
	handle->demod_id = MtFeDmdId_DD630X;

	handle->dmd_get_reg(handle, 0x0001, &data);
	if((data == 0x30) || (data == 0x10))		// fix102281
		handle->demod_id = MtFeDmdId_DD3X0X;
	else if(data ==0xc0)
		handle->demod_id = MtFeDmdId_DD630X;
	else
		return MtFeErr_NoSupportDemod;

	_mt_fe_dmd_reg_init_dd3k_t(handle);
	ret = _mt_fe_dmd_download_fw_dd3k_t(handle);
	mt_fe_dmd_set_bandwidth_t(handle);
	mt_fe_dmd_set_output_mode_dd3k(handle);
	if (handle->tuner_ct_settings.tuner_init != NULL)
	{
		if (handle->tuner_ct_settings.tuner_init_ok == 0)
		{
			handle->tuner_ct_settings.tuner_init(handle);
			handle->tuner_ct_settings.tuner_init_ok = 1;
		}
	}

	if(ret != MtFeErr_Ok)
		return MtFeErr_FirmwareErr;

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_set_bandwidth_t
**
**	DESCRIPTION:
**		set CTTB bandwidth
**
**	IN:
**		MT_FE_DD_Device_Handle handle
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_bandwidth_t(MT_FE_DD_Device_Handle handle)
{

	if(handle->demod_cur_mode != MtFeType_CTTB)
		return MtFeErr_Undef;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT

		U8	reg_0x40 = 0;
		handle->dmd_get_reg(handle, 0x0040, &reg_0x40);

		switch(handle->demod_ct_settings.demod_bandwidth)
		{
			case MtFeBandwidth_6M:
				reg_0x40 &= 0xfc;
				reg_0x40 |= 0x02;
				handle->dmd_set_reg(handle, 0x0040, reg_0x40);
			break;

			case MtFeBandwidth_7M:
				reg_0x40 &= 0xfc;
				reg_0x40 |= 0x01;
				handle->dmd_set_reg(handle, 0x0040, reg_0x40);
			break;

			case MtFeBandwidth_8M:
				reg_0x40 &= 0xfc;
				reg_0x40 |= 0x00;
				handle->dmd_set_reg(handle, 0x0040, reg_0x40);
			break;

			default:
				reg_0x40 &= ~0x04/*0xFB*/;		/* clear bit_2*/
				handle->dmd_set_reg(handle, 0x0040, reg_0x40);
			break;
		}

	#endif

	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		U8	reg_0xac = 0;
		handle->dmd_get_reg(handle, 0x00ac, &reg_0xac);

		switch(handle->demod_ct_settings.demod_bandwidth)
		{
			case MtFeBandwidth_6M:
				reg_0xac &= 0xf3;
				reg_0xac |= 0x08;
				handle->dmd_set_reg(handle, 0x00ac, reg_0xac);
			break;

			case MtFeBandwidth_7M:
				reg_0xac &= 0xf3;
				reg_0xac |= 0x04;
				handle->dmd_set_reg(handle, 0x00ac, reg_0xac);
			break;

			case MtFeBandwidth_8M:
				reg_0xac &= 0xf3;
				reg_0xac |= 0x00;
				handle->dmd_set_reg(handle, 0x00ac, reg_0xac);
			break;

			default:
				reg_0xac &= 0xf3;
				reg_0xac |= 0x00;
				handle->dmd_set_reg(handle, 0x00ac, reg_0xac);
			break;
		}
	}

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_hard_reset_dd3k
**
**	DESCRIPTION:
**		reset m88dd3000
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**		none
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_hard_reset_dd3k(MT_FE_DD_Device_Handle  handle)
{
    int fd = -1;
    fd = open("/dev/TS_GPIO", 0);  // 打开设备
    if (fd < 0) {
        printf("Can't open /dev/gpio_demod_RST\n");
        close(fd);
        return -1;
    }

    ioctl(fd, IOCTL_RST_L, 10);    // 复位
    _mt_sleep(10);
    ioctl(fd, IOCTL_RST_H, 10);    // 置高
    close(fd);
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_dd3k
**
**	DESCRIPTION:
**		soft reset m88dd3000
**	IN:
**		MT_FE_DD_Device_Handle handle
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_dd3k(MT_FE_DD_Device_Handle handle)
{
	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT

		if (handle->demod_cur_mode != MtFeType_DVBC)
		{
			handle->dmd_set_reg(handle, 0x0005, 0x1);
			handle->dmd_set_reg(handle, 0x0005, 0x00);
			handle->dmd_set_reg(handle, 0x0000, 0x01);

			handle->dmd_set_reg(handle, 0x0031, 0xdd);
			handle->dmd_set_reg(handle, 0x0034, 0x85);
			handle->dmd_set_reg(handle, 0x007d, 0x06);
			handle->dmd_set_reg(handle, 0x0078, 0xc9);
			handle->dmd_set_reg(handle, 0x00ed, 0x40);
			handle->dmd_set_reg(handle, 0x00ef, 0x0b);
			handle->dmd_set_reg(handle, 0x0073, 0x70);

			handle->dmd_set_reg(handle, 0x00ed, 0x44);
			handle->dmd_set_reg(handle, 0x00ef, 0x90);

			handle->dmd_set_reg(handle, 0x00c6, 0x09);
			handle->dmd_set_reg(handle, 0x00bd, 0x80);
			handle->dmd_set_reg(handle, 0x00ae,0x05);
			handle->dmd_set_reg(handle, 0x0064, 0x68);
			handle->dmd_set_reg(handle, 0x0069, 0x0b);
			handle->dmd_set_reg(handle, 0x0075, 0x44);

			handle->dmd_set_reg(handle, 0x007d, 0x28);
			handle->dmd_set_reg(handle, 0x007e, 0x00);
			handle->dmd_set_reg(handle, 0x007d, 0x29);
			handle->dmd_set_reg(handle, 0x007e, 0x00);
			handle->dmd_set_reg(handle, 0x007d, 0x2a);
			handle->dmd_set_reg(handle, 0x007e, 0x1a);
			handle->dmd_set_reg(handle, 0x007d, 0x2b);
			handle->dmd_set_reg(handle, 0x007e, 0x4c);
			handle->dmd_set_reg(handle, 0x007d, 0x2c);
			handle->dmd_set_reg(handle, 0x007e, 0x1a);
			handle->dmd_set_reg(handle, 0x007d, 0x2d);
			handle->dmd_set_reg(handle, 0x007e, 0x00);
			handle->dmd_set_reg(handle, 0x007d, 0x2e);
			handle->dmd_set_reg(handle, 0x007e, 0x00);

			if(handle->tuner_ct_settings.tuner_type == MtFeTN_MxL5007T)//for low IF
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x0b);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0x87);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x03);

				handle->dmd_set_reg(handle, 0x0028,0x02);
			}
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18273)
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x0e);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0x38);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0xe4);

				handle->dmd_set_reg(handle, 0x007d, 0x06);
				handle->dmd_set_reg(handle, 0x0078, 0xa9);
				handle->dmd_set_reg(handle, 0x0028, 0x01);
			}
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_TC2800)
			{
				handle->dmd_set_reg(handle, 0x0028, 0x04);
			}
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL601)
			{
				handle->dmd_set_reg(handle, 0x0028, 0x05);
			}
			else if (handle->tuner_ct_settings.tuner_type==MtFeTN_SI2158)//for low IF
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x09);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0xc3);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x0a);
				handle->dmd_set_reg(handle, 0x0028, 0x06);
			}
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18257)//for low IF
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x09);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0xc3);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x0a);

				handle->dmd_set_reg(handle, 0x007d, 0x06);
				handle->dmd_set_reg(handle, 0x0078, 0xa9);
				handle->dmd_set_reg(handle, 0x0028, 0x01);
			}
#if 0
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_NXPTDA18275)//for low IF
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x0c);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0x07);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x62);

				handle->dmd_set_reg(handle, 0x007d, 0x06);
				handle->dmd_set_reg(handle, 0x0078, 0xa9);
				handle->dmd_set_reg(handle, 0x0028, 0x01);
			}
#endif
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL603)
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x0d);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0xbb);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x1c);
			}
#if 0
			else if (handle->tuner_ct_settings.tuner_type == MtFeTN_MxL608)
			{
				handle->dmd_set_reg(handle, 0x0044, 0x00);
				handle->dmd_set_reg(handle, 0x0045, 0x0d);
				handle->dmd_set_reg(handle, 0x0044, 0x01);
				handle->dmd_set_reg(handle, 0x0045, 0xbb);
				handle->dmd_set_reg(handle, 0x0044, 0x02);
				handle->dmd_set_reg(handle, 0x0045, 0x1c);
			}
#endif
			mt_fe_dmd_set_bandwidth_t(handle);
			mt_fe_dmd_set_output_mode_dd3k(handle);

			handle->dmd_set_reg(handle, 0x0000, 0x00);
			handle->dmd_set_reg(handle, 0x00b7, 0x00);
		}
		else
		{
			handle->dmd_set_reg(handle, 0x0080, 0x01);
			handle->dmd_set_reg(handle, 0x0082, 0x00);
			handle->mt_sleep(1);
			handle->dmd_set_reg(handle, 0x0080, 0x00);
		}
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		if (handle->demod_cur_mode != MtFeType_DVBC)
		{
			handle->dmd_set_reg(handle, 0x0032, 0x01);
			handle->dmd_set_reg(handle, 0x0004, 0x01);
// fix102281 start
			handle->dmd_set_reg(handle, 0x00b0, 0x00);
			handle->dmd_set_reg(handle, 0x00af, 0x02);
			handle->mt_sleep(1);
			handle->dmd_set_reg(handle, 0x00af, 0x00);
			handle->mt_sleep(1);
// fix102281 end
			handle->dmd_set_reg(handle, 0x0004, 0x00);
			handle->dmd_set_reg(handle, 0x0032, 0x00);
		}
		else
		{
			handle->dmd_set_reg(handle, 0x0080, 0x01);
			handle->dmd_set_reg(handle, 0x0082, 0x00);
			handle->mt_sleep(1);
			handle->dmd_set_reg(handle, 0x0080, 0x00);
			handle->mt_sleep(10);
		}
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_close_ts_output_dd3k
**
**	DESCRIPTION:
**		Close ts output
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ts_output_dd3k(MT_FE_DD_Device_Handle  handle)
{
	U8	tmp = 0;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			handle->dmd_get_reg(handle, 0x0083, &tmp);
			tmp = (U8)(tmp | 0x20);
			handle->dmd_set_reg(handle, 0x0083, tmp);
		}
		else
		{
			handle->dmd_get_reg(handle, 0x0007, &tmp);
			tmp = (U8)(tmp & 0xfd);
			handle->dmd_set_reg(handle, 0x0007, tmp);
		}
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{

		handle->dmd_get_reg(handle, 0x0009, &tmp);
		tmp = (U8)(tmp & 0xef);
		handle->dmd_set_reg(handle, 0x0009, tmp);
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_open_ts_output_dd3k
**
**	DESCRIPTION:
**		Open ts output
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ts_output_dd3k(MT_FE_DD_Device_Handle  handle)
{
	U8	tmp=0;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			handle->dmd_get_reg(handle, 0x0083, &tmp);
			tmp = (U8)(tmp & 0xdf);
			handle->dmd_set_reg(handle, 0x0083, tmp);
		}
		else
		{
			handle->dmd_get_reg(handle, 0x0007, &tmp);
			tmp = (U8)(tmp | 0x02);
			handle->dmd_set_reg(handle, 0x0007, tmp);
		}
	}
	else 	if(handle->demod_id == MtFeDmdId_DD630X)
	{

		handle->dmd_get_reg(handle, 0x0009, &tmp);
		tmp = (U8)(tmp | 0x10);
		handle->dmd_set_reg(handle, 0x0009, tmp);
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_xclk_merr_select_dd3k
**
**	DESCRIPTION:
**		select output Output M_ERR signal or auxiliary clock on AUXCLK pin.
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		MT_FE_DD_XCLK_MEER_SELECT select
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_xclk_merr_select_dd3k(MT_FE_DD_Device_Handle  handle,MT_FE_DD_XCLK_MEER_SELECT select )
{
	U8	tmp = 0;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			handle->demod_cur_mode = MtFeType_CTTB;
			handle->dmd_get_reg(handle, 0x0003, &tmp);
			if (select != MtFeMerr)
				tmp = (U8)(tmp & 0x7f);
			else
				tmp = (U8)(tmp | 0x80);
			handle->dmd_set_reg(handle, 0x0003, tmp);

			handle->demod_cur_mode = MtFeType_DVBC;
		}
		else
		{
			handle->dmd_get_reg(handle, 0x001f, &tmp);
			if(select != MtFeMerr)
				tmp = (U8)(tmp & 0xfe);
			else
				tmp = (U8)(tmp | 0x01);
			handle->dmd_set_reg(handle, 0x001f, tmp);
		}
	}
	else
	{

	}

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_dd3k
**
**	DESCRIPTION:
**		DVB-C:select the serial interface or parallel interface or common
**		CTTB:select the serial interface or parallel interface
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		mode	-	MtFeDmdTsOutputMode_Serial
**				-MtFeTsOutMode_Parallel
**				-MtFeDmdTsOutputMode_Common
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_output_mode_dd3k(MT_FE_DD_Device_Handle  handle)
{
	U8	regC2H=0;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT
		U8	tmp=0;

		if(handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->demod_dc_settings.ts_out_mode == MtFeTsOutMode_Common)
			{
				handle->dmd_set_reg(handle, 0x0084, 0x7c);
				handle->dmd_set_reg(handle, 0x00c0, 0x43);
				handle->dmd_set_reg(handle, 0x00e2, 0x06);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc0;
				regC2H |= 0x24;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x60);/*common interface*/
			}
			else if (handle->demod_dc_settings.ts_out_mode == MtFeTsOutMode_Serial)
			{
				handle->dmd_set_reg(handle, 0x0084, 0x7a);
				handle->dmd_set_reg(handle, 0x00c0, 0x47); 		/*serial format*/
				handle->dmd_set_reg(handle, 0x00e2, 0x02);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc7;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x00);
			}
			else
			{
				handle->dmd_set_reg(handle, 0x0084, 0x7c);
				handle->dmd_set_reg(handle, 0x00c0, 0x43);		/*parallel format*/
				handle->dmd_set_reg(handle, 0x00e2, 0x06);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc0;
				regC2H |= 0x06;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x00);
			}
		}
		else
		{
			handle->dmd_get_reg(handle, 0x00b1, &tmp);

			if (handle->demod_ct_settings.ts_out_mode == MtFeTsOutMode_Serial)
				tmp = (U8)((tmp | 0x04)& 0x7f);
			else if(handle->demod_ct_settings.ts_out_mode == MtFeTsOutMode_Parallel)
				tmp &= 0x7b;
			else
				tmp = (U8)((tmp | 0x80)& 0xfb);
			handle->dmd_set_reg(handle, 0x00b1, tmp);
		}
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		if(handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->demod_dc_settings.ts_out_mode == MtFeTsOutMode_Common)
			{
				handle->dmd_set_reg(handle, 0x00c0, 0x43);
				handle->dmd_set_reg(handle, 0x00e2, 0x06);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc7;
				regC2H |= 0x10;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x60);		//common interface
			}
			else if (handle->demod_dc_settings.ts_out_mode == MtFeTsOutMode_Serial)
			{
				handle->dmd_set_reg(handle, 0x00c0, 0x47);		//serial format
				handle->dmd_set_reg(handle, 0x00e2, 0x02);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc7;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x00);
			}
			else//if (ts_mode == MtFeTsOutMode_Parallel)
			{
				handle->dmd_set_reg(handle, 0x00c0, 0x43);		//parallel format
				handle->dmd_set_reg(handle, 0x00e2, 0x06);
				handle->dmd_get_reg(handle, 0x00c2, &regC2H);
				regC2H &= 0xc7;
				handle->dmd_set_reg(handle, 0x00c2, regC2H);
				handle->dmd_set_reg(handle, 0x00c1, 0x00);
			}
		}
		else
		{
			if (handle->demod_ct_settings.ts_out_mode == MtFeTsOutMode_Serial)
			{
				handle->dmd_set_reg(handle, 0x01b4, 0x71);
//				printf("set val low!\n");
				//handle->dmd_set_reg(handle, 0x01b5, 0x22);
			}
			else if(handle->demod_ct_settings.ts_out_mode == MtFeTsOutMode_Parallel)
			{
				handle->dmd_set_reg(handle, 0x01b4, 0x61);
				handle->dmd_set_reg(handle, 0x01b5, 0x22);
			}
			else
			{
				handle->dmd_set_reg(handle, 0x01b4, 0xe1);
				handle->dmd_set_reg(handle, 0x01b5, 0x22);
			}
		}
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_chan_info_code_parse_dd3k
**
**	DESCRIPTION:
**		set CTTB module channel information
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		U32 chan_info_code
**
**	OUT:
**		 MT_FE_DD_CHAN_INFO* p_info
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_chan_info_code_parse_dd3k(MT_FE_DD_Device_Handle  handle,MT_FE_DD_CHAN_INFO* p_info)
{
	U8	si_code = 0, sc_mode = 0;
	U32	info_code = 0;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT

		SET_VERIFICATION_CODE(info_code, VERIFICATION_CODE);

		handle->dmd_get_reg(handle, 0x00c2, &sc_mode);
		handle->dmd_get_reg(handle, 0x00c4, &si_code);


		if ((sc_mode & 0x20) == 0)
			SET_CARRIER_MODE(info_code, 0);	/*	Multicarrier	*/
		else
			SET_CARRIER_MODE(info_code, 1);	/*	Single carrier	*/


		if ((si_code & 0x80) == 0)
			SET_SPEC_INVC(info_code, 0);
		else
			SET_SPEC_INVC(info_code, 1);


		if ((si_code & 0x60) == 0x00)
			SET_FRAME_MODE(info_code, 0);
		else if ((si_code & 0x60) == 0x20)
			SET_FRAME_MODE(info_code, 1);
		else if ((si_code & 0x60) == 0x40)
			SET_FRAME_MODE(info_code, 2);


		if ((si_code & 0x10) == 0)
			SET_INTERLEACING_DEPTH(info_code, 0);
		else
			SET_INTERLEACING_DEPTH(info_code, 1);


		SET_QAM_CODE_RATE(info_code, si_code & 0x0f);

		if (GET_VERIFICATION_CODE(info_code) != VERIFICATION_CODE)
		{
			p_info->carrier_mode = MtFeCrrierMode_Undef;
			p_info->spectrum_mode = MtFeSpectrum_Undef;
			p_info->frame_mode = MtFeFrameMode_Undef;
			p_info->interleaving_depth = MtFeInterleavingDepth_Undef;
			p_info->constellation_pattern = MtFeModMode_Undef;
			p_info->fec_code_rate = MtFeFecCodeRate_Undef;
			p_info->si_map_mode = MtFeSiMapMode_Undef;

			return MtFeErr_Param;
		}


		if (GET_CARRIER_MODE(info_code) == 0)
			p_info->carrier_mode = MtFeCrrierMode_Multicarrier;
		else
			p_info->carrier_mode = MtFeCrrierMode_SingleCarrier;



		if (GET_SPEC_INVC(info_code) == 0)
			p_info->spectrum_mode = MtFeSpectrum_NoInversion;
		else
			p_info->spectrum_mode = MtFeSpectrum_Inversion;



		if (GET_FRAME_MODE(info_code) == 0x00)
			p_info->frame_mode = MtFeFrameMode_Pn420;
		else if (GET_FRAME_MODE(info_code) == 0x01)
			p_info->frame_mode = MtFeFrameMode_Pn945;
		else if (GET_FRAME_MODE(info_code) == 0x02)
			p_info->frame_mode = MtFeFrameMode_Pn595;
		else
			p_info->frame_mode = MtFeFrameMode_Undef;



		if (GET_INTERLEACING_DEPTH(info_code) == 0)
			p_info->interleaving_depth = MtFeInterleavingDepth_240;
		else
			p_info->interleaving_depth = MtFeInterleavingDepth_720;



		switch (GET_QAM_CODE_RATE(info_code))
		{
			case 0x01:	/*	4QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x02:	/*	4QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x03:	/*	4QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x07:	/*	4QAM-NR, 0.8	*/
				p_info->constellation_pattern = MtFeModMode_4QamNr;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x09:	/*	16QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x0a:	/*	16QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x0b:	/*	16QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x0c:	/*	32QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_32Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x0d:	/*	64QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x0e:	/*	64QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x0f:	/*	64QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			default:
				p_info->constellation_pattern = MtFeModMode_Undef;
				p_info->fec_code_rate = MtFeFecCodeRate_Undef;
				break;
		}
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		U8 sc_pn = 0;
		handle->dmd_get_reg(handle, 0x00bf, &sc_mode);
		handle->dmd_get_reg(handle, 0x00b0, &si_code);
		handle->dmd_get_reg(handle, 0x00bc, &sc_pn);


		if ((sc_mode&0x01) == 0x00)
		{
			p_info->carrier_mode = MtFeCrrierMode_Multicarrier;
			if ((sc_mode&0x02) != 0x02)
				p_info->spectrum_mode = MtFeSpectrum_NoInversion;
			else
				p_info->spectrum_mode = MtFeSpectrum_Inversion;
		}
		else
		{
			p_info->carrier_mode = MtFeCrrierMode_SingleCarrier;
			if ((si_code&0x02) != 0x02)
				p_info->spectrum_mode = MtFeSpectrum_NoInversion;
			else
				p_info->spectrum_mode = MtFeSpectrum_Inversion;
		}

		if ((sc_mode&0x04) == 0x00)
			p_info->interleaving_depth = MtFeInterleavingDepth_240;
		else
			p_info->interleaving_depth = MtFeInterleavingDepth_720;

		if ((sc_pn&0x03) == 0x00)
			p_info->frame_mode = MtFeFrameMode_Pn420;
		else if ((sc_pn&0x03) == 0x01)
			p_info->frame_mode = MtFeFrameMode_Pn595;
		else if ((sc_pn&0x03) == 0x02)
			p_info->frame_mode = MtFeFrameMode_Pn945;
		else
			p_info->frame_mode = MtFeFrameMode_Undef;


		switch (sc_mode&0xf8)
		{
			case 0x18:	/*	4QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x58:	/*	4QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x98:	/*	4QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_4Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0xa0:	/*	4QAM-NR, 0.8	*/
				p_info->constellation_pattern = MtFeModMode_4QamNr;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x10:	/*	16QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x50:	/*	16QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x90:	/*	16QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_16Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x88:	/*	32QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_32Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			case 0x00:	/*	64QAM, 0.4		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p4;
				break;
			case 0x40:	/*	64QAM, 0.6		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p6;
				break;
			case 0x80:	/*	64QAM, 0.8		*/
				p_info->constellation_pattern = MtFeModMode_64Qam;
				p_info->fec_code_rate = MtFeFecCodeRate_0p8;
				break;
			default:
				p_info->constellation_pattern = MtFeModMode_Undef;
				p_info->fec_code_rate = MtFeFecCodeRate_Undef;
				break;
		}
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_dd3k
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		*p_state	-	MtFeDmdLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_dd3k(MT_FE_DD_Device_Handle  handle,MT_FE_LOCK_STATE *p_state)
{
	U8 value=0;

	if (p_state == NULL)
		return MtFeErr_Param;

// fix102281 start
	*p_state = MtFeLockState_Undef;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT
		if(handle->demod_cur_mode != MtFeType_DVBC)
		{
			handle->dmd_get_reg(handle, 0x00c2, &value);
			if ((value & 0x40) == 0)
			{/*	Unlock	*/
				*p_state = MtFeLockState_Waiting;
				mt_fe_print(("MT:	FEC Waiting!!  [0xc2 = 0x%x]\n", value));
				return MtFeErr_Ok;
			}
			else
				*p_state = MtFeLockState_Locked;
		}
		else/*DVBC*/
		{
			handle->dmd_get_reg(handle, 0x0085, &value);
			if ((value&0x08) == 0x08)
				*p_state = MtFeLockState_Locked;
			else
				*p_state = MtFeLockState_Waiting;
		}
	#endif
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		if(handle->demod_cur_mode != MtFeType_DVBC)
		{
			handle->dmd_get_reg(handle, 0x0240, &value);
			if(value & 0x20)
			{
				if(value & 0x40)
				{
					*p_state = MtFeLockState_Unlocked;
				}
				else
				{
					handle->dmd_get_reg(handle, 0x01cd, &value);
					if ((value & 0x01) == 0x01)
					{
						*p_state = MtFeLockState_Locked;
					}
					else
					{
						*p_state = MtFeLockState_Waiting;
					}
				}
			}
			else
			{
				return MtFeErr_Ok;
			}
		}
		else/*DVBC*/
		{
			handle->dmd_get_reg(handle, 0x0085, &value);
			if ((value&0x08) == 0x08)
				*p_state = MtFeLockState_Locked;
			else
				*p_state = MtFeLockState_Waiting;
		}
	}
// fix102281 end

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_signal_state_dd3k_t
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		*p_state	-	MtFeSignalState_NoSignal
**					-MtFeSignalState_Signal
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_signal_state_dd3k_t(MT_FE_DD_Device_Handle  handle,MT_FE_DD_SIGNAL_STATE *p_state)
{
	U8	value = 0;

	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;		/* MtFeErr_NoMatch*/
	}
	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT

		handle->dmd_get_reg(handle, 0x00be, &value);
		if(value == 0)
		{
			handle->dmd_get_reg(handle, 0x00b8, &value);
			if(value!=0x20)
				*p_state = MtFeSignalState_NoSignal;
			else
				*p_state = MtFeSignalState_Signal;
		}
		else
			*p_state = MtFeSignalState_Signal;
	#endif
	}
	else
	{
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_per_dd3k_t
**
**	DESCRIPTION:
**		get the packet error ratio of CTTB
**
**	IN:
**		none.
**
**	OUT:
**		*p_err_cnt	-	error packet counter
**		*p_pkt_cnt	-	totale packet counter
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_per_dd3k_t(MT_FE_DD_Device_Handle  handle,U16 *p_err_cnt, U16 *p_pkt_cnt)
{
	U8		rega7[3] = {0,0,0};
	U8		rega8[3] = {0,0,0};
	U8		rega1[3] = {0,0,0};
	U8		rega2[3] = {0,0,0};
	U16		new_err_cnt[3] = {0,0,0};
	U16		err_cnt = 0;
	U16		new_pkt_cnt[3] = {0,0,0};
	U16		pkt_cnt = 0;

	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;
	}

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
	#if MT_FE_DMD_DD3X0X_SUPPORT
		handle->dmd_set_reg(handle, 0x00a0, 0x10);

		handle->dmd_get_reg(handle, 0x00a7, &rega7[0]);
		handle->dmd_get_reg(handle, 0x00a7, &rega7[1]);
		handle->dmd_get_reg(handle, 0x00a7, &rega7[2]);
		handle->dmd_get_reg(handle, 0x00a8, &rega8[0]);
		handle->dmd_get_reg(handle, 0x00a8, &rega8[1]);
		handle->dmd_get_reg(handle, 0x00a8, &rega8[2]);
		handle->dmd_get_reg(handle, 0x00a1, &rega1[0]);
		handle->dmd_get_reg(handle, 0x00a1, &rega1[1]);
		handle->dmd_get_reg(handle, 0x00a1, &rega1[2]);
		handle->dmd_get_reg(handle, 0x00a2, &rega2[0]);
		handle->dmd_get_reg(handle, 0x00a2, &rega2[1]);
		handle->dmd_get_reg(handle, 0x00a2, &rega2[2]);

		handle->dmd_set_reg(handle, 0x00a0, 0x90);

		new_err_cnt[0] = (U16)((((U16)rega2[0])<<8)+rega1[0]);
		new_err_cnt[1] = (U16)((((U16)rega2[1])<<8)+rega1[1]);
		new_err_cnt[2] = (U16)((((U16)rega2[2])<<8)+rega1[2]);
		new_pkt_cnt[0] = (U16)((((U16)rega8[0])<<8)+rega7[0]);
		new_pkt_cnt[1] = (U16)((((U16)rega8[1])<<8)+rega7[1]);
		new_pkt_cnt[2] = (U16)((((U16)rega8[2])<<8)+rega7[2]);

		if(new_err_cnt[0]>new_err_cnt[1])
		{
			err_cnt = new_err_cnt[0];
			new_err_cnt[0] = new_err_cnt[1];
			new_err_cnt[1] = err_cnt;
		}
		if(new_err_cnt[1]>new_err_cnt[2])
		{
			err_cnt = new_err_cnt[1];
			new_err_cnt[1] = new_err_cnt[2];
			new_err_cnt[2] = err_cnt;
			if(new_err_cnt[0]>new_err_cnt[1])
			{
				err_cnt = new_err_cnt[0];
				new_err_cnt[0] = new_err_cnt[1];
				new_err_cnt[1] = err_cnt;
			}
		}

		err_cnt = new_err_cnt[1];

		if(new_pkt_cnt[0]>new_pkt_cnt[1])
		{
			pkt_cnt = new_pkt_cnt[0];
			new_pkt_cnt[0] = new_pkt_cnt[1];
			new_pkt_cnt[1] = pkt_cnt;
		}
		if(new_pkt_cnt[1]>new_pkt_cnt[2])
		{
			pkt_cnt = new_pkt_cnt[1];
			new_pkt_cnt[1] = new_pkt_cnt[2];
			new_pkt_cnt[2] = pkt_cnt;
			if(new_pkt_cnt[0]>new_pkt_cnt[1])
			{
				pkt_cnt = new_pkt_cnt[0];
				new_pkt_cnt[0] = new_pkt_cnt[1];
				new_pkt_cnt[1] = pkt_cnt;
			}
		}
		pkt_cnt = new_pkt_cnt[1];

		if(err_cnt == 0)
			err_cnt = new_err_cnt[2];
		if(pkt_cnt == 0)
			pkt_cnt = new_pkt_cnt[2];
		if(err_cnt > pkt_cnt)
			err_cnt = pkt_cnt;

		*p_err_cnt = err_cnt;
		*p_pkt_cnt = pkt_cnt;
	#endif
	}
	else
	{

	}

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_get_noise_power_dd3k_t
**
**	DESCRIPTION:
**		get the noise power of CTTB
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**
**	OUT:
**		*p_noise	-	noise power
**
**
**	RETURN:
*/
// fix102281 start
MT_FE_RET mt_fe_dmd_get_noise_power_dd3k_t(MT_FE_DD_Device_Handle handle, S32 *p_noise, U32 *p_acc)
{
	#define 	LOOP_COUNT		10

	U8		value = 0;
	U32		acc_value = 0;
	U32		acc_left = 0;
	U32		constellation_power = 0;
	U32		cp_value=0;
	S32		noise_power = 0;
	S32		i,m;
//	U32		ch_info_code = 0;
	U32		total_acc = 0;
	MT_FE_DD_CHAN_INFO	ch_info;

	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;
	}
	m = 0;
	mt_fe_dmd_chan_info_code_parse_dd3k(handle, &ch_info);

	if (handle->demod_id == MtFeDmdId_DD3X0X)
	{
		for (i = 0; i < LOOP_COUNT; i++)
		{
			handle->dmd_get_reg(handle, 0x00e1, &value);
			value &= 0xdf;
			handle->dmd_set_reg(handle, 0x00e1, value);
			handle->dmd_get_reg(handle, 0x00f7, &value);
			acc_value = (value>>5)&(0x1f);
			acc_left = (value&0x07);

			handle->dmd_get_reg(handle, 0x00e1, &value);
			value |= 0x20;
			handle->dmd_set_reg(handle, 0x00e1, value);
			handle->dmd_get_reg(handle, 0x00f7, &value);
			acc_value = (((U32)value<<5)+acc_value)<<(acc_left);

			if(value != 0xff)
			{
				total_acc += acc_value;
			}
			else
			{
				m++;
			}

			handle->mt_sleep(10);
		}
		if(LOOP_COUNT-m>0)
			*p_acc = total_acc / (LOOP_COUNT-m);
		else
			*p_acc = 0;
		switch(ch_info.constellation_pattern)
		{
			case MtFeModMode_4Qam:
			case MtFeModMode_4QamNr:
				constellation_power = 405;
				break;

			case MtFeModMode_16Qam:
				constellation_power = 400;
				break;

			case MtFeModMode_32Qam:
				constellation_power = 450;
				break;

			case MtFeModMode_64Qam:
				constellation_power = 420;
				break;

			default:
				/*constellation_power = 400;*/
				break;
		}

		constellation_power *= 4096;
		if(*p_acc == 0)
		{
			*p_noise = 100;
			return MtFeErr_Ok;
		}
		cp_value = constellation_power / (*p_acc);
		noise_power = (log10table(cp_value) - 1) / 10;

		*p_noise = (S32)noise_power;
	}

	return MtFeErr_Ok;
}
// fix102281 end

/*	FUNCTIN:
**		_mt_fe_dmd_acc_2_snr_dd3k_t
**
**	DESCRIPTION:
**		get CTTB snr
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		MT_FE_DD_CHAN_INFO info
**		S32 noise
**	OUT:
**
**	RETURN:
**
*/
// fix102281 start
static S8 _mt_fe_dmd_acc_2_snr_dd3k_t(MT_FE_DD_Device_Handle  handle,  S32 noise)
{
	typedef struct _MT_FE_NOISE_SNR
	{
		S32		noise;
		S8		snr;
	} MT_FE_NOISE_SNR;

	S8			snr = 0;
	U32			i = 0;
	MT_FE_NOISE_SNR	const	*p_array;
	U8			array_len = 0;

	const MT_FE_NOISE_SNR noise_snr[] =
	{
		{-2,3},	{-1,4},	{0,  5},	{1,  7},	{2, 8},
		{3,  9},	{4,  10},	{5,  11},	{6,  13},	{7,  14},
		{8,  16},	{9, 17},	{10, 18},	{11, 19},	{ 12, 21},
		{ 13, 23},	{ 14, 25},	{ 15, 27},	{ 16, 29},	{ 17, 30}
	};

	if(handle->demod_cur_mode!=MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;		/* MtFeErr_NoMatch*/
	}
	if (handle->demod_id == MtFeDmdId_DD3X0X)
	{
		array_len = sizeof(noise_snr)/sizeof(MT_FE_NOISE_SNR);
		p_array = noise_snr;

		if (noise <= p_array[0].noise)
		{
			snr = p_array[0].snr;
		}
		else if (noise >= p_array[array_len - 1].noise)
		{
			snr = p_array[array_len - 1].snr;
		}
		else
		{
			for (i = array_len; i > 0; i--)
			{
				if (noise > p_array[i-1].noise)
					break;
			}

			if (noise > (S32)(p_array[i-1].noise + p_array[i].noise) / 2)
				snr = p_array[i-1].snr;
			else
				snr = p_array[i].snr;
		}
		i = (snr * 33) / 10;

		if (i>100)
			snr = 100;
		else
			snr = (S8)i;
	}
	else if (handle->demod_id == MtFeDmdId_DD630X)
	{
		MT_FE_DD_CHAN_INFO	ch_info;
		U8	value = 0;
		U32	signal_power = 0,noise_power = 0;

		mt_fe_dmd_chan_info_code_parse_dd3k(handle, &ch_info);

		handle->dmd_get_reg(handle, 0x0000, &value);

		handle->dmd_get_reg(handle, 0x019b, &value);
		signal_power = (U32)value;
		handle->dmd_get_reg(handle, 0x019c, &value);
		signal_power = signal_power + (U32)(value<<8);

		switch(ch_info.carrier_mode)
		{
			case MtFeCrrierMode_Multicarrier:
				signal_power = signal_power * 2;
				break;

			case MtFeCrrierMode_SingleCarrier:
				signal_power =  signal_power;
				break;
			default:
				/*constellation_power = 400;*/
				break;
		}

		handle->dmd_get_reg(handle, 0x019d, &value);
		noise_power = (U32)value;
		handle->dmd_get_reg(handle, 0x019e, &value);
		noise_power = noise_power + (U32)(value<<8);

		if(noise_power!=0)
			noise_power = (33 * (log10table((signal_power-noise_power) / noise_power))) / 100;

		if(noise_power > 100)
			snr = 100;
		else
			snr = (S8)noise_power;
	}
	return snr;
}
// fix102281 end


/*	FUNCTIN:
**		_mt_fe_dmd_get_snr_dd3k_c
**
**	DESCRIPTION:
**		get DVB-C snr
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		qam :QAM TYPE
**	OUT:
**		U8 *dvbc_snr
**	RETURN:
**
*/
MT_FE_RET _mt_fe_dmd_get_snr_dd3k_c(MT_FE_DD_Device_Handle  handle,U16 qam,U8 *dvbc_snr)
{
	const U32 mes_log[] =
	{
		0,		3010,	4771,	6021, 	6990,	7781,	8451,	9031,	9542,	10000,
		10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,
		13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,
		14914,	15052,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,
		16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,
		17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,
		17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,
		18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031
	};

	U32	snr;
	U8	i,reg_91 = 0;
	U8	reg_08 = 0;
	U8	reg_07 = 0;
	U32	mse;

	if(handle->demod_cur_mode != MtFeType_DVBC)
	{
		return MtFeErr_NoMatch;		/*MtFeErr_NoMatch;*/
	}
	handle->dmd_get_reg(handle, 0x0091, &reg_91);

	if ((reg_91&0x23) != 0x03)
		return MtFeErr_UnLock;

	mse = 0;
	for (i=0; i<30; i++)
	{
		handle->dmd_get_reg(handle, 0x0008, &reg_08);
		handle->dmd_get_reg(handle, 0x0007, &reg_07);
		mse += (reg_08 << 8) + reg_07;
	}

	mse /= 30;
	if (mse > 80)
		mse = 80;

	switch (qam)
	{
		case 16:	snr = 34080;	break;	/*	16QAM				*/
		case 32:	snr = 37600;	break;	/*	32QAM				*/
		case 64:	snr = 40310;	break;	/*	64QAM				*/
		case 128:	snr = 43720;	break;	/*	128QAM				*/
		case 256:	snr = 46390;	break;	/*	256QAM				*/
		default:	snr = 40310;	break;
	}

	snr -= mes_log[mse-1];					/*	C - 10*log10(MSE)	*/
	snr /= 1000;
	if (snr > 0xff)
		snr = 0xff;

	*dvbc_snr=(U8)snr;

	return MtFeErr_Ok;
}


// fix102281 start
/*	FUNCTIN:
**		_mt_fe_dmd_get_ber_dd3k_c
**
**	DESCRIPTION:
**		get DVB-C BER (bit error rate)
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		qam :QAM TYPE
**	OUT:
**		U32 *dvbc_ber
**	RETURN:
**
*/
MT_FE_RET _mt_fe_dmd_get_ber_dd3k_c(MT_FE_DD_Device_Handle  handle, U32 *dvbc_ber, U32 *dvbc_total)
{
	static U32 ber = 0;
	U16	tmp = 0;
	U8	reg_a0 = 0;
	U8	reg_a1 = 0;
	U8	reg_a2 = 0;
	MT_FE_LOCK_STATE lock_state;

	if(handle->demod_cur_mode != MtFeType_DVBC)
	{
		return MtFeErr_NoMatch;		/*MtFeErr_NoMatch;*/
	}

	mt_fe_dmd_get_lock_state_dd3k(handle,&lock_state);
	if (lock_state != MtFeLockState_Locked)
	{
		ber = 0;
		*dvbc_ber = ber;
		return MtFeErr_UnLock;
	}

	handle->dmd_get_reg(handle, 0x00a0, &reg_a0);
	if ((reg_a0 & 0x80) != 0x80)
	{
		handle->dmd_get_reg(handle, 0x00a1, &reg_a1);
		handle->dmd_get_reg(handle, 0x00a2, &reg_a2);
		tmp = (U16)(reg_a2 << 8);
		tmp = tmp + (U16)(reg_a1);
		ber = tmp;

		handle->dmd_set_reg(handle, 0x00a0, 0x03);
		handle->dmd_set_reg(handle, 0x00a0, 0x83);
	}

	*dvbc_ber = ber;
	*dvbc_total = 2097152;					/*	(2^(2*3+12))*8		*/

	return MtFeErr_Ok;
}
// fix102281 end

MT_FE_RET mt_fe_dmd_get_quality_dd3k_t(MT_FE_DD_Device_Handle  handle, S8 *p_snr)
{
	MT_FE_LOCK_STATE dd3k_state = MtFeLockState_Unlocked;

	U32 acc;
	S32 noise;
	*p_snr = 0;

	if(handle->demod_cur_mode != MtFeType_CTTB)
	{
		return MtFeErr_NoMatch;
	}
	mt_fe_dmd_get_lock_state_dd3k(handle, &dd3k_state);
	if(dd3k_state==MtFeLockState_Locked)
	{
		mt_fe_dmd_get_noise_power_dd3k_t(handle, &noise, &acc);
		*p_snr = _mt_fe_dmd_acc_2_snr_dd3k_t(handle, noise);/*p_snr is %*/		// fix102281
	}
	else
		*p_snr = 0;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_set_tx_mode_dd3k_c
**
**	DESCRIPTION:
**		 Set spectrum inversion and J83 of DVB-C
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		Inverted: 1, inverted; 0, not inverted
**		 J83: 0, J83A; 1, J83C
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_tx_mode_dd3k_c(MT_FE_DD_Device_Handle  handle, U8 inverted, U8 j83)
{
	U8 value = 0;

	if(handle->demod_cur_mode != MtFeType_DVBC)
		return MtFeErr_NoMatch;

	if (inverted)	value |= 0x08;		/*	spectrum inverted			*/
	if (j83)		value |= 0x01;		/*	J83C						*/

	handle->dmd_set_reg(handle, 0x0083, value);

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_set_sym_dd3k_c
**
**	DESCRIPTION:
**		Set symbol rate of DVB-C
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		sym:symbol rate ,unit: KBaud
**		xtal:xtal freq, xtal:	unit, KHz
**	OUT:
**
**	RETURN:
*/

MT_FE_RET mt_fe_dmd_set_sym_dd3k_c(MT_FE_DD_Device_Handle  handle, U32 sym, U32 xtal)
{
	U8	value = 0;
	U8	reg6FH, reg12H;
	U32	fValue;
	U32	dwValue;


	if(handle->demod_cur_mode != MtFeType_DVBC)
		return MtFeErr_NoMatch;

	fValue = ((((sym + 10) << 16) / xtal)  << 16) + ((((sym + 10) << 16) % xtal) << 16) / xtal;
	dwValue = (U32)fValue;

	handle->dmd_set_reg(handle, 0x0058, (U8)((dwValue >> 24) & 0xff));
	handle->dmd_set_reg(handle, 0x0057, (U8)((dwValue >> 16) & 0xff));
	handle->dmd_set_reg(handle, 0x0056, (U8)((dwValue >>  8) & 0xff));
	handle->dmd_set_reg(handle, 0x0055, (U8)((dwValue >>  0) & 0xff));


	fValue = 20480 * xtal / (10*sym);
	dwValue = (U32)fValue;
	handle->dmd_set_reg(handle, 0x005d, (U8)((dwValue >> 8) & 0xff));
	handle->dmd_set_reg(handle, 0x005c, (U8)((dwValue >> 0) & 0xff));


	handle->dmd_get_reg(handle, 0x005a, &value);
	if (((dwValue >> 16) & 0x0001) == 0)
		value &= 0x7f;
	else
		value |= 0x80;
	handle->dmd_set_reg(handle, 0x005a, value);


	handle->dmd_get_reg(handle, 0x0089, &value);
	if (sym <= 1800)
		value |= 0x01;
	else
		value &= 0xfe;
	handle->dmd_set_reg(handle, 0x0089, value);


	if (sym >= 6700)
	{
		reg6FH = 0x0d;
		reg12H = 0x30;
	}
	else if (sym >= 4000)
	{
		fValue = 22 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x30;
	}
	else if (sym >= 2000)
	{
		fValue = 14 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x20;
	}
	else
	{
		fValue = 7 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x10;
	}
	handle->dmd_set_reg(handle, 0x006f, reg6FH);
	handle->dmd_set_reg(handle, 0x0012, reg12H);

	if (sym < 3000)
	{
		handle->dmd_set_reg(handle, 0x006c, 0x16);
		handle->dmd_set_reg(handle, 0x006d, 0x10);
		handle->dmd_set_reg(handle, 0x006e, 0x18);
	}
	else
	{
		handle->dmd_set_reg(handle, 0x006c, 0x14);
		handle->dmd_set_reg(handle, 0x006d, 0x0e);
		handle->dmd_set_reg(handle, 0x006e, 0x36);
	}

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_set_qam_dd3k_c
**
**	DESCRIPTION:
**		Set QAM mode of DVB-C
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		Qam: QAM mode, 16, 32, 64, 128, 256
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_qam_dd3k_c(MT_FE_DD_Device_Handle  handle,U16 qam)
{
	U8 reg00H = 0, reg4AH = 0, regC2H = 0, reg44H = 0, reg4CH = 0, reg4DH = 0,reg74H = 0, value = 0;
	U8 reg8BH = 0, reg8EH = 0;

	if(handle->demod_cur_mode!=MtFeType_DVBC)
		return MtFeErr_NoMatch;

	switch(qam)
	{
		case 16:	/* 16 QAM*/
			reg00H = 0x08;
			reg4AH = 0x0f;
			handle->dmd_get_reg(handle,0x00c2,&regC2H);
			regC2H&= 0xf8;
			regC2H|= 0x02;
			reg44H = 0xaa;
			reg4CH = 0x0c;
			reg4DH = 0xf7;
			reg74H = 0x0e;

			reg8BH = 0x5a;
			reg8EH = 0xbd;

			handle->dmd_set_reg(handle,0x006e, 0x18);
			break;

		case 32:	/* 32 QAM*/
			reg00H = 0x18;
			reg4AH = 0xfb;
			handle->dmd_get_reg(handle,0x00c2,&regC2H);
			regC2H&= 0xf8;
			regC2H|= 0x02;
			reg44H = 0xaa;
			reg4CH = 0x0c;
			reg4DH = 0xf7;
			reg74H = 0x0e;

			reg8BH = 0x5a;
			reg8EH = 0xbd;
			handle->dmd_set_reg(handle, 0x006e, 0x18);
			break;

		case 128:	/*128 QAM*/
			reg00H = 0x28;
			reg4AH = 0xff;
			handle->dmd_get_reg(handle, 0x00c2, &regC2H);
			regC2H&= 0xf8;
			regC2H|= 0x02;
			reg44H = 0xa9;
			reg4CH = 0x08;
			reg4DH = 0xf5;
			reg74H = 0x0e;
			reg8BH = 0x5b;
			reg8EH = 0x9d;
			break;

		case 256:	/* 256 QAM*/
			reg00H = 0x38;
			reg4AH = 0xcd;
			handle->dmd_get_reg(handle, 0x00c2, &regC2H);
			regC2H&= 0xf8;
			regC2H|= 0x02;
			reg44H = 0xa9;
			reg4CH = 0x08;
			reg4DH = 0xf5;
			reg74H = 0x0e;

			reg8BH = 0x5b;
			reg8EH = 0x9d;
			break;

		case 64:	/*64 QAM*/
		default:
			reg00H = 0x48;
			reg4AH = 0xcd;
			handle->dmd_get_reg(handle,0x00c2,&regC2H);
			regC2H&= 0xf8;
			regC2H|= 0x02;
			reg44H = 0xaa;
			reg4CH = 0x0c;
			reg4DH = 0xf7;
			reg74H = 0x0e;

			reg8BH = 0x5a;
			reg8EH = 0xbd;
			break;
	}

	handle->dmd_set_reg(handle, 0x0000,reg00H);

	handle->dmd_get_reg(handle, 0x0088,&value);
	value |= 0x08;
	handle->dmd_set_reg(handle, 0x0088, value);
	handle->dmd_set_reg(handle, 0x004b, 0xff);
	handle->dmd_set_reg(handle, 0x004a, reg4AH);
	value &= 0xf7;
	handle->dmd_set_reg(handle, 0x0088, value);

	handle->dmd_set_reg(handle, 0x00c2, regC2H);
	handle->dmd_set_reg(handle, 0x0044, reg44H);
	handle->dmd_set_reg(handle, 0x004c, reg4CH);
	handle->dmd_set_reg(handle, 0x004d, reg4DH);
	handle->dmd_set_reg(handle, 0x0074, reg74H);

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		handle->dmd_set_reg(handle, 0x008b, reg8BH);
		handle->dmd_set_reg(handle, 0x008e, reg8EH);
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_set_demod_appendix_dd3k_c(MT_FE_DD_Device_Handle handle, U16 qam)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 tmp = 0;

	if(handle->demod_cur_mode != MtFeType_DVBC)
		return MtFeErr_NoMatch;

	if(handle->demod_id == MtFeDmdId_DD630X)	// Jazz mode
	{
		handle->dmd_get_reg(handle, 0x0000, &tmp);
		tmp |= 0x80;
		handle->dmd_set_reg(handle, 0x0000, tmp);

		handle->dmd_set_reg(handle, 0x0001, 0x42);
		handle->dmd_set_reg(handle, 0x0001, 0x62);

		if((qam == 256) || (qam == 128))
		{
			handle->dmd_set_reg(handle, 0x0002, 0x10);
		}
		else
		{
			handle->dmd_set_reg(handle, 0x0002, 0x60);
		}

		handle->dmd_get_reg(handle, 0x0003, &tmp);
		tmp |= 0x30;
		handle->dmd_set_reg(handle, 0x0003, tmp);

		handle->dmd_set_reg(handle, 0x0004, 0x88);

		handle->dmd_set_reg(handle, 0x0024, 0xff);
		handle->dmd_set_reg(handle, 0x0025, 0x00);
		if(handle->tuner_dc_settings.tuner_type == MtFeTN_TC3800)
		{
			handle->dmd_set_reg(handle, 0x0026, 0xf9);
		}
		else
		{
			handle->dmd_set_reg(handle, 0x0026, 0xfa);
		}
		handle->dmd_set_reg(handle, 0x0027, 0x80);

		handle->dmd_get_reg(handle, 0x005f, &tmp);
		tmp &= 0x3f;
		handle->dmd_set_reg(handle, 0x005f, tmp);


		if(qam == 16)
			handle->dmd_set_reg(handle, 0x0062, 0x18);
		else
			handle->dmd_set_reg(handle, 0x0062, 0x28);

		handle->dmd_get_reg(handle, 0x0000, &tmp);
		if( ((tmp & 0x70) == 0x20) || ((tmp & 0x70) == 0x10) || ((tmp & 0x70) == 0x00) )		// 128 QAM or 32QAM or 16QAM
		{
			handle->dmd_set_reg(handle, 0x006c, 0x52);
		}
		else
		{
			handle->dmd_set_reg(handle, 0x006c, 0x62);
		}

		handle->dmd_set_reg(handle, 0x006d, 0xa3);
		handle->dmd_set_reg(handle, 0x006f, 0x00);
		handle->dmd_set_reg(handle, 0x0075, 0x18);
		handle->dmd_set_reg(handle, 0x007f, 0x71);

		handle->dmd_get_reg(handle, 0x0093, &tmp);
		tmp &= 0x7f;
		handle->dmd_set_reg(handle, 0x0093, tmp);

		handle->dmd_set_reg(handle, 0x00e5, 0xa6);
		handle->dmd_set_reg(handle, 0x00e8, 0x3b);
		handle->dmd_set_reg(handle, 0x00f0, 0x83);
		handle->dmd_set_reg(handle, 0x00f4, 0x0f);
		handle->dmd_get_reg(handle, 0x00f9, &tmp);
		tmp &= 0xf8;
		tmp |= 0x02;
		handle->dmd_set_reg(handle, 0x00f9, tmp);

		handle->dmd_set_reg(handle, 0x00fb, 0x44);
		handle->dmd_set_reg(handle, 0x00fc, 0xc1);
		handle->dmd_set_reg(handle, 0x00fd, 0x13);
		handle->dmd_set_reg(handle, 0x00ed, 0x50);
#if 0
		if (handle->tuner_dc_settings.tuner_type == MtFeTN_MxL608)
		{
			handle->dmd_set_reg(handle, 0x002a, 0x2a);
		}
		else
#endif
		{
			handle->dmd_set_reg(handle, 0x002a, 0x27);
		}
	}
	else
	{
	}

	//if(handle->tuner_dc_settings.tuner_type == MtFeTN_NXPTDA18250)
	//{
	//	handle->dmd_set_reg(handle, 0x0036, 0x81);
	//}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_dd3k_t
**
**	DESCRIPTION:
**		connect to a specific channel
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		feq		-channel frequency, in Hz
**
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_dd3k_t(MT_FE_DD_Device_Handle  handle,U32 freq)
{
	if (handle->demod_cur_mode != MtFeType_CTTB)
		return MtFeErr_NoMatch;

	if (handle->tuner_ct_settings.tuner_set)
	{
		handle->tuner_ct_settings.tuner_set(handle, freq);
	}

	handle->mt_sleep(10);

	mt_fe_dmd_soft_reset_dd3k(handle);
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_connect_dd3k_c
**
**	DESCRIPTION:
**		connect to a specific channel of DVB-C
**
**	IN:
**		MT_FE_DD_Device_Handle  handle
**		feq			-channel frequency, in Hz
**		sym			-symbol rate, KBaud
**		qam			-QAM type
**		inverted		-yes or no
**		xtal:			-xtal freq, KHz
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_dd3k_c(MT_FE_DD_Device_Handle  handle,U32 freq, U32 sym, U16 qam, U8 inverted, U32 xtal)
{
	if (handle->demod_cur_mode != MtFeType_DVBC)
		return MtFeErr_NoMatch;

	if(handle->demod_id == MtFeDmdId_DD630X)
	{
		handle->dmd_set_reg(handle, 0x84, 0x00);
		handle->dmd_set_reg(handle, 0x84, 0x40);
		handle->dmd_set_reg(handle, 0x80, 0x01);
	}

	if (handle->tuner_dc_settings.tuner_set)
	{
		handle->tuner_dc_settings.tuner_set(handle, freq);
	}

	_mt_fe_dmd_reg_init_dd3k_c(handle);
	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		if (handle->tuner_dc_settings.tuner_type == MtFeTN_NXPTDA18273)
		{
			handle->dmd_set_reg(handle, 0x0016, inverted ?0x13 : 0xEC );
			handle->dmd_set_reg(handle, 0x0017, inverted ?0x8E : 0X72);
		}
	}
	//if((handle->tuner_settings.tuner_type == TN_NXP_TDA18250) || (handle->tuner_settings.tuner_type == TN_MXL_608))
	//{
	//	handle->dmd_set_reg(handle, 0x16, (inverted == 1) ? 0x13 : 0xEC);
	//	handle->dmd_set_reg(handle, 0x17, (inverted == 1) ? 0x8E : 0x72);
	//}

	mt_fe_dmd_set_sym_dd3k_c(handle, sym, xtal);
	mt_fe_dmd_set_qam_dd3k_c(handle, qam);
	mt_fe_dmd_set_tx_mode_dd3k_c(handle, inverted, 0);
	mt_fe_dmd_set_output_mode_dd3k(handle);
	mt_fe_dmd_set_demod_appendix_dd3k_c(handle, qam);
	if(handle->demod_id == MtFeDmdId_DD630X)
	{
#if 0
		if (handle->tuner_dc_settings.tuner_type == MtFeTN_MxL608)
		{
			if(inverted == 1)
			{
				handle->dmd_set_reg(handle, 0x0016, 0x13);
				handle->dmd_set_reg(handle, 0x0017, 0x8e);
			}
			else
			{
				handle->dmd_set_reg(handle, 0x0016, 0xec );
				handle->dmd_set_reg(handle, 0x0017, 0x72);
			}
		}
		else if (handle->tuner_dc_settings.tuner_type == MtFeTN_TC3800)
#endif
		if (handle->tuner_dc_settings.tuner_type == MtFeTN_TC3800)
		{
			handle->dmd_set_reg(handle, 0x0016, 00);
			handle->dmd_set_reg(handle, 0x0017, 00);
		}
		// fix102281 start
		else if (handle->tuner_dc_settings.tuner_type == MtFeTN_TC6800)
		{
			handle->dmd_set_reg(handle, 0x0016, 00);
			handle->dmd_set_reg(handle, 0x0017, 00);
		}
		// fix102281 end
	}
	mt_fe_dmd_soft_reset_dd3k(handle);

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_dd3k
**
**	DESCRIPTION:
**		connect to a special channel of demod
**
**	IN:
**		feq			-channel frequency, in Hz
**		sym			-symbol rate,KBaud
**		qam			-QAM type
**		inverted		-yes or no
**		xtal			-xtal freq,KHz
**for CTTB mode "sym, qam, inverted, xtal" not be used
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_dd3k(MT_FE_DD_Device_Handle  handle,U32 freq, U32 sym, U16 qam, U8 inverted, U32 xtal)
{
	if (handle->demod_cur_mode != MtFeType_CTTB)
		mt_fe_dmd_connect_dd3k_c(handle,freq, sym, qam, inverted, xtal);
	else
		mt_fe_dmd_connect_dd3k_t(handle, freq);	/*for CTTB "sym, qam, inverted, xtal" not be used*/

	return MtFeErr_Ok;
}
/*	FUNCTIN:
**		mt_fe_dmd_auto_scan_dd3k_c
**
**	DESCRIPTION:
**		connect to a special channel of demod
**
**	IN:
**		feq			-channel frequency, in Hz
**		xtal			-xtal freq,KHz
**
**	OUT:
**		sym			-symbol rate,KBaud
**		qam			-QAM type
**		inverted		-yes or no
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_auto_scan_dd3k_c(MT_FE_DD_Device_Handle  handle,U32 freq,U32 xtal, U32 *sym, U16 *qam, U8 *inverted,MT_FE_LOCK_STATE *p_state)
{
	MT_FE_LOCK_STATE	lock_state = MtFeLockState_Undef;
	U8 	i = 0;
	U32	waiting_time = 0;
	U32	sym_rate = 6888;
	U8 	inverted_c = *inverted;
	U8 	value = 0,reg_0x51 = 0;
	if (handle->demod_cur_mode!=MtFeType_DVBC)
		return MtFeErr_NoMatch;
	if (handle->tuner_dc_settings.tuner_set)
	{
		handle->tuner_dc_settings.tuner_set(handle,freq);
	}

	_mt_fe_dmd_reg_init_dd3k_c(handle);
	mt_fe_dmd_set_sym_dd3k_c(handle, sym_rate, xtal);
	for(i=0; i<handle->dvbc_qam_config.cnt; i++)
	{
		waiting_time = handle->dvbc_qam_config.wait_time;
		mt_fe_dmd_set_qam_dd3k_c(handle,handle->dvbc_qam_config.qam[i]);
		mt_fe_dmd_set_tx_mode_dd3k_c(handle, inverted_c, 0);
		mt_fe_dmd_set_output_mode_dd3k(handle);
		mt_fe_dmd_set_demod_appendix_dd3k_c(handle, handle->dvbc_qam_config.qam[i]);
		mt_fe_dmd_soft_reset_dd3k(handle);
		while (waiting_time>0)
		{
			handle->mt_sleep(50);
			waiting_time -= 50;
			mt_fe_dmd_get_lock_state_dd3k(handle,&lock_state);
			if (lock_state == MtFeLockState_Locked)
			{
				handle->dmd_get_reg(handle, 0x0051, &reg_0x51);
				value = (U8)(reg_0x51&0x80);
				if (value == 0x80)
				{
					value = (U8)(256-reg_0x51);
					*sym=sym_rate+value*sym_rate/2000;
				}
				else
				{
					*sym=sym_rate-reg_0x51*sym_rate/2000;
				}
				*qam=handle->dvbc_qam_config.qam[i];
				*inverted = inverted_c;
				*p_state = lock_state;
				return MtFeErr_Ok;
			}
		}

		if (inverted_c != 0)
			inverted_c = 0;
		else
			inverted_c = 1;
		mt_fe_dmd_set_tx_mode_dd3k_c(handle, inverted_c, 0);
		mt_fe_dmd_set_output_mode_dd3k(handle);
		mt_fe_dmd_set_demod_appendix_dd3k_c(handle, handle->dvbc_qam_config.qam[i]);
		mt_fe_dmd_soft_reset_dd3k(handle);
		waiting_time = handle->dvbc_qam_config.wait_time;
		while (waiting_time>0)
		{
			handle->mt_sleep(50);
			waiting_time -= 50;
			mt_fe_dmd_get_lock_state_dd3k(handle, &lock_state);
			if (lock_state == MtFeLockState_Locked)
			{
				handle->dmd_get_reg(handle, 0x0051, &reg_0x51);
				value = (U8)(reg_0x51 & 0x80);
				if (value==0x80)
				{
					value = (U8)(256-reg_0x51);
					*sym = sym_rate+value*sym_rate/2000;
				}
				else
				{
					*sym = sym_rate-reg_0x51*sym_rate/2000;
				}
				*qam = handle->dvbc_qam_config.qam[i];
				*inverted = inverted_c;
				*p_state = lock_state;
				return MtFeErr_Ok;
			}
		}
	}

	*p_state = MtFeLockState_Unlocked;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_sleep_dd3k(MT_FE_DD_Device_Handle  handle)
{
	U8	data = 0;
	MT_FE_TYPE old_fe_type = MtFeType_Undef;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		// fix102281 start
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->tuner_dc_settings.tuner_sleep != NULL)
			{
				handle->tuner_dc_settings.tuner_sleep(handle);
			}
		}
		else if (handle->tuner_ct_settings.tuner_sleep != NULL)
		{
			handle->tuner_ct_settings.tuner_sleep(handle);
		}
		// fix102281 end

		handle->dmd_get_reg(handle, 0x0007, &data);
		data = (U8)(data & 0xfe);
		handle->dmd_set_reg(handle, 0x0007, data);
		handle->dmd_get_reg(handle, 0x0004, &data);
		data = (U8)(data | 0x01);
		handle->dmd_set_reg(handle, 0x0004, data);
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		old_fe_type = handle->demod_cur_mode;

		// fix102281 start
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->tuner_dc_settings.tuner_sleep != NULL)
			{
				handle->tuner_dc_settings.tuner_sleep(handle);
			}
		}
		else if (handle->tuner_ct_settings.tuner_sleep != NULL)
		{
			handle->tuner_ct_settings.tuner_sleep(handle);
		}
		// fix102281 end

		if (handle->demod_cur_mode!= MtFeType_CTTB)
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x00);
			handle->mt_sleep(1);
			handle->demod_cur_mode = MtFeType_CTTB;
		}

		handle->dmd_get_reg(handle, 0x0009, &data);
		data = (U8)(data |0x07);
		handle->dmd_set_reg(handle, 0x0009, data);

		handle->dmd_get_reg(handle, 0x0022, &data);
		data = (U8)(data | 0x07);
		handle->dmd_set_reg(handle, 0x0022, data);

		handle->mt_sleep(10);

		if (old_fe_type!=MtFeType_CTTB)
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x80);
			handle->mt_sleep(1);
		}

		handle->demod_cur_mode = old_fe_type;
	}
	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_wake_dd3k(MT_FE_DD_Device_Handle  handle)
{
	U8	data = 0;
	MT_FE_TYPE old_fe_type = MtFeType_Undef;

	if(handle->demod_id == MtFeDmdId_DD3X0X)
	{
		handle->dmd_get_reg(handle, 0x0007, &data);
		data = (U8)(data | 0x01);
		handle->dmd_set_reg(handle, 0x0007, data);
		handle->dmd_get_reg(handle, 0x0004, &data);
		data = (U8)(data & 0xfe);
		handle->dmd_set_reg(handle, 0x0004, data);

		// fix102281 start
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->tuner_dc_settings.tuner_wake_up != NULL)
			{
				handle->tuner_dc_settings.tuner_wake_up(handle);
			}
		}
		else if (handle->tuner_ct_settings.tuner_wake_up != NULL)
		{
			handle->tuner_ct_settings.tuner_wake_up(handle);
		}
		// fix102281 end
	}
	else if(handle->demod_id == MtFeDmdId_DD630X)
	{
		old_fe_type = handle->demod_cur_mode;

		if (handle->demod_cur_mode!=MtFeType_CTTB)
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x00);
			handle->mt_sleep(1);
			handle->demod_cur_mode = MtFeType_CTTB;
		}

		handle->dmd_get_reg(handle, 0x0022, &data);
		data = (U8)(data & 0xf8);
		handle->dmd_set_reg(handle, 0x0022, data);

		handle->dmd_get_reg(handle, 0x0009, &data);
		data = (U8)(data & 0xf8);
		handle->dmd_set_reg(handle, 0x0009, data);
		handle->mt_sleep(10);

		if (old_fe_type!=MtFeType_CTTB)
		{
			handle->dmd_set_reg(handle, 0x00fe, 0x80);
			handle->mt_sleep(1);
		}
		handle->demod_cur_mode = old_fe_type;

		// fix102281 start
		if (handle->demod_cur_mode != MtFeType_CTTB)
		{
			if (handle->tuner_dc_settings.tuner_wake_up != NULL)
			{
				handle->tuner_dc_settings.tuner_wake_up(handle);
			}
		}
		else if (handle->tuner_ct_settings.tuner_wake_up != NULL)
		{
			handle->tuner_ct_settings.tuner_wake_up(handle);
		}
		// fix102281 end

		mt_fe_dmd_soft_reset_dd3k(handle);
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_agc_hi_z__dd3k(MT_FE_DD_Device_Handle  handle, BOOL agc_hiz)
{
	U8	data = 0;
	MT_FE_TYPE  current_mode = MtFeType_CTTB;

	current_mode = handle->demod_cur_mode;

	if(agc_hiz)
	{
		handle->demod_cur_mode=MtFeType_CTTB;

		handle->dmd_set_reg(handle, 0x001d, 0x8d);
		handle->dmd_set_reg(handle, 0x001e, 0xba);
		handle->dmd_set_reg(handle, 0x001d, 0x8e);
		handle->dmd_set_reg(handle, 0x001e, 0x95);
		handle->dmd_set_reg(handle, 0x001d, 0x8f);
		handle->dmd_set_reg(handle, 0x001e, 0x05);

		handle->mt_sleep(1);

		handle->dmd_get_reg(handle, 0x0003, &data);
		data = (U8)(data & 0xbf);
		handle->dmd_set_reg(handle, 0x0003, data);
		handle->dmd_set_reg(handle, 0x0006, 0x0c);

		handle->demod_cur_mode=MtFeType_DVBC;
		handle->dmd_set_reg(handle, 0x0080, 0x07);
	}
	else
	{
		handle->demod_cur_mode=MtFeType_DVBC;
		handle->dmd_set_reg(handle, 0x0080, 0x06);

		handle->demod_cur_mode=MtFeType_CTTB;
		handle->dmd_set_reg(handle, 0x001d, 0x8d);
		handle->dmd_set_reg(handle, 0x001e, 0x8a);
		handle->dmd_set_reg(handle, 0x001d, 0x8e);
		handle->dmd_set_reg(handle, 0x001e, 0xf5);
		handle->dmd_set_reg(handle, 0x001d, 0x8f);
		handle->dmd_set_reg(handle, 0x001e, 0x01);

		handle->mt_sleep(1);

		handle->dmd_get_reg(handle, 0x0003, &data);
		data = (U8)(data | 0x41);
		handle->dmd_set_reg(handle, 0x0003, data);
		handle->dmd_set_reg(handle, 0x0006, 0x02);
	}

	handle->mt_sleep(1000);
	handle->demod_cur_mode=current_mode;

	return  MtFeErr_Ok;
}

// fix102281 start
MT_FE_RET mt_fe_dmd_lab_enable_dd3k(MT_FE_DD_Device_Handle  handle, BOOL lab_enble)
{
	U8	reg_value = 0;

	if(handle->demod_id == MtFeDmdId_DD630X)
	{
		handle->dmd_get_reg(handle, 0x0241, &reg_value);
		if(lab_enble)
			reg_value =(U8)(reg_value|0x02);
		else
			reg_value =(U8)(reg_value&0xfd);
		handle->dmd_set_reg(handle, 0x0241, reg_value);

		mt_fe_dmd_soft_reset_dd3k(handle);
	}

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_optimize_nonstandard_mode_dd3k(MT_FE_DD_Device_Handle  handle, U8 mode_cfg)
{
	U8	reg_value = 0;

	reg_value = mode_cfg;

	if(handle->demod_id == MtFeDmdId_DD630X)
	{
		handle->dmd_set_reg(handle, 0x0242, reg_value);
	}

	return MtFeErr_Ok;
}
// fix102281 end
