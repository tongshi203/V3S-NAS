/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2018 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*
 * Filename:		mt_fe_tn_tc6800.c
 *
 * Description:		Montage TC6800 Digital Cable Tuner IC driver.
 *
 *
 * Author:			Lei.Zhu
 *
 * Version:			0.01.22
 * Date:			2018-01-22

 * History:
 * Description		Version		Date			Author
 *---------------------------------------------------------------------------
 * File Create		0.01.00		2016.03.22		Youzhong.Huang
 * Modify			0.01.03		2016.04.01		Roy.Xu
 * Modify			0.01.04		2016.04.05		Roy.Xu
 * Modify			0.01.05		2016.04.05		Roy.Xu
 * Modify			0.01.06		2016.04.07		Roy.Xu
 * Modify			0.01.07		2016.04.19		Roy.Xu
 * Modify			0.01.08		2016.04.22		Roy.Xu
 * Modify			0.01.09		2016.04.25		Roy.Xu
 * Modify			0.01.10		2016.04.27		Roy.Xu
 * Modify			0.01.11		2016.05.30		Roy.Xu
 * Modify			0.01.12		2016.06.14		Roy.Xu
 * Modify			0.01.14		2016.07.18		Roy.Xu
 * Modify			0.01.15		2016.08.23		Roy.Xu
 * Modify			0.01.16		2016.11.21		Roy.Xu
 * Modify			0.01.17		2017.01.19		Roy.Xu
 * Modify			0.01.18		2017.03.23		Roy.Xu
 * Modify			0.01.19		2017.07.24		Roy.Xu
 * Modify			0.01.20		2017.08.23		Roy.Xu
 * Modify			0.01.21		2017.09.20		Roy.Xu
 * Modify			0.01.22		2018.01.22		Roy.Xu
 *---------------------------------------------------------------------------
 *****************************************************************************/
#include "mt_fe_tn_tc6800.h"
#include "mt_fe_fw_tn_tc6800.h"
#include "fe_dd3k_i2c.h"



MT_FE_TN_DEVICE_SETTINGS_TC6800 TC6800_tuner_cfg;
MT_FE_DD_Device_Handle  DD_6800_handle = NULL;

static S32 _mt_fe_tn_get_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data)
{
//	DD_6800_handle->tn_read(DD_6800_handle,&reg_addr,1,reg_data,1);
    _mt_fe_tn_read(DD_6800_handle,&reg_addr, 1, reg_data, 1);
	return 0;
}

static S32 _mt_fe_tn_set_reg_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 reg_data)
{
	U8 p_buf[2] = {0, 0};

	p_buf[0] =  reg_addr;
	p_buf[1] =  reg_data;
    _mt_fe_tn_write(DD_6800_handle,p_buf, 2);
//	DD_6800_handle->tn_write(DD_6800_handle,p_buf,2);
	return 0;
}

static S32 _mt_fe_tn_write_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 *reg_data, U8 data_len)
{
	U8 p_buf[65] =
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0
		};

	U8 i = 0;

	p_buf[0] =  reg_addr;
	for(i = 0; i < data_len; i++)
		p_buf[i+1] =  reg_data[i];
    _mt_fe_tn_write(DD_6800_handle,p_buf, (U16)(data_len+1));
//	DD_6800_handle->tn_write(DD_6800_handle, p_buf,(U16)(data_len+1));

	return 0;
}

void _mt_sleep_tc6800(U32 ticks_ms)
{
    _mt_sleep(ticks_ms);
//	DD_6800_handle->mt_sleep(ticks_ms);
}

/*****************************************************************************/
/*Function: 	read write register				 */
//example:
//_mt_fe_tn_get_reg_expand_tc6800(handle, 0xfe, 0x01, &buf);
//_mt_fe_tn_set_reg_expand_tc6800(handle, 0xfe, 0x01, buf);
//_mt_fe_tn_set_reg_bit_tc6800(handle, 0x01, 7, 4);
/*****************************************************************************/
S32 _mt_fe_tn_get_reg_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 *reg_data)   //reg_addr_expand: 0xfe, 0xb4, 0xb9
{
	_mt_fe_tn_set_reg_tc6800(handle, reg_addr_expand, reg_addr);
	_mt_fe_tn_get_reg_tc6800(handle, (U8)(reg_addr_expand + 1), reg_data);

	return 0;
}

S32 _mt_fe_tn_set_reg_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 reg_data)   //reg_addr_expand: 0xfe, 0xb4, 0xb9
{
	_mt_fe_tn_set_reg_tc6800(handle, reg_addr_expand, reg_addr);
	_mt_fe_tn_set_reg_tc6800(handle, (U8)(reg_addr_expand + 1), reg_data);

	return 0;
}


S32 _mt_fe_tn_set_reg_bit_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit)
{
	U8 tmp = 0, value = 0;

	if(high_bit < low_bit)
	{
		tmp = high_bit;
		high_bit = low_bit;
		low_bit = tmp;
	}

	data <<= (7 + low_bit - high_bit);
	data &= 0xFF;
	data >>= (7 - high_bit);
	data &= 0xFF;

	tmp = 0xFF;
	tmp <<= (7 + low_bit - high_bit);
	tmp &= 0xFF;
	tmp >>= (7 - high_bit);
	tmp &= 0xFF;

	_mt_fe_tn_get_reg_tc6800(handle, reg_addr, &value);
	value &= ~tmp;
	value |= data;
	_mt_fe_tn_set_reg_tc6800(handle, reg_addr, value);

	return 0;
}

S32 _mt_fe_tn_set_reg_bit_expand_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 reg_addr_expand, U8 reg_addr, U8 reg_data, U8 high_bit, U8 low_bit)
{
	U8 tmp = 0, value = 0;
	U8	data = 0;

	data = 	reg_data;

	if(high_bit < low_bit)
	{
		tmp = high_bit;
		high_bit = low_bit;
		low_bit = tmp;
	}

	data <<= (7 + low_bit - high_bit);
	data &= 0xFF;
	data >>= (7 - high_bit);
	data &= 0xFF;

	tmp = 0xFF;
	tmp <<= (7 + low_bit - high_bit);
	tmp &= 0xFF;
	tmp >>= (7 - high_bit);
	tmp &= 0xFF;

	_mt_fe_tn_get_reg_expand_tc6800(handle, reg_addr_expand, reg_addr, &value);
	value &= ~tmp;
	value |= data;
	_mt_fe_tn_set_reg_expand_tc6800(handle, reg_addr_expand, reg_addr, value);

	return 0;
}

void _mt_fe_tn_set_RF_front_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz)
{
	U32	fl = 112000;
	U32	fm = 334000;
	//U8 buf1;
	U8 buf2;

	if(handle->tuner_int_im == 0)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x88, 0x27);
		_mt_fe_tn_set_reg_tc6800(handle, 0x89, 0x2b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x8a, 0x67);
		buf2 = 0x22;
		//_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0x22);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x69, 0x12);
	}
	else if(handle->tuner_int_im == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x88, 0x1b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x89, 0x2b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x8a, 0x1b);
		buf2 = 0x22;
		//_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0x22);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x69, 0x12);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x88, 0x2b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x89, 0x2b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x8a, 0xab);
		if(freq_KHz <= 602000)
		{
			if(handle->tuner_application == 1)
			{
				buf2 = 0x11;
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x69, 0x09);
			}
			else
			{
				buf2 = 0x55;
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x69, 0x2d);
			}
		}
		else
		{
			buf2 = 0x11;
			//_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0x11);
			_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x69, 0x09);
		}
	}

	if(((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt >= 2))
	{
		if(handle->tuner_int_im == 2)
		{
			_mt_fe_tn_get_reg_tc6800(handle, 0x8b, &buf2);
		}
		else
		{
			buf2 = 0x22;
		}

		buf2 = (U8)(buf2 | 0x88);

		if(handle->tuner_custom_cfg == 1)	// Jiuzhou
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0xaa);
		else
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, buf2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xfd, 0x12);
	}
	else
	{
		if(handle->tuner_custom_cfg == 1)	// Jiuzhou
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0xaa);
		else
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, buf2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xfd, 0x1a);
	}

	if(freq_KHz <= fl)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x7f, 0x08);
		if(handle->tuner_application == 1)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x92, 0xac);
		}
		else
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x92, 0x8c);
		}
	}
	else if(freq_KHz <= fm)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x7f, 0x04);
		_mt_fe_tn_set_reg_tc6800(handle, 0x92, 0x8c);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x7f, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0x92, 0x8c);
	}


	_mt_fe_tn_set_reg_tc6800(handle, 0x87, 0xf0);
	if(freq_KHz <= 451000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x90, 0x49);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x90, 0x4a);
	}


	//_mt_fe_tn_set_reg_tc6800(handle, 0x98, 0xf5);
	if(handle->tuner_application == 0)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xaa, 0x84);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xaa, 0x74);
	}
	_mt_fe_tn_set_reg_tc6800(handle, 0xa9, 0xf6);


	if((handle->tuner_application == 3) || (handle->tuner_application == 4))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x98, 0xf7);	// 0xf5
		_mt_fe_tn_set_reg_tc6800(handle, 0x99, 0xa5);	// 0x94
		_mt_fe_tn_set_reg_tc6800(handle, 0x26, 0x14);

		if(handle->tuner_harmonic_imp == 1)
		{
			if(freq_KHz<=334000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x01);
			}
			else if(freq_KHz<=474000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x29);
			}
			else if(freq_KHz<=858000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x15);
			}
			else
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x01);
			}
		}
	}
	else if(handle->tuner_application == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x98, 0xf5);
		_mt_fe_tn_set_reg_tc6800(handle, 0x99, 0x94);
		_mt_fe_tn_set_reg_tc6800(handle, 0x26, 0x14);


		if(handle->tuner_harmonic_imp == 1)
		{
			if(freq_KHz<=334000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x01);
			}
			else if(freq_KHz<=474000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x29);
			}
			else if(freq_KHz<=858000)
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x15);
			}
			else
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x01);
			}
		}

		if(freq_KHz <= 112000)
		{
			_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x0b);
		}
		else if(freq_KHz <= 334000)
		{
			_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x09);
		}
		else
		{
			if(handle->tuner_harmonic_imp == 1)
			{
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x09);
			}
			else
			{
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x0b);
			}
		}
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x98, 0xf8);
		if(freq_KHz <= 700000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x99, 0xb6);
		}
		else if(freq_KHz <= 900000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x99, 0xa5);
		}
		else
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x99, 0xb6);
		}
		_mt_fe_tn_set_reg_tc6800(handle, 0x26, 0x11);


		if(handle->tuner_harmonic_imp == 1)
		{
			if(((freq_KHz >= 610000) && (freq_KHz <= 624000)) || ((freq_KHz >= 480000) && (freq_KHz <= 492000)))
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x15);//R94<5:2>=5
			}
			else if((freq_KHz>=366000)&&(freq_KHz <= 374000))
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x29);//R94<5:2>=10
			}
			else
			{
				_mt_fe_tn_set_reg_tc6800(handle, 0x94, 0x01);//R94<5:2>=0
			}

			if(((freq_KHz >= 800000) && (freq_KHz <= 820000)) ||
			   ((freq_KHz >= 610000) && (freq_KHz <= 624000)) ||
			   ((freq_KHz >= 480000) && (freq_KHz <= 492000)) ||
			   ((freq_KHz >= 366000) && (freq_KHz <= 374000)))
			{
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0E, 0x04);
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0F, 0x04);
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x10, 0x02);
			}
			else
			{
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0E, 0x04);
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0F, 0x04);
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x10, 0x01);
			}
		}
	}

	return;
}



