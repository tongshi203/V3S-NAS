/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_fe_common.h"
#include "fe_dd3k_def.h"
#include "fe_dd3k_i2c.h"

#define TUNER_IF_TDAC3_C02A				36000000
#define TUNER_STEP_TDAC3_C02A_T		166666
#define TUNER_STEP_TDAC3_C02A_C		62500
#define DC_LEVEL_5V						1

/*	FUNCTIN:
**		mt_fe_tn_set_freq_tdac3_c02a
**
**	DESCRIPTION:
**		set the frquency of tuner
**
**	IN:
**		freq	in Hz
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_set_freq_tdac3_c02a(MT_FE_DD_Device_Handle  handle,U32 freq)
{
	MT_FE_RET	ret;
	U32			tmp;
	U8			tuner_reg[5];


	if (freq < 48500000 || freq > 864000000)
		return MtFeErr_Param;

	if(handle->demod_cur_mode!=MtFeType_DVBC)
		{
		tmp = (freq + TUNER_IF_TDAC3_C02A + TUNER_STEP_TDAC3_C02A_T/2) / TUNER_STEP_TDAC3_C02A_T;
		tuner_reg[2] = 0xa8;
		}
	else
		{
		tmp = (freq + TUNER_IF_TDAC3_C02A + TUNER_STEP_TDAC3_C02A_C/2) / TUNER_STEP_TDAC3_C02A_C;
		tuner_reg[2] = 0xab;
		}

	tuner_reg[0] = (U8)((tmp & 0x00007F00) >> 8);
	tuner_reg[1] = (U8)(tmp & 0x000000FF);


	if (freq < 125000000)
		tuner_reg[3] = 0xa0 + 0;
	else if(freq < 174000000)
		tuner_reg[3] = 0xa0 + 2;
	else if(freq < 366000000)
		tuner_reg[3] = 0xe0 + 2;
	else if(freq < 470000000)
		tuner_reg[3] = 0xa0 + 8;
	else if (freq < 766000000)
		tuner_reg[3] = 0xe0 + 8;
	else
		tuner_reg[3] = 0x20 + 8;


	tuner_reg[4]  = 0xc6;


	ret = handle->tn_write(handle, tuner_reg, sizeof(tuner_reg));
	
	return ret;
}

/*	FUNCTIN:
**		mt_fe_tn_get_signal_strength_tdac3_c02a
**
**	DESCRIPTION:
**		get the signal strength
**
**	IN:
**		none.
**
**	OUT:
**		*p_strength	-	signal strength in % (CTTB MODE ) or value (DVBC MODE)
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_get_signal_strength_tdac3_c02a(MT_FE_DD_Device_Handle  handle,S8 *p_strength)
{
	U8	value;
	S8	strength;

	*p_strength = 0;
	
	if (handle->demod_cur_mode != MtFeType_DVBC)
	{
		if(DC_LEVEL_5V)
		{
			handle->dmd_get_reg(handle,0xc2, &value);
		
			if(value&0x20)
			{
				handle->dmd_get_reg(handle,0x39, &value);
			
				if((0x80<value)&&(value<0x85))
				{
					strength=90;
				}
				else if(0x80>=value)
				{
					strength=value-0x77;
					strength=100-strength;
				}
				else 
				{
					strength=90-((value-0x83)*80)/68;
				}
				
			}
			else
			{
				handle->dmd_get_reg(handle,0x39, &value);
			
				if((0x7d<value)&&(value<0x81))
				{
					strength=90;
				}
				else if(0x7d>=value)
				{
					strength=value-0x74;
					strength=100-strength;
				}
				else 
				{
					strength=90-((value-0x81)*80)/70;
				}
			}
			if(strength>100)
				strength=100;
			else if(strength<0)
				strength=0;
			
			*p_strength = strength ;		/* %*/
		}
		else
		{
			handle->dmd_get_reg(handle,0xc2, &value);

			if(value&0x20)/*Single*/
			{
				handle->dmd_get_reg(handle,0x39, &value);
			
				if((0x9e<value)&&(value<0xa1))
				{
					strength=90;
				}
				else if(0x9e>=value)
				{
					strength=0x9f-value;
					strength=90+strength;
				}
				else 
				{
					strength=90-((value-0xa0)*80)/68;
				}
		
			}
			else
			{
				handle->dmd_get_reg(handle,0x39, &value);
			
				if((0x9a<value)&&(value<0x9e))
				{
					strength=90;
				}
				else if(0x9a>=value)
				{
					strength=0x9b-value;
					strength=90+strength;
				}
				else
				{
					strength=90-((value-0x9e)*80)/67;
				}
			}
			if(strength>100)
				strength=100;
			else if(strength<0)
				strength=0;
			
			*p_strength = strength ;		/* %*/
		
		}
	}
	else
	{
		handle->dmd_get_reg(handle,0x43, &value);
	
		if((value & 0x08) == 0x08)
		{
			handle->dmd_get_reg(handle,0x3b, &strength);
			handle->dmd_get_reg(handle,0x3c,&value);
			
			*p_strength=255-(strength>>1)-(value>>1);
		}
	}

	return MtFeErr_Ok;
}


