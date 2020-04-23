#include <stdio.h>
#include "sys_types.h"
#include "sys_define.h"
#include "mtos_printk.h"
#include "mt_fe_common.h"
#include "fe_dd3k_def.h"
#define TUNER_MXL603    1
#define FRONTEND_TUNER_TYPE TUNER_MXL603

#define DBG_MXL   1
#if DBG_MXL
#define DBG_MXL_PRINT OS_PRINTF
#else
#define DBG_MXL_PRINT do{}while(0);
#endif

#if (FRONTEND_TUNER_TYPE == TUNER_MXL603)

#include "MxL603_TunerApi.h"
#include "MxL603_OEM_Drv.h"

#define EXAMPLE_DEV_MAX   2
//#define MXL603_I2C_ADDR   0xC0
#define MXL603_INC_CURRENT
//#define MS_DVB_TYPE_SEL 	DVBT

#if (MS_DVB_TYPE_SEL == DVBC)
MS_U8 dmdConfig[] = {};
#endif

#ifdef MXL603_INC_CURRENT
/* Increase driving current for IFamp
 */
static MXL_STATUS MDrv_Tuner_IncCurrent(void)
{
    UINT8 status = MXL_SUCCESS;

    status |= MxLWare603_OEM_WriteRegister(MXL603_I2C_ADDR, 0x5B, 0x10);
    status |= MxLWare603_OEM_WriteRegister(MXL603_I2C_ADDR, 0x5C, 0xB1);

    return (MXL_STATUS) status;
}
#endif

MS_BOOL MDrv_Tuner_PowerOnOff(MS_BOOL bPowerOn)
{
#if 0
    MS_BOOL ret;
    if (bPowerOn == FALSE)
    {
        ret = MxLWare603_API_CfgDevPowerMode(MXL603_I2C_ADDR, MXL603_PWR_MODE_STANDBY);
        if (MxL_ERR_OTHERS == ret)
            return FALSE;
        return TRUE;
    }
    else
    {
        ret = MxLWare603_API_CfgDevPowerMode(MXL603_I2C_ADDR, MXL603_PWR_MODE_ACTIVE);
        if (MxL_OK == ret)
            return TRUE;
        return FALSE;
    }
#endif
    return TRUE;
}

int MxL603_init_main(void)
{
    MXL_STATUS status;
    UINT8 devId;
    MXL_BOOL singleSupply_3_3V;
    MXL_BOOL loopThroughCtrl;
    MXL603_XTAL_SET_CFG_T xtalCfg;
    MXL603_IF_OUT_CFG_T ifOutCfg;
    MXL603_AGC_CFG_T agcCfg;
    MXL603_TUNER_MODE_CFG_T tunerModeCfg;

    //MXL603_CHAN_TUNE_CFG_T chanTuneCfg;
    //MXL_BOOL refLockPtr = MXL_UNLOCKED;
    //MXL_BOOL rfLockPtr = MXL_UNLOCKED;

    /* If OEM data is implemented, customer needs to use OEM data structure
       related operation. Following code should be used as a reference.
       For more information refer to sections 2.5 & 2.6 of
       MxL603_mxLWare_API_UserGuide document.

      for (devId = 0; devId < EXAMPLE_DEV_MAX; devId++)
      {
        // assigning i2c address for  the device
        device_context[devId].i2c_address = GET_FROM_FILE_I2C_ADDRESS(devId);

        // assigning i2c bus for  the device
        device_context[devId].i2c_bus = GET_FROM_FILE_I2C_BUS(devId);

        // create semaphore if necessary
        device_context[devId].sem = CREATE_SEM();

        // sample stat counter
        device_context[devId].i2c_cnt = 0;

        status = MxLWare603_API_CfgDrvInit(devId, (void *) &device_context[devId]);

        // if you don’t want to pass any oem data, just use NULL as a parameter:
        // status = MxLWare603_API_CfgDrvInit(devId, NULL);
      }

    */

    /* If OEM data is not required, customer should treat devId as
     I2C slave Address */
    devId = MXL603_I2C_ADDR;

    //Step 1 : Soft Reset MxL603
		status = MxLWare603_API_CfgDevSoftReset(MXL603_I2C_ADDR);
		if (status != MXL_SUCCESS)
		{
		    DBG_MXL_PRINT("Error! MxLWare603_API_CfgDevSoftReset devId=%x\n",devId);
		    return status;
		}
    //Step 2 : Overwrite Default
    singleSupply_3_3V = MXL_ENABLE;//MXL_DISABLE;
    status = MxLWare603_API_CfgDevOverwriteDefaults(devId, singleSupply_3_3V);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgDevOverwriteDefaults\n");
        return status;
    }

    //Step 3 : XTAL Setting
    xtalCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    xtalCfg.xtalCap = 25;  //12;
    xtalCfg.clkOutEnable =  MXL_DISABLE;
    xtalCfg.clkOutDiv = MXL_DISABLE;
    xtalCfg.clkOutExt = MXL_DISABLE;
    xtalCfg.singleSupply_3_3V = MXL_ENABLE;
    xtalCfg.XtalSharingMode = MXL_DISABLE;
    status = MxLWare603_API_CfgDevXtal(devId, xtalCfg);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgDevXtal\n");
        return status;
    }

    //MxLWare603_API_CfgTunerLoopThrough( devId, MXL_ENABLE);

    //Step 4 : IF Out setting
