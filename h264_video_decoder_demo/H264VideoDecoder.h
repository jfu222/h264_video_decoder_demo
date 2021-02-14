//
// H264VideoDecoder.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_VIDEO_DECODER_H__
#define __H264_VIDEO_DECODER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include "H264PicturesGOP.h"
#include "H264Picture.h"


//H264解码器，解码完一帧后的回调函数
typedef int (*output_frame_callback)(CH264Picture *outPicture, void *userData, int errorCode);


class CH264VideoDecoder
{
public:
    std::string              m_filename;
    output_frame_callback    m_output_frame_callback; //解码完毕的帧的回调函数，需要用户主动设置
    void *                   m_userData;

public:
    CH264VideoDecoder();
    ~CH264VideoDecoder();

    int init();
    int unInit();

    int set_output_frame_callback_functuin(output_frame_callback output_frame_callback, void *userData);

    int open(const char *url);
    int do_callback(CH264Picture *picture_current, CH264PicturesGOP *pictures_gop, int32_t is_need_flush);
};

#endif //__H264_VIDEO_DECODER_H__
