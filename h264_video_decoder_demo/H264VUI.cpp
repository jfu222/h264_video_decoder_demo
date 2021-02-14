//
// H264VUI.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264VUI.h"
#include "H264Golomb.h"
#include "CommonFunction.h"


CH264VUI::CH264VUI()
{
    aspect_ratio_info_present_flag = 0;
    aspect_ratio_idc = 0;
    sar_width = 0;
    sar_height = 0;
    overscan_info_present_flag = 0;
    overscan_appropriate_flag = 0;
    video_signal_type_present_flag = 0;
    video_format = 0;
    video_full_range_flag = 0;
    colour_description_present_flag = 0;
    colour_primaries = 0;
    transfer_characteristics = 0;
    matrix_coefficients = 0;
    chroma_loc_info_present_flag = 0;
    chroma_sample_loc_type_top_field = 0;
    chroma_sample_loc_type_bottom_field = 0;
    timing_info_present_flag = 0;
    num_units_in_tick = 0;
    time_scale = 0;
    fixed_frame_rate_flag = 0;
    nal_hrd_parameters_present_flag = 0;
    vcl_hrd_parameters_present_flag = 0;
    low_delay_hrd_flag = 0;
    pic_struct_present_flag = 0;
    bitstream_restriction_flag = 0;
    motion_vectors_over_pic_boundaries_flag = 0;
    max_bytes_per_pic_denom = 0;
    max_bits_per_mb_denom = 0;
    log2_max_mv_length_horizontal = 0;
    log2_max_mv_length_vertical = 0;
    max_num_reorder_frames = -1;
    max_dec_frame_buffering = 0;
}


CH264VUI::~CH264VUI()
{

}


int CH264VUI::printInfo()
{
    printf("---------SPS-vui info------------\n");
    printf("aspect_ratio_info_present_flag=%d;\n", aspect_ratio_info_present_flag);
    printf("aspect_ratio_idc=%d;\n", aspect_ratio_idc);
    printf("sar_width=%d;\n", sar_width);
    printf("sar_height=%d;\n", sar_height);
    printf("overscan_info_present_flag=%d;\n", overscan_info_present_flag);
    printf("overscan_appropriate_flag=%d;\n", overscan_appropriate_flag);
    printf("video_signal_type_present_flag=%d;\n", video_signal_type_present_flag);
    printf("video_format=%d;\n", video_format);
    printf("video_full_range_flag=%d;\n", video_full_range_flag);
    printf("colour_description_present_flag=%d;\n", colour_description_present_flag);
    printf("colour_primaries=%d;\n", colour_primaries);
    printf("transfer_characteristics=%d;\n", transfer_characteristics);
    printf("matrix_coefficients=%d;\n", matrix_coefficients);
    printf("chroma_loc_info_present_flag=%d;\n", chroma_loc_info_present_flag);
    printf("chroma_sample_loc_type_top_field=%d;\n", chroma_sample_loc_type_top_field);
    printf("chroma_sample_loc_type_bottom_field=%d;\n", chroma_sample_loc_type_bottom_field);
    printf("timing_info_present_flag=%d;\n", timing_info_present_flag);
    printf("num_units_in_tick=%d;\n", num_units_in_tick);
    printf("time_scale=%d;\n", time_scale);
    printf("fixed_frame_rate_flag=%d;\n", fixed_frame_rate_flag);
    printf("nal_hrd_parameters_present_flag=%d;\n", nal_hrd_parameters_present_flag);
    printf("vcl_hrd_parameters_present_flag=%d;\n", vcl_hrd_parameters_present_flag);
    printf("low_delay_hrd_flag=%d;\n", low_delay_hrd_flag);
    printf("pic_struct_present_flag=%d;\n", pic_struct_present_flag);
    printf("bitstream_restriction_flag=%d;\n", bitstream_restriction_flag);
    printf("motion_vectors_over_pic_boundaries_flag=%d;\n", motion_vectors_over_pic_boundaries_flag);
    printf("max_bytes_per_pic_denom=%d;\n", max_bytes_per_pic_denom);
    printf("max_bits_per_mb_denom=%d;\n", max_bits_per_mb_denom);
    printf("log2_max_mv_length_horizontal=%d;\n", log2_max_mv_length_horizontal);
    printf("log2_max_mv_length_vertical=%d;\n", log2_max_mv_length_vertical);
    printf("max_num_reorder_frames=%d;\n", max_num_reorder_frames);
    printf("max_dec_frame_buffering=%d;\n", max_dec_frame_buffering);
    
    printf("---------SPS-vui-m_hrd_parameter_nal info------------\n");
    m_hrd_parameter_nal.printInfo();

    printf("---------SPS-vui-m_hrd_parameter_vcl info------------\n");
    m_hrd_parameter_vcl.printInfo();

    return 0;
}