void _mt_fe_tn_set_Mixer_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz, U8 mixer_sel)
{
	U8	buf0 = 0;

	if(mixer_sel == 2)
	{
		buf0 = 2;

		if(freq_KHz < 125000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x00);
		}
		else if(freq_KHz < 167000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x10);
		}
		else if(freq_KHz < 250000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x21);
		}
		else if(freq_KHz < 335000)
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x32);
		}
		else
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x43);
		}
	}
	else if(mixer_sel == 3)
	{
		buf0 = 3;
		_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x00);
	}
	else
	{
		buf0 = 1;
		_mt_fe_tn_set_reg_tc6800(handle, 0x2f, 0x00);
	}

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x0b, buf0, 1, 0);


	if((freq_KHz >= 330000) && (freq_KHz <= 400000))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x32, 0x04);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x32, 0x00);
	}

	return;
}


void _mt_fe_tn_set_LO_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz, U8 mixer_sel)
{
	U32 LDIV = 0;
	U8	buf0;

	if((mixer_sel == 2) && (freq_KHz < 500000))
	{
		if(freq_KHz < 63000)
		{
			LDIV = 128;
			buf0 = 139;
		}
		else if(freq_KHz < 83000)
		{
			LDIV = 96;
			buf0 = 122;//74;
		}
		else if(freq_KHz < 125000)
		{
			LDIV = 64;
			buf0 = 10;
		}
		else if(freq_KHz < 167000)
		{
			LDIV = 48;
			buf0 = 13;
		}
		else if(freq_KHz < 250000)
		{
			LDIV = 32;
			buf0 = 9;
		}
		else if(freq_KHz < 335000)
		{
			LDIV = 24;
			buf0 = 12;
		}
		else
		{
			LDIV = 16;
			buf0 = 8;
		}
	}
	else if((mixer_sel == 3) && (freq_KHz >= 250000))
	{
		if(freq_KHz < 333000)
		{
			LDIV = 24;
			buf0 = 248;//200;
		}
		else if(freq_KHz < 500000)
		{
			LDIV = 16;
			buf0 = 136;
		}
		else if(freq_KHz < 666000)
		{
			LDIV = 12;
			buf0 = 116;//68;
		}
		else
		{
			LDIV = 8;
			buf0 = 4;
		}
	}
	else
	{
		if(freq_KHz < 56000)
		{
			LDIV = 128;
			buf0 = 11;
		}
		else if(freq_KHz < 83000)
		{
			LDIV = 96;
			buf0 = 122;//74;
		}
		else if(freq_KHz < 125000)
		{
			LDIV = 64;
			buf0 = 10;
		}
		else if(freq_KHz < 167000)
		{
			LDIV = 48;
			buf0 = 121;//73;
		}
		else if(freq_KHz < 250000)
		{
			LDIV = 32;
			buf0 = 9;
		}
		else if(freq_KHz < 335000)
		{
			LDIV = 24;
			buf0 = 120;//72;
		}
		else if(freq_KHz < 500000)
		{
			LDIV = 16;
			buf0 = 8;
		}
		else if(freq_KHz < 667000)
		{
			LDIV = 12;
			buf0 = 116;//68;
		}
		else
		{
			LDIV = 8;
			buf0 = 4;
		}
	}

	handle->tuner_int_ldiv = LDIV;
	handle->tuner_int_fvco_tg = freq_KHz * LDIV;

	if (handle->tuner_int_fvco_tg < 6700000)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x13, 0x04, 5, 3);
	}
	else
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x13, 0x07, 5, 3);
	}

	//buf0 = buf0 + 48;
	_mt_fe_tn_set_reg_tc6800(handle, 0x14, buf0);

	return;
}


void _mt_fe_tn_set_PLL_freq_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 Delta_sign, U8 Num_step)
{
	U32	LDIV = 0x00;
	U32 Fvco_KHz = 0x00;
	U32 Fvco_in_KHz = 0x00;
	U32	pll_ref = 48000;				// KHz
	U8	pll_fdiv_bit_sel = 4;			// value list: 0/1/2/3/4;
	U32 buf32 = 0x00;
	U32 NS4 = 0x00;
	U32 fin = 0x00;
	//U32 NS16 = 0x00;
	U8	pll_sdm_en=1;
	U8  spur_opti_by_dsm = 0;
	U32	FDIV_F = 0x00;
	U32 FDIV_N = 0x00;
	U32 FDIV_FIN = 0x00;
	//U8	RDIV2 = 2;
	U8	ICP_CODE = 5;
	U8	kvco_tune = 0;
	U8	buf = 0;
	U32 freq_LO_KHz = 0;
	U32 harmK = 1;
	U8 Icp_scale_en = 0;

	// if don't use fine tune, set Num_step= 0 and Delta_sign = 0;

	LDIV = handle->tuner_int_ldiv;
	Fvco_in_KHz = handle->tuner_int_fvco_tg;

	if (Delta_sign == 1)
	{
		if ((Num_step & 0x01) == 0)
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg + (LDIV >> 2) * (Num_step >> 1) * 125;
		}
		else if (LDIV == 12)
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg + 375 * (Num_step >> 1) + (Num_step & 0x01) * 188;
		}
		else
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg + (LDIV >> 3) * Num_step * 125;
		}
	}
	else
	{
		if ((Num_step & 0x01) == 0)
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg - (LDIV >> 2) * (Num_step >> 1) * 125;
		}
		else if (LDIV == 12)
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg - 375 * (Num_step >> 1) - (Num_step & 0x01) * 188;
		}
		else
		{
			Fvco_in_KHz = handle->tuner_int_fvco_tg - (LDIV >> 3) * Num_step * 125;
		}
	}

	freq_LO_KHz = Fvco_in_KHz / LDIV;
	pll_ref = 2 * handle->tuner_crystal / 1;
	harmK = (U32)(((Fvco_in_KHz / (handle->tuner_crystal >> 1)) & (0x01)) + (Fvco_in_KHz / handle->tuner_crystal));

	if (handle->tuner_crystal == 24000)
	{
		if ((freq_LO_KHz > 336000 - 2000) && (freq_LO_KHz < 336000 + 4000))
		{
			pll_ref = 1 * handle->tuner_crystal;
		}
		else
			pll_ref = 2 * handle->tuner_crystal / 1;
	}
	else if (handle->tuner_crystal == 27000)
	{
		if  ( ((Fvco_in_KHz > (harmK * handle->tuner_crystal + 10))
			&& (Fvco_in_KHz < (harmK * handle->tuner_crystal + 1010)))
			|| ((Fvco_in_KHz < (harmK * handle->tuner_crystal - 10))
			&& (Fvco_in_KHz > (harmK * handle->tuner_crystal - 1010))))
		{
			pll_ref = 2 * handle->tuner_crystal / 3;
			Icp_scale_en = 2;
		}
		else if (((Fvco_in_KHz > (harmK * handle->tuner_crystal + 1020))
			&& (Fvco_in_KHz < (harmK * handle->tuner_crystal + 2020)))
			|| ((Fvco_in_KHz < (harmK * handle->tuner_crystal - 1020))
			&& (Fvco_in_KHz > (harmK * handle->tuner_crystal - 2020))))
		{
			pll_ref = 2 * handle->tuner_crystal / 3;
			Icp_scale_en = 1;
		}
		else
		{
			pll_ref = 2 * handle->tuner_crystal / 1;
			Icp_scale_en = 0;
		}
	}
	else
	{
		pll_ref = 2 * handle->tuner_crystal / 1;
		Icp_scale_en = 0;
	}

	if ((pll_ref == 27000) || (pll_ref == 9000))
	{
		NS4 = (U32)(2 * Fvco_in_KHz / (pll_ref >> 3));
		fin = (U32)(4096 * (2 * Fvco_in_KHz - (pll_ref >> 3) * NS4) / (pll_ref >> 3));
	}
	else if (pll_ref == 13500)
	{
		NS4 = (U32)(4 * Fvco_in_KHz / (pll_ref >> 2));
		fin = (U32)(4096 * (4 * Fvco_in_KHz - (pll_ref >> 2) * NS4) / (pll_ref >> 2));
	}
	else
	{
		NS4 = (U32)(Fvco_in_KHz / (pll_ref >> 4));
		fin = (U32)(4096 * (Fvco_in_KHz - (pll_ref >> 4) * NS4) / (pll_ref >> 4));
	}

	FDIV_N = (NS4 >> 4) & 0x3ff;
	FDIV_F = ((NS4 >> (4 - pll_fdiv_bit_sel)) << (4 - pll_fdiv_bit_sel)) & (0x0f);
	FDIV_FIN = (((((NS4 & 0x0f) << 12) | fin) >> (4 - pll_fdiv_bit_sel)) & 0x0fff);

	if ((Num_step == 0) && ((FDIV_FIN <= 32) || (FDIV_FIN >= (4096 - 32))))
	{
		pll_sdm_en = 0;
	}

	if (pll_sdm_en == 0)
	{
		if (fin & 0x800)
			NS4  = NS4 + 0x01;

		FDIV_N = (NS4 >> 4) & 0x3ff;
		FDIV_F = ((NS4 >> (4 - pll_fdiv_bit_sel)) << (4 - pll_fdiv_bit_sel)) & 0x0f;
		FDIV_FIN = 0x00;
	}

	if (spur_opti_by_dsm && (pll_sdm_en == 0))
	{
		if ((((FDIV_N & 0x03) == 0x01) && (FDIV_F == 0x01)) || (((FDIV_N & 0x03) == 0x03) && (FDIV_F == 0x0f)))
		{
			pll_sdm_en = 1;
			FDIV_FIN = 0x03;
		}
	}

	Fvco_KHz = FDIV_N * pll_ref + ((FDIV_F * pll_ref) >> 4) + ((FDIV_FIN * pll_ref) >> 16);

	handle->tuner_Fvco_val = 1.0 * Fvco_KHz / 1000;				//output,MHz

	if (8000000 > Fvco_KHz)
	{
		buf32 = 4 * 64 + ((8 - 4) *(8000000 - Fvco_KHz) / (2600000 / 64));
	}
	else
	{
		buf32 = 4 * 64 - ((8 - 4) *(Fvco_KHz - 8000000) / (2600000 / 64));
	}

	ICP_CODE = (U8)((buf32 >> 6) + ((buf32 >> 5) & 0x01));

	if (pll_ref == handle->tuner_crystal)
	{
		ICP_CODE = (U8)((buf32 >> 5) + ((buf32 >> 4) & 0x01));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x11, 0x11);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x10, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x15, 0x13);
	}
	else if (pll_ref == (2 * handle->tuner_crystal / 3))
	{
		ICP_CODE = (U8)((buf32 >> 5) + ((buf32 >> 4) & 0x01)) + (U8)((buf32 >> 6) + ((buf32 >> 5) & 0x01));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x11, 0x33);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x10, 0x0b);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x15, 0x43);
	}
	else if (pll_ref == (1 * handle->tuner_crystal / 3))
	{
		ICP_CODE = (U8)((buf32 >> 4) + ((buf32 >> 3) & 0x01)) + (U8)((buf32 >> 5) + ((buf32 >> 4) & 0x01));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x11, 0x33);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x10, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x15, 0x63);
	}
	else if (pll_ref == (1 * handle->tuner_crystal / 2))
	{
		ICP_CODE = (U8)((buf32 >> 4) + ((buf32 >> 3) & 0x01));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x11, 0x22);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x10, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x15, 0x13);
	}
	else
	{
		ICP_CODE = (U8)((buf32 >> 6) + ((buf32 >> 5) & 0x01));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x11, 0x11);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x10, 0x0b);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x15, 0x13);
	}

	if (pll_sdm_en)
	{
		ICP_CODE = (U8)(ICP_CODE - 1);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x12, 0x21);
	}
	else
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x12, 0x00);
	}

	if (Icp_scale_en == 1)
		ICP_CODE = (U8)((ICP_CODE >> 1) & 0xff);
	else if (Icp_scale_en == 2)
		ICP_CODE = (U8)((ICP_CODE >> 2) & 0xff);
	else
		ICP_CODE = (U8)(ICP_CODE);

	if (ICP_CODE >= 15)
		ICP_CODE = 15;

	if (ICP_CODE <= 2)
		ICP_CODE = 2;

	if (kvco_tune == 1)
		ICP_CODE = (U8)((ICP_CODE >> 2) & 0xff);


	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x0a, 0x63);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x0f, 0x3d);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x0b, 0xa1);

	_mt_fe_tn_set_reg_tc6800(handle, 0x3d, 0xff);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x1c, 0x7e);

	if (kvco_tune == 1)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x0d, 0x14);
	}

	if (pll_sdm_en)
	{
		buf = (U8)(((FDIV_F & 0x0f) << 3) | (0x03));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x19, buf);
		buf = (U8)(((FDIV_F & 0x0f) << 3) | (0x07)) ;
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x19, buf);
	}
	else
	{
		buf = (U8)(((FDIV_F & 0x0f) << 3) | (0x02));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x19, buf);
		buf = (U8)(((FDIV_F & 0x0f) << 3) | (0x06));
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x19, buf);
	}

	buf = (U8)(((7 - pll_fdiv_bit_sel) << 2) | ((FDIV_N >> 8) & 0x03));
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x17, buf);

	buf = (U8)(FDIV_N & 0xff);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x18, buf);

	buf = (U8)(((FDIV_FIN >> 8) & 0x0f) | 0x30);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x1a, buf);

	buf = (U8)(FDIV_FIN & 0xff);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x1b, buf);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x0e, 0x58);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x14, 0x4c);

	buf = (U8)((ICP_CODE & 0x0f) | 0x10);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x13, buf);


	if (Num_step != 0)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x3d, 0xfe);
	}

	return;
}


