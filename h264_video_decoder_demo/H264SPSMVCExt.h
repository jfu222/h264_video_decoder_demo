//
// H264SPSMVCExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SPS_MVC_EXT_H__
#define __H264_SPS_MVC_EXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"


#define    H264_MAX_NUM_VIEWS    1024
#define    H264_MAX_NUM_LEVEL_VALUES_SIGNALLED    64

/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 630/652/812
 * H.7.3.2.1.4 Sequence parameter set MVC extension syntax
 * MVC(Multiview video coding)
 */
class CH264SPSMVCExt
{
public:
    int32_t     num_views_minus1; //0 ue(v)    num_view_minus1 shall be in the range of 0 to 1023, inclusive.
    int32_t     view_id[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     num_anchor_refs_l0[H264_MAX_NUM_VIEWS]; //0 ue(v)    num_anchor_refs_l0[ i ] shall not be greater than Min( 15, num_views_minus1 ). The value of num_anchor_refs_l0[ 0 ] shall be equal to 0.
    int32_t     anchor_ref_l0[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_anchor_refs_l1[H264_MAX_NUM_VIEWS]; //0 ue(v)    num_anchor_refs_l0[ i ] shall not be greater than Min( 15, num_views_minus1 ). The value of num_anchor_refs_l0[ 0 ] shall be equal to 0.
    int32_t     anchor_ref_l1[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_non_anchor_refs_l0[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     non_anchor_ref_l0[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_non_anchor_refs_l1[H264_MAX_NUM_VIEWS]; //0 ue(v)
    int32_t     non_anchor_ref_l1[H264_MAX_NUM_VIEWS][16]; //0 ue(v)
    int32_t     num_level_values_signalled_minus1; //0 ue(v)    num_level_values_signalled_minus1 shall be in the range of 0 to 63, inclusive.
    int32_t     level_idc[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED]; //0 u(8)
    int32_t     num_applicable_ops_minus1[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED]; //0 ue(v)    num_applicable_ops_minus1[ i ] shall be in the range of 0 to 1023, inclusive.
    int32_t     applicable_op_temporal_id[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED][1024]; //0 u(3)
    int32_t     applicable_op_num_target_views_minus1[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED][1024]; //0 ue(v)
    int32_t     applicable_op_target_view_id[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED][1024][1024]; //0 ue(v)
    int32_t     applicable_op_num_views_minus1[H264_MAX_NUM_LEVEL_VALUES_SIGNALLED][1024]; //0 ue(v)
    int32_t     mfc_format_idc; //0 u(6)
    int32_t     default_grid_position_flag; //0 u(1)
    int32_t     view0_grid_position_x; //0 u(4)
    int32_t     view0_grid_position_y; //0 u(4)
    int32_t     view1_grid_position_x; //0 u(4)
    int32_t     view1_grid_position_y; //0 u(4)
    int32_t     rpu_filter_enabled_flag; //0 u(1)
    int32_t     rpu_field_processing_flag; //0 u(1)

public:
    CH264SPSMVCExt();
    ~CH264SPSMVCExt();

    int printInfo();
    
    int seq_parameter_set_mvc_extension(CBitstream &bs, int profile_idc, int frame_mbs_only_flag);
};

#endif //__H264_SPS_MVC_EXT_H__