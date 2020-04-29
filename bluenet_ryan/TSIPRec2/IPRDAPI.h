///---------------------------------------------------------
///
///      Chen Yongjian @ Xi'an Tongshi Technology Limited
///			2004.4.8
///      This file is implemented:
///			IP Receiver Driver Interface
///-----------------------------------------------------------

#ifndef __IPRD_API_H_20040408__
#define __IPRD_API_H_20040408__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//	Initialize the IPR driver, must call before any operate on the IPRD
bool IPRD_Init(void);

//	Notify the IPR driver that the application will exit
//	the driver should free all resources
void IPRD_Close(void);


#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __IPRD_API_H_20040408__