/*****************************************************************************/
/*Function: 	_mt_fe_tn_freq_fine_tune_tc6800  							 */
/*****************************************************************************/
void _mt_fe_tn_freq_fine_tune_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U8 Delta_sign, U8 Num_step)
{
	_mt_fe_tn_set_PLL_freq_tc6800(handle, Delta_sign, Num_step);

	return;
}


void _mt_fe_tn_set_BB_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U32  bandwidth = 0;

	bandwidth = handle->tuner_bandwidth;

	if (bandwidth == 1700)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x59, 0x09);
		_mt_fe_tn_set_reg_tc6800(handle, 0x29, 0xae);
		_mt_fe_tn_set_reg_tc6800(handle, 0x86, 0x3c);
	}
	else if (bandwidth == 6000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x59, 0x0a);
		_mt_fe_tn_set_reg_tc6800(handle, 0x29, 0xaa);
		_mt_fe_tn_set_reg_tc6800(handle, 0x86, 0x12);
	}
	else if (bandwidth == 7000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x59, 0x0b);
		_mt_fe_tn_set_reg_tc6800(handle, 0x29, 0xa8);
		_mt_fe_tn_set_reg_tc6800(handle, 0x86, 0x12);
	}
	else if (bandwidth == 10000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x59, 0x0d);
		_mt_fe_tn_set_reg_tc6800(handle, 0x29, 0xa6);
		_mt_fe_tn_set_reg_tc6800(handle, 0x86, 0x12);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x59, 0x0c);
		_mt_fe_tn_set_reg_tc6800(handle, 0x29, 0xa8);
		_mt_fe_tn_set_reg_tc6800(handle, 0x86, 0x12);
	}

	return;
}


void _mt_fe_tn_set_BB_Cal_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 bandwidth)
{
	return;
}


void _mt_fe_tn_set_DAC_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz)
{
	U32		DACFreq_KHz = handle->tuner_dac;
	U32		fc = 0, fadc = 0, fsd = 0, f2d = 0;
	U8		tempnumber = 0, flt_bit = 0, gain_bit = 0, R_22 = 0;
	U32		FreqTrue108_Hz = 0;
	S32		f1 = 0, f2 = 0, delta1 = 0, Totalnum1 = 0;
	S32		cntT = 0, cntin = 0, NCOI = 0, z0 = 0, z1 = 0, z2 = 0, tmp = 0;
	S32		f1f2number = 0;

	if(DACFreq_KHz <= 4500)
	{
		flt_bit = 0x40;
	}
	else
	{
		flt_bit = 0x80;
	}

	gain_bit = handle->tuner_dac_gain;
	R_22 = (U8)(flt_bit + (gain_bit << 3) + 5);
	_mt_fe_tn_set_reg_tc6800(handle, 0x22, R_22);

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x20, 0x03, 6, 5);

	FreqTrue108_Hz = 108000000;
	if(handle->tuner_crystal == 24000)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x0d);
	}
	else if(handle->tuner_crystal == 27000)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x08);
	}


	if(freq_KHz != 997375)
	{
		if(handle->tuner_crystal == 24000)
		{
			if(((freq_KHz % 27000) >= 23000) || ((freq_KHz % 27000) <= 4000))
			{
				FreqTrue108_Hz = 120000000;
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x12);

				if(((freq_KHz % 30000) >= 26000) || ((freq_KHz % 30000) <= 4000))
				{
					FreqTrue108_Hz = 129600000;
					_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x16);

					if(((freq_KHz % 32400) >= 28400) || ((freq_KHz % 32400) <= 4000))
					{
						FreqTrue108_Hz = 112800000;
						_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x0f);

						if(((freq_KHz % 28200) >= 24200) || ((freq_KHz % 28200) <= 4000))
						{
							FreqTrue108_Hz = 117600000;
							_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x11);
						}
					}
				}
			}
		}
		else if(handle->tuner_crystal == 27000)
		{
			if(((freq_KHz % 27000) >= 23000) || ((freq_KHz % 27000) <= 4000))
			{
				FreqTrue108_Hz = 118800000;
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x0c);

				if(((freq_KHz % 29700) >= 25700) || ((freq_KHz % 29700) <= 4000))
				{
					FreqTrue108_Hz = 129600000;
					_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x10);

					if(((freq_KHz % 32400) >= 28400) || ((freq_KHz % 32400) <= 4000))
					{
						FreqTrue108_Hz = 113400000;
						_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x0a);

						if(((freq_KHz % 28350) >= 24350) || ((freq_KHz % 28350) <= 4000))
						{
							FreqTrue108_Hz = 116100000;
							_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x0b);
						}
					}
				}
			}

			if(freq_KHz == 498377)
			{
				_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x06, 0x04);
			}
		}
	}

	_mt_sleep_tc6800(2);

	f1f2number = ((DACFreq_KHz * 1000 * 512)/ FreqTrue108_Hz) * 128
				+ (((DACFreq_KHz * 1000 * 512) % FreqTrue108_Hz) * 32 / FreqTrue108_Hz) * 4;

	_mt_fe_tn_set_reg_tc6800(handle, 0xfa, (U8)((f1f2number & 0xff00) >> 8));
	_mt_fe_tn_set_reg_tc6800(handle, 0xfb, (U8)((f1f2number & 0x00ff)));

	f1 = 4096;
	fc = FreqTrue108_Hz;
	fadc = fc / 4;
	fsd = 27000000;


	if(handle->tuner_application == 3)
	{
		f2d = handle->tuner_bandwidth / 2 - 150;
	}
	else if(handle->tuner_application == 4)
	{
		if(handle->tuner_bandwidth == 8000)
		{
			f2d = handle->tuner_bandwidth / 2 - 150;
		}
		else
		{
			f2d = handle->tuner_bandwidth / 2 - 100;
		}
	}
	else if(handle->tuner_application == 1)
	{
		f2d = handle->tuner_bandwidth / 2 - 140;
	}
	else
	{
		f2d = handle->tuner_bandwidth / 2 - 100;
	}

	f2 = (fsd / 250) * f2d / ((fc + 500) / 1000);
	delta1 = ((f1 - f2) << 15) / f2;
	Totalnum1 = ((f1 - f2) << 15) - delta1 * f2;
	cntT = f2;
	cntin = Totalnum1;
	NCOI = delta1;
	z0 = cntin;
	z1 = cntT;
	z2 = NCOI;

	tempnumber = (U8)((z0 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe0, (U8)(tempnumber & 0x0f));
	tempnumber = (U8)(z0 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe1, tempnumber);
	tempnumber = (U8)((z1 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe2, tempnumber);
	tempnumber = (U8)(z1 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe3, tempnumber);
	tempnumber = (U8)((z2 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe4, tempnumber);
	tempnumber = (U8)(z2 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xe5, tempnumber);

	tmp = f1;
	f1 = f2;
	f2 = tmp / 2;
	delta1 = ((f1 - f2) << 15) / f2;
	Totalnum1 = ((f1 - f2) << 15) - delta1 * f2;
	NCOI = (f1 << 15) / f2 - (1 << 15);
	cntT = f2;
	cntin = Totalnum1;
	z0 = cntin;
	z1 = cntT;
	z2 = NCOI;

	tempnumber = (U8)((z0 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xef, (U8)(tempnumber & 0x0f));
	tempnumber = (U8)(z0 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xf0, tempnumber);
	tempnumber = (U8)((z1 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xf1, tempnumber);
	tempnumber = (U8)(z1 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xf2, tempnumber);
	tempnumber = (U8)((z2 & 0xff00) >> 8);
	_mt_fe_tn_set_reg_tc6800(handle, 0xf3, tempnumber);
	tempnumber = (U8)(z2 & 0xff);
	_mt_fe_tn_set_reg_tc6800(handle, 0xf4, tempnumber);

	return;
}



void _mt_fe_tn_preset_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U8 buf;

	if((handle->tuner_mode == 1) || (handle->tuner_mode == 2))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0b, 0x7d);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0b, 0x6d);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x0c, 0xe3);
	_mt_fe_tn_set_reg_tc6800(handle, 0x0d, 0xc4);	// 0xc0

	_mt_fe_tn_get_reg_tc6800(handle, 0x9a, &buf);
	buf = (U8)(buf & 0x0f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x9a, buf);
	_mt_fe_tn_set_reg_tc6800(handle, 0x9d, 0x3f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x31, 0x97);
	_mt_fe_tn_set_reg_tc6800(handle, 0xab, 0xd9);
	_mt_fe_tn_set_reg_tc6800(handle, 0xb0, 0x09);
	_mt_fe_tn_set_reg_tc6800(handle, 0xbd, 0x02);
	_mt_fe_tn_set_reg_tc6800(handle, 0xb9, 0x99);
	_mt_fe_tn_set_reg_tc6800(handle, 0x58, 0x11);

	if(handle->tuner_crystal == 24000)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x77, 0x11);
		_mt_fe_tn_set_reg_tc6800(handle, 0x7e, 0x12);
	}
	else if(handle->tuner_crystal == 27000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x7e, 0x13);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x80, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x82, 0x00);

	_mt_fe_tn_set_reg_tc6800(handle, 0x10, 0x00);

	_mt_fe_tn_set_reg_tc6800(handle, 0x66, 0x00);

	if((handle->tuner_application == 3) || (handle->tuner_application == 4) || (handle->tuner_application == 1))
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x1f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x11, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x12, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x15, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x16, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x17, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x18, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x19, 0x3f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0x8b);	// 0xab
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x10);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xcb);	// 0xcc
	}


	return;
}

