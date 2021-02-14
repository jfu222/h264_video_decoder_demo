//
// H264SPS.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SPS_H__
#define __H264_SPS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264VUI.h"


#define    H264_MAX_SPS_COUNT    32    // 7.4.2.1.1: seq_parameter_set_id shall be in the range of 0 to 31, inclusive.
#define    H264_MAX_OFFSET_REF_FRAME_COUNT    256    // 7.4.2.1.1: num_ref_frames_in_pic_order_cnt_cycle shall be in the range of 0 to 255, inclusive.

#define    MONOCHROME               0    //黑白图像
#define    CHROMA_FORMAT_IDC_420    1
#define    CHROMA_FORMAT_IDC_422    2
#define    CHROMA_FORMAT_IDC_444    3

struct CHROMA_FORMAT_IDC_T
{
    int32_t     chroma_format_idc;
    int32_t     separate_colour_plane_flag;
    int32_t     Chroma_Format;
    int32_t     SubWidthC;
    int32_t     SubHeightC;
};


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 43/64/812
 * 7.3.2.1.1 Sequence parameter set data syntax
 * #define    H264_NAL_SPS                  7
 */
class CH264SPS
{
public:
    int32_t     profile_idc; // 0 u(8)
    int32_t     constraint_set0_flag; // 0 u(1)
    int32_t     constraint_set1_flag; // 0 u(1)
    int32_t     constraint_set2_flag; // 0 u(1)
    int32_t     constraint_set3_flag; // 0 u(1)
    int32_t     constraint_set4_flag; // 0 u(1)
    int32_t     constraint_set5_flag; // 0 u(1)
    int32_t     reserved_zero_2bits; // /* equal to 0 */ 0 u(2)
    int32_t     level_idc; // 0 u(8)
    int32_t     seq_parameter_set_id; // 0 ue(v)
    int32_t     chroma_format_idc; // 0 ue(v)
    int32_t     separate_colour_plane_flag; // 0 u(1)
    int32_t     bit_depth_luma_minus8; // 0 ue(v) When bit_depth_luma_minus8 is not present, it shall be inferred to be equal to 0. bit_depth_luma_minus8 shall be in the range of 0 to 6, inclusive.
    int32_t     bit_depth_chroma_minus8; // 0 ue(v)
    int32_t     qpprime_y_zero_transform_bypass_flag; // 0 u(1)
    int32_t     seq_scaling_matrix_present_flag; // 0 u(1)
    int32_t     seq_scaling_list_present_flag[12]; //seq_scaling_list_present_flag[ i ] 0 u(1)
    int32_t     log2_max_frame_num_minus4; // 0 ue(v)
    int32_t     pic_order_cnt_type; // 0 ue(v)
    int32_t     log2_max_pic_order_cnt_lsb_minus4; // 0 ue(v)
    int32_t     delta_pic_order_always_zero_flag; // 0 u(1)
    int32_t     offset_for_non_ref_pic; // 0 se(v)
    int32_t     offset_for_top_to_bottom_field; // 0 se(v)
    int32_t     num_ref_frames_in_pic_order_cnt_cycle; // 0 ue(v)
    int32_t     offset_for_ref_frame[H264_MAX_OFFSET_REF_FRAME_COUNT]; //offset_for_ref_frame[ num_ref_frames_in_pic_order_cnt_cycle ] 0 se(v)
    int32_t     max_num_ref_frames; // 0 ue(v)
    int32_t     gaps_in_frame_num_value_allowed_flag; // 0 u(1)
    int32_t     pic_width_in_mbs_minus1; // 0 ue(v)
    int32_t     pic_height_in_map_units_minus1; // 0 ue(v)
    int32_t     frame_mbs_only_flag; // 0 u(1)
    int32_t     mb_adaptive_frame_field_flag; // 0 u(1)
    int32_t     direct_8x8_inference_flag; // 0 u(1)
    int32_t     frame_cropping_flag; // 0 u(1)
    int32_t     frame_crop_left_offset; // 0 ue(v)
    int32_t     frame_crop_right_offset; // 0 ue(v)
    int32_t     frame_crop_top_offset; // 0 ue(v)
    int32_t     frame_crop_bottom_offset; // 0 ue(v)
    int32_t     vui_parameters_present_flag; // 0 u(1)

    int32_t     ScalingList4x4[6][16];
    int32_t     ScalingList8x8[6][64];
    int32_t     UseDefaultScalingMatrix4x4Flag[6];
    int32_t     UseDefaultScalingMatrix8x8Flag[6];

    CH264VUI    m_vui;
    
    int32_t     PicWidthInMbs; // PicWidthInMbs = pic_width_in_mbs_minus1 + 1;
    int32_t     FrameHeightInMbs; //FrameHeightInMbs = ( 2 - frame_mbs_only_flag ) * PicHeightInMapUnits;
    int32_t     PicHeightInMapUnits; //PicHeightInMapUnits = pic_height_in_map_units_minus1 + 1;
    int32_t     PicSizeInMapUnits; //PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits;
    int32_t     ChromaArrayType;

    int32_t     BitDepthY; //BitDepthY = 8 + bit_depth_luma_minus8;
    int32_t     QpBdOffsetY; //QpBdOffsetY = 6 * bit_depth_luma_minus8;
    int32_t     BitDepthC; //BitDepthC = 8 + bit_depth_chroma_minus8;
    int32_t     QpBdOffsetC; //QpBdOffsetC = 6 * bit_depth_chroma_minus8;
    
    int32_t     SubWidthC;
    int32_t     SubHeightC;
    int32_t     MbWidthC; //MbWidthC = 16 / SubWidthC;
    int32_t     MbHeightC; //MbHeightC = 16 / SubHeightC;
    int32_t     Chroma_Format;

    int32_t     PicWidthInSamplesL; //PicWidthInSamplesL = PicWidthInMbs * 16; //解码后图片的宽度（单位：像素）
    int32_t     PicWidthInSamplesC; //PicWidthInSamplesC = PicWidthInMbs * MbWidthC;
    int32_t     RawMbBits; //RawMbBits = 256 * BitDepthY + 2 * MbWidthC * MbHeightC * BitDepthC;
    int32_t     MaxFrameNum; //MaxFrameNum = 2 ^ ( log2_max_frame_num_minus4 + 4 );
    int32_t     MaxPicOrderCntLsb; //MaxPicOrderCntLsb = 2 ^ ( log2_max_pic_order_cnt_lsb_minus4 + 4 );
    int32_t     ExpectedDeltaPerPicOrderCntCycle;
    float       fps; //frame_rate = time_scale / num_units_in_tick;

public:
    CH264SPS();
    ~CH264SPS();

    int printInfo();

    int seq_parameter_set_data(CBitstream &bs);
    int get_h264_profile(char str[100]);
};

#endif //__H264_SPS_H__
