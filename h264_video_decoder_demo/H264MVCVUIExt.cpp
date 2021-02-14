//
// H264MVCVUIExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264MVCVUIExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264MVCVUIExt::CH264MVCVUIExt()
{
    memset(this, 0, sizeof(CH264MVCVUIExt));
}


CH264MVCVUIExt::~CH264MVCVUIExt()
{

}


int CH264MVCVUIExt::printInfo()
{
    printf("---------MVC-VUI-Ext------------\n");

    return 0;
}


int CH264MVCVUIExt::mvc_vui_parameters_extension(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;
    int32_t i = 0;
    int32_t j = 0;

    vui_mvc_num_ops_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0; i <= vui_mvc_num_ops_minus1; i++ )
    {
        vui_mvc_temporal_id[ i ] = bs.readBits(3); //0 u(3)
        vui_mvc_num_target_output_views_minus1[ i ] = gb.get_ue_golomb(bs); //5 ue(v)
        for ( j = 0; j <= vui_mvc_num_target_output_views_minus1[ i ]; j++ )
        {
            vui_mvc_view_id[ i ][ j ] = gb.get_ue_golomb(bs); //5 ue(v)
        }
        vui_mvc_timing_info_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvc_timing_info_present_flag[ i ] )
        {
            vui_mvc_num_units_in_tick[ i ] = bs.readBits(32); //0 u(32)
            vui_mvc_time_scale[ i ] = bs.readBits(32); //0 u(32)
            vui_mvc_fixed_frame_rate_flag[ i ] = bs.readBits(1); //0 u(1)
        }
        vui_mvc_nal_hrd_parameters_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvc_nal_hrd_parameters_present_flag[ i ] )
        {
            ret = m_vui_mvc_nal_hrd_parameters.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        vui_mvc_vcl_hrd_parameters_present_flag[ i ] = bs.readBits(1); //0 u(1)
        if ( vui_mvc_vcl_hrd_parameters_present_flag[ i ] )
        {
            ret = m_vui_vcl_nal_hrd_parameters.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        if ( vui_mvc_nal_hrd_parameters_present_flag[ i ] || vui_mvc_vcl_hrd_parameters_present_flag[ i ] )
        {
            vui_mvc_low_delay_hrd_flag[ i ] = bs.readBits(1); //0 u(1)
        }
        vui_mvc_pic_struct_present_flag[ i ] = bs.readBits(1); //0 u(1)
    }
    return 0;
}