#if 0//(MS_DVB_TYPE_SEL == ISDBT)
    ifOutCfg.ifOutFreq = MXL603_IF_6Hz;
#else
    ifOutCfg.ifOutFreq = MXL603_IF_4_1MHz;
#endif
    ifOutCfg.ifInversion = MXL_DISABLE;
    ifOutCfg.gainLevel = 11;     //0-13
    ifOutCfg.manualFreqSet = MXL_DISABLE;//MXL_DISABLE;
    ifOutCfg.manualIFOutFreqInKHz = 0;
    status = MxLWare603_API_CfgTunerIFOutParam(devId, ifOutCfg);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgTunerIFOutParam\n");
        return status;
    }

    //Step 5 : AGC Setting
//#if (MS_DVB_TYPE_SEL == DVBC)
//    agcCfg.agcType = MXL603_AGC_SELF;
//#else
    agcCfg.agcType = MXL603_AGC_EXTERNAL;
//#endif
    agcCfg.setPoint = 66;
    agcCfg.agcPolarityInverstion = MXL_DISABLE;
    status = MxLWare603_API_CfgTunerAGC(devId, agcCfg);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgTunerAGC\n");
        return status;
    }

    //Step 6 : Application Mode setting
//#if (MS_DVB_TYPE_SEL == DVBC)
 //   tunerModeCfg.signalMode = MXL603_DIG_DVB_C;
//#elif ((MS_DVB_TYPE_SEL == DVBT) || (MS_DVB_TYPE_SEL == DVBT2))
    tunerModeCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
//#elif (MS_DVB_TYPE_SEL == ISDBT)
//    tunerModeCfg.signalMode = MXL603_DIG_ISDBT_ATSC;
//    printf("\nISDBT mode\n");
//#endif

#ifdef MXL603_INC_CURRENT

    status = MDrv_Tuner_IncCurrent();
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MDrv_Tuner_IncCurrent\n");
        return status;
    }
#endif
//#if (MS_DVB_TYPE_SEL == ISDBT)
 //   tunerModeCfg.ifOutFreqinKHz = 6000;//6000;//4000;
//#else
    tunerModeCfg.ifOutFreqinKHz = 4100;
//#endif
    tunerModeCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    tunerModeCfg.ifOutGainLevel = 11;
    status = MxLWare603_API_CfgTunerMode(devId, tunerModeCfg);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgTunerMode\n");
        return status;
    }

    //Step : Loop Through setting
    loopThroughCtrl = MXL_DISABLE;
    status = MxLWare603_API_CfgTunerLoopThrough(devId, loopThroughCtrl,NULL,NULL);

    return status;
}

#ifdef DVBT2_STYLE
FEMode TunerMode;
void MDrv_Tuner_SetTunerMode(FEMode mode)
{
    TunerMode.Fetype = mode.Fetype;
}
#endif

MT_FE_RET mt_fe_tn_MDrv_Tuner_SetTuner(U32 dwFreq /*Khz*/, U8 ucBw /*MHz*/)
{
    MXL_STATUS status;
    MXL603_BW_E eBw = ucBw - 6 ;
    MXL603_CHAN_TUNE_CFG_T chanTuneCfg;
    MXL_BOOL refLockPtr = MXL_UNLOCKED;
    MXL_BOOL rfLockPtr = MXL_UNLOCKED;
    UINT8 regData = 0;

    //Step 7 : Channel mode & bandwidth & frequency setting
//#if (MS_DVB_TYPE_SEL == DVBC)
//    chanTuneCfg.signalMode = MXL603_DIG_DVB_C;
//#elif ((MS_DVB_TYPE_SEL == DVBT) || (MS_DVB_TYPE_SEL == DVBT2))
    chanTuneCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
//#elif (MS_DVB_TYPE_SEL == ISDBT)
 //   chanTuneCfg.signalMode = MXL603_DIG_ISDBT_ATSC;
 //   printf("\nISDBT mode\n");
//#endif

    switch (chanTuneCfg.signalMode)
    {
    case MXL603_DIG_DVB_C:
        switch (eBw)
        {
        case MXL603_CABLE_BW_6MHz:
        case MXL603_CABLE_BW_7MHz:
        case MXL603_CABLE_BW_8MHz:
            chanTuneCfg.bandWidth = eBw;
            break;
        default:
            chanTuneCfg.bandWidth = MXL603_CABLE_BW_8MHz;
        }
        break;
    case MXL603_DIG_DVB_T_DTMB:
        eBw = eBw + MXL603_TERR_BW_6MHz;
        switch (eBw)
        {
        case MXL603_TERR_BW_6MHz:
        case MXL603_TERR_BW_7MHz:
        case MXL603_TERR_BW_8MHz:
            chanTuneCfg.bandWidth = eBw;
            break;
        default:
            chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
        }
        break;
    case MXL603_DIG_ISDBT_ATSC:
        eBw = eBw + MXL603_TERR_BW_6MHz;
        switch (eBw)
        {
        case MXL603_TERR_BW_6MHz:
        case MXL603_TERR_BW_7MHz:
        case MXL603_TERR_BW_8MHz:
            chanTuneCfg.bandWidth = eBw;
            break;
        default:
            chanTuneCfg.bandWidth = MXL603_TERR_BW_6MHz;
        }
        break;
    default:
        chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
        break;
    }
#ifdef MXL603_INC_CURRENT
    status = MDrv_Tuner_IncCurrent();
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MDrv_Tuner_IncCurrent\n");
        //return status;
    }
