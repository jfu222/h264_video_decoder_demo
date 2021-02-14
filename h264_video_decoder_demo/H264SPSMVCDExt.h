//
// H264SPSMVCDExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SPS_MVCD_EXT_H__
#define __H264_SPS_MVCD_EXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264MVCDVUIExt.h"


#define    H264_MAX_NUM_VIEWS    1024

/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 692/714/812
 * I.7.3.2.1.5 Sequence parameter set MVCD extension syntax
 * MVC(Multiview video coding)
 */
class CH264SPSMVCDExt
{
public:
    int32_t     num_views_minus1; //0 ue(v)    num_view_minus1 shall be in the range of 0 to 1023, inclusive.
    int32_t     view_id[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     depth_view_present_flag[H264_MAX_NUM_VIEWS]; //0 u(1)
    int32_t     DepthViewId[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     texture_view_present_flag[H264_MAX_NUM_VIEWS]; //0 u(1)
    int32_t     num_anchor_refs_l0[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     anchor_ref_l0[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_anchor_refs_l1[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     anchor_ref_l1[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_non_anchor_refs_l0[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     non_anchor_ref_l0[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_non_anchor_refs_l1[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     non_anchor_ref_l1[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_level_values_signalled_minus1; //0 ue(v)
    int32_t     level_idc[64]; //0 u(8)
    int32_t     num_applicable_ops_minus1[64]; //0 ue(v)
    int32_t     applicable_op_temporal_id[64][1024]; //0 u(3)
    int32_t     applicable_op_num_target_views_minus1[64][1024]; //0 ue(v)
    int32_t     applicable_op_target_view_id[64][1024][1024]; //0 ue(v)
    int32_t     applicable_op_depth_flag[64][1024][1024]; //0 u(1)
    int32_t     applicable_op_texture_flag[64][1024][1024]; //0 u(1)
    int32_t     applicable_op_num_texture_views_minus1[64][1024]; //0 ue(v)
    int32_t     applicable_op_num_depth_views[64][1024]; //0 ue(v)
    int32_t     mvcd_vui_parameters_present_flag; //0 u(1)
    int32_t     texture_vui_parameters_present_flag; //0 u(1)

    CH264MVCDVUIExt    m_mvcd_vui;
    CH264MVCDVUIExt    m_texture_vui;

public:
    CH264SPSMVCDExt();
    ~CH264SPSMVCDExt();

    int printInfo();
    
    int seq_parameter_set_mvcd_extension(CBitstream &bs);
};

#endif //__H264_SPS_MVCD_EXT_H__
