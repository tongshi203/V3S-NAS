/*****************************************************************
*** Copyright (c) 2001 Sigma Designs Inc. All rights reserved. ***
*****************************************************************/

/**
	@file   realmagichwl_userland_api.h
	@brief  

	Wishable way to access the kernel module, rather than calling directly its
	ioctls. This API is more convenient than calling ioctls directly.

	To use decoder properties, include also rm84cmn.h (standalone file).
	To use encoder properties, include also rmencoderproperties_enums.h (standalone file).

	@author Emmanuel Michon
	@date   2002-01-08
*/

/**
	@defgroup kernelmodule Kernel Module API

	Wishable way to access the kernel module, rather than calling directly its
	ioctls. This API is more convenient than calling ioctls directly.

*/

#ifndef __REALMAGICHWL_USERLAND_API_H__
#define __REALMAGICHWL_USERLAND_API_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int RUA_handle;

/**
	@ingroup kernelmodule
	Open the device.
	@param no Device number to open. For the EM85xx, there is only 1 device
	available.  The only valid number is 0.
	@return a handle on the devive if it can be accessed, or NULL if no device is accesible
*/
RUA_handle RUA_OpenDevice(RMint32 no);

/**
	@ingroup kernelmodule
	Close the device.
	@param h handle on the device that has previously been successfully opened.
*/
void RUA_ReleaseDevice(RUA_handle h);

//	REALMAGICHWL_IOCTL_DECODER_RESET,
/**
	@ingroup kernelmodule
	Reset the decoder only.  Normally should not be necessary, except once at startup.
	@param h handle on the device that has previously been successfully opened.
*/
void RUA_DECODER_RESET (RUA_handle h);

//	REALMAGICHWL_IOCTL_DECODER_PLAY,
/**
	@ingroup kernelmodule
	Send a play command to the device.
	@param h handle on the device that has previously been successfully opened.
	See the PLAY_OPTIONS structure in rm84cmn.h for valid modes.
	This call starts both puts both audio and video into play mode.
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_DECODER_PLAY (RUA_handle h, RMuint32 mode);

//	REALMAGICHWL_IOCTL_DECODER_STOP,
/**
	@ingroup kernelmodule
	Send a stop command to the device.
	@param h handle on the device that has previously been successfully opened.
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_DECODER_STOP (RUA_handle h);

//	REALMAGICHWL_IOCTL_DECODER_PAUSE,
/**
	@ingroup kernelmodule
	Send a pause command to the device.
	@param h handle on the device that has previously been successfully opened.
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_DECODER_PAUSE (RUA_handle h);

//	REALMAGICHWL_IOCTL_DECODER_AUDIO_SWITCH,
/** 
   @ingroup kernelmodule
   Application of audio parameters change require stopping/starting audio playback with this function.
   
   @param h on the device that has previously been successfully opened.    
   @param activate FALSE stops audio. TRUE starts audio.
   You should call this function to stop the audio decoder, then after
   setting the necessary audio parameters, you should call thif function
   again to restart the audio decoder.
   @return <ReturnValue>
*/
RMint32 RUA_DECODER_AUDIO_SWITCH (RUA_handle h, RMbool activate);

//	REALMAGICHWL_IOCTL_DECODER_HAPPENING_WAIT,
/**
	@ingroup kernelmodule

	*pX is a bitmask of REALMAGICHWL_HAPPENING_... bits to multiplex
	wait on multiple notifications. When the first notification occurs,
	*pX is set to the appropriate bit (and zero in case of timeout).

	Atimeout has to be specified in microseconds. Since the user should
	have no way to lock a process in a kernel ioctl there is no way to
	specify an infinite timeout.

	An interruption (such as Control-C) will cause this function to return
	immediately as if a timeout had occured.

	@param h handle on the device that has previously been successfully opened   
	@param timeout_microsecond   
	@param pX	
*/
void RUA_DECODER_WAIT (RUA_handle h, RMuint64 timeout_microsecond, RMuint32 *pX);

