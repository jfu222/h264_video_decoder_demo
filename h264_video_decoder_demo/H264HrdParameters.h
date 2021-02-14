//
// H264HrdParameters.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_HRD_PARAMETERS_H__
#define __H264_HRD_PARAMETERS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"


#define    H264_MAX_CPB_CNT    32    // E.2.2: cpb_cnt_minus1 is in [0, 31].
#define    Extended_SAR        255


class CHrdParameters
{
public:
    int32_t     cpb_cnt_minus1;
    int32_t     bit_rate_scale;
    int32_t     cpb_size_scale;
    int32_t     bit_rate_value_minus1[H264_MAX_CPB_CNT];
    int32_t     cpb_size_value_minus1[H264_MAX_CPB_CNT];
    int32_t     cbr_flag[H264_MAX_CPB_CNT];
    int32_t     initial_cpb_removal_delay_length_minus1;
    int32_t     cpb_removal_delay_length_minus1;
    int32_t     dpb_output_delay_length_minus1;
    int32_t     time_offset_length;

public:
    CHrdParameters();
    ~CHrdParameters();

    int printInfo();

    int hrd_parameters(CBitstream &bs);
};

#endif //__H264_HRD_PARAMETERS_H__
