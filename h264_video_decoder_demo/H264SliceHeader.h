//
// H264SliceHeader.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SLICE_HEADER_H__
#define __H264_SLICE_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "H264NalUnit.h"
#include "H264SPS.h"
#include "H264PPS.h"
#include "H264CommonFunc.h"


typedef struct _DEC_REF_PIC_MARKING_
{
    int32_t     memory_management_control_operation; // 2 | 5 ue(v)
    int32_t     difference_of_pic_nums_minus1; // 2 | 5 ue(v)
    int32_t     long_term_pic_num_2; // 2 | 5 ue(v)
    int32_t     long_term_frame_idx; // 2 | 5 ue(v)
    int32_t     max_long_term_frame_idx_plus1; // 2 | 5 ue(v)
}DEC_REF_PIC_MARKING;


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 50/72/812
 * 7.3.3 Slice header syntax
 */
class CH264SliceHeader
{
public:
    int32_t     first_mb_in_slice; // 2 ue(v)
    int32_t     slice_type; // 2 ue(v)
    int32_t     pic_parameter_set_id; // 2 ue(v)
    int32_t     colour_plane_id; // 2 u(2)
    int32_t     frame_num; // 2 u(v)
    int32_t     field_pic_flag; // 2 u(1)
    int32_t     bottom_field_flag; // 2 u(1)
    int32_t     idr_pic_id; // 2 ue(v)
    int32_t     pic_order_cnt_lsb; // 2 u(v)
    int32_t     delta_pic_order_cnt_bottom; // 2 se(v)
    int32_t     delta_pic_order_cnt[2]; // 2 se(v)
    int32_t     redundant_pic_cnt; // 2 ue(v)
    int32_t     direct_spatial_mv_pred_flag; // 2 u(1)
    int32_t     num_ref_idx_active_override_flag; // 2 u(1)
    int32_t     num_ref_idx_l0_active_minus1; // 2 ue(v) [0,31]
    int32_t     num_ref_idx_l1_active_minus1; // 2 ue(v) [0,31]
    int32_t     cabac_init_idc; // 2 ue(v)
    int32_t     slice_qp_delta; // 2 se(v)
    int32_t     sp_for_switch_flag; // 2 u(1)
    int32_t     slice_qs_delta; // 2 se(v)
    int32_t     disable_deblocking_filter_idc; // 2 ue(v) 0：开启环路滤波，滤波可跨越slice边界。1：关闭环路滤波。2：开启环路滤波，只针对同一个slice滤波。
    int32_t     slice_alpha_c0_offset_div2; // 2 se(v)
    int32_t     slice_beta_offset_div2; // 2 se(v)
    int32_t     slice_group_change_cycle; // 2 u(v)

    // ref_pic_list_modification
    int32_t     ref_pic_list_modification_flag_l0; // 2 u(1)
    int32_t     modification_of_pic_nums_idc[2][32]; // 2 ue(v)
    int32_t     abs_diff_pic_num_minus1[2][32]; // 2 ue(v)
    int32_t     long_term_pic_num[2][32]; // 2 ue(v)
    int32_t     ref_pic_list_modification_flag_l1; // 2 u(1)
    int32_t     ref_pic_list_modification_count_l0; //modification_of_pic_nums_idc[0]数组大小
    int32_t     ref_pic_list_modification_count_l1; //modification_of_pic_nums_idc[1]数组大小
    
    // pred_weight_table
    int32_t     luma_log2_weight_denom; // 2 ue(v)
    int32_t     chroma_log2_weight_denom; // 2 ue(v)
    int32_t     luma_weight_l0_flag; // 2 u(1)
    int32_t     luma_weight_l0[32]; // 2 se(v)
    int32_t     luma_offset_l0[32]; // 2 se(v)
    int32_t     chroma_weight_l0_flag; // 2 ue(v)
    int32_t     chroma_weight_l0[32][2]; // 2 se(v)
    int32_t     chroma_offset_l0[32][2]; // 2 se(v)
    int32_t     luma_weight_l1_flag; // 2 u(1)
    int32_t     luma_weight_l1[32]; // 2 se(v)
    int32_t     luma_offset_l1[32]; // 2 se(v)
    int32_t     chroma_weight_l1_flag; // 2 u(1)
    int32_t     chroma_weight_l1[32][2]; // 2 se(v)
    int32_t     chroma_offset_l1[32][2]; // 2 se(v)

