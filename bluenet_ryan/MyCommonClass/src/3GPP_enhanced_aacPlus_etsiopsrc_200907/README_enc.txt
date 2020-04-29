===================================================================

                   3GPP Enhanced aacPlus encoder
           Fixed-Point Implemetation using ETSI operators
                           Linux/Win32
                       for Intel x86 CPUs

===================================================================

CONTENTS: Enhanced aacPlus encoder source code, 
          fixed-point ETSI operators


REVISION NOTES:

 - Contents:

   README.txt             -- this file
   Makefile               -- Linux Makefile
   enhAacPlusenc.dsw      -- Win32 MSVC 6.0 workspace
   enhAacPlusenc.dsp      -- Win32 MSVC 6.0 makefile

   src/                   -- directory for the frontend
   etsiop_bitbuf/         -- bitstream reading/writing library
   etsiop_fastaacenc/     -- AAC encoder library
   etsiop_sbrenc/         -- SBR encoder library
   etsiop_resamplib/      -- resampler library
   etsiop_ffrlib/         -- general purpose functionalities
   etsioplib/             -- ETSI operator library
 
   3g_lib/                -- precompiled MPEG-4 / 3GPP file format 
     	w32/                   and audio file library (for wav support),
  	linux/                 for Linux OS and Windows 32-bit



 - Compilation: 

   o for Win32 use the enhAacPlusenc.dsw 6.0 workspace, activate 
     the enhAacPlusEnc project and choose the appropriate 
     "Win32 Release/Debug" target

   o for Linux use >gmake clean all< in the root directory


 - Instrumentation:

   To measure the weighted mops, set WMOPS=1 on the Linux gmake
   commandline, respectively add the define WMOPS to the Win32 
   workspace makefiles.

 - MONO_ONLY

   This define is introduced to be able to compile a mono-only
   encoder. It is set by default when choosing the Mono compile
   targets in the Win32 workspace, resp. setting the MONO=1 option 
   on the Linux gmake commandline. Note that for the mono 
   executable only mono input wave files are supported.

 - bitrate switching simulation encoder frontend

   If you are interested in an encoder frontend which simulates
   bitrate switching you can use the brswitchmain.c file instead 
   of the main.c file to obtain such an executable.
   For Win32 you may disable the main.c file and enable the 
   brswitchmain.c file instead within your MSVC environment. 
   For Linux this alternative executable is included as a second 
   target and built automatically if the "all" target is select.

===================================================================
for info mailto: 3gpp-support@codingtechnologies.com
===================================================================
