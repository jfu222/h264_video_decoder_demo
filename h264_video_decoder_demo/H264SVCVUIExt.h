//
// H264SVCVUIExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SVC_VUI_H__
#define __H264_SVC_VUI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264HrdParameters.h"


#define    VUI_EXT_MAX_NUM_ENTRIES    1024

/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 623/645/812
 * G.14.1 SVC VUI parameters extension syntax
 * SVC(Scalable video coding)
 */
class CH264SVCVUIExt
{
public:
    int32_t     vui_ext_num_entries_minus1; //0 ue(v)    vui_ext_num_entries_minus1 shall be in the range of 0 to 1023, inclusive.
    int32_t     vui_ext_dependency_id[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(3)
    int32_t     vui_ext_quality_id[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(4)
    int32_t     vui_ext_temporal_id[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(3)
    int32_t     vui_ext_timing_info_present_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    int32_t     vui_ext_num_units_in_tick[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(32)
    int32_t     vui_ext_time_scale[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(32)
    int32_t     vui_ext_fixed_frame_rate_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    int32_t     vui_ext_nal_hrd_parameters_present_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    int32_t     vui_ext_vcl_hrd_parameters_present_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    int32_t     vui_ext_low_delay_hrd_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    int32_t     vui_ext_pic_struct_present_flag[VUI_EXT_MAX_NUM_ENTRIES]; //0 u(1)
    
    CHrdParameters    m_vui_ext_hrd_parameter_nal;
    CHrdParameters    m_vui_ext_hrd_parameter_vcl;

public:
    CH264SVCVUIExt();
    ~CH264SVCVUIExt();

    int printInfo();
    
    int svc_vui_parameters_extension(CBitstream &bs);
};

#endif //__H264_SVC_VUI_H__