void _mt_fe_tn_set_tune_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz, U8 mixer_sel, U32 WaitTime)
{
	_mt_fe_tn_set_reg_tc6800(handle, 0x04, 0x7f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0xf8);

	_mt_fe_tn_preset_tc6800(handle);

	_mt_fe_tn_set_RF_front_tc6800(handle, freq_KHz);

	_mt_fe_tn_set_Mixer_tc6800(handle, freq_KHz, mixer_sel);

	_mt_fe_tn_set_LO_tc6800(handle, freq_KHz, mixer_sel);

	_mt_fe_tn_set_PLL_freq_tc6800(handle, 0, 0);

	_mt_fe_tn_set_BB_tc6800(handle);

	_mt_fe_tn_set_reg_tc6800(handle, 0x04, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);

	_mt_fe_tn_set_DAC_tc6800(handle, freq_KHz);

	_mt_fe_tn_set_reg_tc6800(handle, 0x40, 0x1a);
	_mt_fe_tn_set_reg_tc6800(handle, 0x41, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x44, 0x22);
	_mt_fe_tn_set_reg_tc6800(handle, 0x60, 0x34);

	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x00, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x00, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x00, 0x00);

	_mt_sleep_tc6800(WaitTime);

	return;
}

void _mt_fe_tn_set_sysmon_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{

	return;
}


void _mt_fe_tn_poweron_set_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U8	data1 = 0, data2 = 0, data3 = 0, i = 0;
	U8	buf;
	//U8	buf_addcap;
#if TC6800_ENABLE_PRINT
	tc6800_dbg_print(("TC6800 power on.\n"));
#endif

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 0x01, 0, 0);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x55, 0x01, 0, 0);

	if (handle->tuner_clock_out == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x16, 0x80);
		_mt_fe_tn_set_reg_tc6800(handle, 0x17, 0xf3);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x16, 0x00);
	}

	buf = handle->tuner_crystal_cap;

	if(buf != 0x18)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x1a, 0x04, buf);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x1a, 0x05, buf);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x0e, 0xa4);
	_mt_fe_tn_set_reg_tc6800(handle, 0x03, 0x00);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x02, 0xe9);

	_mt_fe_tn_set_reg_tc6800(handle, 0x68, 0x0f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x69, 0xf1);
	_mt_fe_tn_set_reg_tc6800(handle, 0x76, 0x30);
	_mt_fe_tn_set_reg_tc6800(handle, 0x78, 0x25);
	_mt_fe_tn_set_reg_tc6800(handle, 0x79, 0x1f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x7b, 0x00);

	if((handle->tuner_application == 3) || (handle->tuner_application == 4))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xb1, 0x1a);	// 0x2a
		_mt_fe_tn_set_reg_tc6800(handle, 0xb2, 0x6f);	// 0xb2
		_mt_fe_tn_set_reg_tc6800(handle, 0xb3, 0x34);	// 0x54
		_mt_fe_tn_set_reg_tc6800(handle, 0xb4, 0x45);	// 0x6e
		_mt_fe_tn_set_reg_tc6800(handle, 0xb5, 0x1a);	// 0x2a
		_mt_fe_tn_set_reg_tc6800(handle, 0xb6, 0x6f);	// 0xb2

		_mt_fe_tn_set_reg_tc6800(handle, 0xba, 0x9a);	// 0x90
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xb1, 0x2a);	// 0x2a
		_mt_fe_tn_set_reg_tc6800(handle, 0xb2, 0xb2);	// 0xb2
		_mt_fe_tn_set_reg_tc6800(handle, 0xb3, 0x54);	// 0x54
		_mt_fe_tn_set_reg_tc6800(handle, 0xb4, 0x6e);	// 0x6e
		_mt_fe_tn_set_reg_tc6800(handle, 0xb5, 0x2a);	// 0x2a
		_mt_fe_tn_set_reg_tc6800(handle, 0xb6, 0xb2);	// 0xb2

		_mt_fe_tn_set_reg_tc6800(handle, 0xba, 0x90);	// 0x90
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x27, 0xb4);
	_mt_fe_tn_set_reg_tc6800(handle, 0x25, 0x2a);
	_mt_fe_tn_set_reg_tc6800(handle, 0x84, 0x20);

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xfc, 0x01, 5, 5);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x2a, 0x01, 1, 0);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xa7, 0x00, 2, 0);
	_mt_fe_tn_set_reg_tc6800(handle, 0x2c, 0x0a);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc7, 0x02);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc6, 0x80);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);


	_mt_fe_tn_set_reg_tc6800(handle, 0xad, 0x0a);
	//_mt_fe_tn_set_reg_tc6800(handle, 0x24, 0x02);

	_mt_fe_tn_set_reg_tc6800(handle, 0xfd, 0x1a);

	if((handle->tuner_application == 3) || (handle->tuner_application == 4))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
		_mt_fe_tn_set_reg_tc6800(handle, 0xc7, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);

		_mt_fe_tn_set_reg_tc6800(handle, 0x30, 0x2f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x4e);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x07, 0x0c);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x08, 0x03);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x27);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x08);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x11, 0x18);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x12, 0x6a);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x0c);	//0x0b
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x14, 0x07);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x15, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x16, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x17, 0x35);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x18, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x19, 0x18);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1a, 0x08);	// 0x06
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1b, 0x0d);	// 0x0b
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1c, 0x24);	// 0x28
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1d, 0x2a);	// 0x2e

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0E, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0F, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x10, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2b, 0x0a);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x29);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xee);

		_mt_fe_tn_set_reg_tc6800(handle, 0x7c, 0x0d);	// 0x0f

		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x35, 0x00, 7, 4);
	}
	else if(handle->tuner_application == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
		_mt_fe_tn_set_reg_tc6800(handle, 0xc7, 0x01);
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);

		_mt_fe_tn_set_reg_tc6800(handle, 0x30, 0x2f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x03);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x07, 0x0c);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x08, 0x04);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x7f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x11, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x12, 0x0b);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x13, 0x09);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x14, 0x07);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x15, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x16, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x17, 0x05);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x18, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x19, 0x7f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1a, 0x0d);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1b, 0x12);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1c, 0x28);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1d, 0x2e);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0E, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0F, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x10, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2b, 0x0a);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x20);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x02);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xcc);

		_mt_fe_tn_set_reg_tc6800(handle, 0x7c, 0x0f);

		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x35, 0x00, 7, 4);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
		_mt_fe_tn_set_reg_tc6800(handle, 0xc7, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x03);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x07, 0x0d);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x08, 0x09);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x7f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1a, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x1b, 0x0d);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0E, 0x04);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0F, 0x04);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x10, 0x01);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2b, 0x0a);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x10);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xcc);

		_mt_fe_tn_set_reg_tc6800(handle, 0x7c, 0x10);

		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x35, 0x00, 7, 4);
	}

	if(handle->tuner_application == 3)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x00, 2, 1);
	}
	else
	{
		if (handle->tuner_gpio_out == 0)
		{
			_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x06, 2, 0);
		}
	}

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xbd, 0x01, 7, 7);

	if((handle->tuner_application == 3) || (handle->tuner_application == 4))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x40);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x41);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x42);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x4a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x52);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x9a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xda);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x1a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x05);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x9a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xaa);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb3);
	}
	else if(handle->tuner_application == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x40);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x41);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x42);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x4a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x52);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x9a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x02);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xda);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x1a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x05);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x05);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xaa);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb3);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x40);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x41);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x42);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x4a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x52);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x5a);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xe2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x01);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x22);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x01);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x03);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x05);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0x62);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x05);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xa2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xaa);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xb8, 0x07);
		_mt_fe_tn_set_reg_tc6800(handle, 0xbe, 0xb3);
	}



	_mt_fe_tn_set_reg_tc6800(handle, 0xa0, 0x0c);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x6a, 0x3f);

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x1d, 0x00, 1, 1);

	_mt_fe_tn_set_sysmon_tc6800(handle);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x05, 0xc0);
	_mt_sleep_tc6800(4);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x05, 0x80);

	_mt_fe_tn_set_reg_tc6800(handle, 0x45, 0x5d);
	_mt_fe_tn_set_tune_tc6800(handle, 997375, 1, 100);

	_mt_fe_tn_set_reg_tc6800(handle, 0x14, 0x7b);

	_mt_fe_tn_set_reg_tc6800(handle, 0x12, 0x1e);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xbd, 0x01, 4, 4);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xbc, 0x03, 7, 6);


	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x53, 0x0a, 0x00, 7, 7);
	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x53, 0x0d, 0x00, 7, 7);

	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x53, 0x14, 0x01, 6, 6);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x03, 0x5a);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x04, 0xc4);


	i = 0;
	do
	{
		i++;
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x01, 0x0);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x00, 0xff);
		_mt_sleep_tc6800(2);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x00, 0x00);

		_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x53, 0x02, 0x01, 6, 6);
		_mt_sleep_tc6800(2);
		_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x53, 0x02, 0x00, 6, 6);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x01, 0x3a);

		_mt_sleep_tc6800(20);

		_mt_fe_tn_set_reg_tc6800(handle, 0x53, 0x06);
		_mt_fe_tn_get_reg_tc6800(handle, 0x54, &data1);
		_mt_fe_tn_set_reg_tc6800(handle, 0x53, 0x08);
		_mt_fe_tn_get_reg_tc6800(handle, 0x54, &data2);
		_mt_fe_tn_get_reg_tc6800(handle, 0x50, &data3);
	}while ((((data1 != 0x2d) && (data1 != 0x2e) && (data1 != 0x2c)) ||
			 ((data2 != 0x2d) && (data2 != 0x2e) && (data2 != 0x2c)) ||
			 ((data3 != 0xc5) && (data3 != 0xc6) && (data3 != 0xc7) && (data3 != 0xc8))) && (i < 4));

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x01, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x12, 0x0e);
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0xbd, 0x00, 4, 4);

	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0a, 0x80);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0b, 0x00);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0c, 0x00);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0d, 0x80);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0e, 0x00);
	_mt_fe_tn_set_reg_expand_tc6800(handle, 0x53, 0x0f, 0x00);

	if(handle->tuner_crystal == 24000)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x5b, 0x02, 5, 3);
		/*_mt_fe_tn_set_tune_tc6800(handle, 498377, 1, 200);
		if(handle->tuner_application != 1)
		{
			buf_addcap = 0;
		}
		else
		{
			_mt_fe_tn_set_reg_tc6800(handle, 0x39, 0x77);
			_mt_fe_tn_get_reg_tc6800(handle, 0x3a, &buf);
			if(buf == 0)						{buf_addcap = 0x00;}
			else if(buf == 1)					{buf_addcap = 0x0f;}
			else if(buf == 2)					{buf_addcap = 0x0e;}
			else if((buf >=3) && (buf < 12))	{buf_addcap = 0x0d;}
			else if((buf >=12)&& (buf < 25))	{buf_addcap = 0x0c;}
			else if((buf >=25)&& (buf < 37))	{buf_addcap = 0x0b;}
			else if((buf >=37)&& (buf < 50))	{buf_addcap = 0x0a;}
			else								{buf_addcap = 0x09;}
		}
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x35, buf_addcap, 7, 4);
		*/
	}
	else if(handle->tuner_crystal == 27000)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x5b, 0x06, 5, 3);

		_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x4e, 0x08, 0x01, 7, 7);
		_mt_fe_tn_set_tune_tc6800(handle, 498377, 1, 200);
		_mt_fe_tn_set_reg_tc6800(handle, 0x39, 0x77);
		_mt_fe_tn_get_reg_tc6800(handle, 0x3a, &buf);
		buf = (U8)((buf & 0x3f) | 0x40);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x77, buf);
	}

	if(((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt >= 2))
	{
		_mt_fe_tn_set_mch_cal_tc6800(handle);

		_mt_fe_tn_set_reg_tc6800(handle, 0x45, 0x1d);
		_mt_fe_tn_set_reg_tc6800(handle, 0x40, 0x1a);
		_mt_fe_tn_set_reg_tc6800(handle, 0x41, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x00, 0x00);
		_mt_fe_tn_set_reg_tc6800(handle, 0x60, 0x34);

		_mt_fe_tn_set_tune_tc6800(handle, 498000, 1, 2000);

		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3e, 0x00, 7, 7);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x45, 0x5d);


	return;
}

