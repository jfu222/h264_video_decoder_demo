//
// H264VUI.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_VUI_H__
#define __H264_VUI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include "Bitstream.h"
#include "H264HrdParameters.h"


class CH264VUI
{
public:
    int32_t     aspect_ratio_info_present_flag;
    int32_t     aspect_ratio_idc;
    int32_t     sar_width;
    int32_t     sar_height;
    int32_t     overscan_info_present_flag;
    int32_t     overscan_appropriate_flag;
    int32_t     video_signal_type_present_flag;
    int32_t     video_format;
    int32_t     video_full_range_flag;
    int32_t     colour_description_present_flag;
    int32_t     colour_primaries;
    int32_t     transfer_characteristics;
    int32_t     matrix_coefficients;
    int32_t     chroma_loc_info_present_flag;
    int32_t     chroma_sample_loc_type_top_field;
    int32_t     chroma_sample_loc_type_bottom_field;
    int32_t     timing_info_present_flag;
    int32_t     num_units_in_tick; //the number of time units of a clock operating at the frequency time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter.
    int32_t     time_scale; //the number of time units that pass in one second.
    int32_t     fixed_frame_rate_flag;
    int32_t     nal_hrd_parameters_present_flag;
    int32_t     vcl_hrd_parameters_present_flag;
    int32_t     low_delay_hrd_flag;
    int32_t     pic_struct_present_flag;
    int32_t     bitstream_restriction_flag;
    int32_t     motion_vectors_over_pic_boundaries_flag;
    int32_t     max_bytes_per_pic_denom;
    int32_t     max_bits_per_mb_denom;
    int32_t     log2_max_mv_length_horizontal;
    int32_t     log2_max_mv_length_vertical;
    int32_t     max_num_reorder_frames; //indicates an upper bound for the number of frames buffers, in the decoded picture buffer (DPB). 大于等于2表示含有B帧
    int32_t     max_dec_frame_buffering;

    CHrdParameters    m_hrd_parameter_nal;
    CHrdParameters    m_hrd_parameter_vcl;

public:
    CH264VUI();
    ~CH264VUI();
    
    int printInfo();

    int vui_parameters(CBitstream &bs);
};

#endif //__H264_VUI_H__
