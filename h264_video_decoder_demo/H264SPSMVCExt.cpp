//
// H264SPSMVCExt.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPSMVCExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SPSMVCExt::CH264SPSMVCExt()
{
    memset(this, 0, sizeof(CH264SPSMVCExt));
}


CH264SPSMVCExt::~CH264SPSMVCExt()
{

}


int CH264SPSMVCExt::printInfo()
{
    printf("---------SPS-MVC-Ext------------\n");

    return 0;
}


int CH264SPSMVCExt::seq_parameter_set_mvc_extension(CBitstream &bs, int profile_idc, int frame_mbs_only_flag)
{
    CH264Golomb gb;
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;

    this->num_views_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0; i <= num_views_minus1; i++ )
    {
        this->view_id[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
    }
    for ( i = 1; i <= num_views_minus1; i++ )
    {
        num_anchor_refs_l0[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j < num_anchor_refs_l0[ i ]; j++ )
        {
            anchor_ref_l0[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
        }
        num_anchor_refs_l1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j < num_anchor_refs_l1[ i ]; j++ )
        {
            anchor_ref_l1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
        }
    }
    for ( i = 1; i <= num_views_minus1; i++ )
    {
        num_non_anchor_refs_l0[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j < num_non_anchor_refs_l0[ i ]; j++ )
        {
            non_anchor_ref_l0[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
        }
        num_non_anchor_refs_l1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j < num_non_anchor_refs_l1[ i ]; j++ )
        {
            non_anchor_ref_l1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
        }
    }
    num_level_values_signalled_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for ( i = 0; i <= num_level_values_signalled_minus1; i++ )
    {
        level_idc[ i ] = bs.readBits(8); //0 u(8)
        num_applicable_ops_minus1[ i ] = gb.get_ue_golomb(bs); //0 ue(v)
        for ( j = 0; j <= num_applicable_ops_minus1[ i ]; j++ )
        {
            applicable_op_temporal_id[ i ][ j ] = bs.readBits(3); //0 u(3)
            applicable_op_num_target_views_minus1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            for ( k = 0; k <= applicable_op_num_target_views_minus1[ i ][ j ]; k++ )
            {
                applicable_op_target_view_id[ i ][ j ][ k ] = gb.get_ue_golomb(bs); //0 ue(v)
                applicable_op_num_views_minus1[ i ][ j ] = gb.get_ue_golomb(bs); //0 ue(v)
            }
        }
    }
    if (profile_idc == 134)
    {
        mfc_format_idc = bs.readBits(6); //0 u(6)
        if (mfc_format_idc == 0 || mfc_format_idc == 1)
        {
            default_grid_position_flag = bs.readBits(1); //0 u(1)
            if (!default_grid_position_flag)
            {
                view0_grid_position_x = bs.readBits(4); //0 u(4)
                view0_grid_position_y = bs.readBits(4); //0 u(4)
                view1_grid_position_x = bs.readBits(4); //0 u(4)
                view1_grid_position_y = bs.readBits(4); //0 u(4)
            }
        }
        rpu_filter_enabled_flag = bs.readBits(1); //0 u(1)
        if (!frame_mbs_only_flag)
        {
            rpu_field_processing_flag = bs.readBits(1); //0 u(1)
        }
    }
    return 0;
}
