//	FilePurpose.H
//			文件用途管理程序

#ifndef __FILE_PURPOSE_INCLUDE_H__
#define __FILE_PURPOSE_INCLUDE_H__

typedef enum
{
	TSDB_FP_NORMALDATA = 0,						//	普通文件,无定义具体用途
	TSDB_FP_UPDATE_SYSFILE,						//	系统软件升级
	TSDB_FP_UPDATE_UPDATENOW,					//	立即升级的文件,一般为解密文件
	TSDB_FP_UPDATE_APP,							//	OEM 应用程序升级

	TSDB_FP_LICENCE_MSG = 0x1000,				//	信息解密密码的授权文件

	TSDB_FP_RESFORAPP = 0x7FFF0000,				//	0x7FFF0000 ~ 0x7FFFFFFF 为应用程序自定义用途，主要用来作特定文件升级

	TSDB_FP_RESERVED = -2,						//	保留
	TSDB_FP_UNKNOWN = -1						//	未知文件用途, 保留
}TSDBFILEPURPOSE;


#endif //__FILE_PURPOSE_INCLUDE_H__