void _mt_fe_tn_set_mch_cal_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x4e, 0x0f, 0x03, 5, 4);
	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x4e, 0x10, 0x01, 1, 1);
	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x4e, 0x11, 0x01, 7, 4);
	_mt_fe_tn_set_reg_bit_expand_tc6800(handle, 0x4e, 0x19, 0x00, 1, 1);

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x46, 0x01, 7, 7);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2E);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x26);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x26);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x26);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x22);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x42);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x42);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x42);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x42);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x42);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x24);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x34);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x34);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x54);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x64);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x74);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x18);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x18);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x28);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x28);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x38);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x38);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x38);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x38);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x1C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x19);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x29);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x29);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x39);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x39);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x1D);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2D);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x3D);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x3A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x5A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x7A);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x9B);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xAB);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x9F);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x8F);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x87);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x7B);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA4);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x92);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x86);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA6);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x9E);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x93);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x8B);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x83);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x7B);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x76);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x71);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA5);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA2);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x97);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x8C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x82);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x77);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA3);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x93);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x83);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x99);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x75);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA0);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x89);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0xA1);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x8C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x20);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x20);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x20);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x20);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x2C);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x14);
	_mt_fe_tn_set_reg_tc6800(handle, 0x47, 0x00);

	return;
}

void _mt_fe_tn_set_mch_app_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U16	ccval = 0, ccval_h = 0, ccval_l = 0;
	U8	buf = 0;
	U8	idxh = 0, idxl = 0;
	U32 freq_ch = handle->tuner_freq;
	U32 freq_h = 0, freq_l = 0;

	_mt_fe_tn_get_reg_tc6800(handle, 0x3e, &buf);
	buf = (U8)(buf & 0x80);

	if((((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt >= 2)) && (buf == 0x00))
	{
		if     (freq_ch >= 954000)		{freq_h = 1010000;	freq_l = 954000;	idxh = 0;	idxl = 0;}
		else if(freq_ch >= 858000)		{freq_h =  954000;	freq_l = 858000;	idxh = 0;	idxl = 1;}
		else if(freq_ch >= 810000)		{freq_h =  858000;	freq_l = 810000;	idxh = 1;	idxl = 2;}
		else if(freq_ch >= 738000)		{freq_h =  810000;	freq_l = 738000;	idxh = 2;	idxl = 3;}
		else if(freq_ch >= 658000)		{freq_h =  738000;	freq_l = 658000;	idxh = 3;	idxl = 4;}
		else if(freq_ch >= 586000)		{freq_h =  658000;	freq_l = 586000;	idxh = 4;	idxl = 5;}
		else if(freq_ch >= 538000)		{freq_h =  586000;	freq_l = 538000;	idxh = 5;	idxl = 6;}
		else if(freq_ch >= 498000)		{freq_h =  538000;	freq_l = 498000;	idxh = 6;	idxl = 7;}
		else if(freq_ch >= 474000)		{freq_h =  498000;	freq_l = 474000;	idxh = 7;	idxl = 8;}
		else if(freq_ch >= 443000)		{freq_h =  474000;	freq_l = 443000;	idxh = 8;	idxl = 9;}
		else if(freq_ch >= 419000)		{freq_h =  443000;	freq_l = 419000;	idxh = 9;	idxl = 10;}
		else if(freq_ch >= 395000)		{freq_h =  419000;	freq_l = 395000;	idxh = 10;	idxl = 11;}
		else if(freq_ch >= 371000)		{freq_h =  395000;	freq_l = 371000;	idxh = 11;	idxl = 12;}
		else if(freq_ch >= 355000)		{freq_h =  371000;	freq_l = 355000;	idxh = 12;	idxl = 13;}
		else if(freq_ch >= 339000)		{freq_h =  355000;	freq_l = 339000;	idxh = 13;	idxl = 14;}
		else if(freq_ch >  334000)		{freq_h =  339000;	freq_l = 334000;	idxh = 14;	idxl = 14;}
		else if(freq_ch >= 243000)		{freq_h =  334000;	freq_l = 243000;	idxh = 15;	idxl = 16;}
		else if(freq_ch >= 227000)		{freq_h =  243000;	freq_l = 227000;	idxh = 16;	idxl = 17;}
		else if(freq_ch >= 211000)		{freq_h =  227000;	freq_l = 211000;	idxh = 17;	idxl = 18;}
		else if(freq_ch >= 195000)		{freq_h =  211000;	freq_l = 195000;	idxh = 18;	idxl = 19;}
		else if(freq_ch >= 179000)		{freq_h =  195000;	freq_l = 179000;	idxh = 19;	idxl = 20;}
		else if(freq_ch >= 163000)		{freq_h =  179000;	freq_l = 163000;	idxh = 20;	idxl = 21;}
		else if(freq_ch >= 147000)		{freq_h =  163000;	freq_l = 147000;	idxh = 21;	idxl = 22;}
		else if(freq_ch >= 131000)		{freq_h =  147000;	freq_l = 131000;	idxh = 22;	idxl = 23;}
		else if(freq_ch >  112000)		{freq_h =  131000;	freq_l = 115000;	idxh = 23;	idxl = 24;}
		else if(freq_ch >=  88000)		{freq_h =  112000;	freq_l =  88000;	idxh = 25;	idxl = 25;}
		else if(freq_ch >=  80000)		{freq_h =   88000;	freq_l =  80000;	idxh = 25;	idxl = 26;}
		else if(freq_ch >=  68500)		{freq_h =   80000;	freq_l =  68500;	idxh = 26;	idxl = 27;}
		else if(freq_ch >=  60500)		{freq_h =   68500;	freq_l =  60500;	idxh = 27;	idxl = 28;}
		else if(freq_ch >=  52500)		{freq_h =   60500;	freq_l =  52500;	idxh = 28;	idxl = 29;}
		else							{freq_h =   52500;	freq_l =  42500;	idxh = 29;	idxl = 29;}


		_mt_fe_tn_set_reg_tc6800(handle, 0x48, idxh);
		_mt_fe_tn_get_reg_tc6800(handle, 0x48, &buf);
		ccval_h = (U16)((buf & 0x80) << 1);
		_mt_fe_tn_get_reg_tc6800(handle, 0x49, &buf);
		ccval_h = (U16)(ccval_h + buf);

		_mt_fe_tn_set_reg_tc6800(handle, 0x48, idxl);
		_mt_fe_tn_get_reg_tc6800(handle, 0x48, &buf);
		ccval_l = (U16)((buf & 0x80) << 1);
		_mt_fe_tn_get_reg_tc6800(handle, 0x49, &buf);
		ccval_l = (U16)(ccval_l + buf);

		if(freq_ch >= 1010000)
		{
			ccval_h = 0;
			ccval_l = 0;
		}
		else if(freq_ch >= 954000)
		{
			ccval_h = 0;
		}

		if (ccval_l > ccval_h)
		{
			ccval = (U16)(ccval_h + (ccval_l - ccval_h) * (freq_h - freq_ch) / (freq_h - freq_l));
		}
		else
		{
			ccval = (U16)(ccval_h + (ccval_h - ccval_l) * (freq_h - freq_ch) / (freq_h - freq_l));
		}

		buf = (U8)(((ccval >> 8) & 0x01) | 0x04);
		_mt_fe_tn_set_reg_tc6800(handle, 0x80, buf);
		buf = (U8)(ccval & 0xff);
		_mt_fe_tn_set_reg_tc6800(handle, 0x81, buf);
		buf = (U8)((ccval & 0x7f) | 0x80);
		_mt_fe_tn_set_reg_tc6800(handle, 0x82, buf);

		_mt_fe_tn_set_reg_tc6800(handle, 0x7e, 0x1a);

		_mt_fe_tn_get_reg_tc6800(handle, 0x7f, &buf);
		buf = (U8)(buf | 0x10);
		_mt_fe_tn_set_reg_tc6800(handle, 0x7f, buf);
	}

	return;
}


