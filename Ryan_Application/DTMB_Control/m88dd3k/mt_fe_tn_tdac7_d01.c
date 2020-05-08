/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_fe_common.h"
#include "fe_dd3k_def.h"

#if (MT_FE_TN_SUPPORT_TDAC7_D01 > 0)

#include "fe_dd3k_i2c.h"

#define TUNER_IF_TDAC7_D01				36000000
#define TUNER_STEP_TDAC7_D01_T			166666
#define TUNER_STEP_TDAC7_D01_C		62500


/*	FUNCTIN:
**		mt_fe_tn_set_freq_tdac7_d01
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
MT_FE_RET mt_fe_tn_set_freq_tdac7_d01(MT_FE_DD_Device_Handle  handle,U32 freq)
{
	MT_FE_RET	ret;
	U32			tmp;
	U8			tuner_reg[5];


	if (freq < 47000000 || freq > 870000000)
		return MtFeErr_Param;

	if(handle->demod_cur_mode!=MtFeType_DVBC)
		{
		tmp = (freq + TUNER_IF_TDAC7_D01 + TUNER_STEP_TDAC7_D01_T/2) / TUNER_STEP_TDAC7_D01_T;
		tuner_reg[2] = 0x98;
		
		if (freq < 125000000)
			tuner_reg[3] = 0x84;
		else if(freq < 230000000)
			tuner_reg[3] = 0x86;
		else if(freq < 366000000)
			tuner_reg[3] = 0xc6;
		else if(freq < 602000000)
			tuner_reg[3] = 0x8e;
		else if (freq < 846000000)
			tuner_reg[3] = 0xce;
		else
			tuner_reg[3] = 0x0e;
		}
	else
		{
		tmp = (freq + TUNER_IF_TDAC7_D01 + TUNER_STEP_TDAC7_D01_C/2) / TUNER_STEP_TDAC7_D01_C;
		tuner_reg[2] = 0x9b;
		tuner_reg[3] = 0x8e;
		}

	tuner_reg[0] = (U8)((tmp & 0x00007F00) >> 8);
	tuner_reg[1] = (U8)(tmp & 0x000000FF);

	
	tuner_reg[4]  = 0xc0;


	ret = handle->tn_write(handle, tuner_reg, sizeof(tuner_reg));
	
	return ret;
}

/*	FUNCTIN:
**		mt_fe_tn_get_signal_strength_tdac7_d01
**
**	DESCRIPTION:
**		get the signal strength
**
**	IN:
**		none.
**
**	OUT:
**		*p_strength	-	signal strength in dBuV
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_get_signal_strength_tdac7_d01(MT_FE_DD_Device_Handle  handle,S8 *p_strength)//3.3v
{
	U8	value;
	S8	strength;

	*p_strength = 0;

	if (handle->demod_cur_mode != MtFeType_DVBC)
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
				strength=92+strength;
			}
			else 
			{
				strength=90-((value-0xa0)*90)/68;
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
				strength=90-((value-0x9e)*90)/67;
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
		handle->dmd_get_reg(handle,0x43, &value);
	
		if((value&0x08)==0x08)
		{
			handle->dmd_get_reg(handle,0x3b, &strength);
			handle->dmd_get_reg(handle,0x3c,&value);
			
			*p_strength=255-(strength>>1)-(value>>1);
		}
	}

	return MtFeErr_Ok;
}

#endif /* MT_FE_TN_SUPPORT_TDAC7_D01 */

