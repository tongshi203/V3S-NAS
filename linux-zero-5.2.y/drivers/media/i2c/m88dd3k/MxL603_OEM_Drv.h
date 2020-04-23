/*******************************************************************************
 *
 * FILE NAME          : MxL601_OEM_Drv.h
 * 
 * AUTHOR             : Dong Liu 
 *
 * DATE CREATED       : 11/23/2011
 *
 * DESCRIPTION        : Header file for MxL601_OEM_Drv.c
 *
 *******************************************************************************
 *                Copyright (c) 2010, MaxLinear, Inc.
 ******************************************************************************/

#ifndef __MxL603_OEM_DRV_H__
#define __MxL603_OEM_DRV_H__

/******************************************************************************
    Include Header Files
    (No absolute paths - paths handled by make file)
******************************************************************************/

#include "MaxLinearDataTypes.h"
#include "MxL_Debug.h"

/******************************************************************************
    Macros
******************************************************************************/

/******************************************************************************
    User-Defined Types (Typedefs)
******************************************************************************/

/******************************************************************************
    Global Variable Declarations
******************************************************************************/
extern void * MxL603_OEM_DataPtr[];
/*****************************************************************************
    Prototypes
******************************************************************************/

MXL_STATUS MxLWare603_OEM_WriteRegister(UINT8 devId, UINT8 regAddr, UINT8 regData);
MXL_STATUS MxLWare603_OEM_ReadRegister(UINT8 devId, UINT8 regAddr, UINT8 *regDataPtr);
void MxLWare603_OEM_Sleep(UINT16 delayTimeInMs);  
void MxWare603_OEM_init(UINT32 Demd_t);




#define   DEMOD_NONE      0
#define   DEMOD_ATBM      1
#define   DEMOD_MN88472   2
#define   DEMOD_DS3K      3
#define DEMODE_TYPE DEMOD_DS3K


#endif /* __MxL601_OEM_DRV_H__*/