void _mt_fe_tn_set_appendix_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	return;
}


void mt_fe_tn_wakeup_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U8	buf = 0;

	_mt_fe_tn_get_reg_tc6800(handle, 0x07, &buf);
	buf = buf & 0x08;
	if(buf == 0x08)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x07, 0x0d);
		_mt_fe_tn_set_reg_tc6800(handle, 0x11, 0x00);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0c, 0x63);
		_mt_fe_tn_set_reg_tc6800(handle, 0x07, 0x0d);
		_mt_fe_tn_set_reg_tc6800(handle, 0x0c, 0xe3);
		_mt_fe_tn_set_reg_tc6800(handle, 0x11, 0x00);
	}

	return;
}


void mt_fe_tn_sleep_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	_mt_fe_tn_set_reg_tc6800(handle, 0x07, 0x05);
	_mt_fe_tn_set_reg_tc6800(handle, 0x11, 0x0a);

	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 0x00, 7, 7);

	return;
}

/*******************************************************************
** Function: mt_fe_tn_download_fw_tc6800
**
**
** Description:	download the mcu code to TC6800
**
**
** Inputs:
**
**	Parameter	Type		Description
**	------------------------------------------------------------
**	p_fw		const U8*	pointer of the mcu array
**
** Outputs:
**
**
********************************************************************/
S32 mt_fe_tn_download_fw_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U32 i = 0, firm_size = 0;
	U8	data0 = 0, data1 = 0;
	S32	ret = 0;
	const U8* p_fw = m88tc6800_fm;
	firm_size = sizeof(m88tc6800_fm) /64;

	_mt_fe_tn_get_reg_tc6800(handle, 0x3e, &data0);
	data0 = (U8)(data0 & 0x20);

	if ((handle->tuner_mcu_status != 1) || (data0 != 0))
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x05, 0x01, 2, 2);
		_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x01);

		for(i = 0; i < firm_size; i++)
		{
			ret = _mt_fe_tn_write_fw_tc6800(handle, 0xc0, (U8*)&(p_fw[64 * i]), 64);
		}
		ret = _mt_fe_tn_write_fw_tc6800(handle, 0xc0, (U8*)&(p_fw[64 * i]), (sizeof(m88tc6800_fm) % 64));

		_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x00);
		_mt_fe_tn_get_reg_tc6800(handle, 0xc8, &data0);
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x05, 0x00, 2, 2);

		data1 = handle->tuner_fw_version;

		if(data0 == data1)
		{
			handle->tuner_mcu_status = 1;
			_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3e, 0x00, 5, 5);
			#if TC6800_ENABLE_PRINT
			tc6800_dbg_print(("TC6800: Download firmware 0x%x OK!\n", data1));
			#endif
		}
		else
		{
			handle->tuner_mcu_status = 0;
			return -1;
		}
	}

	return 0;
}


void mt_fe_tn_init_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	if (handle->tuner_init_OK != 1)
	{
	#if TC6800_ENABLE_PRINT
		tc6800_dbg_print(("TC6800 initialize.\n"));
	#endif
		handle->tuner_mcu_status		 = 0;
		//handle->tuner_dev_addr		 = TUNER_I2C_ADDR_TC6800;

		//handle->tuner_application		 = 0;
		handle->tuner_mode				 = 0;
		handle->tuner_int_lt			 = 2;


		//handle->tuner_bandwidth		 = 8000;
		handle->tuner_crystal			 = 27000;
		//handle->tuner_dac				 = 7200;
		handle->tuner_dac_gain			 = 2;

		handle->tuner_crystal_cap		 = 0x18;
		handle->tuner_clock_out			 = 0;

		handle->tuner_int_im			 = 2;

		handle->tuner_custom_cfg		 = MT_TC6800_CUSTOM_CONFIG;
		handle->tuner_version			 = 122;
		handle->tuner_fw_version		 = 0x0c;
		handle->tuner_time				 = 18012216;

		handle->tuner_init_OK			 = 1;

		handle->tuner_signal_delay_en	 = 0;

		handle->tuner_signal_delay_ns	 = 0;

		handle->tuner_mixer_type		 = 0;
		handle->tuner_harmonic_imp		 = 1;

		handle->tuner_gpio_out			 = 0;
	}

	mt_fe_tn_download_fw_tc6800(handle);

	return;
}

void mt_fe_tn_set_freq_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32 freq_KHz)
{
	U8 sysmon_on = 0;
	U8 buf = 0;
	U8 mixer_sel =0;
	U32 buf_freq = 0;
	U8 buf1 = 0;
	U8 buf2 = 0;

#if TC6800_ENABLE_PRINT
	tc6800_dbg_print(("TC6800 set freq, tuner_app is %d\n", handle->tuner_application));
#endif
	handle ->tuner_freq = freq_KHz;

	mt_fe_tn_init_tc6800(handle);

	_mt_fe_tn_get_reg_tc6800(handle, 0x3e, &buf);
	buf = (U8)(buf & 0x1e);
	if(buf == 0x1e)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 0, 7, 7);
		buf = handle->tuner_application;
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3e, buf, 4, 1);
	}
	else
	{
		buf = (U8)(buf >> 1);
		if(handle->tuner_application != buf)
		{
			if(((buf == 3) && (handle->tuner_application == 4)) || ((buf == 4) && (handle->tuner_application == 3)))
			{
				if(handle->tuner_application == 3)
				{
					_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x00, 2, 1);
				}
				else
				{
					if (handle->tuner_gpio_out == 0)
					{
						_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x06, 2, 0);
					}
				}
			}
			else
			{
				_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 0, 7, 7);
			}
			buf = handle->tuner_application;
			_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3e, buf, 4, 1);
		}
	}

	mt_fe_tn_wakeup_tc6800(handle);

	if(handle->tuner_mixer_type == 1)
	{
		mixer_sel = 1;
	}
	else if(handle->tuner_mixer_type == 2)
	{
		mixer_sel = 2;
	}
	else if(handle->tuner_mixer_type == 3)
	{
		mixer_sel = 3;
	}
	else
	{
		if(freq_KHz <= 334000)
		{
			mixer_sel = 2;
		}
		//else if(((handle->tuner_harmonic_imp == 1) && (handle->tuner_application != 0)) || ((((freq_KHz>=800000)&&(freq_KHz<=820000)) || ((freq_KHz>=610000)&&(freq_KHz<=624000)) || ((freq_KHz>=480000)&&(freq_KHz<=492000)) ||((freq_KHz>=366000)&&(freq_KHz<=374000))) && (handle->tuner_application == 0)))
		else if(handle->tuner_harmonic_imp == 1)
		{
			if(handle->tuner_application != 0)
			{
				mixer_sel = 3;
			}
			else
			{
				if(((freq_KHz >= 800000) && (freq_KHz <= 820000)) ||
				   ((freq_KHz >= 610000) && (freq_KHz <= 624000)) ||
				   ((freq_KHz >= 480000) && (freq_KHz <= 492000)) ||
				   ((freq_KHz >= 366000) && (freq_KHz <= 374000)))
				{
					mixer_sel = 3;
				}
				else
				{
					mixer_sel = 1;
				}
			}
		}
		else
		{
			mixer_sel = 1;
		}
	}


	_mt_fe_tn_get_reg_tc6800(handle, 0x3c, &buf);
	buf = (U8)(buf & 0x80);

	if(buf == 0x00)
	{
		_mt_fe_tn_poweron_set_tc6800(handle);
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 0x01, 7, 7);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x45, 0x5d);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x04, 0x7f);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0xd8);	// 0xf8

	_mt_fe_tn_preset_tc6800(handle);

	_mt_fe_tn_set_RF_front_tc6800(handle, freq_KHz);

	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0xf8);

	_mt_fe_tn_set_Mixer_tc6800(handle, freq_KHz, mixer_sel);

	_mt_fe_tn_set_LO_tc6800(handle, freq_KHz, mixer_sel);

	_mt_fe_tn_set_PLL_freq_tc6800(handle, 0, 0);

	_mt_fe_tn_set_BB_tc6800(handle);

	_mt_fe_tn_set_reg_tc6800(handle, 0x04, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);


	_mt_fe_tn_set_mch_app_tc6800(handle);


	_mt_fe_tn_set_DAC_tc6800(handle, freq_KHz);

	if((handle->tuner_application == 1) || (handle->tuner_application == 3))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xc9, 0x15);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xc9, 0x05);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x40, 0x1a);
	_mt_fe_tn_set_reg_tc6800(handle, 0x41, 0x00);

	if(handle->tuner_crystal == 24000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x44, 0x22);
	}
	else if(handle->tuner_crystal == 27000)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x44, 0x2a);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x60, 0x34);

	if(((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt >= 2))
	{
		_mt_fe_tn_get_reg_tc6800(handle, 0x9a, &buf1);
		buf1 = (U8)(buf1 | 0x10);
		_mt_fe_tn_set_reg_tc6800(handle, 0x9a, buf1);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x04);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x00, 0x01);
	_mt_fe_tn_set_reg_tc6800(handle, 0x00, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0xc2, 0x00);
	_mt_fe_tn_set_reg_tc6800(handle, 0x05, 0x00);


	if(sysmon_on == 1)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x00, 0x81);
	}
	else
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x00, 0x00);
	}

	_mt_sleep_tc6800(10);

	if(((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt >= 2))
	{
		buf1 = (U8)(buf1 & 0xef);
		_mt_fe_tn_set_reg_tc6800(handle, 0x9a, buf1);
	}


	_mt_sleep_tc6800(20);

	if((handle->tuner_application == 3) || (handle->tuner_application == 4) || (handle->tuner_application == 1))
	{
		_mt_sleep_tc6800(20);
	}

	if(((handle->tuner_mode == 1) || (handle->tuner_mode == 2)) && (handle->tuner_int_lt == 2))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0xfd, 0x1a);
		_mt_fe_tn_get_reg_tc6800(handle, 0x8b, &buf2);
		buf2 = buf2 & 0x77;
		if(handle->tuner_custom_cfg == 1)	// Jiuzhou
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, 0xaa);
		else
			_mt_fe_tn_set_reg_tc6800(handle, 0x8b, buf2);
	}

	if((handle->tuner_mode == 1) || (handle->tuner_mode == 2))
	{
		//_mt_fe_tn_get_reg_tc6800(handle, 0x3c, &buf);
		//buf = buf & 0x40;

		//if(buf == 0x00)
		if(1)
		{
			_mt_fe_tn_get_reg_tc6800(handle, 0x9a, &buf);
			buf = (U8)(buf & 0x0f);
			if(buf > 9)
				_mt_fe_tn_set_reg_tc6800(handle, 0x1c, 0x93);
			else if (buf <= 3)
				_mt_fe_tn_set_reg_tc6800(handle, 0x1c, 0x43);

			//_mt_fe_tn_set_reg_bit_tc6800(handle, 0x3c, 1, 6, 6);
		}
	}

	if(handle->tuner_mode == 0)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0d, 0xc4);
	}
	else if(handle->tuner_mode == 1)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0d, 0xc4);
	}
	else if(handle->tuner_mode == 2)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0d, 0xcc);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x0d, 0xc4);
	}

	_mt_fe_tn_set_reg_tc6800(handle, 0x3d, 0xfe);

	_mt_fe_tn_get_reg_tc6800(handle, 0x4a, &buf);
	buf &= 0x0f;
	buf_freq = buf << 8;
	_mt_fe_tn_get_reg_tc6800(handle, 0x4b, &buf);
	buf_freq += buf;
	_mt_fe_tn_get_reg_tc6800(handle, 0x4c, &buf);
	buf &= 0x60;
	buf >>= 5;
	buf_freq >>= buf;
	buf_freq = buf_freq * 8;
	if(handle->tuner_crystal == 27000)
	{
		buf_freq = buf_freq * 27 / 24;
	}

	if((((buf_freq * 1000) - freq_KHz) <= 20000) ||
	   ((freq_KHz - (buf_freq * 1000)) <= 20000) ||
	   (((buf_freq * 1000) - (freq_KHz * 3)) <= 20000) ||
	   (((freq_KHz * 3) - (buf_freq * 1000)) <= 20000))
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x10, 0x50);
	}


	_mt_fe_tn_set_reg_tc6800(handle, 0x66, 0x48);
	_mt_fe_tn_set_reg_tc6800(handle, 0xb0, 0x0b);


	if((handle->tuner_application == 3) || (handle->tuner_application == 4))
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x11, 0x18);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x12, 0x6a);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x15, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x16, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x17, 0x35);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x18, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x19, 0x18);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x4e);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x27);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x08);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xae);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x09);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x08);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xee);
	}
	else if(handle->tuner_application == 1)
	{
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x11, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x12, 0x0b);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x15, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x16, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x17, 0x05);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x18, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x19, 0x7f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x05, 0x3f);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x06, 0x03);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x09, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0a, 0x00);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0b, 0x01);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0c, 0x00);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x39, 0x0d, 0x7f);

		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0x8c);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2c, 0x20);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x2f, 0x02);
		_mt_fe_tn_set_reg_expand_tc6800(handle, 0x4e, 0x29, 0xcc);
	}


	if(handle->tuner_application == 3)
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x60, 0xb7);
	}
	else
	{
		_mt_fe_tn_set_reg_tc6800(handle, 0x60, 0x34);
	}

	#if TC6800_ENABLE_PRINT
	tc6800_dbg_print(("TC6800 set finished.\n"));
	#endif
	return;
}


