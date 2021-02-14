//
// H264MVCVUIExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_MVC_VUI_EXT_H__
#define __H264_MVC_VUI_EXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264HrdParameters.h"


#define    H264_MAX_VUI_MVC_NUM_OPS    1024

/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 687/709/812
 * H.14.1 MVC VUI parameters extension syntax
 * MVC(Multiview video coding)
 */
class CH264MVCVUIExt
{
public:
    int32_t     vui_mvc_num_ops_minus1; //0 ue(v)    vui_mvc_num_ops_minus1 shall be in the range of 0 to 1023, inclusive.
    int32_t     vui_mvc_temporal_id[H264_MAX_VUI_MVC_NUM_OPS]; //0 ue(v)
    int32_t     vui_mvc_num_target_output_views_minus1[H264_MAX_VUI_MVC_NUM_OPS]; //0 ue(v)
    int32_t     vui_mvc_view_id[H264_MAX_VUI_MVC_NUM_OPS][1024]; //0 ue(v)
    int32_t     vui_mvc_timing_info_present_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)
    int32_t     vui_mvc_num_units_in_tick[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(32)
    int32_t     vui_mvc_time_scale[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(32)
    int32_t     vui_mvc_fixed_frame_rate_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)
    int32_t     vui_mvc_nal_hrd_parameters_present_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)
    int32_t     vui_mvc_vcl_hrd_parameters_present_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)
    int32_t     vui_mvc_low_delay_hrd_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)
    int32_t     vui_mvc_pic_struct_present_flag[H264_MAX_VUI_MVC_NUM_OPS]; //0 u(1)

    CHrdParameters    m_vui_mvc_nal_hrd_parameters;
    CHrdParameters    m_vui_vcl_nal_hrd_parameters;

public:
    CH264MVCVUIExt();
    ~CH264MVCVUIExt();

    int printInfo();
    
    int mvc_vui_parameters_extension(CBitstream &bs);
};

#endif //__H264_MVC_VUI_EXT_H__
