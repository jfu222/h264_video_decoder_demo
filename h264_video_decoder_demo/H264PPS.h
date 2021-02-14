//
// H264PPS.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_PPS_H__
#define __H264_PPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264VUI.h"
#include "H264SPS.h"


#define    H264_MAX_PPS_COUNT    256   // 7.4.2.2: pic_parameter_set_id shall be in the range of 0 to 255, inclusive.


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 46/68/812
 * 7.3.2.2 Picture parameter set RBSP syntax
 * #define    H264_NAL_PPS                  8
 */
class CH264PPS
{
public:
    int32_t     pic_parameter_set_id; // 1 ue(v)
    int32_t     seq_parameter_set_id; // 1 ue(v)
    int32_t     entropy_coding_mode_flag; // 1 u(1)
    int32_t     bottom_field_pic_order_in_frame_present_flag; // 1 u(1)
    int32_t     num_slice_groups_minus1; // 1 ue(v)
    int32_t     slice_group_map_type; // 1 ue(v)    [A.2.1 Baseline profile] num_slice_groups_minus1 in the range of 0 to 7, inclusive.
    int32_t     run_length_minus1[8]; //[ iGroup ] 1 ue(v)
    int32_t     top_left[8]; //[ iGroup ] 1 ue(v)
    int32_t     bottom_right[8]; //[ iGroup ] 1 ue(v)
    int32_t     slice_group_change_direction_flag; // 1 u(1)
    int32_t     slice_group_change_rate_minus1; // 1 ue(v)
    int32_t     pic_size_in_map_units_minus1; // 1 ue(v)
    int32_t  *  slice_group_id; //[ i ] 1 u(v)
    int32_t     num_ref_idx_l0_default_active_minus1; // 1 ue(v)
    int32_t     num_ref_idx_l1_default_active_minus1; // 1 ue(v)
    int32_t     weighted_pred_flag; // 1 u(1)
    int32_t     weighted_bipred_idc; // 1 u(2)
    int32_t     pic_init_qp_minus26; // /* relative to 26 */ 1 se(v)
    int32_t     pic_init_qs_minus26; // /* relative to 26 */ 1 se(v)
    int32_t     chroma_qp_index_offset; // 1 se(v)
    int32_t     deblocking_filter_control_present_flag; // 1 u(1)
    int32_t     constrained_intra_pred_flag; // 1 u(1)
    int32_t     redundant_pic_cnt_present_flag; // 1 u(1)
    int32_t     transform_8x8_mode_flag; // 1 u(1)
    int32_t     pic_scaling_matrix_present_flag; // 1 u(1)
    int32_t     pic_scaling_list_present_flag[12]; //[ i ] 1 u(1)
    int32_t     second_chroma_qp_index_offset; // 1 se(v)
    
    int32_t     ScalingList4x4[6][16];
    int32_t     ScalingList8x8[6][64];
    int32_t     UseDefaultScalingMatrix4x4Flag[6];
    int32_t     UseDefaultScalingMatrix8x8Flag[6];

public:
    CH264PPS();
    ~CH264PPS();

    int printInfo();
    
    CH264PPS & operator = (const CH264PPS &a);
    int copy(const CH264PPS &pps);
    int pic_parameter_set_rbsp(CBitstream &bs, CH264SPS (&m_spss)[32]);
};

#endif //__H264_PPS_H__