//	REALMAGICHWL_IOCTL_DECODER_SET_PROPERTY,
/**
	@ingroup kernelmodule
	Set a property of the device.
   
	@param h handle on the device that has previously been successfully opened.
	@param PropSet Property Set(see PROPERTY_SETS).     
	@param PropId Property ID (see rm84cmn.h).       
	@param PropTypeLength Property type length in bytes       
	@param pValue Pointer to property Value to set in userland
	@return <ReturnValue>
*/
RMint32 RUA_DECODER_SET_PROPERTY (RUA_handle h, RMuint32 PropSet, RMuint32 PropId, RMuint32 PropTypeLength, void *pValue);

//	REALMAGICHWL_IOCTL_DECODER_GET_PROPERTY,
/**
	@ingroup kernelmodule
	Get a property of the device.

	@param h handle on the device that has previously been successfully opened.   
	@param PropSet Property Set(see PROPERTY_SETS).      
	@param PropId Property ID (see rm84cmn.h).       
	@param PropTypeLength  Property type length in bytes     
	@param pValue Pointer to property Value to get in userland      
	@return <ReturnValue>
*/
RMint32 RUA_DECODER_GET_PROPERTY (RUA_handle h, RMuint32 PropSet,RMuint32 PropId, RMuint32 PropTypeLength, void *pValue);

//	REALMAGICHWL_IOCTL_DECODER_GRAB,
/**
	A grab cannot be done instantly, but in two steps as explained now:

	* first step: call this function with pG->buf=NULL. If the call returns
	RM_OK, the frame dimension are now in pG->width and pG->height. 

	Allocate a suitable buffer of size pG->width*pG->height*3 in
	userland.  Image format is packed BGR (24bit), vertically flipped
	and suitable for TV display (that means you have to ^1/2.2 to get
	right colors on VGA).

	* second step: call this function with this buffer as pG->buf, if the
	call returns RM_OK, the grab has been copied to this buffer.

	Tearingless images is not garanteed. Pause first to get correct images.

	This function is not currently implemented.

	@param h     
	@param pG    
	@return <ReturnValue> is RM_OK if call succeeded.
*/
RMint32 RUA_DECODER_GRAB (RUA_handle h, grabable *pG);

//	REALMAGICHWL_IOCTL_OSDFB_SWITCH,
/**
	@ingroup kernelmodule
	Activate the OSD plane associated with the device.
	The kernel module will continuosly send OSD buffers starting at address 
	"OSDFB_ADDRESS", as defined in common.h.
	For example: if OSDFB_ADDRESS=0x01780000, an osd buffer will start at 0x1780000.
	OSD buffer looks like: [8 byte header][1024 byte palette + alpha value][pixel data]
	The 1024 palette consists of 256 YUV palette entries and a alapha value
	For example a single palette entry would look like: 
	Palette[0] = alpha (0x00=transparent, 0xff=fully opaque)	1 byte
	Palette[1] = Y												1 byte
	Palette[2] = U												1 byte
	Palette[3] = V												1 byte
	@param h handle on the device that has previously been successfully opened.
	@param osdbuffer structure that returns some information on the current OSD plane
	@return RM_OK if success, otherwise returns an error code.
	@remarks Please note that the OSD memory must have been reserverd before activating the OSD plane.
*/
RMint32 RUA_OSDFB_SWITCH (RUA_handle h, OSDBuffer *osdbuffer);

//	REALMAGICHWL_IOCTL_OSDFB_SWITCH_EX,
/**
	@ingroup kernelmodule
	Activate the OSD plane associated with the device.
	This function is currently reserved for sigma designs.  Please use
	RUA_OSDFB_SWITCH instead.
	@param h handle on the device that has previously been successfully opened.
	@param osdbuffer structure that returns some information on the current OSD plane
	osdbuffer->control specifies the device in charge of sending the OSD buffers
	0 = off
	1 = ucode
	2 = arm
	@return RM_OK if success, otherwise returns an error code.
	@remarks Please note that the OSD memory must have been reserverd before activating the OSD plane.
*/
RMint32 RUA_OSDFB_SWITCH_EX (RUA_handle h, OSDBuffer *osdbuffer);