S32 mt_fe_tn_get_signal_strength_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	static S32 xm_list_dBm[16][16] =
	{
		{   0,    0,  602,  954, 1204, 1398, 1556, 1690, 1806, 1908, 2000, 2083, 2158, 2228, 2292, 2352},
		{2408, 2461, 2511, 2558, 2602, 2644, 2685, 2723, 2760, 2796, 2830, 2863, 2894, 2925, 2954, 2983},
		{3010, 3037, 3063, 3088, 3113, 3136, 3160, 3182, 3204, 3226, 3246, 3267, 3287, 3306, 3326, 3344},
		{3362, 3380, 3398, 3415, 3432, 3449, 3465, 3481, 3496, 3512, 3527, 3542, 3556, 3571, 3585, 3599},
		{3612, 3626, 3639, 3652, 3665, 3678, 3690, 3703, 3715, 3727, 3738, 3750, 3762, 3773, 3784, 3795},
		{3806, 3817, 3828, 3838, 3849, 3859, 3869, 3879, 3889, 3899, 3908, 3918, 3928, 3937, 3946, 3955},
		{3965, 3974, 3982, 3991, 4000, 4009, 4017, 4026, 4034, 4042, 4051, 4059, 4067, 4075, 4083, 4091},
		{4098, 4106, 4114, 4121, 4129, 4136, 4144, 4151, 4158, 4166, 4173, 4180, 4187, 4194, 4201, 4208},
		{4214, 4221, 4228, 4235, 4241, 4248, 4254, 4261, 4267, 4273, 4280, 4286, 4292, 4298, 4305, 4311},
		{4317, 4323, 4329, 4335, 4341, 4346, 4352, 4358, 4364, 4369, 4375, 4381, 4386, 4392, 4397, 4403},
		{4408, 4414, 4419, 4424, 4430, 4435, 4440, 4445, 4451, 4456, 4461, 4466, 4471, 4476, 4481, 4486},
		{4491, 4496, 4501, 4506, 4511, 4515, 4520, 4525, 4530, 4534, 4539, 4544, 4548, 4553, 4558, 4562},
		{4567, 4571, 4576, 4580, 4585, 4589, 4593, 4598, 4602, 4606, 4611, 4615, 4619, 4624, 4628, 4632},
		{4636, 4640, 4644, 4649, 4653, 4657, 4661, 4665, 4669, 4673, 4677, 4681, 4685, 4689, 4693, 4697},
		{4700, 4704, 4708, 4712, 4716, 4720, 4723, 4727, 4731, 4735, 4738, 4742, 4746, 4749, 4753, 4757},
		{4760, 4764, 4768, 4771, 4775, 4778, 4782, 4785, 4789, 4792, 4796, 4799, 4803, 4806, 4810, 4813}
	};

	U8 buf, bufH, bufL;

	S32 level = -107;
	S32 gain_x10;
	S32 gain_ref_x10;
	S32 pd_x10;
	S32 pdtref_x10 = -390;

	U32 freq_KHz = handle->tuner_freq;

	pdtref_x10 = -430;

	if(freq_KHz <= 110000)
		gain_ref_x10 = 585;
	else if(freq_KHz <= 200000)
		gain_ref_x10 = 600;
	else if(freq_KHz <= 334000)
		gain_ref_x10 = 620;
	else if(freq_KHz <= 400000)
		gain_ref_x10 = 560;
	else if(freq_KHz <= 474000)
		gain_ref_x10 = 580;
	else if(freq_KHz <= 600000)
		gain_ref_x10 = 580;
	else if(freq_KHz <= 700000)
		gain_ref_x10 = 595;
	else if(freq_KHz <= 800000)
		gain_ref_x10 = 600;
	else if(freq_KHz <= 858000)
		gain_ref_x10 = 590;
	else if(freq_KHz <= 900000)
		gain_ref_x10 = 590;
	else
		gain_ref_x10 = 560;

	_mt_fe_tn_get_reg_tc6800(handle, 0x63, &buf);

	gain_x10 = gain_ref_x10 + 300 - 1350 + (buf * 10);

	gain_x10 = gain_x10 + 150;

	_mt_fe_tn_get_reg_tc6800(handle, 0x8b, &buf);
	buf &= 0x77;
	if(freq_KHz <= 900000)
	{
		if(buf == 0x55)
		{
			gain_x10 = gain_x10 - 20;
		}
		if((buf == 0x11) && (freq_KHz <= 800000))
		{
			gain_x10 = gain_x10 - 40;
		}
		if((buf == 0x11) && (freq_KHz > 800000))
		{
			gain_x10 = gain_x10 - 25;
		}
	}

	if(handle->tuner_application == 0)
	{
		if((freq_KHz <= 374000) && (freq_KHz > 366000))
			gain_x10 = gain_x10 +60;
		else if((freq_KHz <= 492000) && (freq_KHz >= 480000))
			gain_x10 = gain_x10 + 50;
		else if((freq_KHz <= 624000) && (freq_KHz >= 610000))
			gain_x10 = gain_x10 + 60;
		else if((freq_KHz <= 820000) && (freq_KHz >= 800000))
			gain_x10 = gain_x10 + 10;
	}
	else
	{
		if((freq_KHz <= 400000) && (freq_KHz > 334000))
			gain_x10 = gain_x10 + 60;
		else if((freq_KHz <= 474000) && (freq_KHz > 400000))
			gain_x10 = gain_x10 + 50;
		else if((freq_KHz <= 600000) && (freq_KHz > 474000))
			gain_x10 = gain_x10 + 65;
		else if((freq_KHz <= 800000) && (freq_KHz > 600000))
			gain_x10 = gain_x10 + 65;
		else if((freq_KHz <= 858000) && (freq_KHz > 800000))
			gain_x10 = gain_x10 + 65;
		else if((freq_KHz <= 900000) && (freq_KHz > 858000))
			gain_x10 = gain_x10 + 10;
		else if((freq_KHz <= 1020000) && (freq_KHz > 900000))
			gain_x10 = gain_x10 + 15;
		else if((freq_KHz <=110000) && (handle->tuner_application == 1))
			gain_x10 = gain_x10 - 30;

	}

	_mt_fe_tn_get_reg_tc6800(handle, 0xc8, &buf);
	bufH = buf >> 4;
	bufL = buf & 0x0f;

	pd_x10 = xm_list_dBm[bufH][bufL] / 10 + 5 + pdtref_x10;

	level = (pd_x10 - gain_x10) / 10;

	if(level < -107)
		level = -107;

	return level;
}

void mt_fe_tn_get_signal_delay_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U32 delay_sel, signal_delay;
	U8 bufH, bufL, delay_en = 0;

	delay_en = handle->tuner_signal_delay_en;
	signal_delay = handle->tuner_signal_delay_ns;

	delay_sel = ((signal_delay - 4360) + 74) / 148;
	bufH = (U8)((delay_sel >> 8) & 0x01);
	bufL = (U8)(delay_sel & 0xff);

	if(delay_en == 1)
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0xf6, 1, 1, 1);
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0xf6, bufH, 0, 0);
		_mt_fe_tn_set_reg_tc6800(handle, 0xf7, bufL);
	}
	else
	{
		_mt_fe_tn_set_reg_bit_tc6800(handle, 0xf6, 0, 1, 1);
	}
}

/*****************************************************************************/
/*API: 	mt_fe_tn_set_GPIOout_high_tc6800									 */
/*****************************************************************************/
void mt_fe_tn_set_GPIOout_high_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x07, 2, 0);
	return;
}

/*****************************************************************************/
/*API: 	mt_fe_tn_set_GPIOout_lowt_tc6800									 */
/*****************************************************************************/
void mt_fe_tn_set_GPIOout_low_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	_mt_fe_tn_set_reg_bit_tc6800(handle, 0x37, 0x06, 2, 0);
	return;
}

