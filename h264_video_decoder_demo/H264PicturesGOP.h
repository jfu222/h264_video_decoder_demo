//
// H264PicturesGOP.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_PICTURES_GOP_H__
#define __H264_PICTURES_GOP_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "H264Picture.h"
#include "H264SPSExt.h"
#include "H264SEI.h"


class CH264PicturesGOP
{
public:
    CH264SPS            m_spss[H264_MAX_SPS_COUNT]; //sps[32]
    CH264SPSExt         m_sps_ext;
    CH264PPS            m_ppss[H264_MAX_PPS_COUNT]; //pps[256]
    CH264SEI            m_sei;
    
    CH264Picture *      m_DecodedPictureBuffer[H264_MAX_DECODED_PICTURE_BUFFER_COUNT]; //DPB: decoded picture buffer
    CH264Picture *      m_dpb_for_output[H264_MAX_DECODED_PICTURE_BUFFER_COUNT]; //因为含有B帧的视频帧的显示顺序和解码顺序是不一样的，已经解码完的P/B帧不能立即输出给用户，需要先缓存一下，
    int32_t             m_dpb_for_output_length; //m_dpb_index_for_output[]数组的真实大小，此值是动态变化的，取值范围[0, max_num_reorder_frames-1]
    int32_t             max_num_reorder_frames; //m_dpb_index_for_output[]数组的最大大小，来源于m_spss[i].m_vui.max_num_reorder_frames，对于含B帧的视频，此值一般等于2

    CH264Picture        m_picture_previous_reference;
    CH264Picture        m_picture_previous;

    int32_t             m_gop_size;

public:
    CH264PicturesGOP();
    ~CH264PicturesGOP();
    
    int init();
    int unInit();

    int getOneEmptyPicture(CH264Picture *&pic);
    int getOneOutPicture(CH264Picture *newDecodedPic, CH264Picture *&outPic);
};

#endif //__H264_PICTURES_GOP_H__
