/****************************************************************************
*Montage Proprietary and Confidential
* Montage Technology (Shanghai) Co., Ltd.
*@2015 Montage Technology Group Limited and/or its affiliated companies
* --------------------------------------------------------------------------
*
*	FILE:				mt_fe_i2c.c
*
*	VERSION:			1.70.00
*
*	DESCRIPTION:
*			Define the i2c driver interfaces to write/read the registers of the
*		tuner and demodulator.Use the print port to simulate the SCL/SDA signal
*		to connect to the front end devices.
*
*	FUNCTION:
*
* Log:	Description						Version		Date			Author
*		---------------------------------------------------------------------
*		Create					1.00.00		2008.06.05		ChenXiaopeng
*		Modified				1.05.02		2009.01.15		HuangYouzhong
*		Modify					1.30.25		2011.01.06		WangBingju
*		Modify					1.70.00		2014.12.20		WangBingju
****************************************************************************/

#include "fe_dd3k_def.h"
#include "m88dd3k.h"

MT_FE_RET _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte)
{
  /*
		TODO:
			 _mt_fe_i2c_write(U8 dev_addr, U8 *w_buf, U16 w_byte);
  */
//  mtos_printk("%s, dev_addr=0x%x, w_buf[0]=0x%x,w_buf[1]=0x%x, w_byte=0x%x\n",
//    __FUNCTION__, dev_addr, w_buf[0], w_buf[1],w_byte);

//    mtos_sem_take(&g_i2c_dd3k_mutex, 0);
//    i2c_write(g_i2c_dd3k, dev_addr, w_buf, w_byte, I2C_PARAM_DEV_SOC_EXTER);
//    mtos_sem_give(&g_i2c_dd3k_mutex);
    if (ioctl(fp_i2c, I2C_SLAVE, dev_addr>>1) < 0)
    {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        printf("Can't find Chip,the address is 0x%x!\n",dev_addr);
        exit(1);
    }

    if (write(fp_i2c, w_buf, w_byte) != w_byte)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("i2c transaction write failed! dev address is 0x%x\n",dev_addr);
        return MtFeErr_I2cErr;
    }

  return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_i2c_read(U8 dev_addr, U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
  // 8 bit Register Read Protocol:
  // +------+-+-----+-+-+----------+-+
  // |MASTER|S|SADDR|W|  |RegAddr   |
  // +------+-+-----+-+-+-----------+-+
  // |SLAVE |                          |A|               |A| |
  // +------+-+-----+-+-+-----------+-+
  // +------+-+-----+-+-+-----+--+-+
  // |MASTER|S|SADDR|R| |     |MN|P|
  // +------+-+-----+-+-+-----+--+-+
  // |SLAVE |         |A|Data |  | |
  // +------+---------+-+-----+--+-+
  // Legends: SADDR(I2c slave address), S(Start condition), MA(Master Ack), MN(Master NACK),
  // P(Stop condition)

//  *r_buf = *w_buf;

//  mtos_sem_take(&g_i2c_dd3k_mutex, 0);
//  i2c_std_read(g_i2c_dd3k, dev_addr, r_buf, w_byte, r_byte, I2C_PARAM_DEV_SOC_EXTER);
//  mtos_sem_give(&g_i2c_dd3k_mutex);

//  mtos_printk("%s, dev_addr=0x%x, *w_buf=0x%x, w_byte=0x%x, *r_buf=0x%x, r_byte=0x%x\n",
//    __FUNCTION__, dev_addr, *w_buf, w_byte, *r_buf, r_byte);

    if (ioctl(fp_i2c, I2C_SLAVE, dev_addr>>1) < 0)
    {
        /* ERROR HANDLING; you can check errno to see what went wrong */
        printf("Can't find Chip,the address is 0x%x\n!",dev_addr);
        exit(1);
    }

    if (write(fp_i2c, w_buf, w_byte) != w_byte)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("i2c transaction write failed! dev address is 0x%x\n",dev_addr);
        return MtFeErr_I2cErr;
    }

    if (read(fp_i2c, r_buf, r_byte) != r_byte)
    {
        /* ERROR HANDLING: i2c transaction failed */
        printf("i2c transaction read failed!\n");
        return MtFeErr_I2cErr;
    }
  return MtFeErr_Ok ;
}

MT_FE_RET _mt_fe_dmd_write(MT_FE_DD_Device_Handle  handle,U16 reg_addr,U8 *p_buf,U16 n_byte)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8 device_i2c_addr=0;
	U32 i=0;
	U8 write_buf[8]={0,0,0,0,0,0,0,0};

	if(handle->demod_cur_mode==MtFeType_CTTB)
		device_i2c_addr=handle->demod_ct_settings.demod_i2c_addr;
	else
		device_i2c_addr=handle->demod_dc_settings.demod_i2c_addr;

	write_buf[0] = (U8)reg_addr;
	for(i = 1;i <= n_byte;i++)
	{
		write_buf[i] = *(p_buf+i-1);
	}
	ret = _mt_fe_i2c_write(device_i2c_addr, write_buf, (U16)(n_byte+1));

	return ret;
}