//E.1.1 VUI parameters syntax
int CH264VUI::vui_parameters(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;

    this->aspect_ratio_info_present_flag = bs.readBits(1); //0 u(1)
    if (this->aspect_ratio_info_present_flag)
    {
        this->aspect_ratio_idc = bs.readBits(8); //0 u(8)
        if (this->aspect_ratio_idc == Extended_SAR)
        {
            this->sar_width = bs.readBits(16); //0 u(16)
            this->sar_height = bs.readBits(16); //0 u(16)
        }
    }

    this->overscan_info_present_flag = bs.readBits(1); //0 u(1)
    if (this->overscan_info_present_flag)
    {
        this->overscan_appropriate_flag = bs.readBits(1); //0 u(1)
    }
    this->video_signal_type_present_flag = bs.readBits(1); //0 u(1)
    if (this->video_signal_type_present_flag)
    {
        this->video_format = bs.readBits(3); //0 u(3)
        this->video_full_range_flag = bs.readBits(1); //0 u(1)
        this->colour_description_present_flag = bs.readBits(1); //0 u(1)
        if (this->colour_description_present_flag)
        {
            this->colour_primaries = bs.readBits(8); //0 u(8)
            this->transfer_characteristics = bs.readBits(8); //0 u(8)
            this->matrix_coefficients = bs.readBits(8); //0 u(8)
        }
    }
    this->chroma_loc_info_present_flag = bs.readBits(1); //0 u(1)
    if (this->chroma_loc_info_present_flag)
    {
        this->chroma_sample_loc_type_top_field = gb.get_ue_golomb(bs); //0 ue(v)
        this->chroma_sample_loc_type_bottom_field = gb.get_ue_golomb(bs); //0 ue(v)
    }
    this->timing_info_present_flag = bs.readBits(1); //0 u(1)
    if (this->timing_info_present_flag)
    {
        this->num_units_in_tick = bs.readBits(32); //0 u(32)
        this->time_scale = bs.readBits(32); //0 u(32)
        this->fixed_frame_rate_flag = bs.readBits(1); //0 u(1)
    }
    this->nal_hrd_parameters_present_flag = bs.readBits(1); //0 u(1)
    if (this->nal_hrd_parameters_present_flag)
    {
        ret = m_hrd_parameter_nal.hrd_parameters(bs);
    }
    this->vcl_hrd_parameters_present_flag = bs.readBits(1); //0 u(1)
    if (this->vcl_hrd_parameters_present_flag)
    {
        ret = m_hrd_parameter_nal.hrd_parameters(bs);
    }
    if (this->nal_hrd_parameters_present_flag || this->vcl_hrd_parameters_present_flag)
    {
        this->low_delay_hrd_flag = bs.readBits(1); //0 u(1)
    }
    this->pic_struct_present_flag = bs.readBits(1); //0 u(1)
    this->bitstream_restriction_flag = bs.readBits(1); //0 u(1)
    if (this->bitstream_restriction_flag)
    {
        this->motion_vectors_over_pic_boundaries_flag = bs.readBits(1); //0 u(1)
        this->max_bytes_per_pic_denom = gb.get_ue_golomb(bs); //0 ue(v)
        this->max_bits_per_mb_denom = gb.get_ue_golomb(bs); //0 ue(v)
        this->log2_max_mv_length_horizontal = gb.get_ue_golomb(bs); //0 ue(v)
        this->log2_max_mv_length_vertical = gb.get_ue_golomb(bs); //0 ue(v)
        this->max_num_reorder_frames = gb.get_ue_golomb(bs); //0 ue(v)
        this->max_dec_frame_buffering = gb.get_ue_golomb(bs); //0 ue(v)
    }

    return 0;
}