#endif
    chanTuneCfg.freqInHz = dwFreq * 1000;
    chanTuneCfg.xtalFreqSel = MXL603_XTAL_16MHz;
    chanTuneCfg.startTune = MXL_START_TUNE;
    status = MxLWare603_API_CfgTunerChanTune(MXL603_I2C_ADDR, chanTuneCfg);
    if (status != MXL_SUCCESS)
    {
        DBG_MXL_PRINT("Error! MxLWare603_API_CfgTunerChanTune\n");
    }

    // Wait 15 ms
    MxLWare603_OEM_Sleep(15);

    // Read back Tuner lock status
    status = MxLWare603_API_ReqTunerLockStatus(MXL603_I2C_ADDR, &rfLockPtr, &refLockPtr);
    if (status == MXL_TRUE)
    {
        if ((MXL_LOCKED == rfLockPtr) && (MXL_LOCKED == refLockPtr))
        {
            DBG_MXL_PRINT("Tuner locked\n");

            MxLWare603_OEM_ReadRegister(MXL603_I2C_ADDR, 0x5B, &regData);
            DBG_MXL_PRINT("MxL603_0x5B : %x\n", regData);
            MxLWare603_OEM_ReadRegister(MXL603_I2C_ADDR, 0x5C, &regData);
            DBG_MXL_PRINT("MxL603_0x5C : %x\n", regData);
        }
        else
            DBG_MXL_PRINT("Tuner unlocked\n");
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_MDrv_Tuner_Init(void *handle)
{
    if (MxL603_init_main() == MXL_TRUE)
    {
        DBG_MXL_PRINT("\n MXL603 INIT OK\n");
        return MtFeErr_Ok;
    }
    else
    {
        DBG_MXL_PRINT("\n MXL603 INIT FAILED\n");
        return MtFeErr_Fail;
    }
}


/*	FUNCTIN:
**		mt_fe_tn_get_strength_MxL603
**
**	DESCRIPTION:
**		get the signal strength
**
**	IN:
**		none.
**
**	OUT:
**		*p_strength	-	signal strength in %
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_get_strength_MxL603(void  *handle,S8 *p_strength)
{
	S8	value = 0;	
	S8	strength = 0;
	UINT8 devId = 0xc0;
	S16 PwrPtr = 0;
	MXL_BOOL rfLockPtr = MXL_UNLOCKED;
	MXL_BOOL refLockPtr = MXL_UNLOCKED;
	MT_FE_DD_Device_Handle  p_handle = handle;
  
	*p_strength = 0;

	
	MxLWare603_API_ReqTunerLockStatus(devId, &rfLockPtr, &refLockPtr);
	if((rfLockPtr != MXL_LOCKED) && (refLockPtr != MXL_LOCKED))
		return MtFeErr_Ok;

	if (p_handle->demod_cur_mode != MtFeType_DVBC)
	{
		
		MxLWare603_API_ReqTunerRxPower(devId, &PwrPtr);
		strength = PwrPtr/100;
		value = (105+strength)*2;
		
		if(value > 100)
			value = 100;
		else if(value < 0)
			value = 0;
		*p_strength = value;		/* %*/
	}
	else
	{
		MxLWare603_API_ReqTunerRxPower(devId, &PwrPtr);
		strength = PwrPtr/100;
		value = (105+strength)*2;
		
		if(value > 100)
			value = 100;
		else if(value < 0)
			value = 0;
		*p_strength = value;		/* %*/
	}
	return MtFeErr_Ok;
	
}


#endif