    // dec_ref_pic_marking
    int32_t     no_output_of_prior_pics_flag; // 2 | 5 u(1)
    int32_t     long_term_reference_flag; // 2 | 5 u(1)
    int32_t     adaptive_ref_pic_marking_mode_flag; // 2 | 5 u(1)
    DEC_REF_PIC_MARKING    m_dec_ref_pic_marking[32];
    int32_t     dec_ref_pic_marking_count; //m_dec_ref_pic_marking[]数组大小
    
    CH264NalUnit    m_nal_unit;
    CH264SPS        m_sps;
    CH264PPS        m_pps;
    
    int32_t         slice_id;
    int32_t         syntax_element_categories; // 2 | 3 | 4
    int32_t         slice_type_fixed;
    int32_t         mb_cnt; //用于计数已经解码了多少个宏块，范围[0, PicSizeInMbs-1]
    int32_t         QPY_prev; //QPY,PREV 是当前 slice 中前一宏块的量化参数 QPY 的值，在每个 slice 的开始处，对于 slice 中的第一个宏块，QPY,PREV 应该被初始化成等式 7-16 中的 SliceQPY 值。
    int32_t         SliceQPY; //SliceQPY = 26 + pic_init_qp_minus26 + slice_qp_delta;
    int32_t         MbaffFrameFlag; //MbaffFrameFlag = ( mb_adaptive_frame_field_flag && !field_pic_flag );
    int32_t         PicHeightInMbs; //PicHeightInMbs = FrameHeightInMbs / ( 1 + field_pic_flag );
    int32_t         PicHeightInSamplesL; //PicHeightInSamplesL = PicHeightInMbs * 16; //解码后图片的高度（单位：像素）
    int32_t         PicHeightInSamplesC; //PicHeightInSamplesC = PicHeightInMbs * MbHeightC;
    int32_t         PicSizeInMbs; //PicSizeInMbs = PicWidthInMbs * PicHeightInMbs;
    int32_t         MaxPicNum; //MaxPicNum = (field_pic_flag == 0) ? MaxFrameNum : (2 * MaxFrameNum);
    int32_t         CurrPicNum; //CurrPicNum = (field_pic_flag == 0) ? frame_num : (2 * frame_num + 1);
    int32_t         SliceGroupChangeRate; //SliceGroupChangeRate = slice_group_change_rate_minus1 + 1;
    int32_t         MapUnitsInSliceGroup0; //MapUnitsInSliceGroup0 = Min( slice_group_change_cycle * SliceGroupChangeRate, PicSizeInMapUnits );
    int32_t         QSY; //QSY = 26 + pic_init_qs_minus26 + slice_qs_delta
    int32_t         picNumL0Pred;
    int32_t         picNumL1Pred;
    int32_t         refIdxL0;
    int32_t         refIdxL1;

    int32_t  *      mapUnitToSliceGroupMap; //MbToSliceGroupMap[PicSizeInMapUnits];
    int32_t  *      MbToSliceGroupMap; //MbToSliceGroupMap[PicSizeInMbs];
    int32_t         PrevRefFrameNum;
    int32_t         UnusedShortTermFrameNum;
    int32_t         FilterOffsetA;
    int32_t         FilterOffsetB;
    
    int32_t         ScalingList4x4[6][16];
    int32_t         ScalingList8x8[6][64];
    H264_PICTURE_CODED_TYPE    m_picture_coded_type;
    int32_t                    m_is_malloc_mem_self; //是否已经初始化
    
public:
    CH264SliceHeader();
    ~CH264SliceHeader();
    
    CH264SliceHeader & operator = (const CH264SliceHeader &src); //重载等号运算符
    int copyData(const CH264SliceHeader &src, bool isCopyYuvData);

    int printInfo();
    
    int slice_header(CBitstream &bs, const CH264NalUnit &nal_unit, const CH264SPS spss[32], const CH264PPS ppss[256]);
    int ref_pic_list_modification(CBitstream &bs);
    int pred_weight_table(CBitstream &bs);
    int dec_ref_pic_marking(CBitstream &bs);
    int setMbToSliceGroupMap();
    int setMapUnitToSliceGroupMap();
    int set_scaling_lists_values(const CH264SPS &sps, const CH264PPS &pps);

    bool is_first_VCL_NAL_unit_of_a_picture(const CH264SliceHeader &lastSliceHeader);
};

#endif //__H264_SLICE_HEADER_H__