void mt_fe_tn_get_diagnose_info_tc6800(MT_FE_Tuner_Handle_TC6800 handle, U32* data1, U32* data2)
{
	#if TC6800_ENABLE_PRINT
	U8	reg_data0 = 0,reg_data1 = 0,reg_data2 = 0,reg_data3 = 0,reg_data4 = 0;
	U32	value0 = 0,value1 = 0;
	//mt_fe_i2c_repeat_enable_ct6000(handle);

	_mt_fe_tn_get_reg_tc6800(handle, 0x9a, &reg_data0);
	reg_data0 = (U8)(reg_data0&0x0f);
	_mt_fe_tn_get_reg_tc6800(handle, 0x9d, &reg_data1);
	reg_data1 = (U8)(reg_data1&0x3f);
	_mt_fe_tn_get_reg_tc6800(handle, 0xa7, &reg_data4);
	reg_data4 = (U8)((reg_data4&0xe0)>>5);
	tc6800_dbg_print(("TC6800 -- cg: %d, ci: %d, if: %d\n", reg_data0, reg_data1, reg_data4));

	_mt_fe_tn_get_reg_tc6800(handle, 0xab, &reg_data0);
	reg_data0 = (U8)(reg_data0&0x1f);
	_mt_fe_tn_get_reg_tc6800(handle, 0xb8, &reg_data1);
	reg_data1 = (U8)((reg_data1&0xf8)>>3);
	_mt_fe_tn_get_reg_tc6800(handle, 0xb9, &reg_data2);
	reg_data2 = (U8)(reg_data2&0x1f);
	_mt_fe_tn_get_reg_tc6800(handle, 0x22, &reg_data3);
	reg_data3 = (U8)((reg_data3>>3)&0x07);
	_mt_fe_tn_get_reg_tc6800(handle, 0x8b, &reg_data4);
	tc6800_dbg_print(("TC6800 -- tia: %d, bb: %d, vga: %d, dac: %d, fr: %d\n", reg_data0, reg_data1, reg_data2, reg_data3, reg_data4));

	_mt_fe_tn_set_reg_tc6800(handle, 0x4e, 0x1f);
	_mt_fe_tn_get_reg_tc6800(handle, 0x4f, &reg_data0);
	reg_data0 = (U8)(reg_data0&0x01);

	_mt_fe_tn_set_reg_tc6800(handle, 0x39, 0x77);
	_mt_fe_tn_get_reg_tc6800(handle, 0x3a, &reg_data1);
	reg_data1 = (U8)(reg_data1&0x3f);
	_mt_fe_tn_get_reg_tc6800(handle, 0x80, &reg_data2);
	_mt_fe_tn_get_reg_tc6800(handle, 0x81, &reg_data3);
	value0 = (U32)reg_data3+(U32)((reg_data2&0x01)<<8);
	_mt_fe_tn_get_reg_tc6800(handle, 0x95, &reg_data4);
	value1 = (U32)reg_data4+(U32)((reg_data2&0x02)<<7);
	tc6800_dbg_print(("TC6800 -- pll_lock: %d, bpflt_c: %d, lna_cc: %d, cc_final: %d\n", reg_data0, reg_data1, value0, value1));

	_mt_fe_tn_get_reg_tc6800(handle, 0x01, &reg_data0);
	_mt_fe_tn_get_reg_tc6800(handle, 0xc5, &reg_data1);
	_mt_fe_tn_get_reg_tc6800(handle, 0xc6, &reg_data2);
	_mt_fe_tn_get_reg_tc6800(handle, 0xc8, &reg_data3);
	tc6800_dbg_print(("TC6800 -- 0x01: 0x%x, 0xc5: 0x%x, 0xc6: 0x%x, 0xc8: 0x%x\n", reg_data0, reg_data1, reg_data2, reg_data3));

	_mt_fe_tn_get_reg_tc6800(handle, 0x6e, &reg_data0);
	_mt_fe_tn_get_reg_tc6800(handle, 0x6f, &reg_data1);
	_mt_fe_tn_get_reg_tc6800(handle, 0x70, &reg_data2);
	_mt_fe_tn_get_reg_tc6800(handle, 0x71, &reg_data3);
	_mt_fe_tn_get_reg_tc6800(handle, 0x72, &reg_data4);
	tc6800_dbg_print(("TC6800 -- 0x6e: 0x%x, 0x6f: 0x%x, 0x70: 0x%x, 0x71: 0x%x, 0x72: 0x%x\n", reg_data0, reg_data1, reg_data2, reg_data3, reg_data4));

	_mt_fe_tn_get_reg_tc6800(handle, 0x73, &reg_data0);
	_mt_fe_tn_get_reg_tc6800(handle, 0x74, &reg_data1);
	_mt_fe_tn_get_reg_tc6800(handle, 0x75, &reg_data2);
	tc6800_dbg_print(("TC6800 -- 0x73: 0x%x, 0x74: 0x%x, 0x75: 0x%x\n", reg_data0, reg_data1, reg_data2));

	//mt_fe_i2c_repeat_disable_ct6000(handle);
	#endif

	return;
}

S32 mt_fe_tn_self_check_tc6800(MT_FE_Tuner_Handle_TC6800 handle)
{
	U8 tmp = 0;
	S32 ret = 0;

	ret = _mt_fe_tn_get_reg_tc6800(handle, 0x01, &tmp);
	if(ret != 0)
	{
		#if TC6800_ENABLE_PRINT
		tc6800_dbg_print(("\tTC6800: I2C failed! Chip addr = 0x%02X\n", handle->tuner_dev_addr));
		#endif
		return -1;
	}

	#if TC6800_ENABLE_PRINT
	tc6800_dbg_print(("\tTC6800: chip_id[0x01] = 0x%02X, %s\n", tmp, ((tmp == 0xB1) || (tmp == 0xB2)) ? "OK" : "Fail"));
	#endif

	return ((tmp == 0xB1) || (tmp == 0xB2)) ? 1 : 0;
}

MT_FE_RET mt_fe_tn_init_tc6800_tc(MT_FE_DD_Device_Handle  handle)
{
	DD_6800_handle = handle;
	if(handle->current_demod_type == MtFeType_DVBC)
	{
		TC6800_tuner_cfg.tuner_dev_addr = handle->tuner_dc_settings.tuner_dev_addr;
		TC6800_tuner_cfg.tuner_init_OK = handle->tuner_dc_settings.tuner_init_ok;

		TC6800_tuner_cfg.tuner_application = 0;
		if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_8M)
			TC6800_tuner_cfg.tuner_bandwidth = 8000;
		else if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_7M)
			TC6800_tuner_cfg.tuner_bandwidth = 7000;
		else if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_6M)
			TC6800_tuner_cfg.tuner_bandwidth = 6000;

		TC6800_tuner_cfg.tuner_dac = 7200;

	}
	else
	{
		TC6800_tuner_cfg.tuner_application = 1;

		TC6800_tuner_cfg.tuner_init_OK = handle->tuner_ct_settings.tuner_init_ok;

		if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_8M)
			TC6800_tuner_cfg.tuner_bandwidth = 8000;
		else if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_7M)
			TC6800_tuner_cfg.tuner_bandwidth = 7000;
		else if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_6M)
			TC6800_tuner_cfg.tuner_bandwidth = 6000;

		TC6800_tuner_cfg.tuner_dac =7200;
	}

	mt_fe_tn_init_tc6800(&TC6800_tuner_cfg);

	if(handle->current_demod_type == MtFeType_DVBC)
	{
		handle->tuner_dc_settings.tuner_init_ok  = TC6800_tuner_cfg.tuner_init_OK;
	}
	else
	{
		handle->tuner_ct_settings.tuner_init_ok  = TC6800_tuner_cfg.tuner_init_OK;
	}
	return MtFeErr_Ok;

}
MT_FE_RET mt_fe_tn_set_freq_tc6800_tc(MT_FE_DD_Device_Handle  handle, U32 Freq_Hz)
{
	if(handle->current_demod_type == MtFeType_DVBC)
	{
		if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_8M)
			TC6800_tuner_cfg.tuner_bandwidth = 8000;
		else if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_7M)
			TC6800_tuner_cfg.tuner_bandwidth = 7000;
		else if (handle->demod_dc_settings.demod_bandwidth == MtFeBandwidth_6M)
			TC6800_tuner_cfg.tuner_bandwidth = 6000;
	}
	else
	{
		if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_8M)
			TC6800_tuner_cfg.tuner_bandwidth = 8000;
		else if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_7M)
			TC6800_tuner_cfg.tuner_bandwidth = 7000;
		else if (handle->demod_ct_settings.demod_bandwidth == MtFeBandwidth_6M)
			TC6800_tuner_cfg.tuner_bandwidth = 6000;
	}
		if(DD_6800_handle->demod_id == MtFeDmdId_DD630X)
		TC6800_tuner_cfg.tuner_dac_gain = 4;
	else
		TC6800_tuner_cfg.tuner_dac_gain = 2;

	mt_fe_tn_set_freq_tc6800(&TC6800_tuner_cfg, Freq_Hz/1000);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_strength_tc6800_tc(MT_FE_DD_Device_Handle  handle,S8 *p_strength)
{
	S8 strength = 0;
	/*
	if (handle->m_device_ctt2.tuner_cfg.tuner_lna_type == 1)
	{
		if (handle->m_device_ctt2.input_params.input_freq_kHz < 474000)
			strength = (S8)((mt_fe_tn_get_signal_strength_tc6800(&TC6800_tuner_cfg)) -9);
		else if (handle->m_device_ctt2.input_params.input_freq_kHz <=666000)
			strength = (S8)((mt_fe_tn_get_signal_strength_tc6800(&TC6800_tuner_cfg)) -14);
		else if (handle->m_device_ctt2.input_params.input_freq_kHz <=794000)
			strength = (S8)((mt_fe_tn_get_signal_strength_tc6800(&TC6800_tuner_cfg)) -12);
		else
			strength = (S8)((mt_fe_tn_get_signal_strength_tc6800(&TC6800_tuner_cfg)) -9);
	}
	else
	*/
		strength = (S8)(mt_fe_tn_get_signal_strength_tc6800(&TC6800_tuner_cfg));

	*p_strength = strength+107;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_sleep_tc6800_tc(MT_FE_DD_Device_Handle  handle)
{
	mt_fe_tn_sleep_tc6800(&TC6800_tuner_cfg);
	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_wake_up_tc6800_tc(MT_FE_DD_Device_Handle  handle)
{
	if(handle->demod_cur_mode!=MtFeType_CTTB)
		handle->tuner_dc_settings.tuner_init_ok = 0;
	else
		handle->tuner_ct_settings.tuner_init_ok = 0;

	mt_fe_tn_init_tc6800_tc(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_xtal_tc6800_tc(MT_FE_DD_Device_Handle  handle,U32 xtal_khz)
{
	TC6800_tuner_cfg.tuner_crystal = xtal_khz;
	return MtFeErr_Ok;
}

