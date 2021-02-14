//
// H264SPSSVCExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPSSVCExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SPSSVCExt::CH264SPSSVCExt()
{
    inter_layer_deblocking_filter_control_present_flag = 0;
    extended_spatial_scalability_idc = 0;
    chroma_phase_x_plus1_flag = 0;
    chroma_phase_y_plus1 = 0;
    seq_ref_layer_chroma_phase_x_plus1_flag = 0;
    seq_ref_layer_chroma_phase_y_plus1 = 0;
    seq_scaled_ref_layer_left_offset = 0;
    seq_scaled_ref_layer_top_offset = 0;
    seq_scaled_ref_layer_right_offset = 0;
    seq_scaled_ref_layer_bottom_offset = 0;
    seq_tcoeff_level_prediction_flag = 0;
    adaptive_tcoeff_level_prediction_flag = 0;
    slice_header_restriction_flag = 0;
}


CH264SPSSVCExt::~CH264SPSSVCExt()
{

}


int CH264SPSSVCExt::printInfo()
{
    printf("---------SVC-Ext------------\n");
    printf("inter_layer_deblocking_filter_control_present_flag=%d;\n", inter_layer_deblocking_filter_control_present_flag);
    printf("extended_spatial_scalability_idc=%d;\n", extended_spatial_scalability_idc);
    printf("chroma_phase_x_plus1_flag=%d;\n", chroma_phase_x_plus1_flag);
    printf("chroma_phase_y_plus1=%d;\n", chroma_phase_y_plus1);
    printf("seq_ref_layer_chroma_phase_x_plus1_flag=%d;\n", seq_ref_layer_chroma_phase_x_plus1_flag);
    printf("seq_ref_layer_chroma_phase_y_plus1=%d;\n", seq_ref_layer_chroma_phase_y_plus1);
    printf("seq_scaled_ref_layer_left_offset=%d;\n", seq_scaled_ref_layer_left_offset);
    printf("seq_scaled_ref_layer_top_offset=%d;\n", seq_scaled_ref_layer_top_offset);
    printf("seq_scaled_ref_layer_right_offset=%d;\n", seq_scaled_ref_layer_right_offset);
    printf("seq_scaled_ref_layer_bottom_offset=%d;\n", seq_scaled_ref_layer_bottom_offset);
    printf("seq_tcoeff_level_prediction_flag=%d;\n", seq_tcoeff_level_prediction_flag);
    printf("adaptive_tcoeff_level_prediction_flag=%d;\n", adaptive_tcoeff_level_prediction_flag);
    printf("slice_header_restriction_flag=%d;\n", slice_header_restriction_flag);

    return 0;
}


int CH264SPSSVCExt::seq_parameter_set_svc_extension(CBitstream &bs, int ChromaArrayType)
{
    CH264Golomb gb;

    this->inter_layer_deblocking_filter_control_present_flag = bs.readBits(1); //0 u(1)
    this->extended_spatial_scalability_idc = bs.readBits(2); //0 u(2)
    if (ChromaArrayType == 1 || ChromaArrayType == 2)
    {
        this->chroma_phase_x_plus1_flag = bs.readBits(1); //0 u(1)
    }
    if (ChromaArrayType == 1)
    {
        this->chroma_phase_y_plus1 = bs.readBits(2); //0 u(2)
    }
    if (extended_spatial_scalability_idc == 1)
    {
        if (ChromaArrayType > 0)
        {
            this->seq_ref_layer_chroma_phase_x_plus1_flag = bs.readBits(1); //0 u(1)
            this->seq_ref_layer_chroma_phase_y_plus1 = bs.readBits(2); //0 u(2)
        }
        this->seq_scaled_ref_layer_left_offset = gb.get_se_golomb(bs); //0 se(v)
        this->seq_scaled_ref_layer_top_offset = gb.get_se_golomb(bs); //0 se(v)
        this->seq_scaled_ref_layer_right_offset = gb.get_se_golomb(bs); //0 se(v)
        this->seq_scaled_ref_layer_bottom_offset = gb.get_se_golomb(bs); //0 se(v)
    }
    this->seq_tcoeff_level_prediction_flag = bs.readBits(1); //0 u(1)
    if (seq_tcoeff_level_prediction_flag)
    {
        this->adaptive_tcoeff_level_prediction_flag = bs.readBits(1); //0 u(1)
    }
    this->slice_header_restriction_flag = bs.readBits(1); //0 u(1)

    return 0;
}
