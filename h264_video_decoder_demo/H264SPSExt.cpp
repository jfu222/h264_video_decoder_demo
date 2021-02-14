//
// H264SPSExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPSExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SPSExt::CH264SPSExt()
{
    profile_idc = 0;
    seq_parameter_set_id = 0;
    aux_format_idc = 0;
    bit_depth_aux_minus8 = 0;
    alpha_incr_flag = 0;
    alpha_opaque_value = 0;
    alpha_transparent_value = 0;
    additional_extension_flag = 0;
}


CH264SPSExt::~CH264SPSExt()
{

}


int CH264SPSExt::printInfo()
{
    printf("---------SPS-Ext info------------\n");
    printf("profile_idc=%d;\n", profile_idc);
    printf("seq_parameter_set_id=%d;\n", seq_parameter_set_id);
    printf("aux_format_idc=%d;\n", aux_format_idc);
    printf("bit_depth_aux_minus8=%d;\n", bit_depth_aux_minus8);
    printf("alpha_incr_flag=%d;\n", alpha_incr_flag);
    printf("alpha_opaque_value=%d;\n", alpha_opaque_value);
    printf("alpha_transparent_value=%d;\n", alpha_transparent_value);
    printf("additional_extension_flag=%d;\n", additional_extension_flag);

    return 0;
}


int CH264SPSExt::seq_parameter_set_extension_rbsp(CBitstream &bs)
{
    CH264Golomb gb;
    
    this->seq_parameter_set_id = gb.get_ue_golomb(bs); //10 ue(v)
    this->aux_format_idc = gb.get_ue_golomb(bs); //10 ue(v)
    if (this->aux_format_idc != 0)
    {
        this->bit_depth_aux_minus8 = gb.get_ue_golomb(bs); //10 ue(v)
        this->alpha_incr_flag = bs.readBits(1); //10 u(1)
        this->alpha_opaque_value = bs.readBits(this->bit_depth_aux_minus8 + 9); //10 u(v)
        this->alpha_transparent_value = bs.readBits(this->bit_depth_aux_minus8 + 9); //10 u(v)
    }
    this->additional_extension_flag = bs.readBits(1); //10 u(1)
    //rbsp_trailing_bits(); //10

    int ret2 = printInfo();

    return 0;
}
