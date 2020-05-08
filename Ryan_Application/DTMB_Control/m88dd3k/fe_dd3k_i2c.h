/****************************************************************************
*Montage Proprietary and Confidential
* Montage Technology (Shanghai) Co., Ltd.
*@2015 Montage Technology Group Limited and/or its affiliated companies
* --------------------------------------------------------------------------
*
*	FILE:				mt_fe_i2c.h
*
*	VERSION:			1.70.00
*
*	DESCRIPTION:
*
*Log:	Description					Version		Date			Author
*---------------------------------------------------------------------
*	Create					1.00.00		2008.06.05		ChenXiaopeng
*	Modified				1.05.02		2009.01.15		HuangYouzhong
*	Modify					1.30.25		2011.01.06		WangBingju
*	Modify					1.70.00		2014.12.06	WangBingju
****************************************************************************/
#ifndef __MT_FE_I2C_H__
#define __MT_FE_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fe_dd3k_def.h"


extern void _mt_sleep(U32 ms);
MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte);
MT_FE_RET _mt_fe_i2c_read(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);
MT_FE_RET _mt_fe_dmd_set_reg(MT_FE_DD_Device_Handle  handle,U16 reg_addr, U8 reg_data);
MT_FE_RET _mt_fe_dmd_get_reg(MT_FE_DD_Device_Handle  handle,U16 reg_addr, U8 *p_buf);
MT_FE_RET _mt_fe_dmd_write(MT_FE_DD_Device_Handle  handle,U16 reg_addr,U8 *p_buf,U16 n_byte);
MT_FE_RET _mt_fe_tn_write(MT_FE_DD_Device_Handle  handle,U8 *p_buf, U16 n_byte);
MT_FE_RET _mt_fe_tn_read(MT_FE_DD_Device_Handle  handle,U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte);

//#define _mt_sleep delay_ms

#ifdef __cplusplus
}
#endif

#endif /* __MT_FE_I2C_H__ */
