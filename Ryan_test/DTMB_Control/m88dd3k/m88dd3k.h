#ifndef __M88DD3K_H__
#define __M88DD3K_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /*对于read和write等函数的支持*/
#include <sys/ioctl.h> /*IO指令流函数，如cmd等，除了打开函数之外，其他的函数定义*/
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "include/i2c/smbus.h"

#include "mt_fe_common.h"
#include "fe_dd3k_def.h"
#include "fe_dd3k_i2c.h"
#include "mt_fe_tn_tc6800.h"


#define M88DD6301_CHIP_ADRRESS       0x20

void Tunner_Demod_Init(void);
MT_FE_RET DM6301_tuner_channel(U32 Freq_KHz, MT_FE_DD_BANDWIDTH demod_bandwidth);
MT_FE_RET Get_lock_state(void);
U8 Get_SNR(void);

//void delay_ms(U16 ms);

extern int fp_i2c;

#endif /* __M88DD3K_H__ */