MT_FE_RET _mt_fe_dmd_set_reg(MT_FE_DD_Device_Handle  handle,U16 reg_addr, U8 reg_data)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8 device_i2c_addr=0;
	U8	temp[2] = {0, 0};
	if(handle->demod_cur_mode==MtFeType_CTTB)
	{
		device_i2c_addr=handle->demod_ct_settings.demod_i2c_addr;
		if(handle->demod_id == MtFeDmdId_DD630X)
		{
			temp[0] = (U8) 0xfe;
			temp[1] = (U8) (reg_addr>>8);
			ret= _mt_fe_i2c_write(device_i2c_addr, temp, (U16)2);
		}
	}
	else
	{

		if(handle->demod_id == MtFeDmdId_DD630X)
			device_i2c_addr=handle->demod_ct_settings.demod_i2c_addr;
		else
			device_i2c_addr=handle->demod_dc_settings.demod_i2c_addr;
	}

	temp[0] = (U8) reg_addr;
	temp[1] = (U8)reg_data ;
	ret = _mt_fe_i2c_write(device_i2c_addr, temp, (U16)2);
	return ret;
}

MT_FE_RET _mt_fe_dmd_get_reg(MT_FE_DD_Device_Handle  handle,U16 reg_addr, U8 *p_buf)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 device_i2c_addr=0;
	U8	temp[2] = {0,0};

	if(handle->demod_cur_mode==MtFeType_CTTB)
	{
		device_i2c_addr=handle->demod_ct_settings.demod_i2c_addr;
		if(handle->demod_id != MtFeDmdId_DD3X0X)
		{
			temp[0] = (U8)0xfe;
			temp[1] = (U8)(reg_addr>>8);
			ret= _mt_fe_i2c_write(device_i2c_addr, temp, (U16)2);
		}
	}
	else
	{
		if(handle->demod_id == MtFeDmdId_DD630X)
			device_i2c_addr=handle->demod_ct_settings.demod_i2c_addr;
		else
			device_i2c_addr=handle->demod_dc_settings.demod_i2c_addr;
	}

	temp[0] = (U8)reg_addr;
	ret = _mt_fe_i2c_read(device_i2c_addr, temp, (U16)(1),p_buf, (U16)(1));

	return ret;
}

MT_FE_RET _mt_fe_tn_write(MT_FE_DD_Device_Handle  handle,U8 *p_buf, U16 n_byte)
{
	MT_FE_RET ret = MtFeErr_I2cErr;
	U8 data = 0; //, num = 0;
	MT_FE_TYPE  last_mode;
	U8 device_i2c_addr=0;

	if ((handle->demod_id == MtFeDmdId_DD630X)&&(handle->demod_cur_mode != MtFeType_CTTB))
	{
	  _mt_fe_dmd_get_reg(handle,0x0087,&data);
		data=(U8)((data&0x0f)|0x90);
						// bit7		 = 1, Enable I2C repeater
						// bit[6:4]	 = 1, Enable I2C repeater for 1 time
		ret = _mt_fe_dmd_set_reg(handle,0x0087,data);
		if (ret != MtFeErr_Ok)
			return ret;
		device_i2c_addr=handle->tuner_dc_settings.tuner_dev_addr;
	}
	else
	{
  	last_mode=handle->demod_cur_mode;
		handle->demod_cur_mode=MtFeType_CTTB;
		_mt_fe_dmd_get_reg(handle,0x0003,&data);
		data=(U8)((data&0xf8)|0x11);
		ret = _mt_fe_dmd_set_reg(handle,0x0003,data);  //open demod IIC repeater 1 time!
		handle->demod_cur_mode=last_mode;      //,and hardware close the repeater automatically
		if (ret != MtFeErr_Ok)
			return ret;

		if(handle->demod_cur_mode==MtFeType_CTTB)
			device_i2c_addr=handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr=handle->tuner_dc_settings.tuner_dev_addr;
	}
	ret = _mt_fe_i2c_write(device_i2c_addr, p_buf, (U16)n_byte);


	return ret;
}

MT_FE_RET _mt_fe_tn_read(MT_FE_DD_Device_Handle  handle,U8 *w_buf, U16 w_byte, U8 *r_buf, U16 r_byte)
{
	MT_FE_RET ret = MtFeErr_I2cErr;
	U8 data=0;
	MT_FE_TYPE  last_mode;
	U8 device_i2c_addr=0;

	if ((handle->demod_id == MtFeDmdId_DD630X)&&(handle->demod_cur_mode != MtFeType_CTTB))
	{
		_mt_fe_dmd_get_reg(handle,0x0087,&data);
		data=(U8)((data&0x0f)|0xa0);
						// bit7		 = 1, Enable I2C repeater
						// bit[6:4]	 = 1, Enable I2C repeater for 1 time
		ret = _mt_fe_dmd_set_reg(handle,0x0087,data);
		if (ret != MtFeErr_Ok)
			return ret;

		if(handle->demod_cur_mode==MtFeType_CTTB)
			device_i2c_addr=handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr=handle->tuner_dc_settings.tuner_dev_addr;
	}
	else
	{
		last_mode=handle->demod_cur_mode;
		handle->demod_cur_mode=MtFeType_CTTB;
		_mt_fe_dmd_get_reg(handle,0x0003,&data);
		data=(U8)((data&0xf8)|0x12);
		ret = _mt_fe_dmd_set_reg(handle,0x0003,data);  //open demod IIC repeater 2 time!
		handle->demod_cur_mode=last_mode;           //,and hardware close the repeater automatically
		if (ret != MtFeErr_Ok)
			return ret;

		if(handle->demod_cur_mode==MtFeType_CTTB)
			device_i2c_addr=handle->tuner_ct_settings.tuner_dev_addr;
		else
			device_i2c_addr=handle->tuner_dc_settings.tuner_dev_addr;
	}

	ret = _mt_fe_i2c_read(device_i2c_addr, w_buf, w_byte,r_buf, r_byte);

	return ret;
}


