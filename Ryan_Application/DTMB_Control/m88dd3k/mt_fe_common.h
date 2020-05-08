/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/* --------------------------------------------------------------------------
*
* File:				mt_fe_common.h
*
* Current version:	1.00.00
*
* Description: Definitions for all Montage front-end projects common use.
*
* Log:	Description			Version			Date			Author
*		---------------------------------------------------------------------
*		Create				1.00.00			2011.01.27		YZ.Huang
****************************************************************************/
#ifndef __MT_FE_COMMON_H__
#define __MT_FE_COMMON_H__


#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************/


#define MT_FE_DEBUG							0		/*	0 off, 1 on*/
#if (MT_FE_DEBUG == 1)
	#define mt_fe_print(str)				OS_PRINTF str;
	#define mt_fe_assert(bool,msg)
#else
	#define mt_fe_print(str)
	#define mt_fe_assert(bool,msg)
#endif



#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)		((void)(x))
#endif

/*	VARIABLE TYPE DEFINES*/
#if 1
#define	U8	unsigned char							/* 8bit unsigned					*/
#define	S8	signed char								/* 8bit unsigned					*/
#define	U16	unsigned short							/* 16bit unsigned					*/
#define	S16	signed short							/* 16bit unsigned					*/
#define	U32	unsigned int							/* 32bit unsigned					*/
#define	S32	signed int								/* 16bit unsigned					*/
#define	DB	double
#else
typedef	unsigned char	U8							/* 8bit unsigned					*/
typedef	unsigned char	S8							/* 8bit unsigned					*/
typedef	unsigned short	U16							/* 16bit unsigned					*/
typedef	signed short	S16							/* 16bit unsigned					*/
typedef	unsigned int	U32							/* 32bit unsigned					*/
typedef	signed int		S32							/* 16bit unsigned					*/
typedef	double			DB
#endif


#ifndef NULL
#define NULL	0
#endif

#ifndef BOOL
#define BOOL	int
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif


/************************ TS OUTPUT DEFINES *****************************/

typedef enum _MT_FE_TS_OUT_MODE
{
	MtFeTsOutMode_Unknown = 0,
	MtFeTsOutMode_Serial,
	MtFeTsOutMode_Parallel,
	MtFeTsOutMode_Common
} MT_FE_TS_OUT_MODE;


typedef enum _MT_FE_TS_OUT_MAX_CLOCK
{
	MtFeTSOut_Max_Clock_Unknown = 0,
	MtFeTSOut_Max_Clock_12_MHz,						// for CI & Parallel mode
	MtFeTSOut_Max_Clock_16_MHz,						// for Parallel mode only
	MtFeTSOut_Max_Clock_19_p_2_MHz,					// for Parallel mode only
	MtFeTSOut_Max_Clock_24_MHz,						// for CI & Parallel mode
	MtFeTSOut_Max_Clock_9_p_6_MHz,
	MtFeTSOut_Max_Clock_10_p_7_MHz,
	MtFeTSOut_Max_Clock_Userdefine,
	MtFeTSOut_Max_Clock_AutoCI
} MT_FE_TS_OUT_MAX_CLOCK;


/*************************************************************************************/

typedef enum _MT_FE_BOOL
{
	MtFe_False = 0,
	MtFe_True
} MT_FE_BOOL;

typedef enum _MT_FE_ON_OFF
{
	MtFe_Off = 0,
	MtFe_On
} MT_FE_ON_OFF;

typedef enum _MT_FE_RET
{
	MtFeErr_Ok					 = 0,
	MtFeErr_Undef				 = -1,
	MtFeErr_Uninit				 = -2,
	MtFeErr_Param				 = -3,
	MtFeErr_NoSupportFunc		 = -4,
	MtFeErr_NoSupportTuner		 = -5,
	MtFeErr_NoSupportDemod		 = -6,
	MtFeErr_UnLock				 = -7,
	MtFeErr_I2cErr				 = -8,
	MtFeErr_DiseqcBusy			 = -9,
	MtFeErr_NoMemory			 = -10,
	MtFeErr_NullPointer			 = -11,
	MtFeErr_TimeOut				 = -12,
	MtFeErr_Fail				 	= -13,
	MtFeErr_NoMatch				=-14,
	MtFeErr_FirmwareErr			=-15
} MT_FE_RET;

typedef enum _MT_FE_TYPE
{
	MtFeType_Undef = 0,
	MtFeType_DVBC,
	MtFeType_DVBT,
	MtFeType_CTTB,
	MtFeType_DvbS,
	MtFeType_DvbS2,
	MtFeType_ABS,
	MtFeType_TMS,
	MtFeType_DTV_Unknown,
	MtFeType_DTV_Checked
} MT_FE_TYPE;

typedef enum _MT_FE_MOD_MODE
{
	MtFeModMode_Undef = 0,
	MtFeModMode_4Qam,
	MtFeModMode_4QamNr,
	MtFeModMode_16Qam,
	MtFeModMode_32Qam,
	MtFeModMode_64Qam,
	MtFeModMode_128Qam,
	MtFeModMode_256Qam,
	MtFeModMode_Qpsk,
	MtFeModMode_8psk,
	MtFeModMode_16Apsk,
	MtFeModMode_32Apsk,
	MtFeModMode_Auto
} MT_FE_MOD_MODE;

typedef enum _MT_FE_LOCK_STATE
{
	MtFeLockState_Undef = 0,
	MtFeLockState_Unlocked,
	MtFeLockState_Locked,
	MtFeLockState_Waiting
} MT_FE_LOCK_STATE;

typedef enum _MT_FE_SUPPORTED_DEMOD
{
	MT_FE_DEMOD_NOTSUPPORT,
	MT_FE_DEMOD_M88DA3100,
	MT_FE_DEMOD_M88DS3002,
	MT_FE_DEMOD_M88DD2000,
	MT_FE_DEMOD_M88DD3000,
	MT_FE_DEMOD_M88DC2800,
	MT_FE_DEMOD_UNDEF
} MT_FE_SUPPORTED_DEMOD;
/*************************************************************************************/


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_COMMON_H__ */

