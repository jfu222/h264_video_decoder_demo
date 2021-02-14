//
// H264PPS.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264PPS.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"


CH264PPS::CH264PPS()
{
    pic_parameter_set_id = 0;
    seq_parameter_set_id = 0;
    entropy_coding_mode_flag = 0;
    bottom_field_pic_order_in_frame_present_flag = 0;
    num_slice_groups_minus1 = 0;
    slice_group_map_type = 0;
    memset(run_length_minus1, 0, sizeof(int32_t) * 8);
    memset(top_left, 0, sizeof(int32_t) * 8);
    memset(bottom_right, 0, sizeof(int32_t) * 8);
    slice_group_change_direction_flag = 0;
    slice_group_change_rate_minus1 = 0;
    pic_size_in_map_units_minus1 = 0;
    slice_group_id = NULL;
    num_ref_idx_l0_default_active_minus1 = 0;
    num_ref_idx_l1_default_active_minus1 = 0;
    weighted_pred_flag = 0;
    weighted_bipred_idc = 0;
    pic_init_qp_minus26 = 0;
    pic_init_qs_minus26 = 0;
    chroma_qp_index_offset = 0;
    deblocking_filter_control_present_flag = 0;
    constrained_intra_pred_flag = 0;
    redundant_pic_cnt_present_flag = 0;
    transform_8x8_mode_flag = 0;
    pic_scaling_matrix_present_flag = 0;
    memset(pic_scaling_list_present_flag, 0, sizeof(int32_t) * 12);
    second_chroma_qp_index_offset = 0;
    memset(ScalingList4x4, 0, sizeof(int32_t) * 6 * 16);
    memset(ScalingList8x8, 0, sizeof(int32_t) * 6 * 64);
}


CH264PPS::~CH264PPS()
{
    if (slice_group_id)
    {
        my_free(slice_group_id);
        slice_group_id = NULL;
    }
}


int CH264PPS::printInfo()
{
    printf("---------PPS info------------\n");
    printf("pic_parameter_set_id=%d;\n", pic_parameter_set_id);
    printf("seq_parameter_set_id=%d;\n", seq_parameter_set_id);
    printf("entropy_coding_mode_flag=%d;\n", entropy_coding_mode_flag);
    printf("bottom_field_pic_order_in_frame_present_flag=%d;\n", bottom_field_pic_order_in_frame_present_flag);
    printf("num_slice_groups_minus1=%d;\n", num_slice_groups_minus1);
    printf("slice_group_map_type=%d;\n", slice_group_map_type);

    printf("run_length_minus1[0..7]: ");
    for (int i = 0; i< 8; ++i)
    {
        printf("%d ", run_length_minus1[i]);
    }
    printf("\n");
    
    printf("top_left[0..7]: ");
    for (int i = 0; i< 8; ++i)
    {
        printf("%d ", top_left[i]);
    }
    printf("\n");

    printf("bottom_right[0..7]: ");
    for (int i = 0; i< 8; ++i)
    {
        printf("%d ", bottom_right[i]);
    }
    printf("\n");

    printf("slice_group_change_direction_flag=%d;\n", slice_group_change_direction_flag);
    printf("slice_group_change_rate_minus1=%d;\n", slice_group_change_rate_minus1);
    printf("pic_size_in_map_units_minus1=%d;\n", pic_size_in_map_units_minus1);
    printf("slice_group_id=0x%p;\n", slice_group_id);
    printf("num_ref_idx_l0_default_active_minus1=%d;\n", num_ref_idx_l0_default_active_minus1);
    printf("num_ref_idx_l1_default_active_minus1=%d;\n", num_ref_idx_l1_default_active_minus1);
    printf("weighted_pred_flag=%d;\n", weighted_pred_flag);
    printf("weighted_bipred_idc=%d;\n", weighted_bipred_idc);
    printf("pic_init_qp_minus26=%d;\n", pic_init_qp_minus26);
    printf("pic_init_qs_minus26=%d;\n", pic_init_qs_minus26);
    printf("chroma_qp_index_offset=%d;\n", chroma_qp_index_offset);
    printf("deblocking_filter_control_present_flag=%d;\n", deblocking_filter_control_present_flag);
    printf("constrained_intra_pred_flag=%d;\n", constrained_intra_pred_flag);
    printf("redundant_pic_cnt_present_flag=%d;\n", redundant_pic_cnt_present_flag);
    printf("transform_8x8_mode_flag=%d;\n", transform_8x8_mode_flag);
    printf("pic_scaling_matrix_present_flag=%d;\n", pic_scaling_matrix_present_flag);
    
    printf("pic_scaling_list_present_flag[0..7]: ");
    for (int i = 0; i< 12; ++i)
    {
        printf("%d ", pic_scaling_list_present_flag[i]);
    }
    printf("\n");

    printf("second_chroma_qp_index_offset=%d;\n", second_chroma_qp_index_offset);

    return 0;
}


CH264PPS & CH264PPS::operator = (const CH264PPS &a)
{
	int ret = copy(a);

	return *this;
}