//	REALMAGICHWL_IOCTL_OSDFB_REFRESH,
/**
	@ingroup kernelmodule
	Updates the OSD display with what is currently present in the
	OSD frame buffer.  The current OSD frame buffer refresh mode must
	be off.  i.e. RUA_OSDFB_SWITCH (h, 0) was called before.	
	@param h handle on the device that has previously been successfully opened.
	@param osdbuffer structure that returns some information on the current OSD plane
	if osdbuffer is NULL, then the current OSD frame is updated on to the display
	@return RM_OK if success, otherwise returns an error code.
	@remarks Please note that the OSD memory must have been reserverd before activating the OSD plane.
*/
RMint32 RUA_OSDFB_REFRESH (RUA_handle h, OSDBuffer *osdbuffer);

//	REALMAGICHWL_IOCTL_OSDFB_GENERAL_ALPHA,
/**
	@ingroup kernelmodule
	Set the alpha cofficient between the OSD plane and the video plane for the complete video output.
	It is probably more effiecient to set the alpha values directly, using
	the address of the palette and alpha values obtained in RUA_OSDFB_SWITCH
	@param h handle on the device that has previously been successfully opened.
	@param alpha alpha blending coefficient (between 0 and 255).
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_OSDFB_GENERAL_ALPHA (RUA_handle h,RMint32 alpha);

//	REALMAGICHWL_IOCTL_DECODER_CLEAR_SCREEN,
/**
	@ingroup kernelmodule
	This function will clear the video plane by sending a VIDEO_BLACKFRAME_UNPACKED
	command to the mpeg engine microcode.  The only time the video plane is in
	"unpacked" mode when the application is displaying YUV data, that is when
	the mpeg engine is not actually decoding anything
	@param h handle on the device that has previously been successfully opened.
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_DECODER_CLEAR_SCREEN (RUA_handle h);

//	REALMAGICHWL_IOCTL_DECODER_DISPLAY_YUV,
/**
	@ingroup kernelmodule
	Displays YUV data on the video plane.
	@param h handle on the device that has previously been successfully opened.
	@param pF pointer to a YUVframe structure that holds information for the 
	YUV data to be displayed.
	@return RM_OK if success, otherwise returns an error code.
*/
RMint32 RUA_DECODER_DISPLAY_YUV (RUA_handle h, YUVframe *pF); 

// The following are encoder specific APIs and are currently not 
// supported in all EM85xx based platforms.  
// These APIs require the SM2288 mpeg2 encoder asic.
// Please contact Sigma Designs if you wish to incorporate a MPEG2 encoder
// into your design.
// REALMAGICHWL_IOCTL_SM2288_DIAGNOSIS,
RMint32 RUA_SM2288_DIAGNOSIS (RUA_handle h, SM2288diagnosis_type test);
RMint32 RUA_ENCODER_RESET (RUA_handle h);
RMint32 RUA_ENCODER_START (RUA_handle h);
RMint32 RUA_ENCODER_PAUSE (RUA_handle h);
RMint32 RUA_ENCODER_RESUME (RUA_handle h);
RMint32 RUA_ENCODER_STOP (RUA_handle h);
RMint32 RUA_ENCODER_SETPARAMETERS (RUA_handle h, unsigned short params[SM2288_PARAMHSIZEINWORDS]);

RMint32 RUA_DSP_INIT(RUA_handle h);
RMint32 RUA_DSP_CODEC_INIT(RUA_handle h, RMuint32 codec_id, RMuint32 *params);
RMint32 RUA_DSP_CODEC_PLAY(RUA_handle h);
RMint32 RUA_DSP_CODEC_PAUSE(RUA_handle h);
RMint32 RUA_DSP_CODEC_STOP(RUA_handle h);
RMint32 RUA_DSP_FULLNESS(RUA_handle h);
RMint32 RUA_DSP_SET_AUDIO_PARAMS(RUA_handle h, dsp_audio_params *p);

#ifdef __cplusplus
}
#endif


#endif // __REALMAGICHWL_USERLAND_API_H__
