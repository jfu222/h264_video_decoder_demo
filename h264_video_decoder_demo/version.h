//
// version.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __VERSION_H__
#define __VERSION_H__


#define MAJOR_VERSION    1
#define MINOR1_VERSION   0
#define MINOR2_VERSION   0
#define MINOR3_VERSION   1

#define VERSION_DLL     MAJOR_VERSION,MINOR1_VERSION,MINOR2_VERSION,MINOR3_VERSION
#define VERSION_STR     MAJOR_VERSION.MINOR1_VERSION.MINOR2_VERSION.MINOR3_VERSION

#define VERSION_STR2(S)     #S
#define VERSION_STR3(S)     VERSION_STR2(S)


#endif //__VERSION_H__