int CH264PPS::copy(const CH264PPS &pps)
{
    memcpy(this, &pps, sizeof(CH264PPS));

    if (pps.slice_group_id && pps.pic_size_in_map_units_minus1 >= 0)
    {
        this->slice_group_id = (int32_t *)my_malloc(pps.pic_size_in_map_units_minus1 + 1);
        RETURN_IF_FAILED(this->slice_group_id == NULL, -1);
        memcpy(this->slice_group_id, pps.slice_group_id, sizeof(int32_t) * pps.pic_size_in_map_units_minus1 + 1);
    }

    return 0;
}


int CH264PPS::pic_parameter_set_rbsp(CBitstream &bs, CH264SPS (&m_spss)[32])
{
    int ret = 0;
    
    int32_t i = 0;
    int32_t iGroup = 0;
    CH264Golomb gb;

    pic_parameter_set_id = gb.get_ue_golomb(bs); //1 ue(v)
    seq_parameter_set_id = gb.get_ue_golomb(bs); //1 ue(v)
    entropy_coding_mode_flag = bs.readBits(1); //1 u(1)
    bottom_field_pic_order_in_frame_present_flag = bs.readBits(1); //1 u(1)
    num_slice_groups_minus1 = gb.get_ue_golomb(bs); //1 ue(v)
    if (num_slice_groups_minus1 > 0)
    {
        slice_group_map_type = gb.get_ue_golomb(bs); //1 ue(v)
        if (slice_group_map_type == 0)
        {
            for (iGroup = 0; iGroup <= num_slice_groups_minus1; iGroup++)
            {
                run_length_minus1[ iGroup ] = gb.get_ue_golomb(bs); //1 ue(v)
            }
        }
        else if (slice_group_map_type == 2)
        {
            for (iGroup = 0; iGroup < num_slice_groups_minus1; iGroup++)
            {
                top_left[ iGroup ] = gb.get_ue_golomb(bs); //1 ue(v)
                bottom_right[ iGroup ] = gb.get_ue_golomb(bs); //1 ue(v)
            }
        }
        else if (slice_group_map_type == 3 || slice_group_map_type == 4 || slice_group_map_type == 5)
        {
            slice_group_change_direction_flag = bs.readBits(1); //1 u(1)
            slice_group_change_rate_minus1 = gb.get_ue_golomb(bs); //1 ue(v)
        }
        else if (slice_group_map_type == 6)
        {
            pic_size_in_map_units_minus1 = gb.get_ue_golomb(bs); //1 ue(v)
            slice_group_id = (int32_t  *)my_malloc(pic_size_in_map_units_minus1 + 1); // PicSizeInMapUnits − 1 = PicWidthInMbs * PicHeightInMapUnits - 1;
            int v = h264_log2(num_slice_groups_minus1 + 1); //Ceil( Log2( num_slice_groups_minus1 + 1 ) );
            for (i = 0; i <= pic_size_in_map_units_minus1; i++)
            {
                slice_group_id[ i ] = bs.readBits(v); //1 u(v)    v = Ceil( Log2( num_slice_groups_minus1 + 1 ) ) bits.
            }
        }
    }
    num_ref_idx_l0_default_active_minus1 = gb.get_ue_golomb(bs); //1 ue(v)
    num_ref_idx_l1_default_active_minus1 = gb.get_ue_golomb(bs); //1 ue(v)
    weighted_pred_flag = bs.readBits(1); //1 u(1)
    weighted_bipred_idc = bs.readBits(2); //1 u(2)
    pic_init_qp_minus26 = gb.get_se_golomb(bs); // /* relative to 26 */ 1 se(v)
    pic_init_qs_minus26 = gb.get_se_golomb(bs); // /* relative to 26 */ 1 se(v)
    chroma_qp_index_offset = gb.get_se_golomb(bs); //1 se(v)
    deblocking_filter_control_present_flag = bs.readBits(1); //1 u(1)
    constrained_intra_pred_flag = bs.readBits(1); //1 u(1)
    redundant_pic_cnt_present_flag = bs.readBits(1); //1 u(1)

    second_chroma_qp_index_offset = chroma_qp_index_offset; //When second_chroma_qp_index_offset is not present, it shall be inferred to be equal to chroma_qp_index_offset.
    
    if (more_rbsp_data(bs))
    {
        transform_8x8_mode_flag = bs.readBits(1); //1 u(1)
        pic_scaling_matrix_present_flag = bs.readBits(1); //1 u(1)
        if (pic_scaling_matrix_present_flag)
        {
            for (i = 0; i < 6 + ((m_spss[seq_parameter_set_id].chroma_format_idc != 3) ? 2 : 6) * transform_8x8_mode_flag; i++)
            {
                pic_scaling_list_present_flag[ i ] = bs.readBits(1); //1 u(1)
                if (pic_scaling_list_present_flag[ i ] == 1)
                {
                    if (i < 6)
                    {
                        ret = scaling_list(bs, ScalingList4x4[ i ], 16, UseDefaultScalingMatrix4x4Flag[ i ]);
                        RETURN_IF_FAILED(ret != 0, ret);
                    }else
                    {
                        ret = scaling_list(bs, ScalingList8x8[ i - 6 ], 64, UseDefaultScalingMatrix8x8Flag[ i - 6 ]);
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                }
            }
            second_chroma_qp_index_offset = gb.get_se_golomb(bs); //1 se(v)
        }
    }

    ret = rbsp_trailing_bits(bs);

    int ret2 = printInfo();

    return 0;
}
